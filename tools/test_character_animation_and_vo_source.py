"""Source regression coverage for hair, pose transitions and character VO.

These checks do not compile Unreal, audition recordings, import .uassets,
measure production loudness or verify animation visual quality.
"""
import ast
import json
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "unreal/MGRemaster/Source/MGRemaster"
ANIM_H = (SOURCE / "Public/MGCharacterAnimInstance.h").read_text(encoding="utf-8")
ANIM = (SOURCE / "Private/MGCharacterAnimInstance.cpp").read_text(encoding="utf-8")
SNAKE_H = (SOURCE / "Public/MGPlayerCharacter.h").read_text(encoding="utf-8")
SNAKE = (SOURCE / "Private/MGPlayerCharacter.cpp").read_text(encoding="utf-8")
MISSION = (SOURCE / "Private/MGGrayFoxMission.cpp").read_text(encoding="utf-8")
GUARD = (SOURCE / "Private/MGGuardActor.cpp").read_text(encoding="utf-8")
BOSS_H = (SOURCE / "Public/MGShootGunnerActor.h").read_text(encoding="utf-8")
VOICE = json.loads((ROOT / "data/voice_performance_manifest.json").read_text())
INVENTORY = json.loads((ROOT / "data/vertical_slice_asset_inventory.json").read_text())

class CharacterHairSourceTests(unittest.TestCase):
    def test_geometry_is_real_grouped_obj_not_an_unattached_helmet_proxy(self):
        path = ROOT / "assets/source/snake_hair/snake_1995_hair.obj"
        lines = path.read_text(encoding="utf-8").splitlines()
        vertices = [line for line in lines if line.startswith("v ")]
        uvs = [line for line in lines if line.startswith("vt ")]
        faces = [line for line in lines if line.startswith("f ")]
        groups = [line for line in lines if line.startswith("g ")]
        self.assertGreaterEqual(len(vertices), 400)
        self.assertEqual(len(vertices), len(uvs))
        self.assertGreaterEqual(len(faces), 600)
        self.assertGreaterEqual(len(groups), 30)
        self.assertLess(lines.index(vertices[0]), lines.index(groups[0]))
        for f in faces:
            for part in f.split()[1:]:
                v, uv = map(int, part.split("/"))
                self.assertGreaterEqual(v, 1)
                self.assertLessEqual(v, len(vertices))
                self.assertGreaterEqual(uv, 1)
                self.assertLessEqual(uv, len(uvs))
        self.assertIn("mtllib snake_1995_hair.mtl", lines)

    def test_hair_attaches_to_actual_head_and_survives_costume_changes(self):
        for required in (
            'HairVisual->SetupAttachment(GetMesh(), TEXT("head"))',
            'GetMesh()->DoesSocketExist(HairAttachmentBone)',
            'HairVisual->AttachToComponent(GetMesh()',
            'SetCollisionEnabled(ECollisionEnabled::NoCollision)',
            'UpdateHairPresentation();',
            'SM_SnakeHair_1995'
        ):
            self.assertIn(required, SNAKE)
        self.assertIn("HairAttachmentBone", SNAKE_H)
        self.assertIn("UpdateHairPresentation();", SNAKE.split("bDisarmedArtApplied = bCaptured;")[1])

    def test_importer_is_non_destructive_and_not_proof_of_import(self):
        py = (ROOT / "unreal/MGRemaster/Content/Python/import_snake_hair.py").read_text()
        ast.parse(py)
        self.assertIn("replace_existing", py)
        self.assertIn("if unreal.EditorAssetLibrary.does_asset_exist(target)", py)
        self.assertIn("unreal.StaticMesh", py)
        record = next(x for x in INVENTORY["entries"] if x["id"] == "snake_1995_hair_source")
        self.assertEqual(record["status"], "prototype_source")
        self.assertFalse(record["imported_into_ue"])
        self.assertFalse(record["production_approved"])

class AnimationTransitionSourceTests(unittest.TestCase):
    def test_every_role_gets_action_arbitration(self):
        for name in (
            "Melee","WallKnock","CoverPeek","RadioIdle","GuardNotice",
            "GuardInvestigate","GuardSearch","GuardChallenge","GuardReport",
            "GuardStrike","Incapacitated","FoxPrisoner","FoxRescued",
            "GunnerAim","GunnerVolley","GunnerRecovery","GunnerDefeated"
        ):
            self.assertIn(name, ANIM_H)
            self.assertIn("EMGPerformanceAction::" + name, ANIM)
        self.assertIn("AMGShootGunnerActor", ANIM)
        self.assertIn("IsRecovering() const", BOSS_H)
        self.assertIn("OnPerformanceActionTransition", ANIM)
        self.assertIn("CrouchTransitionAlpha", ANIM_H)
        self.assertIn("ProneTransitionAlpha", ANIM_H)
        self.assertIn("CoverTransitionAlpha", ANIM_H)
        self.assertIn("CrouchTransitionAlpha = FMath::FInterpTo", ANIM)
        self.assertIn("CoverTransitionAlpha = FMath::FInterpTo", ANIM)

    def test_interruption_and_additive_radio_are_presentation_only(self):
        self.assertIn("bPresentationInterrupted", ANIM)
        self.assertIn("PerformanceTransitionAlpha", ANIM)
        self.assertIn("PerformanceActionElapsedSeconds / BlendSeconds", ANIM)
        self.assertIn("bRadioListeningOverlay = Player->IsCodecOpen()", ANIM)
        self.assertIn("RadioListenOverlayAlpha = FMath::FInterpTo", ANIM)
        self.assertNotIn("ApplyDamage(", ANIM)
        self.assertNotIn("ApplyAction(", ANIM)
        self.assertNotIn("SaveProgress(", ANIM)

class CharacterVoiceSourceTests(unittest.TestCase):
    def test_scene_and_speaker_names_form_safe_stable_clip_ids(self):
        speaker_set = set(VOICE["profiles"])
        self.assertTrue({"snake","big_boss","schneider","gray_fox","shotmaker","guard"}.issubset(speaker_set))
        seen = set()
        count_rows = count_clips = 0
        for scene in VOICE["scenes"]:
            self.assertRegex(scene["scene_id"], r"^[a-z0-9_]+$")
            prior = {}
            for index, line in enumerate(scene["lines"], start=1):
                count_rows += 1
                self.assertIn(line["speaker"], speaker_set)
                self.assertTrue(line["text"])
                if line["clip"] is None:
                    self.assertIn(line["continuation_of"], prior)
                    self.assertEqual(line["speaker"], prior[line["continuation_of"]])
                else:
                    clip = line["clip"]
                    self.assertRegex(clip, r"^VO_[a-z0-9_]+_[0-9]{2}$")
                    self.assertTrue(clip.endswith(f"_{index:02d}"))
                    key = (line["speaker"], clip)
                    self.assertNotIn(key, seen)
                    seen.add(key)
                    prior[clip] = line["speaker"]
                    count_clips += 1
        self.assertEqual(count_rows, 49)
        self.assertEqual(count_clips, 45)
        self.assertEqual(VOICE["subtitle_line_count"], count_rows)
        self.assertEqual(VOICE["audio_clip_count"], count_clips)
        self.assertTrue(all(not x["recorded"] for s in VOICE["scenes"] for x in s["lines"]))

    def test_radio_cancel_and_audio_duration_control_caption_reveal(self):
        for token in (
            "GetTransceiverVoiceSceneKey", "StopRadioDialogue();",
            "UpdateRadioDialogue(DeltaSeconds)", "Cue->GetDuration()",
            "VoiceLineAdvanceDelay = PlayRadioDialogueLine",
            "bStagedOpeningCall", "NextVoiceLine += Consumed",
            "RevealedRadioLines", "NextSpeaker != Speaker"
        ):
            self.assertIn(token, SNAKE)
        self.assertIn('VO_%s_%02d', SNAKE)
        self.assertIn('OnNarrativeVoiceStarted(FName(TEXT("gray_fox"))', MISSION)
        self.assertIn('OnNarrativeVoiceStarted(FName(TEXT("shotmaker"))', MISSION)
        self.assertIn('VO_guard_%s_01', GUARD)
        self.assertIn("StartCameraFade(1.f, 0.f, .82f", MISSION)
        self.assertIn('GetActiveGuardCallout()', (SOURCE / "Private/MGPrototypeHUD.cpp").read_text())

    def test_every_radio_context_exposed_in_mission_and_processors_parse(self):
        for scene in VOICE["scenes"]:
            if scene["context"] == "1995-radio":
                self.assertIn('TEXT("' + scene["scene_id"] + '")', MISSION)
        for path in (
            ROOT / "tools/prepare_voice_assets.py",
            ROOT / "unreal/MGRemaster/Content/Python/import_character_vo.py"
        ):
            source = path.read_text()
            ast.parse(source)
            self.assertIn("voice_performance_manifest.json", source)
        self.assertIn("measured_I", (ROOT / "tools/prepare_voice_assets.py").read_text())
        self.assertIn("loudnorm", (ROOT / "tools/prepare_voice_assets.py").read_text())
        self.assertIn('Path="/Game/Audio/VO"', (ROOT / "unreal/MGRemaster/Config/DefaultGame.ini").read_text())

    def test_no_fake_voice_complete_status(self):
        self.assertEqual(VOICE["status"], "all_voice_recordings_pending")
        for name in ("radio_vo", "character_vo", "voice_performance_metadata"):
            entry = next(x for x in INVENTORY["entries"] if x["id"] == name)
            self.assertFalse(entry["imported_into_ue"])
            self.assertFalse(entry["production_approved"])
        self.assertEqual(INVENTORY["count"], len(INVENTORY["entries"]))

if __name__ == "__main__":
    unittest.main()
