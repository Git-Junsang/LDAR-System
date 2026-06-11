#!/usr/bin/env python3
"""LDAR traffic-sign YOLOv8 fine-tune.

Clean from-scratch training of a 4-class sign detector
(Stop / No Entry / Speed_Limit_60 / Speed_Limit_30) for the AI-G NPU.

We fine-tune **yolov8s** because the proven NPU path (yolov8s.bin, Enlight
compiled) is yolov8s — keeping the same architecture minimizes tc-nn-toolkit
compile risk, which is where the previous attempt failed.

Examples
--------
# quick pipeline smoke test (a few epochs, CPU)
python3 train.py --epochs 3 --imgsz 416 --name smoke

# full training (run on a GPU PC for speed: --device 0)
python3 train.py --epochs 120 --imgsz 640 --batch 16 --device cpu
"""
import argparse
from pathlib import Path

HERE = Path(__file__).resolve().parent
DEFAULT_DATA = HERE / "dataset" / "data.yaml"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--model", default="yolov8s.pt", help="pretrained weights to fine-tune (default yolov8s.pt)")
    ap.add_argument("--data", default=str(DEFAULT_DATA), help="dataset yaml")
    ap.add_argument("--epochs", type=int, default=120)
    ap.add_argument("--imgsz", type=int, default=640, help="train resolution (match NPU input; 640 default)")
    ap.add_argument("--batch", type=int, default=16)
    ap.add_argument("--device", default="cpu", help="'cpu' or GPU index like '0'")
    ap.add_argument("--workers", type=int, default=6)
    ap.add_argument("--name", default="signs_yolov8s", help="run name under runs/detect/")
    ap.add_argument("--patience", type=int, default=30, help="early-stop patience (0 disables)")
    ap.add_argument("--resume", action="store_true", help="resume the named run")
    args = ap.parse_args()

    from ultralytics import YOLO

    # Make the dataset path machine-independent: rewrite `path` to the dataset
    # dir next to this script, so train.py works on any PC without editing yaml.
    data_path = Path(args.data)
    if data_path.resolve() == DEFAULT_DATA.resolve() and (HERE / "dataset").is_dir():
        import yaml
        spec = yaml.safe_load(DEFAULT_DATA.read_text())
        spec["path"] = str(HERE / "dataset")
        resolved = HERE / "dataset" / ".data.resolved.yaml"
        resolved.write_text(yaml.safe_dump(spec, allow_unicode=True, sort_keys=False))
        args.data = str(resolved)
        print(f"[train] resolved dataset path -> {spec['path']}")

    print(f"[train] model={args.model} data={args.data} imgsz={args.imgsz} "
          f"epochs={args.epochs} batch={args.batch} device={args.device}")

    model = YOLO(args.model)
    model.train(
        data=args.data,
        epochs=args.epochs,
        imgsz=args.imgsz,
        batch=args.batch,
        device=args.device,
        workers=args.workers,
        name=args.name,
        patience=args.patience,
        resume=args.resume,
        project=str(HERE / "runs" / "detect"),
        # fixed-demo overfit strategy: light aug, no destructive flips
        # (a sign mirrored is a different sign; keep fliplr off)
        fliplr=0.0,
        flipud=0.0,
        plots=True,
    )

    # validate on the test split too, for an honest number
    metrics = model.val(data=args.data, split="test", imgsz=args.imgsz,
                        device=args.device, name=args.name + "_test")
    print(f"[train] done. test mAP50={metrics.box.map50:.4f} mAP50-95={metrics.box.map:.4f}")
    print(f"[train] best weights: {HERE / 'runs' / 'detect' / args.name / 'weights' / 'best.pt'}")


if __name__ == "__main__":
    main()
