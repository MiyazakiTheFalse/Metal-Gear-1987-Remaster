"""Unreal Editor Python: import Poly Haven CC0 PBR sets as live materials.

Precondition:
    python tools/acquire_polyhaven_pbr.py --resolution 2k
Then in Unreal Editor enable "Python Editor Script Plugin" and run this script.

Never import Fab or Mixamo raw assets through this redistributable CC0 pipeline.
No automatic runtime downloads, no raw marketplace file commits.
"""
import json
from pathlib import Path
import unreal

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "unreal" / "MGRemaster" / "ArtSource" / "PolyHavenCC0"
ART_ROOT = "/Game/OuterHeaven/Art"
MATERIAL_ROOT = ART_ROOT + "/Materials"
TEXTURE_ROOT = ART_ROOT + "/Textures"

def log(msg):
    unreal.log("[Outer Heaven CC0] " + str(msg))

def import_texture(file: Path, slug: str, kind: str):
    destination = f"{TEXTURE_ROOT}/{slug}"
    unreal.EditorAssetLibrary.make_directory(destination)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(file))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", f"T_{slug}_{kind}")
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    tools.import_asset_tasks([task])
    imported = task.get_editor_property("imported_object_paths")
    if not imported:
        raise RuntimeError(f"Texture import failed for {file}")
    asset = unreal.EditorAssetLibrary.load_asset(str(imported[0]))
    if not asset:
        raise RuntimeError(f"Could not load imported texture for {file}")
    try:
        if kind != "diffuse":
            asset.set_editor_property("srgb", False)
        if kind in ("normal_dx", "normal_gl"):
            asset.set_editor_property(
                "compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
            if kind == "normal_gl":
                # Poly Haven OpenGL normal -> Unreal DirectX tangent convention.
                asset.set_editor_property("flip_green_channel", True)
        unreal.EditorAssetLibrary.save_loaded_asset(asset)
    except Exception as exc:
        log(f"Texture metadata warning for {file.name}: {exc}")
    return asset

def node(material, cls, x, y):
    return unreal.MaterialEditingLibrary.create_material_expression(
        material, cls, x, y)

def connect(material, sampler, output, prop):
    unreal.MaterialEditingLibrary.connect_material_property(
        sampler, output, prop)

def build_material(slug: str, textures: dict):
    unreal.EditorAssetLibrary.make_directory(MATERIAL_ROOT)
    asset_path = f"{MATERIAL_ROOT}/M_{slug}"
    mat = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not mat:
        mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            f"M_{slug}", MATERIAL_ROOT, unreal.Material,
            unreal.MaterialFactoryNew())
    if not mat:
        raise RuntimeError(f"Material creation failed: {asset_path}")
    unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)

    # One scalar tiling control gives C++ authored floor/wall actors consistent
    # real-world texel scale instead of stretching a 2K image over 80 metres.
    coord = node(mat, unreal.MaterialExpressionTextureCoordinate, -900, -250)
    scale = node(mat, unreal.MaterialExpressionScalarParameter, -900, 0)
    scale.set_editor_property("parameter_name", "UVTiling")
    scale.set_editor_property("default_value", 1.0)
    multiply = node(mat, unreal.MaterialExpressionMultiply, -660, -200)
    unreal.MaterialEditingLibrary.connect_material_expressions(coord, "", multiply, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(scale, "", multiply, "B")

    properties = {
        "diffuse": ("RGB", unreal.MaterialProperty.MP_BASE_COLOR),
        "normal_dx": ("RGB", unreal.MaterialProperty.MP_NORMAL),
        "normal_gl": ("RGB", unreal.MaterialProperty.MP_NORMAL),
        "roughness": ("R", unreal.MaterialProperty.MP_ROUGHNESS),
        "metallic": ("R", unreal.MaterialProperty.MP_METALLIC),
        "ambient_occlusion": ("R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION),
    }
    occupied = set()
    for index, (kind, texture) in enumerate(textures.items()):
        tex = node(mat, unreal.MaterialExpressionTextureSample, -400,
                   -500 + index * 210)
        tex.set_editor_property("texture", texture)
        unreal.MaterialEditingLibrary.connect_material_expressions(
            multiply, "", tex, "UVs")
        if kind in ("normal_dx", "normal_gl"):
            try:
                tex.set_editor_property(
                    "sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
            except Exception:
                pass
        if kind in properties:
            channel, prop = properties[kind]
            if kind == "normal_gl" and "normal_dx" in textures:
                continue
            connect(mat, tex, channel, prop)
            occupied.add(kind)
        elif kind == "arm_packed":
            if "ambient_occlusion" not in textures:
                connect(mat, tex, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
            if "roughness" not in textures:
                connect(mat, tex, "G", unreal.MaterialProperty.MP_ROUGHNESS)
            if "metallic" not in textures:
                connect(mat, tex, "B", unreal.MaterialProperty.MP_METALLIC)
    if not any(x in textures for x in ("roughness", "arm_packed")):
        rough = node(mat, unreal.MaterialExpressionConstant, -360, 800)
        rough.set_editor_property("r", 0.68)
        connect(mat, rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    if not any(x in textures for x in ("metallic", "arm_packed")):
        metal = node(mat, unreal.MaterialExpressionConstant, -360, 960)
        metal.set_editor_property("r", 0.0 if "concrete" in slug or "floor" in slug
                                    else 0.75)
        connect(mat, metal, "", unreal.MaterialProperty.MP_METALLIC)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    log(f"Imported PBR material: {asset_path}")
    return mat

def main():
    manifest_path = SOURCE / "_download_manifest.json"
    if not manifest_path.exists():
        raise RuntimeError("Download the CC0 source maps first using "
                           "tools/acquire_polyhaven_pbr.py")
    sets = json.loads(manifest_path.read_text(encoding="utf-8"))
    for item in sets:
        slug = item["slug"]
        if item.get("license") != "CC0-1.0":
            raise ValueError(f"Unapproved licence: {slug}")
        folder = SOURCE / slug
        textures = {}
        for kind, file in item["files"].items():
            source = folder / file["filename"]
            if not source.is_file():
                raise FileNotFoundError(source)
            textures[kind] = import_texture(source, slug, kind)
        build_material(slug, textures)
    log(f"Finished {len(sets)} CC0 material sets. "
        "Reopen PIE to inspect live geometry material assignments.")

if __name__ == "__main__":
    main()
