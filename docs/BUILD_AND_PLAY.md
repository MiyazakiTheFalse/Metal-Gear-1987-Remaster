# Build and play plan

## Immediate development
The UE5 project contains an **asset-free test chamber** with placeholder geometry/characters. It is source code, not a tested Windows release. The Rust library implements perception; Unreal controls rendering, line-of-sight, camera and movement.

## Rust
Install stable Rust. Run:
\`\`\`sh
cd rust
cargo test
cargo build --release
cargo run --bin mg_probe -- ../content/legacy_raw
\`\`\`
On Windows, Unreal Build.cs links rust/target/release/mg_legacy_core.lib if present. Without it the Unreal guard prototype has a reduced C++ fallback. The fallback is not a replacement for all Rust functionality.

## Unreal
On a machine with Unreal 5.7:
1. Open \`unreal/MGRemaster/MGRemaster.uproject\`.
2. Compile the C++ module. The default map is the Engine Entry map, and the GameMode spawns an untextured test chamber at runtime.
3. Test with mouse + keyboard, then a gamepad. Package Windows from the editor or Unreal AutomationTool.
4. Distribute the resulting **packaged Windows build**, not a .uproject, to testers. The recipient does not need Unreal installed.

The core has **not** been compiled in this environment: Rust and UE are unavailable. Compatibility must be established by an actual Unreal build/test run before claiming that packaged binaries work.

## Switch
Requires a licensed Nintendo development environment and supported Unreal console toolchain. Windows binaries cannot be converted to Switch builds. The Rust simulation remains engine-neutral, but its native library must be built for the authorized target and linked through the console toolchain.

## Low-spec first
The base install uses a Performance preset. Higher presets are explicitly opted into with F6; HD content is a future optional package, never required for missions/saves. The test chamber does not yet represent graphics or performance of a final title.
