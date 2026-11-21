"""UE5.7 Editor-only source importer. Does NOT run inside ordinary Python.

Execute in the editor after enabling Python Editor Script Plugin + Interchange/OBJ import:
exec(open(r"/PATH/TO/REPO/unreal/MGRemaster/Content/Python/import_snake_hair.py", encoding="utf-8").read())
"""
from pathlib import Path
import unreal

PROJECT = Path(unreal.Paths.project_dir()).resolve()
SOURCE = PROJECT.parents[1] / "assets" / "source" / "snake_hair" / "snake_1995_hair.obj"
DESTINATION = "/Game/Characters/Snake/Hair"
ASSET = "SM_SnakeHair_1995"
def main():
    target = DESTINATION + "/" + ASSET
    if unreal.EditorAssetLibrary.does_asset_exist(target):
        unreal.log_warning(f"Not overwriting existing artist-approved asset: {target}")
        return
    if not SOURCE.is_file():
        raise FileNotFoundError(f"Missing hair source: {SOURCE}")
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(SOURCE))
    task.set_editor_property("destination_path", DESTINATION)
    task.set_editor_property("destination_name", ASSET)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", False)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    result = unreal.EditorAssetLibrary.load_asset(target)
    if not isinstance(result, unreal.StaticMesh):
        raise RuntimeError(f"Expected StaticMesh at {target}; check OBJ importer and logs")
    unreal.log(f"Imported provisional Snake hair: {target}. Still requires head fit, shading and gameplay validation.")

if __name__ == "__main__":
    main()
