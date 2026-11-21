"""Unreal Editor-only import of our five original 48kHz transceiver WAVs.
Run in UE 5.7 Editor Python console, with Python Editor Script Plugin enabled:
  exec(open(r'/path/repo/unreal/MGRemaster/Content/Python/import_transceiver_audio.py', encoding='utf-8').read())
Source-only workflow; not run in a UE Editor in this environment.
"""
from pathlib import Path
import wave
import unreal

PROJECT = Path(unreal.Paths.project_dir()).resolve()
REPO = PROJECT.parents[1]
SOURCE = REPO / "assets" / "source" / "transceiver" / "audio"
DESTINATION = "/Game/Audio/Transceiver"
CUES = (
    "Transceiver_Incoming",
    "Transceiver_Connect",
    "Transceiver_Tune",
    "Transceiver_Close",
    "Transceiver_Signal",
)


def main():
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    for cue in CUES:
        source = SOURCE / f"{cue}.wav"
        if not source.is_file():
            raise FileNotFoundError(f"Required transceiver sound source missing: {source}")
        with wave.open(str(source), "rb") as w:
            if (w.getframerate(), w.getnchannels(), w.getsampwidth()) != (48000, 1, 2):
                raise ValueError(f"Unexpected audio format for {cue}")
            if not w.getnframes():
                raise ValueError(f"Empty audio for {cue}")

        path = f"{DESTINATION}/{cue}"
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            asset = unreal.EditorAssetLibrary.load_asset(path)
            if not isinstance(asset, unreal.SoundWave):
                raise TypeError(f"Existing transceiver asset at {path} is not SoundWave.")
            unreal.log_warning(f"Verified existing SoundWave, no silent overwrite: {path}")
            continue
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(source))
        task.set_editor_property("destination_path", DESTINATION)
        task.set_editor_property("destination_name", cue)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", False)
        task.set_editor_property("save", True)
        tools.import_asset_tasks([task])
        if not unreal.EditorAssetLibrary.does_asset_exist(path):
            raise RuntimeError(f"Transceiver import failed for {path}: check editor logs.")
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if not isinstance(asset, unreal.SoundWave):
            raise TypeError(f"Imported {path} but not as SoundWave: {type(asset)}")
        unreal.log(f"Imported original transceiver SoundWave {path}")

    unreal.log("All five transceiver WAVs imported; cinema VO and audio mix QA remain.")


if __name__ == "__main__":
    main()
