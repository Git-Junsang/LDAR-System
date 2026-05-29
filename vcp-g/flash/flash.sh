#!/usr/bin/env bash
# VCP-G FWDN 플래시 (이 폴더만 있으면 어디서든 실행 가능)
#
# 사전 조건:
#   1) 보드 FWDN 스위치(SW101) 누른 채 12V 어댑터 연결 → FWDN 모드
#   2) USB-C로 PC↔보드 연결 (WSL2면 usbipd attach 먼저)
#
# 사용:
#   cd vcp-g/flash && ./flash.sh
#   (sudo 권한 자동 요청)

set -euo pipefail

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

FWDN="$DIR/fwdn"
FWDN_ROM="$DIR/vcp_fwdn.rom"
APP_ROM="$DIR/tcc70xx_pflash_boot_2M_ECC.rom"

for f in "$FWDN" "$FWDN_ROM" "$APP_ROM"; do
    [ -f "$f" ] || { echo "missing: $f" >&2; exit 1; }
done

echo "Flashing $APP_ROM ..."
sudo "$FWDN" --fwdn "$FWDN_ROM" -w "$APP_ROM"
echo "Done. Remove power, release FWDN switch, reapply power for Run mode."
