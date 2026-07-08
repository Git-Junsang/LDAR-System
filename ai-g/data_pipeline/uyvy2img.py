#!/usr/bin/env python3
"""UYVY raw → jpg 변환 (PC에서 실행).

AI-G 보드의 vcap 이 떨군 *.uyvy (UYVY 4:2:2, 기본 1288x956) 를 PC 로 가져와
jpg 로 변환하고 학습 해상도로 축소한다. (보드엔 인코더가 없어 변환은 PC 몫)

예:
  # 보드에서 받은 폴더 통째로 → 640x384 jpg, 거의 같은 프레임 dedup
  python3 uyvy2img.py data/raw_uyvy --out data/frames --src-size 1288x956 --scale 640x384 --dedup 4.0
"""
import argparse
import glob
import os

import cv2
import numpy as np


def parse_args():
    p = argparse.ArgumentParser(description="UYVY raw -> jpg (PC side)")
    p.add_argument("src", help="*.uyvy 파일 또는 그것들이 든 디렉터리")
    p.add_argument("--out", default="data/frames")
    p.add_argument("--src-size", default="1288x956", help="원본 WxH (vcap 캡처 해상도)")
    p.add_argument("--scale", default="640x384", help="저장 WxH (빈 문자열이면 원본유지)")
    p.add_argument("--dedup", type=float, default=0.0,
                   help="직전 저장본과 평균차 < 값이면 건너뜀 (0=off)")
    p.add_argument("--ext", default="jpg", choices=["jpg", "png"])
    return p.parse_args()


def wh(s):
    w, h = s.lower().split("x")
    return int(w), int(h)


def uyvy_to_bgr(buf, w, h):
    """UYVY 4:2:2 (2 bytes/pixel) → BGR."""
    arr = np.frombuffer(buf, dtype=np.uint8)
    if arr.size != w * h * 2:
        return None  # 크기 안 맞음(해상도 오지정)
    yuv = arr.reshape(h, w, 2)
    return cv2.cvtColor(yuv, cv2.COLOR_YUV2BGR_UYVY)


def list_raw(src):
    if os.path.isdir(src):
        return sorted(glob.glob(os.path.join(src, "**", "*.uyvy"), recursive=True))
    return [src]


def sig(img):
    g = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    return cv2.resize(g, (64, 64)).astype(np.float32)


def main():
    a = parse_args()
    sw, sh = wh(a.src_size)
    files = list_raw(a.src)
    if not files:
        raise SystemExit(f"*.uyvy 없음: {a.src}")
    os.makedirs(a.out, exist_ok=True)
    scale = wh(a.scale) if a.scale else None

    saved, last = 0, None
    for f in files:
        with open(f, "rb") as fh:
            bgr = uyvy_to_bgr(fh.read(), sw, sh)
        if bgr is None:
            print(f"  [skip] 크기 불일치(--src-size 확인): {os.path.basename(f)}")
            continue
        if scale:
            bgr = cv2.resize(bgr, scale, interpolation=cv2.INTER_AREA)
        if a.dedup > 0:
            s = sig(bgr)
            if last is not None and float(np.mean(np.abs(s - last))) < a.dedup:
                continue
            last = s
        name = os.path.splitext(os.path.basename(f))[0] + "." + a.ext
        cv2.imwrite(os.path.join(a.out, name), bgr)
        saved += 1
    print(f"완료: {saved}장 → {a.out}  ({'%dx%d' % scale if scale else '원본'})")


if __name__ == "__main__":
    main()
