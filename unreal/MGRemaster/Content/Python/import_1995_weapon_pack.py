"""Non-destructive import of original higher-detail 1995 weapon source meshes.

Run inside Unreal Editor (Python Editor Script Plugin; OBJ/Interchange importer).
The source OBJ geometry is made for this project and is NOT an approved PBR,
rigged, animated or canon-identified licensed weapon. Existing .uassets
always take precedence; no replacement or source-cook success is implied.
"""
from pathlib import Path
import unreal

PROJECT = Path(unreal.Paths.project_dir()).resolve()
REPO = PROJECT.parents[1]
SOURCE = REPO / "assets" / "source" / "weapons"

ENTRY = [
    ("shotmaker_main_1995_lod0.obj", "/Game/Weapons/Shotmaker", "SM_RiotShotgun_1995"),
    ("shotmaker_pump_1995_lod0.obj", "/Game/Weapons/Shotmaker", "SM_RiotShotgunPump_1995"),
    ("snake_handgun_1995_lod0.obj", "/Game/Weapons/Snake", "SM_Handgun_1995"),
    ("snake_smg_1995_lod0.obj", "/Game/Weapons/Snake", "SM_SubmachineGun_1995"),
    ("grenade_launcher_1995_lod0.obj", "/Game/Weapons/Snake", "SM_GrenadeLauncher_1995"),
    ("rocket_launcher_1995_lod0.obj", "/Game/Weapons/Snake", "SM_RocketLauncher_1995"),
    ("guided_missile_1995_lod0.obj", "/Game/Weapons/Snake", "SM_RemoteMissile_1995"),
    ("mine_1995_lod0.obj", "/Game/Weapons/Snake", "SM_LandMine_1995"),
    ("plastic_explosive_1995_lod0.obj", "/Game/Weapons/Snake", "SM_PlasticExplosive_1995"),
    ("stun_test_1995_lod0.obj", "/Game/Weapons/Testing", "SM_StunPistol_Test"),
    ("handgun_spent_casing_1995.obj", "/Game/Weapons/Shared", "SM_HandgunCasing_1995"),
    ("riotgun_spent_hull_1995.obj", "/Game/Weapons/Shared", "SM_RiotHull_1995"),
]

def main():
    importer = unreal.AssetToolsHelpers.get_asset_tools()
    imported = 0
    skipped = 0
    for relative, folder, asset in ENTRY:
        source = SOURCE / relative
        target = f"{folder}/{asset}"
        if unreal.EditorAssetLibrary.does_asset_exist(target):
            skipped += 1
            unreal.log_warning(f"Asset preserved, not overwritten: {target}")
            continue
        if not source.is_file():
            raise FileNotFoundError(source)
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(source))
        task.set_editor_property("destination_path", folder)
        task.set_editor_property("destination_name", asset)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", False)
        task.set_editor_property("save", True)
        importer.import_asset_tasks([task])
        if not isinstance(unreal.EditorAssetLibrary.load_asset(target), unreal.StaticMesh):
            raise RuntimeError(f"Weapon OBJ not imported as StaticMesh: {target}")
        imported += 1
        unreal.log(f"Authored source imported (NOT quality-approved): {target}")
    unreal.log_warning(f"Imported {imported} weapon sources; preserved {skipped}. "
        "NEXT: PBR material, physical weapon sockets, true grip/IK, authored "
        "reload/ejection animations, LODs, UVs, 720p readability, engine build and playtest.")

if __name__ == "__main__":
    main()
