#!/usr/bin/env python3
"""Inspect a licensed Connect2Media Metal Gear J2ME archive locally.
Do not run Java ME bytecode. No original game data is committed to Git.
"""
import argparse
from collections import Counter
import hashlib
import io
import json
from pathlib import Path
import re
import struct
from zipfile import ZipFile

PNG = b"\x89PNG\r\n\x1a\n"
PREFERRED = "(a1).jar"
RECORD_NAME = re.compile(r"f([1-9][0-9]*)\Z")


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def parse_record(data):
    if len(data) < 4:
        raise ValueError("Truncated numbered record")
    header = int.from_bytes(data[:4], "big")
    if (header & 0xC0000000) != 0x40000000:
        raise ValueError("Unknown record wrapper flags")
    payload = data[4:]
    if (header & 0x3FFFFFFF) != len(payload):
        raise ValueError("Declared record payload length differs from actual length")
    return payload


def embedded_pngs(data):
    """Iterate whole PNGs, validating boundaries; CRC validation remains future work."""
    offset = 0
    while True:
        start = data.find(PNG, offset)
        if start < 0:
            return
        cursor = start + len(PNG)
        dimensions = None
        for _ in range(500):
            if cursor + 12 > len(data):
                raise ValueError("Truncated PNG chunk")
            size = struct.unpack_from(">I", data, cursor)[0]
            kind = data[cursor + 4:cursor + 8]
            end = cursor + 12 + size
            if end > len(data):
                raise ValueError("PNG chunk exceeds record")
            if kind == b"IHDR":
                if size < 8:
                    raise ValueError("Corrupt PNG header")
                dimensions = struct.unpack_from(">II", data, cursor + 8)
            cursor = end
            if kind == b"IEND":
                if dimensions is None:
                    raise ValueError("PNG has no image header")
                yield data[start:cursor], dimensions, start
                offset = cursor
                break
        else:
            raise ValueError("Too many PNG chunks")


def audit(archive_path, output_path):
    output_path = Path(output_path)
    output_path.mkdir(parents=True, exist_ok=True)
    with ZipFile(archive_path) as outer:
        jars = [name for name in outer.namelist() if name.lower().endswith(".jar")]
        chosen = [name for name in jars if name.endswith(PREFERRED)]
        if len(chosen) != 1:
            raise ValueError("Expected exactly one (a1).jar MIDP 2.0 reference")
        selected_name = chosen[0]
        source = outer.read(selected_name)
        with ZipFile(io.BytesIO(source)) as game:
            names = set(game.namelist())
            manifest = game.read("META-INF/MANIFEST.MF").decode("utf-8", "replace")
            if "MIDP-2.0" not in manifest or "CLDC-1.1" not in manifest:
                raise ValueError("Unexpected J2ME runtime requirements")
            reference = output_path / "reference"
            reference.mkdir(exist_ok=True)
            (reference / "source.jar").write_bytes(source)
            (reference / "manifest.mf").write_text(manifest, encoding="utf-8")

            numbered = sorted((n for n in names if RECORD_NAME.fullmatch(n)),
                              key=lambda n: int(n[1:]))
            if numbered != [f"f{x}" for x in range(1, 75)]:
                raise ValueError("Expected exactly the numbered records f1..f74")

            metadata = []
            png_count = 0
            for name in numbered:
                raw = game.read(name)
                payload = parse_record(raw)
                dest = output_path / "legacy_raw" / name
                dest.parent.mkdir(exist_ok=True)
                dest.write_bytes(raw)
                entry = {"name": name, "bytes": len(raw), "payload_bytes": len(payload),
                         "sha256": sha256(raw), "pngs": []}
                for index, (image, size, offset) in enumerate(embedded_pngs(raw), start=1):
                    fname = f"{name}_{index:02d}.png"
                    png_path = output_path / "embedded_png" / fname
                    png_path.parent.mkdir(exist_ok=True)
                    png_path.write_bytes(image)
                    entry["pngs"].append({"file": fname, "size": list(size), "offset": offset})
                    png_count += 1
                metadata.append(entry)

            tracks = sorted(n for n in names if n.lower().endswith(".mid") and "/" not in n)
            for name in tracks:
                dest = output_path / "music" / name
                dest.parent.mkdir(exist_ok=True)
                dest.write_bytes(game.read(name))

        comparisons = {}
        for name in jars:
            with ZipFile(io.BytesIO(outer.read(name))) as variant:
                variants = set(variant.namelist())
                changed = [entry["name"] for entry in metadata if
                           entry["name"] not in variants or
                           sha256(variant.read(entry["name"])) != entry["sha256"]]
                comparisons[name] = {"size": len(outer.read(name)),
                                     "changed_numbered_records": changed}

    report = {"reference": selected_name, "sha256": sha256(source),
              "numbered_records": len(metadata), "embedded_pngs": png_count,
              "midi_tracks": len(tracks), "comparisons": comparisons,
              "records": metadata}
    (output_path / "audit.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({k: report[k] for k in
        ("reference", "numbered_records", "embedded_pngs", "midi_tracks")}, indent=2))
    return report


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("archive", type=Path, help="Outer ZIP containing J2ME JAR editions")
    parser.add_argument("--output", type=Path, default=Path("content"))
    args = parser.parse_args()
    audit(args.archive, args.output)
