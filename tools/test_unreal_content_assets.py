"""Source-model delivery gates for the Unreal tester.

These verify the actual bytes checked into Unreal Content, not their filenames.
Passing does not assert native UE import / AnimBlueprint / realistic art quality.
"""
import unittest

from verify_unreal_source_assets import validate


class UnrealSourceAssetDeliveryTests(unittest.TestCase):
    def test_all_four_skeletal_glbs_are_physically_present_in_unreal_content(self):
        manifest = validate()
        self.assertEqual(manifest["status"], "self_contained_source_assets_verified")
        self.assertEqual(len(manifest["characters"]), 4)
        self.assertEqual(
            {row["role"] for row in manifest["characters"]},
            {"snake", "fox", "guard", "shoot_gunner"},
        )

    def test_each_binary_is_a_real_animated_skin_not_static_placeholder(self):
        manifest = validate()
        for entry in manifest["characters"]:
            self.assertEqual(entry["joints"], 18)
            self.assertGreaterEqual(entry["animations"], 17)
            self.assertGreaterEqual(entry["mesh_primitives"], 1)
            self.assertGreater(entry["bytes"], 100_000)
            self.assertEqual(len(entry["sha256"]), 64)

    def test_native_unreal_assets_must_not_be_falsely_reported_present(self):
        manifest = validate()
        self.assertIs(manifest["native_uassets_verified"], False)
        self.assertIs(manifest["editor_import_required"], True)


if __name__ == "__main__":
    unittest.main()
