"""Check evidence status of asset candidates; zero downloads assumed."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def validate(manifest):
    errors=[]
    if manifest.get("schema_version")!=1:
        errors.append("invalid asset manifest version")
    weights=manifest.get("criteria_weight",{})
    if sum(weights.values())!=100:
        errors.append("weights must total 100")
    ids=set()
    for candidate in manifest.get("candidates",[]):
        i=candidate.get("id","?")
        if i in ids: errors.append("duplicate source ID "+i)
        ids.add(i)
        for k in ("uri","rights","roles","note","status"):
            if not candidate.get(k):
                errors.append(i+" missing "+k)
        if candidate.get("approved"):
            for k in ("downloaded","UE5_7_import_tested","performance_capture"):
                if not candidate.get(k):
                    errors.append(i+" approval without "+k)
            for k in ("local_archive_sha256","exact_release_version"):
                if not candidate.get(k):
                    errors.append(i+" approval without "+k)
            if candidate.get("status")!="player_tested":
                errors.append(i+" approval before final review")
    for delivery in manifest.get("deliverables",[]):
        if delivery.get("status")=="approved" and not any(
            c.get("approved") and delivery.get("id") in c.get("roles",[])
            for c in manifest.get("candidates",[])
        ):
            errors.append("final character approved without traced asset source")
    return errors
if __name__=="__main__":
    d=json.loads((ROOT/"data/asset_source_quality_gates.json").read_text())
    e=validate(d)
    if e: raise SystemExit("\n".join(e))
    print("Asset provenance/approval contracts structurally valid; no file or UE import is claimed.")
