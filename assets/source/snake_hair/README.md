# Young Snake's 1995 hair — original provisional geometry

The `snake_1995_hair.obj` is an **authored original, unrigged source mesh**: scalp shell, face-framing short locks and a restrained rear/nape silhouette. Brown colour values in the MTL are first-pass only, not a reviewed skin/hair shading system or a claim to have reproduced a specific officially verified hairstyle.

**Intended pipeline:** import via `unreal/MGRemaster/Content/Python/import_snake_hair.py`, then set head attachment placement in `AMGPlayerCharacter::HairOffsetCm` and `HairRotation` after inspecting the actual licensed/created young-Snake skeletal head. Keep hair visible both before and after gear confiscation. A rig with no `head` joint/socket deliberately hides the accessory rather than rendering it floating mid-air. The existing `SM_SnakeHair_1995` Blueprint/artist asset is never overwritten by the importer.

**Unmet production requirements:** topology fitting to the final head; hairline/UV and anisotropic PBR strand material; card transparency/temporal antialiasing; separate short rear strands responding to motion; LOD and overdraw; headset/720p first-time-player identification; collision with bandana/collar; texture and any approved period-specific styling; full-body rig and AnimBP still absent. These meshes are **not** MGS4-quality hair and are not a substitute for a finished character model.
