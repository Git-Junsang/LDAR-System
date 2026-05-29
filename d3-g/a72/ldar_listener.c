// SPDX-License-Identifier: Apache-2.0
/*
 * D3-G A72 IPC listener.
 *
 * Reads LdarIpcUpstream_t packets forwarded by the R5 CAN bridge and prints
 * them. Build natively on the D3-G board:
 *   cd d3-g/a72 && make
 *   sudo ./ldar_listener            # /dev/tcc_ipc_micom needs root
 *
 * Phase 1 T1.4 deliverable: confirms VCP-G→CAN→R5→IPC→A72 path end-to-end.
 * Later (Phase 2+) the judgment app will take the place of this printer.
 */

#define _POSIX_C_SOURCE 200809L

#include "../shared/ldar_ipc_proto.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define IPC_DEV  "/dev/tcc_ipc_micom"

static volatile sig_atomic_t s_stop = 0;

static void HandleSig(int signo)
{
    (void)signo;
    s_stop = 1;
}

static const char *TurnName(uint8_t code)
{
    switch (code) {
        case 0x00: return "off";
        case 0x01: return "L";
        case 0x02: return "R";
        default:   return "?";
    }
}

static void StampNow(char *buf, size_t bufsz)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tm;
    localtime_r(&ts.tv_sec, &tm);
    snprintf(buf, bufsz, "%02d:%02d:%02d.%03ld",
             tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec / 1000000);
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    signal(SIGINT,  HandleSig);
    signal(SIGTERM, HandleSig);

    int fd = open(IPC_DEV, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open %s: %s\n", IPC_DEV, strerror(errno));
        return 1;
    }
    fprintf(stderr, "[ldar-listener] reading from %s (Ctrl-C to stop)\n", IPC_DEV);

    LdarIpcUpstream_t pkt;
    char ts[32];

    while (!s_stop) {
        ssize_t n = read(fd, &pkt, sizeof(pkt));
        if (n < 0) {
            if (errno == EINTR) { continue; }
            fprintf(stderr, "read: %s\n", strerror(errno));
            break;
        }
        if (n == 0) {
            /* EOF / driver closed — retry shortly */
            struct timespec ms10 = { .tv_sec = 0, .tv_nsec = 10 * 1000 * 1000 };
            nanosleep(&ms10, NULL);
            continue;
        }
        if ((size_t)n != sizeof(pkt)) {
            fprintf(stderr, "short read: %zd / %zu\n", n, sizeof(pkt));
            continue;
        }

        StampNow(ts, sizeof(ts));
        if (pkt.canId == LDAR_CAN_ID_DRIVER_INPUT) {
            uint8_t code = (pkt.dataLen > 0) ? pkt.data[0] : 0xFFU;
            printf("[%s] DriverInput  id=0x%03X len=%u turn=%s (raw=0x%02X)\n",
                   ts, pkt.canId, pkt.dataLen, TurnName(code), code);
        } else {
            printf("[%s] other        id=0x%03X len=%u data=", ts, pkt.canId, pkt.dataLen);
            for (uint8_t i = 0; i < pkt.dataLen && i < LDAR_IPC_DATA_MAX; i++) {
                printf("%02X ", pkt.data[i]);
            }
            printf("\n");
        }
        fflush(stdout);
    }

    close(fd);
    fprintf(stderr, "[ldar-listener] stopped\n");
    return 0;
}
