"""Verify all authored weapon/reload source contracts.

Static checks only: passing this suite does NOT certify Unreal compilation,
author-approved animation, imported sound, hands-on playtest or animation quality.
"""
import ast
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "unreal/MGRemaster/Source/MGRemaster"
GUARD = (SOURCE / "Private/MGGuardActor.cpp").read_text(encoding="utf-8")
GUARD_H = (SOURCE / "Public/MGGuardActor.h").read_text(encoding="utf-8")
GUNNER = (SOURCE / "Private/MGShootGunnerActor.cpp").read_text(encoding="utf-8")
GUNNER_H = (SOURCE / "Public/MGShootGunnerActor.h").read_text(encoding="utf-8")
SNAKE = (SOURCE / "Private/MGPlayerCharacter.cpp").read_text(encoding="utf-8")
SNAKE_H = (SOURCE / "Public/MGPlayerCharacter.h").read_text(encoding="utf-8")
CASING = (SOURCE / "Private/MGSpentCasingActor.cpp").read_text(encoding="utf-8")
ANIM = (SOURCE / "Private/MGCharacterAnimInstance.cpp").read_text(encoding="utf-8")
ANIM_H = (SOURCE / "Public/MGCharacterAnimInstance.h").read_text(encoding="utf-8")
CONTRACT = json.loads((ROOT / "data/weapon_reload_and_hand_quality_contract.json").read_text())
IMPORTER = (ROOT / "unreal/MGRemaster/Content/Python/import_1995_weapon_pack.py").read_text()
ART = ROOT / "assets/source/weapons"

class RealWeaponSourceTests(unittest.TestCase):
    def test_header_and_source_paths_not_inverted(self):
        for header, source in (
            (GUARD_H, GUARD), (GUNNER_H, GUNNER),
            (SNAKE_H, SNAKE), (ANIM_H, ANIM),
            ((SOURCE / "Public/MGSpentCasingActor.h").read_text(), CASING)
        ):
            self.assertTrue(header.lstrip().startswith("#pragma once"))
            self.assertTrue(source.lstrip().startswith("#include"))
        self.assertIn("MGSpentCasingActor.generated.h", (SOURCE / "Public/MGSpentCasingActor.h").read_text())

    def test_shoot_gunner_consumes_and_reload_shells_one_by_one(self):
        for code in (
            "EAttackState::Reloading", "LoadedShells <= 0",
            "LoadedShells = FMath::Min(LoadedShells + 1",
            "OnReloadStarted();", "OnReloadShellInserted(LoadedShells);",
            "OnReloadFinished();", "--LoadedShells",
            "FMath::Clamp(TubeCapacity, 3, 8)",
            "PumpVisualRemainingSeconds", "OnSpentShellEjected(PumpedShot)",
            "EMGCasingType::RiotGunHull"
        ):
            self.assertIn(code, GUNNER)
        for code in ("IsReloading()", "GetLoadedShells()",
                     "OnReloadShellInserted(int32 LoadedCount)",
                     "EAttackState::Reloading"):
            self.assertIn(code, GUNNER_H)
        self.assertIn("ShotsFired >= 3", GUNNER)
        self.assertNotIn("OnPumpAction(CompletedShotIndex)", GUNNER)
        self.assertIn("SetEncounterActive(bool bEnabled)", GUNNER)

    def test_guards_have_visual_confirmation_cooldown_real_reload(self):
        for code in (
            "bConfirmedVisualNow", "bLegitimateRangedTarget",
            "VisualConfirmationSeconds >= .35f",
            "Distance > 180.f && Distance < 800.f",
            "FacingDot >= .90f", "SidearmRounds > 0",
            "--SidearmRounds", "SidearmShotCooldown = .72f",
            "MGGuardActualSidearmShot", "ShotHit.GetActor() == Player",
            "bSidearmReloading = true", "SidearmReloadSeconds -=",
            "SidearmRounds = FMath::Clamp(SidearmMagazineCapacity",
            "--SpareSidearmMagazines", "OnSidearmReloadInterrupted();",
            "StaggerSeconds > 0.f", "bCustodyChallenge",
            "EMGGuardPosture::SidearmReload",
        ):
            self.assertIn(code, GUARD)
        self.assertIn("if (!bSidearmReloading && !bCustodyChallenge", GUARD)
        self.assertIn("OnSidearmFired", GUARD_H)
        self.assertIn("IsReloadingSidearm()", GUARD_H)
        self.assertIn("SidearmMuzzleLight->SetVisibility", GUARD)
        self.assertIn("EMGCasingType::HandgunBrass", GUARD)

    def test_snake_dart_does_not_eject_ballistic_brass(self):
        self.assertIn("if (bHandgun)\n        PendingHandgunCasingSeconds = .065f", SNAKE)
        self.assertIn("EMGCasingType::HandgunBrass", SNAKE)
        self.assertIn("OnHandgunCasingEjected();", SNAKE)
        self.assertIn("bStun ? Equipment->ConsumeStunRound()", SNAKE)
        self.assertNotIn("if (bStun)\n        PendingHandgunCasingSeconds", SNAKE)
        self.assertEqual(CONTRACT["actors"][3]["id"], "debug_stun_dart_pistol")

    def test_casing_physics_do_not_block_gameplay_and_have_lifespan(self):
        for code in (
            "SetSimulatePhysics(true)", "AddImpulse(", "SetMassOverrideInKg",
            "SetLifeSpan(7.f)", "ECC_Pawn, ECR_Ignore", "ECC_Visibility, ECR_Ignore",
            "if (!Art) return false", "Actor->Destroy()"
        ):
            self.assertIn(code, CASING)
        self.assertIn("SM_HandgunCasing_1995", CASING)
        self.assertIn("SM_RiotHull_1995", CASING)

    def test_animation_reloading_and_hand_socket_gate(self):
        for a in ("GuardSidearmAim", "GuardSidearmFire", "GuardSidearmReload",
                  "GunnerReload", "bHasValidRightWeaponSocket", "bHasValidSupportHandSocket",
                  "SupportHandTargetWS", "WeaponReloadBlendAlpha"):
            self.assertIn(a, ANIM_H)
        for code in ("Guard->IsReloadingSidearm()", "Gunner->IsReloading()",
                     "ActualRig->DoesSocketExist(TEXT(\"weapon_r\"))",
                     "ActualWeapon->DoesSocketExist(TEXT(\"support_hand\"))",
                     "TargetLeftGrip = bHasValidSupportHandSocket ? TargetLeftGrip : 0.f"):
            self.assertIn(code, ANIM)

    def test_importer_lists_each_source_and_is_no_overwrite(self):
        ast.parse(IMPORTER)
        self.assertIn('set_editor_property("replace_existing", False)', IMPORTER)
        self.assertIn("does_asset_exist(target)", IMPORTER)
        for filename in ("shotmaker_main_1995_lod0.obj",
                         "shotmaker_pump_1995_lod0.obj", "snake_handgun_1995_lod0.obj",
                         "snake_smg_1995_lod0.obj", "grenade_launcher_1995_lod0.obj",
                         "rocket_launcher_1995_lod0.obj", "guided_missile_1995_lod0.obj",
                         "mine_1995_lod0.obj", "plastic_explosive_1995_lod0.obj",
                         "stun_test_1995_lod0.obj", "handgun_spent_casing_1995.obj",
                         "riotgun_spent_hull_1995.obj"):
            self.assertIn(filename, IMPORTER)
            mesh = (ART / filename).read_text(encoding="utf-8")
            verts = [x for x in mesh.splitlines() if x.startswith("v ")]
            faces = [x for x in mesh.splitlines() if x.startswith("f ")]
            self.assertGreater(len(verts), 100, filename)
            self.assertGreater(len(faces), 100, filename)
            for face in faces:
                for v in face.split()[1:]:
                    index = int(v.split("/")[0])
                    self.assertGreaterEqual(index, 1)
                    self.assertLessEqual(index, len(verts))

    def test_canonical_actors_have_individual_actions(self):
        roster = {x["id"]: x for x in CONTRACT["actors"]}
        self.assertEqual(CONTRACT["historical_anchor"], "1987_MSX2")
        self.assertEqual(roster["shoot_gunner_1987"]["source_canon"], "3-shot spread volley")
        self.assertEqual(roster["machinegun_kid"]["implementation"], "design_only")
        self.assertIn("fuel", roster["fire_trooper_1987"]["mechanism"])
        self.assertIn("boomerang", roster["coward_duck_1987"]["mechanism"])
        self.assertIn("Hind D original MSX2", roster["hind_d_1987"]["source_canon"])
        self.assertEqual(CONTRACT["verification"]["ue_5_7_compile_status"], "not_executed")

if __name__ == "__main__":
    unittest.main()
