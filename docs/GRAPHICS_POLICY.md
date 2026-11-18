# Performance-first graphics policy

Goal: users with modest PCs should be able to launch packaged builds without the Unreal Editor.
This is a target policy; it does not certify that current code runs well on a given device.

The default install is 1280x720 windowed with a 70% internal 3D render scale,
conservative scalability and 60fps cap. Nanite, Lumen and virtual shadow maps are
disabled in the initial prototype. These settings are declared in
\`Config/DefaultGameUserSettings.ini\` and \`Config/DefaultEngine.ini\`.

| Preset | Screen scale | Overall scalability | Opt in |
|---|---:|---:|---|
| Performance | 70% | 0 | Default |
| Balanced | 85% | 1 | F6 |
| High | 100% | 2 | F6 |
| Ultra | 100% | 3 | F6 |

F6 cycles settings, then stores preferences using Unreal GameUserSettings.
A proper accessibility-friendly options menu with independent sliders is not built yet.
Changing a preset does not yet install high-resolution textures.
An optional HD texture pack is planned as a separately downloadable pak.

Platform limits:
- PC: run multiple GPU/CPU targets through a performance and thermal test matrix.
- Nintendo Switch: lower-spec platform-specific profile, subject to official SDK,
  Nintendo approval, and an actual console devkit test. Do not promise parity.

The quality difference should be render quality, not game mechanics: all presets
should retain the same enemy logic, mission script, controls and save compatibility.
