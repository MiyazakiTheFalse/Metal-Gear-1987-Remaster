# Original-data reference: confirmed facts vs unknowns

The supplied archive is Metal-Gear_J2ME_EN_v330.zip, with three Connect2Media J2ME editions.
The selected reference is the MIDP 2.0 / CLDC 1.1 (a1) JAR. Previous analysis of the supplied source recorded:
- 74 numbered f1..f74 game-data records with 0x40000000 | byte-count wrappers.
- 56 MIDI tracks.
- 84 embedded PNG images, counted separately from any standalone graphics.
- Matching numbered resource bytes across the three bundled variants.

This is an **asset/container audit only**, not proof that room maps, story flags or enemy scripting have been interpreted. We must trace and validate the legacy bytecode/data format before claiming faithful level reproduction.

Do not put the JAR, MIDI, textures, extracted bytecode or raw mission files in this public repository. Keep \`content/\` local, and commit only original remaster source/scripts. Rights clearance is assumed for development, but project visibility is currently public and publication should be a deliberate choice.
