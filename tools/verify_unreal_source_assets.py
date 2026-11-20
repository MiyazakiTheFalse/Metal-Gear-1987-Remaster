#!/usr/bin/env python3
"""Validate the Unreal project's self-contained character source assets.

Runs with ordinary Python (no Unreal needed) and fails on incomplete copies,
missing skeleton channels, invalid glTF headers, or missing source animations.
A passing report does NOT mean native .uasset meshes / AnimBlueprints exist.
"""
from __future__ import annotations

import hashlib
import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CHARACTERS = ROOT / "unreal" / "MGRemaster" / "Content" / "SourceAssets" / "Characters"
BROWSER = ROOT / "browser" / "assets" / "models"
ROLE_NAMES = ("snake", "fox", "guard", "shoot_gunner")
COMMON_CLIPS = {
    "Idle", "Walk", "Run", "Crouch", "Prone", "Down",
    "Aim", "Cover", "Melee", "Reaching", "Hit",
    "RadioListen", "CrouchAim", "ProneAim",
}
REQUIRED_JOINTS = {
    "pelvis", "chest", "neck", "head", "jaw", "brow",
    "hand_L", "hand_R", "foot_L", "foot_R",
}


def inspect_glb(path: Path) -> dict:
    payload = path.read_bytes()
    if len(payload) < 28 or payload[:4] != b"glTF":
        raise ValueError(f"{path}: missing glTF magic / binary payload")
    version, length, json_length = struct.unpack_from("<III", payload, 4)
    if version != 2 or length != len(payload) or payload[16:20] != b"JSON":
        raise ValueError(f"{path}: invalid GLB 2.0 structure")
    if 20 + json_length + 8 > len(payload):
        raise ValueError(f"{path}: GLB JSON extends past binary")
    document = json.loads(payload[20:20+json_length])
    skins = document.get("skins", [])
    if len(skins) != 1 or len(skins[0].get("joints", [])) != 18:
        raise ValueError(f"{path}: expected one 18-joint skin")
    nodes = document.get("nodes", [])
    joints = {nodes[index].get("name", "") for index in skins[0]["joints"]}
    if not REQUIRED_JOINTS.issubset(joints):
        raise ValueError(f"{path}: joints missing {REQUIRED_JOINTS-joints}")
    primitives = document.get("meshes", [{}])[0].get("primitives", [])
    if not primitives:
        raise ValueError(f"{path}: missing skinned mesh")
    for mesh in primitives:
        attrs = mesh.get("attributes", {})
        for channel in ("POSITION", "NORMAL", "COLOR_0", "JOINTS_0", "WEIGHTS_0"):
            if channel not in attrs:
                raise ValueError(f"{path}: missing {channel}")
    animations = {a.get("name") for a in document.get("animations", [])}
    expected = set(COMMON_CLIPS)
    if path.stem == "fox":
        expected.add("Captive")
    if path.stem == "shoot_gunner":
        expected.update(("Address", "Shoulder", "Recoil", "Search"))
    if not expected.issubset(animations):
        raise ValueError(f"{path}: animation clips missing {expected-animations}")
    if len(payload) < 100_000:
        raise ValueError(f"{path}: unexpectedly short binary")
    return {
        "role": path.stem,
        "bytes": len(payload),
        "sha256": hashlib.sha256(payload).hexdigest(),
        "joints": len(skins[0]["joints"]),
        "animations": len(animations),
        "mesh_primitives": len(primitives),
    }


def validate() -> dict:
    if not CHARACTERS.is_dir():
        raise FileNotFoundError(f"Unreal source asset folder missing: {CHARACTERS}")
    records = []
    for role in ROLE_NAMES:
        native_source = CHARACTERS / f"{role}.glb"
        browser_source = BROWSER / f"{role}.glb"
        if not native_source.is_file() or not browser_source.is_file():
            raise FileNotFoundError(f"Missing cross-engine source model: {role}")
        report = inspect_glb(native_source)
        if native_source.read_bytes() != browser_source.read_bytes():
            raise ValueError(f"{role}: browser and Unreal source GLBs diverged")
        records.append(report)
    return {
        "status": "self_contained_source_assets_verified",
        "native_uassets_verified": False,
        "editor_import_required": True,
        "characters": records,
    }


def main():
    try:
        print(json.dumps(validate(), indent=2))
    except (OSError, ValueError, IndexError, KeyError, TypeError) as exc:
        print(f"Unreal source-asset check FAILED: {exc}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
