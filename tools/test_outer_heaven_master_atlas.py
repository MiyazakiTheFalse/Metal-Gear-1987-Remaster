"""Whole Outer Heaven blueprint integrity tests. Source-model only.

Static architectural design is not canonical MSX2 tile verification, a playable
UE level or a physical visibility/collision test.
"""
import json
import re
import unittest
from collections import defaultdict, deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ATLAS_PATH = ROOT / "browser/atlas/outer_heaven_1995.json"
LEDGER_PATH = ROOT / "data/FULL_CAMPAIGN_BEAT_BY_BEAT_IMPLEMENTATION_LEDGER.json"
DOC_PATH = ROOT / "docs/OUTER_HEAVEN_MASTER_ARCHITECTURE_1995.md"
MAP_HTML = ROOT / "browser/atlas.html"
MAP_JS = ROOT / "browser/atlas/map.js"


class AtlasTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.atlas = json.loads(ATLAS_PATH.read_text(encoding="utf8"))
        cls.ledger = json.loads(LEDGER_PATH.read_text(encoding="utf8"))

    def test_every_ledger_zone_and_every_actual_beat_is_exposed(self):
        a = self.atlas
        ledger = self.ledger
        area_ids = {zone["id"] for zone in a["areas"]}
        self.assertEqual(len(area_ids), 52)
        self.assertEqual(area_ids, {zone["id"] for zone in ledger["areas"]})
        original_beats = {
            beat["id"]: (beat["ordinal"], beat["name"], beat["area"])
            for mission in ledger["missions"] for beat in mission["beats"]
        }
        mapped_beats = {
            beat["id"]: (beat["ordinal"], beat["name"], zone["id"])
            for zone in a["areas"] for beat in zone["beats"]
        }
        self.assertEqual(len(original_beats), 114)
        self.assertEqual(mapped_beats, original_beats)
        self.assertEqual(
            {m["id"] for m in a["missions"]},
            {m["id"] for m in ledger["missions"]},
        )

    def test_11_boss_arena_placement_and_floor_layers_are_explicit(self):
        a = self.atlas
        arenas = [x for x in a["areas"] if x["boss"]]
        self.assertEqual(len(arenas), 11)
        floor_ids = {x["id"] for x in a["levels"]}
        self.assertTrue({"SITE", "B1.1F", "B1.2F", "B1.3F", "B1.ROOF",
                         "B1.B1", "B2.1F", "B2.2F", "B2.B1",
                         "B3.1F", "B3.B100", "TUNNEL"}.issubset(floor_ids))
        self.assertIn("Hind D", {x["boss"] for x in arenas})
        self.assertIn("TX-55 Metal Gear", {x["boss"] for x in arenas})
        self.assertIn("Big Boss", {x["boss"] for x in arenas})

    def test_all_rooms_fit_site_and_building_footprints(self):
        a = self.atlas
        width, height = a["site"]["size_m"]
        bboxes = {
            f["id"]: f["rect_m"]
            for f in a["features"] if f["kind"] == "building"
        }
        for z in a["areas"]:
            x, y = z["center_m"]
            self.assertTrue(0 <= x <= width and 0 <= y <= height, z["id"])
            self.assertTrue(all(k > 0 for k in z["footprint_m"]), z["id"])
            prefix = z["level"].split(".")[0].lower()
            if prefix not in bboxes:
                continue
            bx, by, bw, bh = bboxes[prefix]
            self.assertTrue(bx <= x <= bx + bw and by <= y <= by + bh, z["id"])
        self.assertEqual(
            {f["id"] for f in a["features"] if f["kind"] == "building"},
            {"b1", "b2", "b3"},
        )

    def test_all_connections_resolve_and_world_is_one_graph(self):
        a = self.atlas
        zones = {z["id"] for z in a["areas"]}
        edges = defaultdict(set)
        keys = set()
        for link in a["links"]:
            start, finish = link["from"], link["to"]
            self.assertIn(start, zones)
            self.assertIn(finish, zones)
            self.assertNotEqual(start, finish)
            pair = tuple(sorted((start, finish)))
            self.assertNotIn(pair, keys, "duplicate connector: " + str(pair))
            keys.add(pair)
            edges[start].add(finish)
            edges[finish].add(start)
            self.assertTrue(link.get("gate"))
            self.assertTrue(link.get("status"))
        visited = set()
        queue = deque([next(iter(zones))])
        while queue:
            z = queue.popleft()
            if z in visited:
                continue
            visited.add(z)
            queue.extend(edges[z] - visited)
        self.assertEqual(visited, zones, "unreachable zones = " + str(zones - visited))

    def test_source_truth_is_not_mistaken_for_surveyed_geometry(self):
        a = self.atlas
        self.assertRegex(a["warning"], r"DESIGN PROPOSALS")
        self.assertRegex(a["authority"], r"1987 MSX2")
        self.assertTrue(all("hypothesis" in z["sourceStatus"] for z in a["areas"]))
        self.assertTrue(all(f["sourceStatus"] == "remake_proposed_not_canon"
                            for f in a["features"]))
        self.assertIn("NES", " ".join(
            p.read_text(encoding="utf8") for p in [DOC_PATH]
        ))
        self.assertIn("Duck", DOC_PATH.read_text(encoding="utf8"))
        self.assertIn("conflict", DOC_PATH.read_text(encoding="utf8").lower())

    def test_viewer_loads_local_json_and_has_direct_navigation(self):
        html = MAP_HTML.read_text(encoding="utf8")
        script = MAP_JS.read_text(encoding="utf8")
        for label in ('id="floor"', 'id="mission"', 'id="directory"',
                      'id="map"', 'id="detail"', 'id="ghost"'):
            self.assertIn(label, html)
        self.assertIn("atlas/outer_heaven_1995.json", script)
        self.assertIn("addEventListener('keydown'", script)
        self.assertIn("sourceStatus", script)
        self.assertIn("atlas.areas.length!==52", script)
        self.assertIn("!==114", script)
        self.assertNotIn("mapbox", html.lower())
        self.assertNotIn("google maps", html.lower())


if __name__ == "__main__":
    unittest.main()
