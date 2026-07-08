/* vcap — AI-G(Yocto, 도구 전무) 카메라 캡처 정적 바이너리.
 *
 * /dev/video2 (Telechips tccvin, V4L2 Multiplanar) 에서 UYVY 1288x956 프레임을
 * 그대로(raw) 파일로 저장한다. 보드엔 python/gst/ffmpeg/인코더가 없으므로 인코딩은
 * PC에서 한다(uyvy2img.py). v4l2-ctl --stream-to 가 multiplanar 를 안 써주는 문제 우회.
 *
 * 빌드(코드서버에서):
 *   aarch64-linux-gnu-gcc -O2 -static vcap.c -o vcap
 * 사용(AI-G 보드에서):
 *   ./vcap <outdir> <prefix> <save_count> [skip] [device] [W] [H]
 *   예) ./vcap /home/root/data/center center 60 5
 *       → center_00000.uyvy ... 60장, 매 6프레임 중 1장(=30fps에서 ~5fps)
 *
 * 인자: save_count=저장할 프레임 수, skip=저장 사이에 버릴 프레임 수(기본5),
 *       device=/dev/video2, W=1288, H=956. 처음 10프레임은 AE 안정용으로 버림.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/videodev2.h>

#define NBUF 4
#define WARMUP 10

static int xioctl(int fd, unsigned long req, void *arg) {
    int r;
    do { r = ioctl(fd, req, arg); } while (r == -1 && errno == EINTR);
    return r;
}

static void die(const char *m) { fprintf(stderr, "vcap: %s: %s\n", m, strerror(errno)); exit(1); }

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "usage: %s <outdir> <prefix> <save_count> [skip] [device] [W] [H]\n", argv[0]);
        return 2;
    }
    const char *outdir = argv[1];
    const char *prefix = argv[2];
    int save_count = atoi(argv[3]);
    int skip   = argc > 4 ? atoi(argv[4]) : 5;
    const char *dev = argc > 5 ? argv[5] : "/dev/video2";
    int W = argc > 6 ? atoi(argv[6]) : 1288;
    int H = argc > 7 ? atoi(argv[7]) : 956;

    mkdir(outdir, 0755);

    int fd = open(dev, O_RDWR);
    if (fd < 0) die("open device");

    /* 포맷: UYVY, multiplanar */
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    fmt.fmt.pix_mp.width = W;
    fmt.fmt.pix_mp.height = H;
    fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_UYVY;
    fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    fmt.fmt.pix_mp.num_planes = 1;
    if (xioctl(fd, VIDIOC_S_FMT, &fmt) < 0) die("S_FMT");

    unsigned planes_n = fmt.fmt.pix_mp.num_planes;
    unsigned img_size = fmt.fmt.pix_mp.plane_fmt[0].sizeimage;
    fprintf(stderr, "vcap: %dx%d UYVY, planes=%u, sizeimage=%u\n",
            fmt.fmt.pix_mp.width, fmt.fmt.pix_mp.height, planes_n, img_size);

    /* 버퍼 요청 */
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof req);
    req.count = NBUF;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    req.memory = V4L2_MEMORY_MMAP;
    if (xioctl(fd, VIDIOC_REQBUFS, &req) < 0) die("REQBUFS");

    void *map[NBUF];
    unsigned maplen[NBUF];
    for (unsigned i = 0; i < req.count; i++) {
        struct v4l2_plane planes[VIDEO_MAX_PLANES];
        struct v4l2_buffer b;
        memset(&b, 0, sizeof b);
        memset(planes, 0, sizeof planes);
        b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        b.memory = V4L2_MEMORY_MMAP;
        b.index = i;
        b.length = planes_n;
        b.m.planes = planes;
        if (xioctl(fd, VIDIOC_QUERYBUF, &b) < 0) die("QUERYBUF");
        maplen[i] = planes[0].length;
        map[i] = mmap(NULL, planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED,
                      fd, planes[0].m.mem_offset);
        if (map[i] == MAP_FAILED) die("mmap");
        if (xioctl(fd, VIDIOC_QBUF, &b) < 0) die("QBUF init");
    }

    enum v4l2_buf_type t = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if (xioctl(fd, VIDIOC_STREAMON, &t) < 0) die("STREAMON");

    int got = 0, frame = 0, saved = 0;
    while (saved < save_count) {
        struct v4l2_plane planes[VIDEO_MAX_PLANES];
        struct v4l2_buffer b;
        memset(&b, 0, sizeof b);
        memset(planes, 0, sizeof planes);
        b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        b.memory = V4L2_MEMORY_MMAP;
        b.length = planes_n;
        b.m.planes = planes;
        if (xioctl(fd, VIDIOC_DQBUF, &b) < 0) die("DQBUF");

        unsigned used = planes[0].bytesused ? planes[0].bytesused : img_size;
        int keep = (got >= WARMUP) && ((got - WARMUP) % (skip + 1) == 0);
        if (keep) {
            char path[512];
            snprintf(path, sizeof path, "%s/%s_%05d.uyvy", outdir, prefix, saved);
            int of = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (of < 0) die("open out");
            unsigned off = 0;
            while (off < used) {
                ssize_t w = write(of, (char *)map[b.index] + off, used - off);
                if (w <= 0) die("write");
                off += w;
            }
            close(of);
            saved++;
            if (saved % 10 == 0 || saved == save_count)
                fprintf(stderr, "vcap: saved %d/%d\n", saved, save_count);
        }
        got++; frame++;
        if (xioctl(fd, VIDIOC_QBUF, &b) < 0) die("QBUF");
    }

    xioctl(fd, VIDIOC_STREAMOFF, &t);
    for (unsigned i = 0; i < req.count; i++) munmap(map[i], maplen[i]);
    close(fd);
    fprintf(stderr, "vcap: done. %d frames -> %s/%s_*.uyvy (%ux%u UYVY)\n",
            saved, outdir, prefix, W, H);
    printf("%dx%d UYVY %d frames in %s\n", W, H, saved, outdir);
    return 0;
}
