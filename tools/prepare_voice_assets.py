"""Prepare *licensed, human-performed* character VO; this creates no voices.

Run locally: python tools/prepare_voice_assets.py [--strict]
Requires ffmpeg/ffprobe on PATH. Input: assets/source/vo/raw/<speaker>/<clip>.wav
Output: assets/source/vo/processed/<speaker>/<clip>.wav

Two-pass loudnorm makes actual measured loudness predictable. Each character's
voice identity comes from casting and performance, not pitch-warping a generic
voice or cloning a copyrighted actor's performance.
"""
from __future__ import annotations
import argparse
import json
import math
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "data" / "voice_performance_manifest.json"
RAW = ROOT / "assets" / "source" / "vo" / "raw"
PROCESSED = ROOT / "assets" / "source" / "vo" / "processed"
ID = re.compile(r"[a-z0-9_]+\Z")
CLIP = re.compile(r"VO_[a-z0-9_]+_[0-9]{2}\Z")

def command(args: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(args, capture_output=True, text=True, check=True)

def measure_loudness(source: Path, filters: str, target: float) -> dict:
    stats = command([
        "ffmpeg", "-hide_banner", "-nostdin", "-i", str(source),
        "-af", filters + f",loudnorm=I={target}:TP=-1.5:LRA=7:print_format=json",
        "-f", "null", "-"
    ]).stderr
    match = re.search(r'\{[^{}]*"input_i"[^{}]*\}', stats, re.S)
    if not match:
        raise RuntimeError(f"ffmpeg returned no loudness measurements: {source}")
    measured = json.loads(match.group(0))
    for field in ("input_i", "input_tp", "input_lra", "input_thresh", "target_offset"):
        value = float(measured[field])
        if not math.isfinite(value):
            raise ValueError(f"Silence/invalid loudness {field} in {source}")
    return measured

def render(source: Path, target_path: Path, profile: dict) -> None:
    # Keep recordings dry; radio filtering is clear and restrained, not an
    # aggressively narrow telephone pass that destroys consonant clarity.
    hp = int(profile["radio_highpass_hz"])
    lp = int(profile["radio_lowpass_hz"])
    filters = f"highpass=f={hp},lowpass=f={lp},acompressor=threshold=0.18:ratio=1.7:attack=12:release=180"
    goal = float(profile["target_lufs"])
    m = measure_loudness(source, filters, goal)
    loudnorm = (
        f"loudnorm=I={goal}:TP=-1.5:LRA=7"
        f":measured_I={m['input_i']}:measured_TP={m['input_tp']}"
        f":measured_LRA={m['input_lra']}:measured_thresh={m['input_thresh']}"
        f":offset={m['target_offset']}:linear=true:print_format=summary"
    )
    target_path.parent.mkdir(parents=True, exist_ok=True)
    command([
        "ffmpeg", "-hide_banner", "-nostdin", "-y", "-i", str(source),
        "-map_metadata", "-1", "-vn", "-af", filters + "," + loudnorm,
        "-ar", "48000", "-ac", "1", "-c:a", "pcm_s24le", str(target_path)
    ])
    probe = command([
        "ffprobe", "-v", "error", "-select_streams", "a:0",
        "-show_entries", "stream=sample_rate,channels,sample_fmt",
        "-of", "json", str(target_path)
    ])
    streams = json.loads(probe.stdout).get("streams", [])
    if len(streams) != 1 or streams[0].get("sample_rate") != "48000" or streams[0].get("channels") != 1:
        raise RuntimeError(f"Output stream validation failed for {target_path}")

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--strict", action="store_true",
                        help="Fail if a dialogue line has no source master")
    args = parser.parse_args()
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    total = completed = 0
    missing = []
    used = set()
    for scene in data["scenes"]:
        scene_id = scene["scene_id"]
        if not ID.fullmatch(scene_id):
            raise ValueError(f"Invalid scene ID: {scene_id}")
        for line in scene["lines"]:
            speaker, clip = line["speaker"], line["clip"]
            if clip is None:
                if not line.get("continuation_of"):
                    raise ValueError(f"Unexplained unvoiced continuation: {scene_id}")
                continue
            if speaker not in data["profiles"] or not ID.fullmatch(speaker) or not CLIP.fullmatch(clip):
                raise ValueError(f"Unsafe/missing clip/speaker: {speaker} / {clip}")
            key = (speaker, clip)
            if key in used:
                raise ValueError(f"Duplicate VO clip identity: {key}")
            used.add(key)
            total += 1
            raw = RAW / speaker / (clip + ".wav")
            if not raw.is_file():
                missing.append(str(raw.relative_to(ROOT)))
                continue
            target = PROCESSED / speaker / (clip + ".wav")
            render(raw, target, data["profiles"][speaker])
            completed += 1
    print(f"Processed {completed}/{total} recorded lines; {len(missing)} still unrecorded.")
    if missing:
        print("Missing masters (first 8):\n" + "\n".join(missing[:8]))
    return 2 if args.strict and missing else 0

if __name__ == "__main__":
    raise SystemExit(main())
