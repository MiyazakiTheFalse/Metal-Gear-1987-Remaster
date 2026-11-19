"""Source-only inventory audit; never infer UE import from authored reference code."""
from pathlib import Path
import json
import wave

ROOT=Path(__file__).resolve().parents[1]
DATA=ROOT/"data/vertical_slice_asset_inventory.json"

def validate(d):
    errors=[]
    if d.get("schema_version") != 1: errors.append("schema")
    entries=d.get("entries",[])
    if d.get("count") != len(entries): errors.append("count")
    ids=[e.get("id") for e in entries]
    if len(set(ids))!=len(ids): errors.append("duplicate IDs")
    for item in entries:
        if item.get("status") not in d["readiness_values"]:
            errors.append("invalid readiness: "+str(item.get("id")))
        for path in item.get("source_paths",[]):
            if path.startswith("/") or ".." in Path(path).parts:
                errors.append("invalid source path "+str(path))
            elif not (ROOT/path).is_file():
                errors.append("source missing "+path)
        if item.get("imported_into_ue") or item.get("production_approved"):
            errors.append("false approval without editor provenance: "+str(item.get("id")))
    for cue in ("Transceiver_Incoming","Transceiver_Connect",
                "Transceiver_Tune","Transceiver_Close","Transceiver_Signal"):
        path=ROOT/"assets/source/transceiver/audio"/(cue+".wav")
        if not path.is_file():
            errors.append("missing transceiver "+cue)
            continue
        try:
            with wave.open(str(path),"rb") as w:
                if (w.getnchannels(),w.getsampwidth(),w.getframerate()) != (1,2,48000) or w.getnframes() < 5000:
                    errors.append("invalid transceiver WAV "+cue)
        except (wave.Error, EOFError):
            errors.append("unreadable transceiver WAV "+cue)
    return errors

if __name__ == "__main__":
    result=validate(json.loads(DATA.read_text(encoding="utf-8")))
    if result: raise SystemExit("\n".join(result))
    print("Slice asset source inventory verified (NO Unreal import/approval assumed).")
