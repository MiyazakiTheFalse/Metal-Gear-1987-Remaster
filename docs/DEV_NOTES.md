# Source milestones and engineering constraints

**2026-11-18**: Imported the validated Rust awareness ABI, resource wrapper reader
and synthetic tests. Added an Unreal Engine 5.7 C++ source vertical slice with:
third-person free look, camera-relative movement, over-shoulder aiming, aim
strafe, shoulder swap, crouch, sprint, guard patrol/vision/hearing/alert, prototype
stun, procedural cover and HUD. Source pushed, but UE was unavailable in the
authoring environment so this milestone is **not compiled or playtested**.

Key limitations:
- Meshes are primitives; animations, CQC, inventory and original areas are missing.
- Guards use simple straight-line patrol/chase and need navigation/animation.
- Aiming uses camera and muzzle line traces, not authentic weapon ballistics.
- J2ME records are extracted and validated, but room/event data are not decoded.
- No native Switch build without Nintendo-authorised tools.
- No Windows packaged executable exists as part of these commits.

Checks that can be run without Unreal:
\`\`\`sh
python -m unittest discover -s tools -p "test_*.py"
python tools/inspect_and_extract.py /path/to/Metal-Gear_J2ME_EN_v330.zip --output content
cd rust && cargo test
\`\`\`

No paid CI pipeline has been configured. Run checks locally or on a build host,
then publish a packaged Windows test build as a GitHub Release asset.
