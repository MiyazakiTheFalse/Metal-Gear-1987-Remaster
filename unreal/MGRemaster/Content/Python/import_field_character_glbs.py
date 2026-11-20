"""Import original 18-joint prototype GLBs into Unreal Editor, without claiming final art.

Run inside Unreal 5.7 Editor Python after enabling the glTF/Interchange importer.
The import can fail if your engine plugin does not support skinned GLB; failure
is deliberate, never silently downgrade to a static mesh. No project files or
production-ready labels are made by running this script.
"""
from __future__ import annotations
import json
import struct
from pathlib import Path

import unreal

ROOT = Path(__file__).resolve().parents[4]
SOURCES = ROOT / "browser" / "assets" / "models"
ROLES = {
    "snake": ("/Game/Characters/Snake", "SK_Snake_1995_Infiltration"),
    "fox": ("/Game/Characters/GrayFox", "SK_GrayFox_1995_Prisoner"),
    "guard": ("/Game/Characters/Guard", "SK_Guard_1995"),
    "shoot_gunner": ("/Game/Characters/Shotmaker", "SK_Shotmaker_1995"),
}
REQUIRED_JOINTS = {
    "pelvis", "chest", "neck", "head", "jaw", "brow",
    "hand_L", "hand_R", "foot_L", "foot_R",
}
REQUIRED_ANIMS = {
    "Idle", "Walk", "Run", "Crouch", "Prone", "Down", "Aim",
    "Cover", "Melee", "Reaching", "Hit", "RadioListen",
    "CrouchAim", "ProneAim",
}


def validate_glb(source: Path) -> dict:
    raw = source.read_bytes()
    if len(raw) < 28 or raw[:4] != b"glTF":
        raise ValueError(f"Not a GLB: {source}")
    version, declared, chunk_length = struct.unpack_from("<III", raw, 4)
    if version != 2 or declared != len(raw) or raw[16:20] != b"JSON":
        raise ValueError(f"Malformed GLB header: {source}")
    document = json.loads(raw[20:20 + chunk_length])
    if len(document.get("skins", [])) != 1:
        raise ValueError(f"Expected one skinned body: {source}")
    skin = document["skins"][0]
    if len(skin["joints"]) != 18:
        raise ValueError(f"Expected 18-joint skeleton: {source}")
    joints = {document["nodes"][i]["name"] for i in skin["joints"]}
    if not REQUIRED_JOINTS.issubset(joints):
        raise ValueError(f"Missing joints: {sorted(REQUIRED_JOINTS - joints)}")
    clips = {a["name"] for a in document.get("animations", [])}
    if not REQUIRED_ANIMS.issubset(clips):
        raise ValueError(f"Missing clips: {sorted(REQUIRED_ANIMS - clips)}")
    if not document.get("meshes", [{}])[0].get("primitives"):
        raise ValueError(f"Missing renderable mesh: {source}")
    return document


def import_prototype(role: str):
    destination, desired_name = ROLES[role]
    source = SOURCES / f"{role}.glb"
    if not source.exists():
        raise FileNotFoundError(source)
    meta = validate_glb(source)
    target = f"{destination}/{desired_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(target):
        current = unreal.EditorAssetLibrary.load_asset(target)
        if not isinstance(current, unreal.SkeletalMesh):
            raise TypeError(f"Existing target is NOT skeletal: {target}")
        unreal.log_warning(f"Not overwriting artist-authored asset: {target}")
        return target

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", desired_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", False)
    task.set_editor_property("save", True)
    # Interchange may emit a different asset name. Inspect task results,
    # rather than assuming a GLB is imported as a SkeletalMesh.
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = list(task.get_editor_property("imported_object_paths") or [])
    skeletal = [p for p in imported
                if isinstance(unreal.EditorAssetLibrary.load_asset(p), unreal.SkeletalMesh)]
    if len(skeletal) != 1:
        raise RuntimeError(
            f"{role}: expected exactly one SkeletalMesh, found {skeletal}. "
            f"Imported paths: {imported}. Check glTF/Interchange skeletal importer."
        )
    mesh = unreal.EditorAssetLibrary.load_asset(skeletal[0])
    skeleton = mesh.get_editor_property("skeleton")
    if skeleton is None:
        raise RuntimeError(f"{role}: no real Skeleton after import")
    unreal.log(
        f"{role}: skeletal source imported at {skeletal[0]}, "
        f"{len(meta['animations'])} authored source animation clips. "
        "Retarget orientation, verify all AnimationSequences, AnimBP, PBR and "
        "720p appearance separately. NOT production art."
    )
    # Do not rename importer output: callers must validate the exact loaded
    # path. No blueprints are auto-assigned from a mismatching generated name.
    return skeletal[0]


def main():
    results = {}
    for role in ROLES:
        results[role] = import_prototype(role)
    unreal.log("PROTO RIG IMPORT RESULT (not engine playtested): " + json.dumps(results))


if __name__ == "__main__":
    main()
