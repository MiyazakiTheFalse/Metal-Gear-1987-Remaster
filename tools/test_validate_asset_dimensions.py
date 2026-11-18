"""Tests for original authored mesh dimensions and provenance."""
import json
from pathlib import Path
import tempfile
import unittest

from validate_asset_dimensions import read_vertices, extents, validate

ROOT = Path(__file__).resolve().parents[1]


class AssetChecks(unittest.TestCase):
    def test_actual_modules_fit_manifest(self):
        self.assertEqual(validate(ROOT), [])

    def test_door_has_true_clear_opening(self):
        obj = ROOT / "assets" / "greybox" / "door_170.obj"
        v = read_vertices(obj)
        self.assertEqual(len(v), 24)
        self.assertAlmostEqual(min(p[0] for p in v[8:16]) - max(p[0] for p in v[:8]), 130)
        self.assertAlmostEqual(min(p[2] for p in v[16:24]), 240)
        self.assertEqual(extents(v), (170,30,270))

    def test_original_metres_are_not_invented(self):
        data = json.loads((ROOT / "assets" / "asset_dimensions.json").read_text())
        self.assertIsNone(data["original_map"]["physical_tile_size_m"])

    def test_strict_rejects_unplaytested_assets(self):
        self.assertTrue(any("playtest" in message for message in validate(ROOT, strict=True)))


if __name__=="__main__":
    unittest.main()
