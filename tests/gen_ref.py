import subprocess
import sys
import argparse
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("scene_name")
    parser.add_argument("--build_dir", required=False, default="build")
    args = parser.parse_args()

    SCRIPT_DIR = Path(__file__).resolve().parent
    PROJECT_ROOT = SCRIPT_DIR.parent

    out_path = Path(PROJECT_ROOT / args.build_dir / "tests/out" / (args.scene_name + ".png"))
    ref_path = Path(PROJECT_ROOT / "tests/references" / (args.scene_name + ".png"))
    executable = Path(PROJECT_ROOT / args.build_dir / "tests" / args.scene_name)

    subprocess.check_call([executable, str(out_path)])

    print(f"Regenerating reference: {ref_path}")
    out_path.replace(ref_path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
