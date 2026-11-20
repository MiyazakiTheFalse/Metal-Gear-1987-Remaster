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

# Source assets are checked into the Unreal project itself, so a tester can
# archive/copy just unreal/MGRemaster without losing the source character rigs.
PROJECT = Path(__file__).resolve().parents[2]
SOURCES = PROJECT / "Content" / "SourceAssets" / "Characters"
ROLES = {
    "snake": ("/Game/Characters/Snake", "SK_Snake_1995_Infiltration"),
    "fox": ("/Game/Characters/GrayFox", "SK_GrayFox_1995_Prisoner"),
    "guard": ("/Game/Characters/Guard", "SK_Guard_1995"),
    "shoot_gunner": ("/Game/Characters/Bosses/Shotmaker", "SK_Shotmaker_1995"),
}
REQUIRED_JOINTS = {
    "pelvis", "chest", "neck", "head", "jaw", "brow",
    "hand_L", "hand_R", "foot_L", "foot_R",
}
REQUIRED_ANIMS = {
    "Idle", "IdleAlert", "IdleLook", "IdleWary", "Walk", "Run", "Crouch", "Prone", "Down", "Aim",
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
    if not imported:
        raise RuntimeError(
            f"{role}: importer produced no assets. Verify UE 5.7 Interchange "
            f"skeletal glTF plugin is enabled; source={source}"
        )
    skeletal = [p for p in imported
                if isinstance(unreal.EditorAssetLibrary.load_asset(p), unreal.SkeletalMesh)]
    if len(skeletal) != 1:
        raise RuntimeError(
            f"{role}: expected exactly one SkeletalMesh, found {skeletal}. "
            f"Imported paths: {imported}. Check glTF/Interchange skeletal importer."
        )
    if not unreal.EditorAssetLibrary.does_asset_exist(target):
        # UE5.7 Interchange ignores AssetImportTask.destination_name for glTF;
        # safely rename only our newly imported SkeletalMesh to the exact path
        # consumed by the C++ characters. Never replace existing artist assets.
        if not unreal.EditorAssetLibrary.rename_asset(skeletal[0], target):
            raise RuntimeError(
                f"{role}: imported {skeletal[0]} but could not rename to "
                f"the runtime path {target}. Resolve in Content Browser."
            )
    if not unreal.EditorAssetLibrary.does_asset_exist(target):
        raise RuntimeError(f"{role}: missing runtime target even after import: {target}")
    mesh = unreal.EditorAssetLibrary.load_asset(target)
    if not isinstance(mesh, unreal.SkeletalMesh):
        raise TypeError(f"Target {target} does not resolve to a skeletal mesh.")
    skeleton = mesh.get_editor_property("skeleton")
    if skeleton is None:
        raise RuntimeError(f"{role}: no real Skeleton after import")
    unreal.log(
        f"{role}: skeletal source imported at {target}, "
        f"{len(meta['animations'])} authored source animation clips. "
        "Retarget orientation, verify all AnimationSequences, AnimBP, PBR and "
        "720p appearance separately. NOT production art."
    )
    # Convert imported clips to STABLE named assets so native C++ can play a
    # genuinely breathing idle even when an artist AnimBlueprint is not built.
    # Reject an importer which silently produces a static skeleton only.
    sequences = [p for p in imported
                 if isinstance(unreal.EditorAssetLibrary.load_asset(p),
                               unreal.AnimSequence)]
    if not sequences:
        raise RuntimeError(
            f"{role}: skinned mesh imported, but UE Interchange produced NO "
            "AnimationSequence. Install/enable skeletal animation import; "
            "an unanimated character is not acceptable for a playtest."
        )
    role_prefix = {
        "snake": "Snake", "fox": "GrayFox",
        "guard": "Guard", "shoot_gunner": "Shotmaker",
    }[role]
    expected_names = set(REQUIRED_ANIMS)
    expected_names.update({"IdleAlert", "IdleLook", "IdleWary"})
    if role == "fox":
        expected_names.add("Captive")
    if role == "shoot_gunner":
        expected_names.update({"Address", "Shoulder", "Recoil", "Search"})
    found = set()
    organized = {}
    for source_path in sequences:
        name = str(source_path).split("/")[-1].split(".")[0]
        matches = [clip for clip in expected_names
                   if name.lower() == clip.lower()
                   or name.lower().endswith("_" + clip.lower())]
        if len(matches) != 1:
            unreal.log_warning(
                f"{role}: cannot uniquely identify imported animation "
                f"{name} from {source_path}; retaining its original path"
            )
            continue
        clip = matches[0]
        target_anim = f"{destination}/Animations/AS_{role_prefix}_{clip}"
        if source_path != target_anim:
            if unreal.EditorAssetLibrary.does_asset_exist(target_anim):
                raise RuntimeError(
                    f"{role}: refusing to overwrite existing animation "
                    f"{target_anim}"
                )
            if not unreal.EditorAssetLibrary.rename_asset(source_path, target_anim):
                raise RuntimeError(
                    f"{role}: could not move {source_path} to {target_anim}"
                )
        found.add(clip)
        organized[clip] = target_anim
    missing = expected_names - found
    if missing:
        raise RuntimeError(
            f"{role}: missing real Unreal AnimationSequences: {sorted(missing)}. "
            "Check Interchange import clip naming or use Editor import settings."
        )
    unreal.log(
        f"{role}: imported 18-joint skinned mesh {target}; "
        f"{len(found)} verified animation sequences: {organized}. "
        "These are procedural source performances, not mocap/PBR final art."
    )
    return {"mesh": target, "animations": organized}


def main():
    results = {}
    for role in ROLES:
        results[role] = import_prototype(role)
    unreal.log("PROTOTYPE CHARACTER IMPORT RESULT: " + json.dumps(results))
    unreal.log_warning(
        "Import script does not create finished animation Blueprints or "
        "realistic/PBR character art. Open Content Browser, confirm imported "
        "skeletons, assign animation graphs and verify movement in PIE."
    )


if __name__ == "__main__":
    main()
