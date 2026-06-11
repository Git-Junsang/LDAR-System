#!/usr/bin/env python3
"""Export a trained 4-class YOLOv8s to the Telechips/Enlight "extracted" ONNX.

This is THE step the previous attempt got wrong. The Enlight `converter.py`
does NOT take a normal full-head YOLO ONNX — it takes a backbone+neck+head-conv
graph that is *cut* right before the DFL/decode/concat tail, exactly 6 outputs:

    /model.22/cv3.0/cv3.0.2/Conv_output_0   class branch  (nc ch)   80x80
    /model.22/cv2.0/cv2.0.2/Conv_output_0   box/DFL branch (64 ch)   80x80
    /model.22/cv3.1/cv3.1.2/Conv_output_0   class branch  (nc ch)   40x40
    /model.22/cv2.1/cv2.1.2/Conv_output_0   box/DFL branch (64 ch)   40x40
    /model.22/cv3.2/cv3.2.2/Conv_output_0   class branch  (nc ch)   20x20
    /model.22/cv2.2/cv2.2.2/Conv_output_0   box/DFL branch (64 ch)   20x20

The stock yolov8s_extracted.onnx in this folder has the cv3.* branches at
**80** channels (COCO). For our signs model they MUST be **4**. The converter
re-attaches DFL+sigmoid+NMS later (--add-detection-post-process / --num-class).

Order matters: outputs are interleaved (class, box) per scale = `--output-order cl`.

Run on the training host (ultralytics + onnx). Produces yolov8s_signs_extracted.onnx.

  python3 export_onnx.py --weights runs/detect/signs_yolov8s/weights/best.pt
"""
import argparse
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent

# Verified against this repo's stock yolov8s_extracted.onnx (opset 10, 640x640).
STOCK_OUTPUTS = [
    "/model.22/cv3.0/cv3.0.2/Conv_output_0",
    "/model.22/cv2.0/cv2.0.2/Conv_output_0",
    "/model.22/cv3.1/cv3.1.2/Conv_output_0",
    "/model.22/cv2.1/cv2.1.2/Conv_output_0",
    "/model.22/cv3.2/cv3.2.2/Conv_output_0",
    "/model.22/cv2.2/cv2.2.2/Conv_output_0",
]


def discover_head_outputs(model):
    """Fallback: find the 6 head-conv output tensors and order them cls,box per scale."""
    pat = re.compile(r"/model\.\d+/cv([23])\.(\d)/cv[23]\.\d\.2/Conv_output_0$")
    found = {}
    for node in model.graph.node:
        for o in node.output:
            m = pat.search(o)
            if m:
                branch, scale = m.group(1), int(m.group(2))
                found[(scale, branch)] = o
    outs = []
    for scale in sorted({k[0] for k in found}):
        for branch in ("3", "2"):  # cls (cv3) then box (cv2) == output-order 'cl'
            if (scale, branch) in found:
                outs.append(found[(scale, branch)])
    return outs


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--weights", default=str(HERE / "runs/detect/signs_yolov8s/weights/best.pt"))
    ap.add_argument("--imgsz", type=int, default=640, help="must match NPU input (640)")
    ap.add_argument("--opset", type=int, default=10, help="match stock (10)")
    ap.add_argument("--out", default=str(HERE / "yolov8s_signs_extracted.onnx"))
    ap.add_argument("--nc", type=int, default=4, help="expected class count (for verification)")
    args = ap.parse_args()

    import onnx
    from onnx.utils import extract_model
    from ultralytics import YOLO

    weights = Path(args.weights)
    assert weights.exists(), f"weights not found: {weights}"

    # 1) full-head ONNX export (the cv2/cv3 conv outputs exist as intermediate tensors)
    print(f"[export] {weights} -> full ONNX (opset {args.opset}, imgsz {args.imgsz})")
    full = YOLO(str(weights)).export(format="onnx", opset=args.opset, imgsz=args.imgsz,
                                     simplify=False, dynamic=False, nms=False)
    full = Path(full)

    # 2) pick the 6 head-conv outputs (verified names, regex fallback)
    m = onnx.load(str(full))
    names = {o for n in m.graph.node for o in n.output}
    outs = STOCK_OUTPUTS if all(x in names for x in STOCK_OUTPUTS) else discover_head_outputs(m)
    assert len(outs) == 6, f"expected 6 head outputs, found {len(outs)}: {outs}"
    print("[export] cut outputs:")
    for o in outs:
        print("   ", o)

    # 3) extract the subgraph: input 'images' -> the 6 conv outputs
    extract_model(str(full), args.out, ["images"], outs)

    # 4) verify shapes: cv3.* == nc channels, cv2.* == 64, grids 80/40/20
    e = onnx.load(args.out)
    print(f"[verify] {args.out}")
    ok = True
    for o in e.graph.output:
        dims = [d.dim_value for d in o.type.tensor_type.shape.dim]
        ch = dims[1] if len(dims) > 1 else None
        is_cls = "/cv3." in o.name
        want = args.nc if is_cls else 64
        flag = "OK" if ch == want else "*** WRONG ***"
        if ch != want:
            ok = False
        print(f"   {o.name}  {dims}  ch={ch} want={want}  {flag}")
    if not ok:
        raise SystemExit(
            "\n[FAIL] class-branch channels != nc. If cv3.* shows 80 you exported the COCO head — "
            "retrain/re-export from the 4-class best.pt, not yolov8s.pt.")
    print(f"\n[OK] {args.out} ready for Enlight converter.py (--num-class {args.nc}).")


if __name__ == "__main__":
    main()
