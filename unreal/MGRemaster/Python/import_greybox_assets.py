"""UE 5.7 Editor-only import of dimension-checked original greybox OBJ assets.

Requires PythonScriptPlugin in the editor. Refuses mesh bounds drift.
Run in Unreal Python console:
    exec(open(r"C:\path\repo\unreal\MGRemaster\Python\import_greybox_assets.py").read())
This is NOT run in the packaged game or on the user's low-spec machine.
"""
import json
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()
REPO = PROJECT_DIR.parents[1]
MANIFEST = REPO / "assets" / "asset_dimensions.json"
SOURCE_DIR = REPO / "assets" / "greybox"
DEST = "/Game/Greybox"
TOLERANCE_CM = 1.0


def import_and_check():
    if not MANIFEST.exists():
        raise RuntimeError("Asset ledger not found: " + str(MANIFEST))
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    if data.get("obj_vertex_units") != "centimetres":
        raise RuntimeError("Asset import requires centimetre-sized OBJ coordinates")

    jobs = []
    for entry in data["assets"]:
        path = SOURCE_DIR / (entry["id"] + ".obj")
        if not path.exists():
            raise RuntimeError("Missing source mesh: " + str(path))
        task = unreal.AssetImportTask()
        task.filename = str(path)
        task.destination_path = DEST
        task.automated = True
        task.replace_existing = True
        task.save = True
        jobs.append((entry, task))

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(
        [task for _, task in jobs]
    )
    failures = []
    for entry, task in jobs:
        asset_path = DEST + "/" + entry["id"]
        mesh = unreal.load_asset(asset_path)
        if mesh is None or not isinstance(mesh, unreal.StaticMesh):
            failures.append(entry["id"] + ": missing imported StaticMesh")
            continue

        bounds = mesh.get_bounding_box()
        if bounds is None:
            failures.append(entry["id"] + ": no static mesh bounding box")
            continue
        measured = [
            bounds.max.x - bounds.min.x,
            bounds.max.y - bounds.min.y,
            bounds.max.z - bounds.min.z
        ]
        expected = [entry[k] * 100 for k in ("w","d","h")]
        deviations = [abs(a-b) for a,b in zip(measured, expected)]
        if any(delta > TOLERANCE_CM for delta in deviations):
            failures.append(
                "{}: measured {} cm vs expected {} cm; check OBJ import scale".format(
                    entry["id"], measured, expected
                )
            )
        else:
            unreal.log("Dimension verified: {} -> {} cm".format(
                entry["id"], measured
            ))
    if failures:
        for error in failures:
            unreal.log_error(error)
        raise RuntimeError("Greybox import failed dimension audit: " + "; ".join(failures))
    unreal.log("Greybox import dimension audit passed for {} assets".format(len(jobs)))


import_and_check()
