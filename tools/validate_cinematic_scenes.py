"""Structural cinematic-doctrine validation. Does not build or run Unreal."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
EXPECTED={"capture","gray_fox","shotgunner_intro","shotgunner_aftermath","madnar","tx55","final_boss"}
def validate(c):
    errors=[]
    if c.get("schema_version")!=1 or c.get("doctrine")!="DOCTRINE_004":
        errors.append("cinematic doctrine mismatch")
    if c.get("implementation_status")!="design_only_uncompiled":
        errors.append("false implementation status")
    scenes=c.get("scenes",[])
    ids=[s.get("id") for s in scenes]
    if len(ids)!=len(set(ids)) or not EXPECTED.issubset(ids):
        errors.append("duplicate or missing required scenes")
    for s in scenes:
        p=s.get("id","?")
        for k in ("player_knowledge","world_trigger","state_commit","skip","asset_gates","failures","shots"):
            if not s.get(k):
                errors.append(p+" missing "+k)
        if len(s.get("shots",[]))<3:
            errors.append(p+" lacks detailed shot purpose")
        for shot in s.get("shots",[]):
            if not all(shot.get(k) for k in ("id","camera","purpose")):
                errors.append(p+" has an incomplete shot")
        if s.get("status")!="design_only":
            errors.append(p+" falsely marked produced")
    c0=next((s for s in scenes if s.get("id")=="capture"),{})
    if "nonlethal" not in c0.get("asset_gates",[]).__str__().lower() and "nonlethal" not in c0.get("world_trigger","").lower():
        errors.append("capture must specify nonlethal premise")
    tx=next((s for s in scenes if s.get("id")=="tx55"),{})
    if "sixteen" not in tx.get("state_commit","").lower():
        errors.append("TX-55 destroyed without ordered sabotage state")
    return errors
if __name__=="__main__":
    d=json.loads((ROOT/"data/cinematic_scene_contracts.json").read_text(encoding="utf-8"))
    errors=validate(d)
    if errors: raise SystemExit("\n".join(errors))
    print("Cinematic source/skip/asset contracts valid; NOT an engine/scene playtest.")
