"""Import authored original 1995 radio portrait expression PNGs into UE.

Unreal Editor -> Tools -> Execute Python Script -> this file.
Generate them first via 'python tools/build_codec_portraits.py' outside UE.
No fake import: a missing PNG aborts before altering Unreal packages.
"""
from pathlib import Path
import unreal

ROOT = Path(__file__).resolve().parents[4]
SRC = ROOT / "assets" / "source" / "codec" / "png"
IDENTITIES = (
    "snake", "bigBoss", "schneider", "diane", "jennifer", "fox",
    "pettrovich", "ellen"
)
EXPRESSIONS = ("Neutral", "Talk", "Blink")
files = [
    SRC / f"T_Codec_{identity}_{expression}.png"
    for identity in IDENTITIES
    for expression in EXPRESSIONS
]
missing = [str(path) for path in files if not path.is_file()]
if missing:
    raise FileNotFoundError(
        "Generate all 24 source-authored PNG expressions before import. "
        "Missing examples: " + ", ".join(missing[:4])
    )
destination = "/Game/UI/Codec"
tools = unreal.AssetToolsHelpers.get_asset_tools()
tasks = []
for path in files:
    name = path.stem
    # Skip existing imports to avoid overwriting a later artist-authored
    # higher-fidelity replacement without a deliberate art review.
    if unreal.EditorAssetLibrary.does_asset_exist(f"{destination}/{name}"):
        unreal.log_warning(f"Codec portrait already present; preserving artist asset: {name}")
        continue
    task = unreal.AssetImportTask()
    task.filename = str(path)
    task.destination_path = destination
    task.destination_name = name
    task.automated = True
    task.save = True
    task.replace_existing = False
    task.replace_existing_settings = False
    tasks.append(task)
if tasks:
    tools.import_asset_tasks(tasks)
    unsuccessful = [t.filename for t in tasks if not t.imported_object_paths]
    if unsuccessful:
        raise RuntimeError("Portrait texture import failed: " + ", ".join(unsuccessful))
    unreal.log(f"Imported {len(tasks)} distinct original codec portrait textures.")
else:
    unreal.log("All codec portrait textures already exist; no import performed.")
