#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""
LDAR 판정·명령 앱 (D3-G A72).

흐름:  AI-G ──(Ethernet TCP, 차선 JSON)──▶ [이 앱]
            이탈 판정 + 상태머신 + P 복귀각  ──(IPC→R5→CAN)──▶ VCP-G

VCP-G 가 0x110(제어권)을 보고 USER(로컬 조이스틱) / BOARD(복귀각) 를 중재한다.
즉 차선을 벗어나면 이 앱이 BOARD 로 전환하고 0x107(서보각)을 좌/우로 보내 복귀시킨다.

입력 소스는 인터페이스로 분리되어 있어 Mock(단독 검증)과 실제 AI-G TCP 를 교체 가능:
  python3 ldar_decision.py --source mock           # AI-G 없이 상태머신 검증 (Phase 2)
  python3 ldar_decision.py --source tcp --port 9999 # 실제 AI-G 수신 (Phase 4)
  python3 ldar_decision.py --source mock --dry-run  # IPC 송신 없이 콘솔만

차선 JSON(AI-G→D3-G, 잠정 — 모델 출력 확정 시 Phase 4 동기화):
  {"ts":1234.5, "offset":0.1, "heading":0.0,
   "left_type":1, "right_type":2, "risk":0.2}
   offset  : -1(좌측 끝)..0(중앙)..+1(우측 끝)
   heading : 진행방향 오차 (+ = 우측 향함)
   *_type  : 1=실선(solid), 2=점선(dashed)
"""

import argparse
import json
import math
import socket
import sys
import time
from dataclasses import dataclass

import ldar_can as C

# =========================================================
# 튜닝 파라미터 (Phase 4 에서 트랙 실측으로 확정)
# =========================================================
CFG = {
    "RATE_HZ":      20,       # 판정 주기
    "T_IN":         0.25,     # 가장자리까지 거리 < T_IN → 개입 검토 (거리=1-|offset|)
    "T_OUT":        0.40,     # 거리 > T_OUT → 안전(USER 복귀 자격)
    "T_HOLD":       0.6,      # BOARD→USER 복귀 전 안정 유지 시간(s)
    "HEADING_OK":   0.10,     # |heading| 이내면 차선과 평행으로 간주
    "CONTACT":      0.95,     # |offset| >= CONTACT → 차선 접촉/통과(CRITICAL)
    # P 복귀 제어 (각도 단위, center 63 기준)
    "KP_OFFSET":    55.0,     # offset 1.0 당 보정 각
    "KP_HEADING":   25.0,     # heading 1.0 당 보정 각
    "STEER_SIGN":   -1,       # 복귀가 반대로 돌면 +1 로 뒤집기 (서보 장착 방향)
    # 속도
    "CRUISE_DUTY":  40,       # BOARD 복귀 주행 속도
    "CRITICAL_DUTY": 0,       # CRITICAL 즉시 감속
}

# 상태
USER, WARNING, BOARD, CRITICAL = "USER", "WARNING", "BOARD", "CRITICAL"


@dataclass
class Lane:
    ts: float
    offset: float          # -1..+1
    heading: float         # + = 우향
    left_type: int         # 1 solid / 2 dashed
    right_type: int
    risk: float = 0.0

    @staticmethod
    def from_json(d):
        return Lane(
            ts=float(d.get("ts", time.time())),
            offset=float(d.get("offset", 0.0)),
            heading=float(d.get("heading", 0.0)),
            left_type=int(d.get("left_type", C.LINE_SOLID)),
            right_type=int(d.get("right_type", C.LINE_SOLID)),
            risk=float(d.get("risk", 0.0)),
        )


# =========================================================
# 입력 소스
# =========================================================
class MockSource:
    """삼각파로 offset 을 -1.1..+1.1 스윕 — 좌(점선)/우(실선) 이탈을 번갈아 유발.
       우측=실선 → OVERRIDE, 좌측=점선 → WARNING 으로 매트릭스 양쪽을 보여준다."""
    def __init__(self, period=8.0):
        self.t0 = time.time()
        self.period = period

    def read(self):
        t = time.time() - self.t0
        phase = (t % self.period) / self.period          # 0..1
        tri = 4.0 * abs(phase - 0.5) - 1.0               # -1..+1 삼각파
        offset = 1.1 * tri
        heading = 0.3 * math.cos(2 * math.pi * phase)
        return Lane(ts=time.time(), offset=offset, heading=heading,
                    left_type=C.LINE_DASHED, right_type=C.LINE_SOLID,
                    risk=min(1.0, abs(offset)))


class TcpSource:
    """AI-G 가 보내는 줄단위(JSON\\n) 차선 데이터를 수신."""
    def __init__(self, host="0.0.0.0", port=9999):
        self.srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.srv.bind((host, port))
        self.srv.listen(1)
        print(f"[TCP] AI-G 접속 대기 {host}:{port}")
        self.conn, addr = self.srv.accept()
        self.conn.settimeout(1.0)
        self.buf = b""
        print(f"[TCP] AI-G 연결됨 {addr}")

    def read(self):
        while b"\n" not in self.buf:
            try:
                chunk = self.conn.recv(4096)
            except socket.timeout:
                return None
            if not chunk:
                raise ConnectionError("AI-G 연결 종료")
            self.buf += chunk
        line, self.buf = self.buf.split(b"\n", 1)
        line = line.strip()
        if not line:
            return None
        return Lane.from_json(json.loads(line.decode("utf-8")))


# =========================================================
# 판정 + P 복귀
# =========================================================
class Decision:
    def __init__(self, cfg=CFG):
        self.cfg = cfg
        self.state = USER
        self._safe_since = None

    def _near_line(self, lane):
        """이탈 중인 쪽의 차선 종류."""
        return lane.right_type if lane.offset > 0 else lane.left_type

    def step(self, lane, driver_intent=0):
        """driver_intent: 0=없음, 1=좌 방향지시, 2=우 방향지시 (상향 0x120)."""
        cfg = self.cfg
        dist = 1.0 - abs(lane.offset)            # 가장자리까지 거리
        near_solid = (self._near_line(lane) == C.LINE_SOLID)
        # 이탈 방향으로 방향지시를 켰으면 의도적 차선변경
        intent_match = (driver_intent == 2 and lane.offset > 0) or \
                       (driver_intent == 1 and lane.offset < 0)

        # --- 상태 전이 ---
        if abs(lane.offset) >= cfg["CONTACT"]:
            self.state = CRITICAL
        elif dist < cfg["T_IN"]:
            if near_solid and not intent_match:
                self.state = BOARD              # 실선 이탈 + 의도 없음 → 오버라이드
            elif not near_solid and not intent_match:
                self.state = WARNING            # 점선 이탈 → 경고만(허용)
            # 점선 + 방향지시 일치 → 전이 없음(USER 유지)
        elif dist > cfg["T_OUT"]:
            if self.state in (BOARD, CRITICAL):
                # 복귀 자격: 거리 충분 + heading 평행 + 홀드시간 경과
                if abs(lane.heading) <= cfg["HEADING_OK"]:
                    if self._safe_since is None:
                        self._safe_since = lane.ts
                    elif lane.ts - self._safe_since >= cfg["T_HOLD"]:
                        self.state = USER
                else:
                    self._safe_since = None
            else:
                self.state = USER
        if self.state not in (BOARD, CRITICAL):
            self._safe_since = None

        return self._command(lane)

    def _recovery_angle(self, lane):
        cfg = self.cfg
        corr = cfg["KP_OFFSET"] * lane.offset + cfg["KP_HEADING"] * lane.heading
        return C.SERVO_CENTER + cfg["STEER_SIGN"] * corr

    def _command(self, lane):
        """반환: (authority_board, lane_status, line_code, angle_or_None, duty_or_None)"""
        line = C.LINE_SOLID if self._near_line(lane) == C.LINE_SOLID else C.LINE_DASHED
        if self.state == USER:
            return (False, C.LANE_SAFE, line, None, None)
        if self.state == WARNING:
            return (False, C.LANE_WARN, line, None, None)   # 제어권 유지, 경고만
        if self.state == BOARD:
            return (True, C.LANE_DEPART, line,
                    self._recovery_angle(lane), self.cfg["CRUISE_DUTY"])
        # CRITICAL
        return (True, C.LANE_DEPART, line,
                self._recovery_angle(lane), self.cfg["CRITICAL_DUTY"])


# =========================================================
# 메인 루프
# =========================================================
def main():
    ap = argparse.ArgumentParser(description="LDAR 판정·명령 앱 (D3-G A72)")
    ap.add_argument("--source", choices=["mock", "tcp"], default="mock")
    ap.add_argument("--port", type=int, default=9999)
    ap.add_argument("--host", default="0.0.0.0")
    ap.add_argument("--dev", default="/dev/tcc_ipc_micom")
    ap.add_argument("--dry-run", action="store_true", help="IPC 송신 없이 콘솔 출력만")
    args = ap.parse_args()

    src = MockSource() if args.source == "mock" else TcpSource(args.host, args.port)
    can = None if args.dry_run else C.LdarCan(args.dev)
    dec = Decision()
    period = 1.0 / CFG["RATE_HZ"]
    last_state = None

    print(f"[LDAR] decision up — source={args.source} dry_run={args.dry_run}")
    try:
        while True:
            t = time.time()
            lane = src.read()
            if lane is not None:
                board, status, line, angle, duty = dec.step(lane)

                if can:
                    can.authority(board)
                    can.lane_status(status, line)
                    if angle is not None:
                        can.wheel(angle)
                    if duty is not None:
                        can.speed(duty)

                if dec.state != last_state:
                    print(f"\n[STATE] {last_state} -> {dec.state}")
                    last_state = dec.state
                a = "----" if angle is None else f"{int(round(angle)):4d}"
                print(f"\r off={lane.offset:+.2f} head={lane.heading:+.2f} "
                      f"st={dec.state:8s} auth={'BOARD' if board else 'USER'} "
                      f"angle={a} duty={duty if duty is not None else '--'}   ",
                      end="", flush=True)

            dt = period - (time.time() - t)
            if dt > 0:
                time.sleep(dt)
    except (KeyboardInterrupt, ConnectionError) as e:
        print(f"\n[LDAR] stop ({e})")
    finally:
        if can:
            can.authority(False)   # 종료 시 제어권 USER 로 반납
            can.close()


if __name__ == "__main__":
    main()
