#!/usr/bin/env python3
"""Download a vetted, reproducible CC0 PBR subset from Poly Haven.

Run on a workstation with internet access:
  python tools/acquire_polyhaven_pbr.py --resolution 2k

Source files remain under unreal/MGRemaster/ArtSource/PolyHavenCC0.
Use tools/unreal_import_pbr.py in the Unreal Python Editor to import/cook them.

No marketplace content, game-ripped art or vendor user credentials involved.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
from urllib.parse import urlparse
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / "data" / "outer_heaven_art_sources.json"
DEFAULT_OUT = ROOT / "unreal" / "MGRemaster" / "ArtSource" / "PolyHavenCC0"
API = "https://api.polyhaven.com/files/"
USER_AGENT = "OuterHeavenRemake-ArtImporter/0.1 (CC0 provenance)"
MAX_MAP_BYTES = 40 * 1024 * 1024

# Important: Unreal expects DirectX normal convention (green channel down).
MAP_ALIASES = {
    "diffuse": ("diff", "diffuse", "basecolor", "base_color", "albedo", "color"),
    "normal_dx": ("nor_dx", "normal_dx", "normaldx"),
    "normal_gl": ("nor_gl", "normal", "nor", "normalgl"),
    "roughness": ("rough", "roughness"),
    "metallic": ("metal", "metallic", "metalness"),
    "ambient_occlusion": ("ao", "ambientocclusion", "ambient_occlusion"),
    "arm_packed": ("arm", "orm", "rma", "arm_packed"),
}

def _clean(s: str) -> str:
    return "".join(c for c in s.lower() if c.isalnum())

def secure_url(url: str, api: bool = False) -> bool:
    address = urlparse(url)
    host = (address.hostname or "").lower()
    if address.scheme != "https" or address.username or address.password:
        return False
    if api:
        return host == "api.polyhaven.com"
    return host == "dl.polyhaven.org" or host.endswith(".polyhaven.org")

def request_bytes(url: str, *, api: bool = False, max_bytes=MAX_MAP_BYTES) -> bytes:
    if not secure_url(url, api=api):
        raise ValueError(f"Unexpected/unsafe asset host: {url!r}")
    req = Request(url, headers={"User-Agent": USER_AGENT})
    with urlopen(req, timeout=40) as response:
        limit = max_bytes + 1
        blob = response.read(limit)
        if len(blob) > max_bytes:
            raise ValueError(f"Asset file too large (> {max_bytes} bytes): {url}")
        return blob

def choose_file_map(api_data: dict, resolution: str) -> dict:
    """Take named diffuse/normal/rough/metal/AO maps from Poly Haven files API.

    Source variants differ: separate maps and packed ORM are both accepted;
    no formats or sizes are fabricated if absent.
    """
    selected = {}
    for role, aliases in MAP_ALIASES.items():
        chosen = None
        for original, candidates in api_data.items():
            if _clean(original) not in {_clean(k) for k in aliases}:
                continue
            if not isinstance(candidates, dict):
                continue
            variant = candidates.get(resolution)
            if variant is None:
                variant = candidates.get("1k") if resolution != "1k" else None
            if not isinstance(variant, dict):
                continue
            for fmt in ("jpg", "png", "webp", "jpeg"):
                payload = variant.get(fmt)
                if isinstance(payload, dict) and secure_url(payload.get("url", "")):
                    chosen = {"source_map": original, "resolution": resolution
                              if resolution in candidates else "1k",
                              "format": fmt, **payload}
                    break
            if chosen:
                break
        if chosen:
            selected[role] = chosen
    if "diffuse" not in selected:
        raise ValueError("Poly Haven did not supply a recognized color map")
    if "normal_dx" in selected:
        selected.pop("normal_gl", None)
    if "arm_packed" in selected and all(
        k not in selected for k in ("roughness","metallic","ambient_occlusion")
    ):
        selected["arm_packed"]["channel_layout"] = "R=AO,G=roughness,B=metallic"
    return selected

def download_one(slug: str, destination: Path, resolution: str,
                 dry_run: bool = False) -> dict:
    api_data = json.loads(request_bytes(API + slug, api=True, max_bytes=3_000_000))
    found = choose_file_map(api_data, resolution)
    destination.mkdir(parents=True, exist_ok=True)
    files = {}
    for role, info in found.items():
        ext = "jpg" if info["format"] == "jpeg" else info["format"]
        path = destination / f"{slug}_{role}.{ext}"
        url = info["url"]
        expected = str(info.get("md5", "")).lower()
        if not dry_run:
            if path.is_file() and expected and hashlib.md5(path.read_bytes()).hexdigest() == expected:
                pass  # already acquired and checksum confirmed
            else:
                blob = request_bytes(url)
                digest = hashlib.md5(blob).hexdigest()
                if expected and digest != expected:
                    raise ValueError(f"Checksum mismatch for {slug}/{role}")
                temp = path.with_suffix(path.suffix + ".part")
                temp.write_bytes(blob)
                temp.replace(path)
        files[role] = {"filename": path.name, "source_url": url,
                       "source_map": info["source_map"],
                       "resolution": info["resolution"],
                       "md5": expected,
                       "normal_green_flip_in_unreal": role == "normal_gl",
                       "downloaded": not dry_run}
        print(f"  {slug}/{role}: {path.name} ({info['resolution']})")
    metadata = {
        "slug": slug, "license": "CC0-1.0",
        "license_url": "https://polyhaven.com/license",
        "asset_page": f"https://polyhaven.com/a/{slug}",
        "source": "Poly Haven", "requested_resolution": resolution,
        "files": files
    }
    if not dry_run:
        (destination / "source_manifest.json").write_text(
            json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
    return metadata

def main() -> None:
    cli = argparse.ArgumentParser()
    cli.add_argument("--resolution", choices=("1k","2k","4k"), default="2k")
    cli.add_argument("--out", type=Path, default=DEFAULT_OUT)
    cli.add_argument("--dry-run", action="store_true")
    args = cli.parse_args()
    catalog = json.loads(CATALOG.read_text(encoding="utf-8"))
    manifest = []
    for material in catalog["materials"]:
        slug = material["polyhaven_slug"]
        print(f"Poly Haven CC0: {slug}")
        info = download_one(slug, args.out / slug, args.resolution,
                            dry_run=args.dry_run)
        info["unreal_material"] = material["unreal_path"]
        manifest.append(info)
    if not args.dry_run:
        args.out.mkdir(parents=True, exist_ok=True)
        (args.out / "_download_manifest.json").write_text(
            json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Selected {len(manifest)} licensed PBR materials; "
          f"dry_run={args.dry_run}")

if __name__ == "__main__":
    main()
