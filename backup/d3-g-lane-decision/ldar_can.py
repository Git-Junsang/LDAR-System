# SPDX-License-Identifier: Apache-2.0
"""
LDAR 하향 CAN 명령 계층 (D3-G A72 → R5 → CAN → VCP-G).

전송 경로: A72가 (canID, data)를 교육용 IPC 패킷으로 /dev/tcc_ipc_micom 에 write
→ R5가 해당 CAN 채널로 송신 → VCP-G가 수신해 제어권 중재/액추에이션.
(transport 는 검증된 Library/IPC_Library.py 재사용 — 유사 프로젝트 demo_rc_ver.py 와 동일 방식)

CAN 표는 CLAUDE.md / VCP-G ldar_pins.h 와 일치:
  0x106 Vehicle Speed     data[0] = 0..80  (DC 모터 duty %)
  0x107 Wheel Angle       data[0] = 0..127 (서보 각, center 63)
  0x110 Control Authority data[0] = USER(0x01) / BOARD(0x02)
  0x111 Lane Status       data[0] = safe/warn/depart, data[1] = solid/dashed
"""

from Library.IPC_Library import IPC_SendPacketWithIPCHeader

# --- CAN IDs (R5 → VCP-G) -------------------------------------------------
CAN_VEHICLE_SPEED   = 0x106
CAN_WHEEL_ANGLE     = 0x107
CAN_CONTROL_AUTH    = 0x110
CAN_LANE_STATUS     = 0x111

# --- payload codes --------------------------------------------------------
AUTH_USER   = 0x01
AUTH_BOARD  = 0x02

LANE_SAFE   = 0x00
LANE_WARN   = 0x01
LANE_DEPART = 0x02

LINE_SOLID  = 0x01
LINE_DASHED = 0x02

# --- transport config -----------------------------------------------------
CAN_CH_BITMASK = 0x01      # CAN 채널 0 (VCP-G, LDAR_CAN_CH=0)
TX_ONLY        = 0x00
SERVO_MIN, SERVO_MAX, SERVO_CENTER = 0, 127, 63   # VCP-G servo_pwm 와 동일
SPEED_MAX = 80                                    # 0x106 스펙 상한


class LdarCan:
    """IPC 로 LDAR 제어 CAN 프레임을 송신한다. 값이 바뀔 때만 보내 버스 부하를 줄인다."""

    def __init__(self, dev="/dev/tcc_ipc_micom"):
        self._f = open(dev, "wb")
        self._cache = {}

    def _send(self, can_id, data_bytes, dedup=True):
        key = (can_id, bytes(data_bytes))
        if dedup and self._cache.get(can_id) == key[1]:
            return
        self._cache[can_id] = key[1]
        IPC_SendPacketWithIPCHeader(self._f, CAN_CH_BITMASK, TX_ONLY,
                                    can_id, bytes(data_bytes))

    # 제어권: BOARD=오버라이드(자동 복귀), USER=조이스틱 수동
    def authority(self, board: bool):
        self._send(CAN_CONTROL_AUTH, [AUTH_BOARD if board else AUTH_USER])

    # 차선 상태 + 종류(실선/점선) — 경고/부저 패턴 분기에 사용
    def lane_status(self, status: int, line: int):
        self._send(CAN_LANE_STATUS, [status & 0xFF, line & 0xFF])

    # 복귀 조향각 (0..127, center 63)
    def wheel(self, angle: int):
        a = max(SERVO_MIN, min(SERVO_MAX, int(round(angle))))
        self._send(CAN_WHEEL_ANGLE, [a])

    # 복귀 속도 (0..80 duty%)
    def speed(self, duty: int):
        d = max(0, min(SPEED_MAX, int(round(duty))))
        self._send(CAN_VEHICLE_SPEED, [d])

    def close(self):
        try:
            self._f.close()
        except Exception:
            pass
