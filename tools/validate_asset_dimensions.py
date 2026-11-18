#!/usr/bin/env python3
"""Enforce game-source provenance, geometry bounds and camera/actor clearance.

This is static geometry verification, not a substitute for playtesting Unreal.
"""
import argparse
import json
import math
from pathlib import Path


def read_vertices(path):
    vertices = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if line.startswith("v "):
            data = line.split()
            if len(data) != 4:
                raise ValueError(f"{path}: malformed vertex")
            vertex = tuple(float(part) for part in data[1:])
            if any(not math.isfinite(value) for value in vertex):
                raise ValueError(f"{path}: nonfinite vertex")
            vertices.append(vertex)
    if not vertices:
        raise ValueError(f"{path}: empty mesh")
    return vertices


def extents(vertices):
    return tuple(max(p[i] for p in vertices) - min(p[i] for p in vertices)
                 for i in range(3))


def validate(root, strict=False):
    root = Path(root)
    problems = []
    path = root / "assets" / "asset_dimensions.json"
    data = json.loads(path.read_text(encoding="utf-8"))
    if data.get("schema_version") != 1:
        problems.append("Unsupported catalogue schema")
    if data.get("units") != "metres" or data.get("obj_vertex_units") != "centimetres":
        problems.append("Unexpected coordinate units")
    original = data.get("original_map", {})
    if original.get("status") != "verified_room_map" and original.get("physical_tile_size_m") is not None:
        problems.append("Unverified original map cannot assert real metres per tile")
    char = data.get("character", {})
    if not (1.5 <= char.get("reference_height_m", 0) <= 2.1):
        problems.append("Character height missing or implausible")
    if not char.get("source") or not char.get("temporal_note"):
        problems.append("Character dimensions require dated source context")
    radius = char.get("collision_radius_m", 0)
    gate = data.get("provisional_greybox_thresholds", {})
    walkway = gate.get("minimum_walkway_m", 0)
    headroom = gate.get("minimum_headroom_m", 0)
    if walkway < 2 * radius + 0.2:
        problems.append("Walkway narrower than player collision diameter plus allowance")
    if headroom < char.get("reference_height_m", 0) + 0.15:
        problems.append("Headroom has insufficient character clearance")
    if gate.get("camera_clearance_m", 0) < 0.25:
        problems.append("Camera clearance below provisional safety floor")
    ref_ids = {s.get("id") for s in data.get("references", [])}
    names = set()
    for a in data.get("assets", []):
        name = a.get("id")
        if not isinstance(name, str) or not name or name in names:
            problems.append("Missing or duplicate asset id")
            continue
        names.add(name)
        if any(ref not in ref_ids for ref in a.get("refs", [])) or not a.get("refs"):
            problems.append(f"{name}: missing recognised provenance")
        if not a.get("purpose"):
            problems.append(f"{name}: purpose not documented")
        dims = [a.get(k) for k in ("w", "d", "h")]
        if any(not isinstance(x, (float, int)) or
               isinstance(x, bool) or not math.isfinite(x) or not 0 < x <= 30
               for x in dims):
            problems.append(f"{name}: invalid dimensions")
            continue
        if a.get("kind") not in ("box", "door"):
            problems.append(f"{name}: unsupported module")
        if a.get("kind") == "door":
            if not 0 < a.get("opening_w", 0) < a["w"]:
                problems.append(f"{name}: invalid door opening width")
            if not 0 < a.get("opening_h", 0) < a["h"]:
                problems.append(f"{name}: invalid door opening height")
            if a.get("opening_w", 0) < walkway:
                problems.append(f"{name}: door clear width below walkway specification")
            if a.get("opening_h", 0) < headroom:
                problems.append(f"{name}: door clear height below headroom specification")
        mesh_path = root / "assets" / "greybox" / f"{name}.obj"
        if not mesh_path.is_file():
            problems.append(f"{name}: missing OBJ source")
            continue
        try:
            vertices = read_vertices(mesh_path)
        except ValueError as err:
            problems.append(str(err))
            continue
        expected = [dimension * 100 for dimension in dims]
        for axis, observed, wanted in zip("XYZ", extents(vertices), expected):
            if abs(observed - wanted) > 1.0:
                problems.append(f"{name}: {axis}-axis OBJ bounds {observed:.2f}cm vs expected {wanted:.2f}cm")
        if a.get("kind") == "door" and len(vertices) == 24:
            # Generator's three box segments: left post, right post, lintel.
            clear_width_cm = min(p[0] for p in vertices[8:16]) - max(p[0] for p in vertices[:8])
            clear_height_cm = min(p[2] for p in vertices[16:24])
            if abs(clear_width_cm - a["opening_w"] * 100) > 1.0:
                problems.append(f"{name}: actual door opening width differs from design")
            if abs(clear_height_cm - a["opening_h"] * 100) > 1.0:
                problems.append(f"{name}: actual door opening height differs from design")
        elif a.get("kind") == "door":
            problems.append(f"{name}: door requires three box components to verify opening")
    for mesh_path in (root / "assets" / "greybox").glob("*.obj"):
        if mesh_path.stem not in names:
            problems.append(f"{mesh_path.name}: mesh missing from source ledger")
    if strict:
        problems.append("Final approval requires in-engine collision, camera, guard and controller playtest evidence; static OBJ bounds cannot certify gameplay")
    return problems


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--strict", action="store_true")
    args = parser.parse_args()
    errors = validate(args.root, strict=args.strict)
    if errors:
        for error in errors:
            print("FAIL:", error)
        raise SystemExit(1)
    print("PASS: authored mesh dimensions, player/door clearances and source provenance are internally consistent")


if __name__ == "__main__":
    main()
