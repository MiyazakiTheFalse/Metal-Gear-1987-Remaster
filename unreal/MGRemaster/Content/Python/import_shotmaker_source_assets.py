"""Run from Unreal Editor Python, UE5.7; source-authored not executed here.
Editor console:
exec(open(r"/YOUR_REPO/unreal/MGRemaster/Content/Python/import_shotmaker_source_assets.py", encoding="utf-8").read())
Requires Python Editor Script Plugin and OBJ/Interchange import support.
"""
from pathlib import Path
import unreal

PROJECT = Path(unreal.Paths.project_dir()).resolve()
SOURCE = PROJECT.parents[1] / "assets" / "source" / "shotmaker"
ENTRY = [
    ("riot_gun_game_mesh.obj", "/Game/Weapons/Shotmaker", "SM_RiotShotgun_1995"),
    ("audio/Shotmaker_AimCue.wav", "/Game/Audio/Shotmaker", "Shotmaker_AimCue"),
    ("audio/Shotmaker_Blast.wav", "/Game/Audio/Shotmaker", "Shotmaker_Blast"),
    ("audio/Shotmaker_Cycle.wav", "/Game/Audio/Shotmaker", "Shotmaker_Cycle"),
]

def main():
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    for relative, destination, name in ENTRY:
        source = SOURCE / relative
        path = destination + "/" + name
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            unreal.log_warning(f"Existing Shotmaker asset not overwritten: {path}")
            continue
        if not source.is_file():
            raise FileNotFoundError(source)
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(source))
        task.set_editor_property("destination_path", destination)
        task.set_editor_property("destination_name", name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", False)
        task.set_editor_property("save", True)
        tools.import_asset_tasks([task])
        if not unreal.EditorAssetLibrary.does_asset_exist(path):
            raise RuntimeError(f"Failed import {path}; check editor logs and OBJ importer. No success claimed.")
        result = unreal.EditorAssetLibrary.load_asset(path)
        expected = unreal.StaticMesh if source.suffix.lower() == ".obj" else unreal.SoundWave
        if not isinstance(result, expected):
            raise TypeError(f"Wrong type for {path}: {type(result)}")
        unreal.log(f"Imported Shotmaker source: {path}")
    unreal.log("Imported source meshes/sounds; rig, animation and playtesting still pending.")

if __name__ == "__main__":
    main()
