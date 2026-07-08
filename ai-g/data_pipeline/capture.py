#!/usr/bin/env python3
"""PiCam 촬영 — LDAR YOLOv8-seg 학습 데이터 수집.

황금률: 추론(데모) 때 쓸 그 카메라·그 마운트·그 해상도 그대로 찍는다.
  --width/--height 는 NPU 추론 입력 비율과 맞출 것. 마운트 고정 후 촬영.

두 모드:
  video  (기본) — 시나리오별 mp4 클립 녹화 → 이후 extract_frames.py 로 프레임 추출.
  frames        — 프레임을 바로 jpg 로 저장 (영상 인코딩 없이 한 방에).

카메라 백엔드:
  picamera2(libcamera) 우선, 없으면 OpenCV VideoCapture 로 폴백 → Pi 외 환경에서도 동작.

예시:
  # 중앙 직진 10초 클립 녹화
  python3 capture.py --scenario center --duration 10
  # 왼쪽 드리프트, 엔터 누를 때까지 녹화
  python3 capture.py --scenario drift_left --duration 0
  # 프레임 직접 저장(5fps, 200장)
  python3 capture.py --mode frames --scenario weave --fps 5 --max-frames 200

시나리오 라벨 권장: center / drift_left / drift_right / weave / heading
"""
import argparse
import os
import sys
import time


def parse_args():
    p = argparse.ArgumentParser(description="PiCam capture for LDAR lane dataset")
    p.add_argument("--out", default="data/raw", help="출력 루트 (기본 data/raw)")
    p.add_argument("--scenario", default="clip", help="시나리오 라벨 (파일명·폴더에 사용)")
    p.add_argument("--mode", choices=["video", "frames"], default="video")
    p.add_argument("--width", type=int, default=640, help="추론 입력 비율에 맞출 것")
    p.add_argument("--height", type=int, default=384)
    p.add_argument("--fps", type=int, default=30, help="video=녹화 fps, frames=저장 fps")
    p.add_argument("--duration", type=float, default=10.0,
                   help="초 단위. 0이면 Enter 누를 때까지")
    p.add_argument("--max-frames", type=int, default=0,
                   help="frames 모드 상한 (0=무제한, duration 으로 제어)")
    p.add_argument("--bitrate", type=int, default=8_000_000, help="video 모드 H264 비트레이트")
    return p.parse_args()


def _ts():
    return time.strftime("%Y%m%d_%H%M%S")


def _next_path(out_dir, scenario, ext):
    """같은 시나리오를 여러 번 찍어도 안 덮어쓰게 인덱스 부여."""
    os.makedirs(out_dir, exist_ok=True)
    n = 0
    while True:
        name = f"{scenario}_{_ts()}_{n:02d}.{ext}"
        path = os.path.join(out_dir, name)
        if not os.path.exists(path):
            return path
        n += 1


def _wait(duration):
    """duration>0 이면 그 시간만큼, 0이면 Enter 까지 대기."""
    if duration and duration > 0:
        time.sleep(duration)
    else:
        try:
            input(">>> 녹화 중. 멈추려면 Enter...\n")
        except (EOFError, KeyboardInterrupt):
            pass


# ---------------------------------------------------------------- picamera2
def has_picamera2():
    try:
        import picamera2  # noqa: F401
        return True
    except Exception:
        return False


def capture_video_picam(args):
    from picamera2 import Picamera2
    from picamera2.encoders import H264Encoder
    from picamera2.outputs import FfmpegOutput

    out_dir = os.path.join(args.out, args.scenario)
    path = _next_path(out_dir, args.scenario, "mp4")

    picam2 = Picamera2()
    cfg = picam2.create_video_configuration(
        main={"size": (args.width, args.height), "format": "RGB888"},
        controls={"FrameRate": args.fps},
    )
    picam2.configure(cfg)
    encoder = H264Encoder(bitrate=args.bitrate)
    picam2.start_recording(encoder, FfmpegOutput(path))
    print(f"[picam2] 녹화 시작 → {path}  ({args.width}x{args.height}@{args.fps})")
    try:
        _wait(args.duration)
    finally:
        picam2.stop_recording()
    print(f"[picam2] 저장 완료: {path}")
    return path


def capture_frames_picam(args):
    import cv2
    from picamera2 import Picamera2

    out_dir = os.path.join(args.out, args.scenario + "_frames")
    os.makedirs(out_dir, exist_ok=True)

    picam2 = Picamera2()
    cfg = picam2.create_preview_configuration(
        main={"size": (args.width, args.height), "format": "RGB888"})
    picam2.configure(cfg)
    picam2.start()
    time.sleep(0.5)  # AE/AWB 안정화

    interval = 1.0 / max(args.fps, 1)
    deadline = (time.time() + args.duration) if args.duration > 0 else None
    print(f"[picam2] 프레임 저장 → {out_dir}  ({args.fps}fps)")
    i, t_next = 0, time.time()
    try:
        while True:
            frame = picam2.capture_array()           # RGB
            frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
            cv2.imwrite(os.path.join(out_dir, f"{args.scenario}_{i:05d}.jpg"), frame)
            i += 1
            if args.max_frames and i >= args.max_frames:
                break
            if deadline and time.time() >= deadline:
                break
            t_next += interval
            time.sleep(max(0.0, t_next - time.time()))
    except KeyboardInterrupt:
        pass
    finally:
        picam2.stop()
    print(f"[picam2] {i}장 저장: {out_dir}")
    return out_dir


# ---------------------------------------------------------------- OpenCV 폴백
def _open_cv_cam(args):
    import cv2
    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, args.width)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, args.height)
    cap.set(cv2.CAP_PROP_FPS, args.fps)
    if not cap.isOpened():
        sys.exit("[cv2] 카메라를 열 수 없음 (/dev/video0). 권한·연결 확인.")
    return cap


def capture_video_cv(args):
    import cv2
    out_dir = os.path.join(args.out, args.scenario)
    path = _next_path(out_dir, args.scenario, "mp4")
    cap = _open_cv_cam(args)
    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    writer = cv2.VideoWriter(path, fourcc, args.fps, (args.width, args.height))
    print(f"[cv2] 녹화 시작 → {path}")
    deadline = (time.time() + args.duration) if args.duration > 0 else None
    try:
        while True:
            ok, frame = cap.read()
            if not ok:
                break
            frame = cv2.resize(frame, (args.width, args.height))
            writer.write(frame)
            if deadline and time.time() >= deadline:
                break
    except KeyboardInterrupt:
        pass
    finally:
        writer.release()
        cap.release()
    print(f"[cv2] 저장 완료: {path}")
    return path


def capture_frames_cv(args):
    import cv2
    out_dir = os.path.join(args.out, args.scenario + "_frames")
    os.makedirs(out_dir, exist_ok=True)
    cap = _open_cv_cam(args)
    interval = 1.0 / max(args.fps, 1)
    deadline = (time.time() + args.duration) if args.duration > 0 else None
    print(f"[cv2] 프레임 저장 → {out_dir} ({args.fps}fps)")
    i, t_next = 0, time.time()
    try:
        while True:
            ok, frame = cap.read()
            if not ok:
                break
            frame = cv2.resize(frame, (args.width, args.height))
            cv2.imwrite(os.path.join(out_dir, f"{args.scenario}_{i:05d}.jpg"), frame)
            i += 1
            if args.max_frames and i >= args.max_frames:
                break
            if deadline and time.time() >= deadline:
                break
            t_next += interval
            time.sleep(max(0.0, t_next - time.time()))
    except KeyboardInterrupt:
        pass
    finally:
        cap.release()
    print(f"[cv2] {i}장 저장: {out_dir}")
    return out_dir


def main():
    args = parse_args()
    picam = has_picamera2()
    print(f"백엔드: {'picamera2' if picam else 'OpenCV(폴백)'} | mode={args.mode}")
    if args.mode == "video":
        (capture_video_picam if picam else capture_video_cv)(args)
    else:
        (capture_frames_picam if picam else capture_frames_cv)(args)


if __name__ == "__main__":
    main()
