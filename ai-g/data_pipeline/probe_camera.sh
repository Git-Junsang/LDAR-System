#!/bin/sh
# AI-G 카메라 캡처 방식 진단 — 이 출력을 그대로 복사해 알려주면 capture 방법을 확정한다.
# AI-G(보드)에서 실행:  sh probe_camera.sh
echo "===== 1) 비디오 노드 ====="
ls -l /dev/video* 2>/dev/null || echo "  (/dev/video* 없음)"

echo "===== 2) Python / OpenCV(cv2) 설치 여부 ====="
command -v python3 >/dev/null 2>&1 && python3 --version 2>&1 || echo "  (python3 없음 — Yocto 최소 이미지)"
python3 -c "import cv2; print('  cv2 OK', cv2.__version__)" 2>&1 | head -3
python3 -c "import numpy; print('  numpy OK', numpy.__version__)" 2>&1 | head -3

echo "===== 3) v4l2-ctl 존재 & 장치 목록 ====="
command -v v4l2-ctl >/dev/null 2>&1 && v4l2-ctl --list-devices 2>&1 | head -20 || echo "  (v4l2-ctl 없음)"

echo "===== 4) /dev/video0 포맷(있으면) ====="
command -v v4l2-ctl >/dev/null 2>&1 && v4l2-ctl -d /dev/video0 --list-formats-ext 2>&1 | head -30 || echo "  (skip)"

echo "===== 5) gstreamer 존재 여부 ====="
command -v gst-launch-1.0 >/dev/null 2>&1 && echo "  gst OK" || echo "  (gstreamer 없음)"

echo "===== 6) 텔레칩스 카메라/추론 앱 위치 ====="
ls -l ./tcnnapp ./motrex_app 2>/dev/null
echo "  (tcnnapp 사용법 확인:  ./tcnnapp -h  또는  ./tcnnapp --help)"

echo "===== 7) cv2 로 /dev/video0 실제 한 장 잡히는지(있을 때만) ====="
python3 - <<'PY' 2>&1 | head -5
try:
    import cv2
    cap = cv2.VideoCapture(0)
    ok, frame = cap.read()
    print("  grab", "OK" if ok else "FAIL", None if not ok else frame.shape)
    cap.release()
except Exception as e:
    print("  cv2 grab skip:", e)
PY
echo "===== 끝 — 위 1~7 출력을 그대로 복사해 전달 ====="
