# SPDX-License-Identifier: Apache-2.0
"""
LDAR 하향 CAN 명령 계층 (D3-G A72 → R5 → CAN → VCP-G).

표지판 인식 기반 속도 오버라이드. AI-G가 교통표지판을 검출하면 D3-G가 판정해
VCP-G에 '속도제한 / 정지 / 해제' 중 하나를 CAN 0x110으로 명령한다.

전송 경로: A72가 (canID, data)를 교육용 IPC 패킷으로 /dev/tcc_ipc_micom 에 write
→ R5가 CAN 채널 0으로 송신 → VCP-G가 수신(override_can.c)해 부드럽게 감속/정지.
(transport 는 검증된 Library/IPC_Library.py 재사용)

CAN 표는 ../README.md / VCP-G app.ldar.vcp/ldar_pins.h 와 일치:
  0x110 Speed Override  data[0]=mode(0 RELEASE/1 LIMIT/2 STOP), data[1]=limit(km/h≈duty%)
"""

from Library.IPC_Library import IPC_SendPacketWithIPCHeader

# --- CAN ID (R5 → VCP-G) --------------------------------------------------
CAN_SPEED_OVERRIDE = 0x110

# --- payload: data[0] mode ------------------------------------------------
OVR_RELEASE = 0x00   # 상한 해제 (조이스틱 자유)
OVR_LIMIT   = 0x01   # 속도 상한 제한 (data[1]=km/h)
OVR_STOP    = 0x02   # 강제 정지

# --- transport config -----------------------------------------------------
CAN_CH_BITMASK = 0x01      # CAN 채널 0 (VCP-G, LDAR_CAN_CH=0)
TX_ONLY        = 0x00
KMH_MAX        = 90        # 한계속도(km/h)를 듀티% 상한으로 1:1 사용 — 모터 상한과 동일


class LdarCan:
    """IPC 로 Speed Override CAN 프레임을 송신한다. 값이 바뀔 때만 보내 버스 부하를 줄인다."""

    def __init__(self, dev="/dev/tcc_ipc_micom"):
        self._f = open(dev, "wb")
        self._cache = {}

    def _send(self, can_id, data_bytes, dedup=True):
        payload = bytes(data_bytes)
        if dedup and self._cache.get(can_id) == payload:
            return
        self._cache[can_id] = payload
        IPC_SendPacketWithIPCHeader(self._f, CAN_CH_BITMASK, TX_ONLY, can_id, payload)

    # 속도 상한 제한 — 표지판 한계속도(km/h). VCP-G가 듀티% 상한으로 적용(30→30%, 60→60%).
    def limit(self, kmh: int):
        k = max(0, min(KMH_MAX, int(round(kmh))))
        self._send(CAN_SPEED_OVERRIDE, [OVR_LIMIT, k])

    # 강제 정지 — 정지/진입금지 표지판.
    def stop(self):
        self._send(CAN_SPEED_OVERRIDE, [OVR_STOP, 0])

    # 상한 해제 — 제한구역 종료(선택).
    def release(self):
        self._send(CAN_SPEED_OVERRIDE, [OVR_RELEASE, 0])

    def close(self):
        try:
            self._f.close()
        except Exception:
            pass
