"""Unreal Editor-only VO import. Never claims a missing line exists.

Run from Unreal 5.7 Editor Python after `tools/prepare_voice_assets.py`.
Avoid overwriting an existing artist/mastered SoundWave by default.
"""
from pathlib import Path
import json
import unreal

PROJECT = Path(unreal.Paths.project_dir()).resolve()
REPO = PROJECT.parents[1]
MANIFEST = REPO / "data" / "voice_performance_manifest.json"
SOURCE = REPO / "assets" / "source" / "vo" / "processed"

def main():
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    imported, missing, existing = 0, 0, 0
    for scene in data["scenes"]:
        for line in scene["lines"]:
            speaker, name = line["speaker"], line["clip"]
            filename = SOURCE / speaker / (name + ".wav")
            destination = f"/Game/Audio/VO/{speaker}"
            target = destination + "/" + name
            if unreal.EditorAssetLibrary.does_asset_exist(target):
                existing += 1
                continue
            if not filename.is_file():
                missing += 1
                continue
            task = unreal.AssetImportTask()
            task.set_editor_property("filename", str(filename))
            task.set_editor_property("destination_path", destination)
            task.set_editor_property("destination_name", name)
            task.set_editor_property("automated", True)
            task.set_editor_property("replace_existing", False)
            task.set_editor_property("save", True)
            tools.import_asset_tasks([task])
            if not isinstance(unreal.EditorAssetLibrary.load_asset(target), unreal.SoundWave):
                raise RuntimeError(f"VO import failed or wrong asset type: {target}")
            imported += 1
    unreal.log_warning(
        f"Dialogue asset pass: imported {imported}, skipped existing {existing}, "
        f"still missing masters {missing}. Nothing marked performance-approved.")

if __name__ == "__main__":
    main()
