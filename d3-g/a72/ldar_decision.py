#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""
LDAR 판정·명령 앱 (D3-G A72) — 표지판 인식 기반 속도 오버라이드.

흐름:  AI-G ──(Ethernet TCP, 표지판 JSON)──▶ [이 앱]
            표지판 → 속도제한/정지 판정  ──(IPC→R5→CAN 0x110)──▶ VCP-G

조이스틱 수동주행은 VCP-G 로컬에서 그대로 돈다. 이 앱은 표지판이 보이면 0x110으로
'속도 상한 / 정지 / 해제' 명령만 내리고, VCP-G가 그 한계까지 부드럽게 감속/정지한다.
(차선은 인식·판정하지 않는다.)

입력 소스는 인터페이스로 분리 — Mock(단독 검증)과 실제 AI-G TCP 를 교체:
  python3 ldar_decision.py --source mock --dry-run   # AI-G 없이 매핑 검증 (콘솔만)
  python3 ldar_decision.py --source mock             # 실제 IPC 송신
  python3 ldar_decision.py --source tcp --port 9999  # 실제 AI-G 수신

표지판 JSON(AI-G→D3-G, 잠정 — 모델 클래스 확정 시 동기화):
  {"ts":1234.5, "sign":"speed_30", "conf":0.92}
   sign : speed_30 | speed_60 | stop | no_entry | clear | none
   conf : 검출 신뢰도 0..1
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
    "RATE_HZ":      20,      # 판정 주기
    "CONF_THRESH":  0.5,     # 이 신뢰도 미만 검출은 무시(none 취급)
    "CONFIRM":      2,       # 같은 명령이 N프레임 연속이어야 전환(검출 깜박임 방지)
}

# 표지판 → (명령종류, 값). 'none'=유지(명령 없음)
SIGN_TO_CMD = {
    "speed_30": ("limit", 30),   # 시속 30 → 듀티 상한 30%
    "speed_60": ("limit", 60),   # 시속 60 → 듀티 상한 60%
    "stop":     ("stop", 0),     # 정지
    "no_entry": ("stop", 0),     # 진입금지 → 정지
    "clear":    ("release", 0),  # 제한구역 종료(선택)
    "none":     None,            # 표지판 없음 → 직전 명령 유지
}

# AI-G NPU cls 정수 → sign 문자열. ai-g/ai_model/dataset/data.yaml 순서 고정(재정렬 금지):
#   0 Stop / 1 No Entry / 2 Speed_Limit_60 / 3 Speed_Limit_30
CLS_TO_SIGN = {0: "stop", 1: "no_entry", 2: "speed_60", 3: "speed_30"}


@dataclass
class Detection:
    ts: float
    sign: str
    conf: float

    @staticmethod
    def from_json(d):
        # AI-G(NnAppMain) 포맷: {"boxes":[{"cls":int,"score":float,...}, ...]}
        if "boxes" in d:
            boxes = d.get("boxes") or []
            if not boxes:
                return Detection(ts=time.time(), sign="none", conf=0.0)
            top = max(boxes, key=lambda b: float(b.get("score", 0.0)))  # 최고 점수 박스
            score = float(top.get("score", 0.0))
            if score > 1.0:                # score가 0~100 스케일이면 0~1로
                score /= 100.0
            sign = CLS_TO_SIGN.get(int(top.get("cls", -1)), "none")
            return Detection(ts=time.time(), sign=sign, conf=score)
        # 단순 포맷 fallback: {"ts":..,"sign":"speed_30","conf":0.92}
        return Detection(
            ts=float(d.get("ts", time.time())),
            sign=str(d.get("sign", "none")),
            conf=float(d.get("conf", 0.0)),
        )


# =========================================================
# 입력 소스
# =========================================================
class MockSource:
    """표지판 시나리오를 순환 — LIMIT(60)→LIMIT(30)→STOP→RELEASE 를 모두 유발."""
    SEQ = ["none", "speed_60", "speed_30", "stop", "clear"]

    def __init__(self, dwell=2.5):
        self.t0 = time.time()
        self.dwell = dwell

    def read(self):
        idx = int((time.time() - self.t0) / self.dwell) % len(self.SEQ)
        return Detection(ts=time.time(), sign=self.SEQ[idx], conf=0.9)


class TcpSource:
    """AI-G(NnAppMain, TCP 서버)에 **클라이언트로 접속**해 줄단위(JSON\\n) 표지판 검출을 수신.
       AI-G가 192.168.0.100:9999 서버라서 D3-G가 접속하는 쪽이다."""
    def __init__(self, host="192.168.0.100", port=9999):
        self.conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print(f"[TCP] AI-G 접속 시도 {host}:{port} ...")
        self.conn.connect((host, port))
        self.conn.settimeout(1.0)
        self.buf = b""
        print(f"[TCP] AI-G 연결됨 {host}:{port}")

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
        try:
            return Detection.from_json(json.loads(line.decode("utf-8")))
        except (ValueError, json.JSONDecodeError):
            return None   # 깨진 줄은 건너뜀


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

        if cmd is None:                 # none/미정 → 유지
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
    ap = argparse.ArgumentParser(description="LDAR 판정·명령 앱 (D3-G A72) — 표지판 속도 오버라이드")
    ap.add_argument("--source", choices=["mock", "tcp"], default="mock")
    ap.add_argument("--port", type=int, default=9999)
    ap.add_argument("--host", default="192.168.0.100", help="AI-G TCP 서버 IP")
    ap.add_argument("--dev", default="/dev/tcc_ipc_micom")
    ap.add_argument("--dry-run", action="store_true", help="IPC 송신 없이 콘솔 출력만")
    args = ap.parse_args()

    src = MockSource() if args.source == "mock" else TcpSource(args.host, args.port)
    can = None if args.dry_run else C.LdarCan(args.dev)
    dec = Decision()
    period = 1.0 / CFG["RATE_HZ"]
    last_cmd = None

    print(f"[LDAR] decision up — source={args.source} dry_run={args.dry_run}")
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
                    print(f"\n[CMD] -> {label}")
                    last_cmd = cmd
                print(f"\r sign={det.sign:9s} conf={det.conf:.2f} "
                      f"cmd={last_cmd[0]:7s} val={last_cmd[1]:3d}   ",
                      end="", flush=True)

            dt = period - (time.time() - t)
            if dt > 0:
                time.sleep(dt)
    except (KeyboardInterrupt, ConnectionError) as e:
        print(f"\n[LDAR] stop ({e})")
    finally:
        if can:
            can.release()   # 종료 시 상한 해제
            can.close()


if __name__ == "__main__":
    main()
