"""Offline simulation of Unreal Editor re-import recovery policies.

Mocks only the minimal documented Unreal Python types used by the existing
native-asset verification branch. Does not claim successful actual import.
Requires the committed local source GLBs; intentionally no Engine/Editor.
"""
from __future__ import annotations

import importlib.util
import sys
import types
import unittest
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
IMPORTER = ROOT / "unreal/MGRemaster/Content/Python/import_field_character_glbs.py"


class Skeletal:
    def __init__(self, skeleton=True):
        self.skeleton = object() if skeleton else None

    def get_editor_property(self, key):
        if key != "skeleton":
            raise KeyError(key)
        return self.skeleton


class Animation:
    pass


class FakeAssetStore:
    def __init__(self):
        self.assets = {}
        self.lookup_count = 0

    def does_asset_exist(self, path):
        return path in self.assets

    def load_asset(self, path):
        self.lookup_count += 1
        return self.assets.get(path)


def load_importer(store):
    fake = types.ModuleType("unreal")
    fake.SkeletalMesh = Skeletal
    fake.AnimSequence = Animation
    fake.EditorAssetLibrary = store
    fake.log_warning = lambda message: None
    fake.log = lambda message: None
    spec = importlib.util.spec_from_file_location(
        "_mg_unreal_character_import_recovery_test", IMPORTER
    )
    module = importlib.util.module_from_spec(spec)
    with patch.dict(sys.modules, {"unreal": fake}):
        spec.loader.exec_module(module)
    return module


class UnrealImporterRecoveryTests(unittest.TestCase):
    def setUp(self):
        self.store = FakeAssetStore()
        self.mod = load_importer(self.store)
        self.role = "machinegun_kid"
        self.dest, self.mesh_name = self.mod.ROLES[self.role]
        self.target = f"{self.dest}/{self.mesh_name}"

    def test_manifest_includes_twenty_two_legal_source_prototypes(self):
        self.assertEqual(len(self.mod.ROLES), 22)
        self.assertIn("machinegun_kid", self.mod.ROLES)
        self.assertIn("fox", self.mod.ROLES)
        self.assertIn("shoot_gunner", self.mod.ROLES)

    def test_existing_skeletal_mesh_with_missing_clips_fails_loudly(self):
        self.store.assets[self.target] = Skeletal()
        with self.assertRaisesRegex(RuntimeError, "incomplete") as err:
            self.mod.import_prototype(self.role)
        self.assertIn("AS_MachinegunKid_Aim", str(err.exception))
        self.assertIn("Do NOT overwrite artist work", str(err.exception))
        self.assertGreater(self.store.lookup_count, 1)

    def test_existing_mesh_and_all_sequences_are_safe_idempotent_reentry(self):
        self.store.assets[self.target] = Skeletal()
        for clip in self.mod.required_clips(self.role):
            self.store.assets[
                f"{self.dest}/Animations/AS_MachinegunKid_{clip}"
            ] = Animation()
        report = self.mod.import_prototype(self.role)
        self.assertEqual(report["mesh"], self.target)
        self.assertEqual(report["source"], "existing")
        self.assertEqual(set(report["animations"]), self.mod.required_clips(self.role))

    def test_existing_mesh_must_have_skeleton(self):
        self.store.assets[self.target] = Skeletal(skeleton=False)
        with self.assertRaisesRegex(RuntimeError, "no Skeleton"):
            self.mod.import_prototype(self.role)

    def test_existing_static_mesh_cannot_pass_as_skeletal(self):
        self.store.assets[self.target] = object()
        with self.assertRaisesRegex(TypeError, "NOT skeletal"):
            self.mod.import_prototype(self.role)

    def test_source_glb_is_a_real_skin_with_all_named_performances(self):
        path = self.mod.SOURCES / f"{self.role}.glb"
        meta = self.mod.validate_glb(path)
        self.assertIn("skins", meta)
        clips = {item["name"] for item in meta["animations"]}
        self.assertTrue(self.mod.required_clips(self.role).issubset(clips))


if __name__ == "__main__":
    unittest.main()
