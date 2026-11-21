"""Offline regression of the UE5.7 native-asset validation code.

Fakes asset registry and native skeletons; not an Unreal engine runtime test.
Tests must fail closed on missing, wrongly scaled and incompatible clips.
"""
from __future__ import annotations

import importlib.util
import runpy
import sys
import types
import unittest
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "unreal/MGRemaster/Content/Python/audit_native_character_assets.py"
ROLE = "snake"
DEST = "/Game/Characters/Snake"
CLIPS = {"Idle", "Walk", "Run", "Aim", "Down"}


class Skeleton:
    def __init__(self, path="/Game/Characters/Snake/SK_Snake_Skeleton"):
        self.path = path

    def get_path_name(self):
        return self.path


class Skeletal:
    def __init__(self, skeleton, height=178.0):
        self.skeleton, self.height = skeleton, height

    def get_editor_property(self, prop):
        if prop != "skeleton":
            raise KeyError(prop)
        return self.skeleton

    def get_bounds(self):
        return types.SimpleNamespace(
            box_extent=types.SimpleNamespace(x=25.0, y=15.0, z=self.height / 2.0)
        )


class Sequence:
    def __init__(self, skeleton, frames=49, length=2.4):
        self.skeleton, self.frames, self.length = skeleton, frames, length

    def get_skeleton(self):
        return self.skeleton

    def get_editor_property(self, prop):
        return {
            "number_of_sampled_frames": self.frames,
            "sequence_length": self.length,
        }[prop]


class Assets:
    def __init__(self, objects):
        self.objects = objects

    def load_asset(self, name):
        return self.objects.get(name)


class NativeAuditTests(unittest.TestCase):
    def setUp(self):
        self.log = []
        self.sk = Skeleton()
        self.mesh_path = f"{DEST}/SK_Snake_1995_Infiltration"
        self.assets = {self.mesh_path: Skeletal(self.sk)}
        for clip in CLIPS:
            self.assets[f"{DEST}/Animations/AS_Snake_{clip}"] = Sequence(self.sk)

    def execute_audit(self):
        fake = types.ModuleType("unreal")
        fake.SkeletalMesh = Skeletal
        fake.AnimSequence = Sequence
        fake.EditorAssetLibrary = Assets(self.assets)
        fake.log = lambda message: self.log.append(str(message))
        fake.log_error = lambda message: self.log.append(str(message))
        metadata = {
            "ROLES": {ROLE: (DEST, "SK_Snake_1995_Infiltration")},
            "ROLE_PREFIXES": {ROLE: "Snake"},
            "required_clips": lambda role: CLIPS.copy(),
        }
        spec = importlib.util.spec_from_file_location("_mg_native_audit_test", SCRIPT)
        mod = importlib.util.module_from_spec(spec)
        with patch.dict(sys.modules, {"unreal": fake}), patch.object(
            runpy, "run_path", return_value=metadata
        ):
            spec.loader.exec_module(mod)
        return mod.audit()

    def test_valid_native_skeleton_and_samples_accept(self):
        result = self.execute_audit()
        self.assertEqual(result["roles_validated"], 1)
        self.assertEqual(result["native_clips_found"], len(CLIPS))
        self.assertTrue(any("MG_NATIVE_CHARACTER_AUDIT_PASS" in x for x in self.log))

    def test_missing_skeletal_mesh_blocks_success_marker(self):
        del self.assets[self.mesh_path]
        with self.assertRaisesRegex(RuntimeError, "AUDIT_FAIL"):
            self.execute_audit()
        self.assertFalse(any("AUDIT_PASS" in x for x in self.log))

    def test_metres_vs_centimetres_error_is_detected(self):
        self.assets[self.mesh_path] = Skeletal(self.sk, height=1.78)
        with self.assertRaisesRegex(RuntimeError, "AUDIT_FAIL"):
            self.execute_audit()
        self.assertTrue(any("unit conversion" in x for x in self.log))

    def test_wrong_animation_skeleton_is_rejected(self):
        self.assets[f"{DEST}/Animations/AS_Snake_Run"] = Sequence(
            Skeleton("/Game/Characters/Guard/GuardSkeleton")
        )
        with self.assertRaisesRegex(RuntimeError, "AUDIT_FAIL"):
            self.execute_audit()
        self.assertTrue(any("WRONG skeleton" in x for x in self.log))

    def test_empty_native_animation_does_not_pass(self):
        self.assets[f"{DEST}/Animations/AS_Snake_Walk"] = Sequence(
            self.sk, frames=1, length=0.0
        )
        with self.assertRaisesRegex(RuntimeError, "AUDIT_FAIL"):
            self.execute_audit()
        self.assertTrue(any("empty or malformed" in x for x in self.log))


if __name__ == "__main__":
    unittest.main()
