# Metal Gear (1987) Remaster

PC-first, console-aware 3D stealth remaster of the supplied Connect2Media J2ME v3.3.0 edition. Aim: modern MGSV-style responsiveness while preserving the original mission structure.

## Technology
- **Unreal Engine 5** for rendering, controls, animation and packaged builds.
- **Rust** for portable, testable gameplay systems and legacy resource conversion.
- J2ME archive and original game data remain **outside this public source repository**, with a local import workflow.

## Graphics policy: performance by default
First launch should use **Performance** (720p windowed / 70% 3D render scale, conservative shadows, effects and textures). Balanced, High and Ultra require explicit user selection from the video settings menu. User settings are saved and should not be silently raised by autodetection.

An **optional HD texture pack** will be separately packaged when asset production starts. Do not require this pack for base play, game saves, missions, or Switch. Switch graphics are configured and profiled independently.

See `docs/GRAPHICS_POLICY.md` for exact presets, implementation and validation requirements.

## Status
This repository is an **early engineering scaffold**. There is no verified executable Windows build or Switch build yet. UE source has to be packaged on a machine with Unreal Engine installed; the end user will not need the editor to play a packaged build.

## Next
1. Port the local first-pass Rust guard/perception core and J2ME audit script into this source repository.
2. Add Unreal video settings menu backed by the preset API.
3. Build a small playable PC vertical slice, with performance profiling and user-testable packaged executables.
4. Reverse-map the uploaded J2ME game's room and mission data, then validate each rebuilt mission against it.

### Repository hygiene
Do not commit licensed game archives/assets, third-party SDKs, binaries, build intermediates or local asset extractions. `.gitignore` protects the obvious paths but is not a substitute for checking commits.