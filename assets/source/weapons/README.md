# Outer Heaven 1995 — source-authored weapons and hand-quality gate

The weapon source pack contains **10 original editable static OBJ meshes**, including a large riot gun split into *receiver/barrel/stock body* and a separately animated pump; handgun, SMG, grenade launcher, rocket launcher, guided missile body, mine, plastic explosive and a visibly non-canonical stun test prop. They are authored geometry sources only. No imported and tested UE assets are claimed.

## Import
In UE5.7 Editor, run:
```python
exec(open(r"/YOUR_REPO/unreal/MGRemaster/Content/Python/import_1995_weapon_pack.py", encoding="utf-8").read())
```
The importer preserves existing artist .uassets and checks types. It matches the exact C++ paths for Snake and security handgun `/Game/Weapons/Snake/SM_Handgun_1995` and Shoot Gunner `/Game/Weapons/Shotmaker/SM_RiotShotgun_1995` plus `SM_RiotShotgunPump_1995`.

Do **not** import a source OBJ and then report the asset as shipping-ready; these models currently have colour-only MTL and provisional source UVs. Final assets need verified silhouettes, UV0/UV1, PBR textures and material instances, modular pumping/bolt/magazine, LOD, Nanite/overdraw evaluation, close-camera screenshots and visual QA on fully rigged humans. Weapon outlines need to read clearly at 720p under the actual game lights.

## 1995 weapon-hand quality baseline (all humans, all equipped weapons)

**Authentic body/attachment**: real humanoid hands, individually posed fingers, two-hand IK for supported weapons, complete palm/grip and support-hand contact, barrel bore/muzzle matching the trace, correct stock/shoulder contact and weapon axis. Mandatory `weapon_r` bone/socket in every armed rig, and approved `muzzle`, `ejection_port`, `magazine`, `support_hand` (or `pump_support`) sockets for the specific exported weapon. Missing sockets must be surfaced as blockers rather than replaced with a false-quality animation.

**Authoritative sequence**: aim → lock/settle → fire (shot from the barrel) → slide or action cycles → visible recoil → casing/hull ejection if applicable → firing-ready or empty → reload (magazine removal/retention, fresh magazine/individual shell insertion, charge/ready action) → verify ready. Each clip has two-hand ownership rules. Keep hips/feet planted or genuinely traveling; gestures and cinematic calls cannot award ammo/damage.

**Guard behaviour**: ordinary guards in this proposed 3D interpretation carry an 8-round security handgun with 3 spare magazines. A visible line-of-sight, weapon-bearing and ~.31s aim interval precede a shot; after magazines empty, guard cannot pretend to fire. ~1.65s replacement window is intentionally interruptible by a hit; an interrupted reload doesn't grant a full magazine. These values are **not confirmed MSX2 ammunition stats**. Distinct role-based custody challenge remains nonviolent toward a compliant player.

**Shoot Gunner**: three source-faithful wide blasts; pump after each shot at ~.17s as a distinct action, with delayed spent-hull ejection. After recovery and when tube empty, individual shells insert at provisional ~.58s per shell. Reload is a proper vulnerable and visually readable state. Tube capacity 3 for the default test is a design hypothesis; the **three-shot original attack pattern** is canon, an identified real-world gun make and internal cartridge count are not.

**Snake**: conventional semi-automatic handgun may eject a metal case on cycling. A *wired-dart stun test device* (itself NOT part of the 1987 arsenal) may eject/replace a disposable dart cartridge only when reloaded, **not** a brass handgun casing after firing. The game's true remote missiles, explosives, mines, SMG and launchers need independent firing/reload/throw/placing state machines and appropriate real physics before becoming playable.

**Evaluation**: record 30/60 FPS close-up captures of hand contact, seated/standing/crouch/prone/cover transitions, in-place 180° turns, low ceilings, all 3 boss shots, interrupted sentry reload, muzzle/cover collision, in-engine casings, dry-fire/safe reload, sounds, equipment confiscation, and aim after retuning radio. Run with no VO/music/HUD to confirm understandable acting. Target a coherent 1987 successor to the *Phantom Pain* lineage, without asserting speculative 1984 events as unseen 1987 canon.

**Status:** source systems/geometry, no certified editor compiled build, hand retargeted animation clips, IK rig, professional weapon foley, UE materials, character art or final user playtesting.
