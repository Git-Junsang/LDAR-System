#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""
TSRC 판정·명령 앱 (D3-G A72) — 표지판 인식 기반 속도 오버라이드.

흐름:  AI-G ──(Ethernet TCP, 표지판 JSON)──▶ [이 앱]
            표지판 → 속도제한/정지 판정  ──(IPC→R5→CAN 0x110)──▶ VCP-G

[수정 사항 - 원본 대비]
  1. TcpSource : 서버 → 클라이언트 (AI-G가 서버, D3-G가 접속)
  2. Detection.from_json : 우리 JSON 포맷 파싱
       {"sign_id":0,"sign_name":"Stop","score":0.92,"bbox_h":120}
  3. bbox_h 임계값 추가 : 너무 멀면 none 취급 (거리 판단)
  4. SIGN_TO_CMD : sign 이름 우리 포맷에 맞게 업데이트

사용법:
  python3 ldar_decision_v2.py --source mock --dry-run  # AI-G 없이 로직 검증
  python3 ldar_decision_v2.py --source mock            # IPC 실제 전송 검증
  python3 ldar_decision_v2.py --source tcp             # 실제 AI-G 연결
"""

import argparse
import json
import socket
import time
from dataclasses import dataclass

import ldar_can as C

# =========================================================
# 튜닝 파라미터
# =========================================================
CFG = {
    "RATE_HZ":      20,     # 판정 주기
    "CONF_THRESH":  0.5,    # 이 신뢰도 미만 검출은 무시(none 취급)
    "CONFIRM":      2,      # 같은 명령이 N프레임 연속이어야 전환 (깜박임 방지)
}

# bbox_h 임계값: 이 픽셀 이상이어야 "충분히 가까운 것"으로 판단
# 카메라 해상도 기준, 실제 RC카 테스트 후 조정
BBOX_THRESHOLD = 80

# 표지판 이름 매핑: 우리 sign_name → 내부 sign key
# [수정] Stop/NoEntry/SpeedLimit30/SpeedLimit60 → 내부 key로 변환
SIGN_NAME_MAP = {
    "Stop":         "stop",
    "NoEntry":      "no_entry",
    "SpeedLimit30": "speed_30",
    "SpeedLimit60": "speed_60",
    "None":         "none",
}

# 내부 sign key → (명령종류, 값). 'none'=유지(명령 없음)
SIGN_TO_CMD = {
    "speed_30": ("limit",   30),
    "speed_60": ("limit",   60),
    "stop":     ("stop",     0),
    "no_entry": ("stop",     0),
    "none":     None,           # 표지판 없음 → 직전 명령 유지
}


@dataclass
class Detection:
    ts: float
    sign: str
    conf: float

    @staticmethod
    def from_json(d):
        """
        [수정] 우리 NnAppMain.c JSON 포맷 파싱.
        {"sign_id":0,"sign_name":"Stop","score":0.92,"bbox_h":120}

        - sign_name을 내부 key로 변환
        - bbox_h < BBOX_THRESHOLD 면 none 처리 (너무 멀면 무시)
        """
        sign_name = d.get("sign_name", "None")
        sign = SIGN_NAME_MAP.get(sign_name, "none")

        # 거리 판단: bbox_h 임계값 미달 → none 취급
        bbox_h = int(d.get("bbox_h", 0))
        if bbox_h < BBOX_THRESHOLD:
            sign = "none"

        return Detection(
            ts=float(d.get("ts", time.time())),
            sign=sign,
            conf=float(d.get("score", 0.0)),   # [수정] "conf" → "score"
        )


# =========================================================
# 입력 소스
# =========================================================
class MockSource:
    """표지판 시나리오 순환 — 모든 명령(LIMIT60→LIMIT30→STOP→NONE)을 유발."""
    SEQ = ["none", "speed_60", "speed_30", "stop", "none"]

    def __init__(self, dwell=2.5):
        self.t0 = time.time()
        self.dwell = dwell

    def read(self):
        idx = int((time.time() - self.t0) / self.dwell) % len(self.SEQ)
        return Detection(ts=time.time(), sign=self.SEQ[idx], conf=0.9)


class TcpSource:
    """
    [수정] 서버 → 클라이언트.
    AI-G(NnAppMain.c)가 서버(192.168.0.100:9999)로 대기하고
    D3-G(이 앱)가 클라이언트로 접속한다.
    """
    def __init__(self, host="192.168.0.100", port=9999):
        self.sock = None
        self.f = None
        self._connect(host, port)
        self.host = host
        self.port = port

    def _connect(self, host, port):
        print(f"[TCP] AI-G 접속 시도 {host}:{port}")
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(3.0)
        self.sock.connect((host, port))
        self.sock.settimeout(1.0)
        self.f = self.sock.makefile("r", encoding="utf-8", newline="\n")
        print(f"[TCP] AI-G 연결됨 {host}:{port}")

    def read(self):
        try:
            line = self.f.readline()
            if not line:
                raise ConnectionError("AI-G 연결 종료")
            line = line.strip()
            if not line:
                return None
            return Detection.from_json(json.loads(line))
        except socket.timeout:
            return None


# =========================================================
# 판정 (표지판 → 명령)
# =========================================================
class Decision:
    def __init__(self, cfg=CFG):
        self.cfg = cfg
        self.current = ("release", 0)   # 시작: 해제 상태
        self._pending = None
        self._count = 0

    def step(self, det):
        sign = det.sign if det.conf >= self.cfg["CONF_THRESH"] else "none"
        cmd = SIGN_TO_CMD.get(sign, None)

        if cmd is None:                 # none → 직전 명령 유지
            self._pending, self._count = None, 0
            return self.current
        if cmd == self.current:         # 이미 적용 중 → 유지
            self._pending, self._count = None, 0
            return self.current

        # 새 명령 후보 — CONFIRM 프레임 연속이어야 전환
        if cmd == self._pending:
            self._count += 1
        else:
            self._pending, self._count = cmd, 1
        if self._count >= self.cfg["CONFIRM"]:
            self.current = cmd
            self._pending, self._count = None, 0
        return self.current


def _apply(can, cmd):
    kind, val = cmd
    if can:
        if kind == "limit":
            can.limit(val)
        elif kind == "stop":
            can.stop()
        else:
            can.release()


# =========================================================
# 메인 루프
# =========================================================
def main():
    ap = argparse.ArgumentParser(
        description="TSRC 판정·명령 앱 (D3-G A72) — 표지판 속도 오버라이드"
    )
    ap.add_argument("--source", choices=["mock", "tcp"], default="mock")
    ap.add_argument("--port",   type=int, default=9999)
    ap.add_argument("--host",   default="192.168.0.100")    # [수정] AI-G IP
    ap.add_argument("--dev",    default="/dev/tcc_ipc_micom")
    ap.add_argument("--dry-run", action="store_true",
                    help="IPC 송신 없이 콘솔 출력만")
    args = ap.parse_args()

    src = MockSource() if args.source == "mock" else TcpSource(args.host, args.port)
    can = None if args.dry_run else C.LdarCan(args.dev)
    dec = Decision()
    period = 1.0 / CFG["RATE_HZ"]
    last_cmd = None

    print(f"[TSRC] 판정 앱 시작 — source={args.source} dry_run={args.dry_run}")
    print(f"[TSRC] bbox 임계값: {BBOX_THRESHOLD}px")
    try:
        while True:
            t = time.time()
            det = src.read()
            if det is not None:
                cmd = dec.step(det)
                _apply(can, cmd)

                if cmd != last_cmd:
                    kind, val = cmd
                    label = f"{kind}({val})" if kind == "limit" else kind
                    print(f"\n[CMD] → {label}")
                    last_cmd = cmd

                print(f"\r sign={det.sign:9s} conf={det.conf:.2f} "
                      f"cmd={last_cmd[0]:7s} val={last_cmd[1]:3d}   ",
                      end="", flush=True)

            dt = period - (time.time() - t)
            if dt > 0:
                time.sleep(dt)

    except (KeyboardInterrupt, ConnectionError) as e:
        print(f"\n[TSRC] 종료 ({e})")
    finally:
        if can:
            can.release()
            can.close()


if __name__ == "__main__":
    main()
