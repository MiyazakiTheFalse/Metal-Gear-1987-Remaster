#!/usr/bin/env python3
"""Make a portable browser-only ZIP from the CURRENT checked-out branch.

Usage: python scripts/package_browser_playtest.py
Requires Node for regression tests; package contains playable browser assets,
not Unreal binaries. No internet requirement for normal 2D/WebGL field game.
The optional Three.js character inspector loads its library from a CDN.
"""
from __future__ import annotations

from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "browser"
OUTPUT = ROOT / "dist" / "Outer_Heaven_Browser_Playtest_Latest.zip"

REQUIRED = (
    "Play-Windows.cmd",
    "index.html",
    "game.js",
    "dialogue.js",
    "tutorial.js",
    "story_relationships.js",
    "cinematic_director.js",
    "codec_portraits.js",
    "equipment_strip.js",
    "animation_intent.js",
    "atlas.html",
    "atlas/map.js",
    "atlas/map.css",
    "atlas/outer_heaven_1995.json",
    "tools/serve_playtest.py",
    "tools/serve_playtest.ps1",
    "assets/models/snake.glb",
    "assets/models/fox.glb",
    "assets/models/guard.glb",
    "assets/models/shoot_gunner.glb",
    "assets/models/machinegun_kid.glb",
    "assets/models/pow_informant.glb",
)
EXCLUDE_PARTS = {"__pycache__", ".git", ".pytest_cache", "node_modules"}
EXCLUDE_SUFFIXES = {".pyc", ".tmp", ".bak"}


def checked_git_head() -> str:
    try:
        return subprocess.check_output(
            ["git", "-C", str(ROOT), "rev-parse", "HEAD"],
            text=True, stderr=subprocess.DEVNULL,
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return "HEAD_UNKNOWN_LOCAL_COPY"


def run():
    absent = [name for name in REQUIRED if not (SOURCE / name).is_file()]
    if absent:
        raise RuntimeError("Missing required browser source/GLB: " + ", ".join(absent))
    for name in REQUIRED:
        if name.endswith(".glb") and (SOURCE / name).stat().st_size < 100_000:
            raise RuntimeError("GLB is missing skeleton source or suspiciously tiny: " + name)
    html = (SOURCE / "index.html").read_text(encoding="utf8")
    # Every local script actually referenced by the entrypoint must be packaged.
    import re
    for name in re.findall(r'<script\s+src="([^"]+)"', html):
        if name.startswith("http://") or name.startswith("https://"):
            continue
        if not (SOURCE / name).is_file():
            raise RuntimeError("Entry point points at an unbundled script: " + name)
    atlas = json.loads((SOURCE / "atlas/outer_heaven_1995.json").read_text(encoding="utf8"))
    if (len(atlas["areas"]), sum(len(z["beats"]) for z in atlas["areas"])) != (52, 114):
        raise RuntimeError("Atlas coverage dropped below 52 areas / 114 original beats.")

    node = shutil.which("node")
    if not node:
        raise RuntimeError("Packaging needs Node.js to validate scripts; end-user extraction does not.")
    for test in ("test_equipment_strip.cjs", "test_animation_intent.cjs", "test_campaign.cjs"):
        subprocess.run([node, str(SOURCE / "tests" / test)], cwd=ROOT, check=True)
    assets = sorted(p for p in SOURCE.rglob("*")
                    if p.is_file() and not EXCLUDE_PARTS.intersection(p.parts)
                    and p.suffix.lower() not in EXCLUDE_SUFFIXES)
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    commit = checked_git_head()
    manifest = {
        "project": "Outer Heaven 1995 - independently authored browser prototype",
        "commit": commit,
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "atlas_areas": 52,
        "atlas_beats": 114,
        "boss_arenas": 11,
        "source_caveat": "Design atlas placement and graphical assets are provisional; not an end-to-end canonical or Unreal-verified shipping game.",
        "launch_windows": "Play-Windows.cmd",
        "launch_other": "python3 tools/serve_playtest.py",
        "atlas_url": "http://127.0.0.1:19870/atlas.html",
        "file_count": len(assets),
    }
    with zipfile.ZipFile(OUTPUT, "w", compression=zipfile.ZIP_DEFLATED,
                         compresslevel=7, allowZip64=True) as bundle:
        for p in assets:
            bundle.write(p, p.relative_to(SOURCE).as_posix())
        bundle.writestr("BUILD_MANIFEST.json", json.dumps(manifest, indent=2))
    with zipfile.ZipFile(OUTPUT) as bundle:
        corrupt = bundle.testzip()
        if corrupt:
            raise RuntimeError("Output ZIP CRC failed: " + corrupt)
        paths = set(bundle.namelist())
        if not set(REQUIRED).issubset(paths):
            raise RuntimeError("ZIP missing required assets after writing")
    digest = hashlib.sha256(OUTPUT.read_bytes()).hexdigest()
    print(f"PASS: {OUTPUT} ({OUTPUT.stat().st_size:,} bytes, {len(assets)} files)")
    print("Commit:", commit, "SHA256:", digest)
    print("Extract and run Play-Windows.cmd. Atlas: http://127.0.0.1:19870/atlas.html")


if __name__ == "__main__":
    try:
        run()
    except (OSError, RuntimeError, subprocess.CalledProcessError) as exc:
        print("BROWSER PACKAGE FAILED:", exc, file=sys.stderr)
        raise SystemExit(1)
