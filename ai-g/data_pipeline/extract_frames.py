#!/usr/bin/env python3
"""프레임 추출 — capture.py 로 찍은 mp4 클립 → 라벨링용 jpg.

- 소스 fps 대비 목표 fps 로 다운샘플링.
- 거의 똑같은 프레임은 dedup(그레이 다운스케일 평균차) 으로 솎아냄.
- 출력 파일명에 원본 클립명 prefix → 어느 시나리오에서 왔는지 추적 가능.

예시:
  # data/raw 아래 모든 mp4 를 2fps 로 추출
  python3 extract_frames.py data/raw --out data/frames --fps 2
  # 단일 클립, dedup 강하게
  python3 extract_frames.py data/raw/center/center_..._00.mp4 --fps 3 --dedup 6.0
"""
import argparse
import glob
import os

import cv2
import numpy as np


def parse_args():
    p = argparse.ArgumentParser(description="Extract frames from capture clips")
    p.add_argument("src", help="mp4 파일 또는 mp4 들이 든 디렉터리")
    p.add_argument("--out", default="data/frames", help="프레임 출력 디렉터리")
    p.add_argument("--fps", type=float, default=2.0, help="초당 추출 장수")
    p.add_argument("--dedup", type=float, default=4.0,
                   help="직전 저장 프레임과 평균차가 이 값 미만이면 건너뜀(0=off)")
    p.add_argument("--ext", default="jpg", choices=["jpg", "png"])
    return p.parse_args()


def list_videos(src):
    if os.path.isdir(src):
        vids = []
        for e in ("*.mp4", "*.avi", "*.mov", "*.mkv", "*.h264"):
            vids += glob.glob(os.path.join(src, "**", e), recursive=True)
        return sorted(vids)
    return [src]


def _signature(frame):
    """dedup용 64x64 그레이 축소본 (float)."""
    g = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    return cv2.resize(g, (64, 64)).astype(np.float32)


def extract_one(path, out_dir, target_fps, dedup, ext):
    cap = cv2.VideoCapture(path)
    if not cap.isOpened():
        print(f"  [skip] 못 엶: {path}")
        return 0
    src_fps = cap.get(cv2.CAP_PROP_FPS) or 30.0
    step = max(1, int(round(src_fps / max(target_fps, 0.1))))
    prefix = os.path.splitext(os.path.basename(path))[0]

    os.makedirs(out_dir, exist_ok=True)
    idx, saved, last_sig = 0, 0, None
    while True:
        ok, frame = cap.read()
        if not ok:
            break
        if idx % step == 0:
            keep = True
            if dedup > 0:
                sig = _signature(frame)
                if last_sig is not None and float(np.mean(np.abs(sig - last_sig))) < dedup:
                    keep = False
                else:
                    last_sig = sig
            if keep:
                name = f"{prefix}_f{saved:05d}.{ext}"
                cv2.imwrite(os.path.join(out_dir, name), frame)
                saved += 1
        idx += 1
    cap.release()
    print(f"  {os.path.basename(path)}: {idx}프레임 → {saved}장 (src {src_fps:.0f}fps, step {step})")
    return saved


def main():
    args = parse_args()
    vids = list_videos(args.src)
    if not vids:
        raise SystemExit(f"영상 없음: {args.src}")
    print(f"클립 {len(vids)}개 → {args.out} (목표 {args.fps}fps, dedup {args.dedup})")
    total = sum(extract_one(v, args.out, args.fps, args.dedup, args.ext) for v in vids)
    print(f"완료: 총 {total}장 → {args.out}")


if __name__ == "__main__":
    main()
