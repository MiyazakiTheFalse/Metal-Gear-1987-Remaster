"""Unreal 5.7 editor-side import of all original project music/SFX WAV files.

Run via Unreal Editor Python console AFTER copying the audio bundle's
AudioSource/Music and AudioSource/FX directories to:
    unreal/MGRemaster/ArtSource/Audio/Music
    unreal/MGRemaster/ArtSource/Audio/FX
or set MG_AUDIO_SOURCE_DIR to the extracted AudioSource folder.

The import creates actual USoundWave assets at paths used by
AMGAdaptiveAudioDirector: /Game/Audio/Music/music_title, etc.

No copied Metal Gear copyrighted audio files are required or included.
"""
from __future__ import annotations
import hashlib
import json
import os
from pathlib import Path
import unreal

ROOT=Path(__file__).resolve().parents[1]
SOURCE=Path(os.environ.get("MG_AUDIO_SOURCE_DIR",
    ROOT/"unreal"/"MGRemaster"/"ArtSource"/"Audio")).resolve()
DEST="/Game/Audio"
EXPECTED=json.loads((SOURCE/"audio_bank.json").read_text(encoding="utf-8"))

def imported_files():
    if EXPECTED.get("copyright","").lower().find("original")<0:
        raise RuntimeError("Audio provenance has not been recorded")
    for group,samples in (("Music",EXPECTED["music"]),("FX",EXPECTED["fx"])):
        for cue,meta in samples.items():
            path=SOURCE/meta["path"]
            if path.resolve().is_relative_to(SOURCE) is False:
                raise RuntimeError(f"Unsafe path: {path}")
            if not path.is_file():
                raise FileNotFoundError(path)
            digest=hashlib.sha256(path.read_bytes()).hexdigest()
            if digest!=meta["sha256"]:
                raise RuntimeError(f"Asset hash mismatch: {cue}")
            yield group,cue,path,meta

def import_sound(group,cue,file,meta):
    folder=DEST+"/"+group
    unreal.EditorAssetLibrary.make_directory(folder)
    task=unreal.AssetImportTask()
    task.set_editor_property("filename",str(file))
    task.set_editor_property("destination_path",folder)
    task.set_editor_property("destination_name",cue)
    task.set_editor_property("automated",True)
    task.set_editor_property("replace_existing",True)
    task.set_editor_property("save",True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    paths=task.get_editor_property("imported_object_paths")
    if not paths:
        raise RuntimeError(f"No SoundWave imported for {cue}")
    sound=unreal.EditorAssetLibrary.load_asset(str(paths[0]))
    if not sound:
        raise RuntimeError(f"Cannot find imported soundwave {cue}")
    if not sound.is_a(unreal.SoundWave):
        raise TypeError(f"Expected SoundWave for {cue}, got {sound.get_class().get_name()}")
    # Loop at the wave level so SpawnSound2D cross-fades never stop at 8 bars.
    # On engine builds where this API is read-only, report the error rather
    # than pretending the track loops correctly.
    if group=="Music":
        sound.set_editor_property("looping",True)
    try:
        sound.set_editor_property("volume",.77 if group=="Music" else .85)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(sound)
    unreal.log(f"[MG Audio] imported {group}/{cue} ({meta['seconds']}s)")
    return sound

def main():
    counts={"Music":0,"FX":0}
    for group,cue,file,meta in imported_files():
        import_sound(group,cue,file,meta)
        counts[group]+=1
    if counts["Music"]<8 or counts["FX"]<30:
        raise RuntimeError(f"Incomplete audio import: {counts}")
    unreal.log("[MG Audio] All 8 missions can now resolve the original audio bus. "
               +str(counts)+" (verify in PIE and after cooking).")

if __name__=="__main__":
    main()
