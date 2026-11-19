"""Enforce source/honesty gates for cast bibles and TX-55. stdlib only."""
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXPECTED_SNAKE = ROOT / "docs/SOLID_SNAKE_1995_PROTAGONIST_BIBLE.md"
EXPECTED_TX = ROOT / "docs/TX55_METAL_GEAR_BOSS_BIBLE.md"
SEQUENCE = "R R L R L L R L L R R L R L R R".split()
CORE_CAST = {
    "REAL_BIG_BOSS", "VENOM_SNAKE", "GRAY_FOX", "KYLE_SCHNEIDER",
    "DIANE", "STEVE", "JENNIFER", "DRAGO_MADNAR", "ELLEN_MADNAR",
    "SHOOT_GUNNER", "MACHINEGUN_KID", "FIRE_TROOPER", "BLOODY_BRAD",
    "DIRTY_DUCK", "OUTER_HEAVEN_GUARD", "TX55",
}

def validate(data: dict, tx: dict, root: Path = ROOT) -> list[str]:
    errors = []
    entries = data.get("entries", [])
    if data.get("schema_version") != 1 or not isinstance(entries, list):
        return ["invalid cast registry schema"]
    ids = [e.get("id") for e in entries]
    if len(set(ids)) != len(ids):
        errors.append("duplicate cast id")
    for missing in CORE_CAST - set(ids):
        errors.append("missing significant character: " + missing)
    if data.get("count") != len(entries):
        errors.append("count mismatch")
    for entry in entries:
        path = entry.get("path", "")
        if not path.startswith("docs/CHARACTER_CAST_BIBLES/") or ".." in path:
            errors.append("invalid cast path: " + str(path))
            continue
        doc = root / path
        if not doc.is_file():
            errors.append("missing cast bible: " + str(path))
            continue
        text = doc.read_text(encoding="utf-8")
        for anchor in ("## Who this character is", "## Art and assets",
                       "## Acting, movement", "## Gameplay authority",
                       "## Actual review"):
            if anchor not in text:
                errors.append(path + " missing " + anchor)
        if entry.get("status") != "source_authored_unverified":
            errors.append(path + " falsely verified without evidence")
    if tx.get("fixed_order") != SEQUENCE:
        errors.append("TX-55 original 16-leg order mismatch")
    if tx.get("known_to_player") != 15 or not tx.get("player_must_infer_last"):
        errors.append("TX-55 missing-final-direction puzzle erased")
    if tx.get("wrong_leg_consumes_charge") or tx.get("stale_notify_commits"):
        errors.append("TX-55 unsafe charge transaction")
    if tx.get("verification_status") != "source_authoring_only":
        errors.append("TX-55 cannot be marked tested by source-only validator")
    if not (root / "docs/TX55_METAL_GEAR_BOSS_BIBLE.md").is_file():
        errors.append("TX-55 boss bible absent")
    if not (root / "docs/SOLID_SNAKE_1995_PROTAGONIST_BIBLE.md").is_file():
        errors.append("Snake protagonist bible absent")
    return errors

if __name__ == "__main__":
    registry = json.loads((ROOT / "data/character_bible_registry.json").read_text())
    contract = json.loads((ROOT / "data/tx55_boss_contract.json").read_text())
    failures = validate(registry, contract)
    if failures:
        raise SystemExit("\n".join(failures))
    print("Cast/source and TX-55 design contracts passed (NOT an Unreal validation).")
