import subprocess
import sys
import argparse
from pathlib import Path
import numpy as np
from PIL import Image

TOLERANCE = 1
DIFF_SCALE = 20


def save_diff_images(diff, out_path):
    out_path = Path(out_path)
    diff_path = out_path.with_suffix(".diff.png")
    mask_path = out_path.with_suffix(".mask.png")

    # Amplified visual diff
    vis = np.clip(diff * DIFF_SCALE, 0, 255).astype(np.uint8)
    Image.fromarray(vis).save(diff_path)

    # Binary mask of failing pixels
    mask = np.any(diff > TOLERANCE, axis=2).astype(np.uint8) * 255
    Image.fromarray(mask).save(mask_path)

    print(f"Saved diff image: {diff_path}")
    print(f"Saved mask image: {mask_path}")


def compare(img1_path, img2_path, out_path):
    img1 = np.array(Image.open(img1_path)).astype(np.float64)
    img2 = np.array(Image.open(img2_path)).astype(np.float64)

    if img1.shape != img2.shape:
        print("Size mismatch!")
        return False

    size = img1.shape[0] * img1.shape[1]

    diff = np.abs(img1 - img2)
    max_diff = diff.max()
    mse = np.mean(diff ** 2)
    differing_pixels = np.sum(np.any(diff > TOLERANCE, axis=2))
    percent_diff = 100.0 * differing_pixels / size

    print(f"Max diff: {max_diff}; ", end="")
    print(f"MSE: {mse:.3f}; ", end="")
    print(f"Differing pixels: {differing_pixels} / {size} ({percent_diff:.3f}%)")

    if max_diff > TOLERANCE:
        # Locate first failing pixel
        y, x = np.argwhere(np.any(diff > TOLERANCE, axis=2))[0]
        print(f"First failing pixel: ({x}, {y})")
        save_diff_images(diff, out_path)
        return False

    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", required=True)
    parser.add_argument("--ref", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args()

    out_path = Path(args.out)
    ref_path = Path(args.ref)

    # Run scene executable
    subprocess.check_call([args.exe, str(out_path)])

    if not ref_path.exists():
        print(f"Reference image missing: {ref_path}")
        return 1

    if compare(out_path, ref_path, out_path):
        print("OK")
        return 0
    else:
        return 1


if __name__ == "__main__":
    sys.exit(main())
