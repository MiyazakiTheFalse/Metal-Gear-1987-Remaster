"""Export 1995 radio portrait expression frames and (optionally) raster PNGs.

Run from repository root:
    python tools/build_codec_portraits.py
    python tools/build_codec_portraits.py --svg-only

The editable SVGs are authored original illustrations based on the Japanese
1987 MSX2 manual character plate. Unreal imports PNGs; no plugin purchase.
Requires local Node and (unless --svg-only) the optional 'cairosvg' package.
Rendering mouth motion is estimate-based until actual VO viseme data is authored.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PORTRAITS = ROOT / "assets" / "source" / "codec"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--svg-only", action="store_true")
    args = parser.parse_args()

    command = ["node", str(ROOT / "tools" / "export_codec_portraits.cjs")]
    try:
        subprocess.run(command, cwd=ROOT, check=True)
    except (FileNotFoundError, subprocess.CalledProcessError) as exc:
        print(f"Portrait source export failed: {exc}", file=sys.stderr)
        return 1

    frames = sorted(PORTRAITS.glob("T_Codec_*_*.svg"))
    if len(frames) != 24:
        print(f"Expected 24 character/expression SVGs, got {len(frames)}", file=sys.stderr)
        return 1
    if args.svg_only:
        print("SVG expression assets ready; Unreal PNG conversion not requested.")
        return 0

    try:
        import cairosvg
    except ImportError:
        print("PNG conversion unavailable: install cairosvg, or run --svg-only.",
              file=sys.stderr)
        return 2
    output = PORTRAITS / "png"
    output.mkdir(exist_ok=True)
    for frame in frames:
        cairosvg.svg2png(bytestring=frame.read_bytes(),
                        write_to=str(output / (frame.stem + ".png")),
                        output_width=480, output_height=566)
    print(f"Generated {len(frames)} PNGs at {output}. Import them with the Unreal Python task.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
