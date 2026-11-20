# 1995 cast model and rig register

The 20 cast dossiers include ensembles, machines and two identities associated with Big Boss. Solid Snake has a separate protagonist dossier. The table covers all 21 records. It also tracks the kennel dogs and the extra hostages needed to stage encounters. A dash in the measurement column means this audit has no attributable height. Give each unnamed performer's turnaround an asset scale when casting. Keep that art decision separate from biography.

| Dossier | Height / mass reference | 1995 role, ability or defining equipment | Current representation and next modelling requirement |
|---|---|---|---|
| Solid Snake | 178 cm / 75 kg, MG2 reference | Rookie FOXHOUND operative on N313. Acquires equipment in the field. The original manual identifies the handgun as an M92F. MG2 credits him with parachuting, diving and climbing. | Dedicated browser model. Match the anatomical landmark scale in Unreal, preserve his face between custody and field variants, and verify both hands on equipment. |
| Gray Fox | 179 cm / 85 kg, MG2 reference | Experienced FOXHOUND infiltrator, holder of the FOX title. Captive and source of the Metal Gear intelligence. His prison appearance uses his human body. | Dedicated browser model with a captive pose. Needs a recognisable face, restrained breathing, credible contact and a safe interrupted release. |
| Real Big Boss | 180 cm / 89 kg, MG2 reference | FOXHOUND commander, radio adviser on equipment. Command presence and timing carry most of his role. | Radio presentation first. Needs an approved portrait and voice/performance reference. |
| Venom Snake | — | Retrospective production identity of the fortress commander. Snake experiences the final encounter as a confrontation with Big Boss. | Bespoke final silhouette and concealed continuity details. Approve the 1995 costume independently of the commander reference. |
| Kyle Schneider | — | Architect and resistance leader familiar with the fortress. | Radio portrait and measured delivery. A field body depends on the shot list. |
| Diane | — | Former Thin Wall singer and resistance intelligence contact with knowledge of the mercenaries. | Radio portrait and vocal rhythm. Information and humour need clear direction. |
| Steve | — | Occasional radio substitute in the edition-specific dossier. Verify his appearances against the chosen script. | Voice and portrait decision pending the edition check. |
| Jennifer | — | Resistance operative inside the medical staff, assisting Snake while trying to rescue her brother. | Distinct medical silhouette and practical hand props if physically staged. |
| Jennifer's brother | — | Captive with escape-route information. | Individual hostage identity, eyelines and rescue performance. |
| Dr Pettrovich Madnar | — | Coerced designer of TX-11 and TX-55. His daughter is held hostage. | Distinct face, workwear and pointing gestures that make his technical explanation legible. |
| Ellen Madnar | — | Captive and former Bolshoi dancer. Her history gives a performer a reference for balance and posture. | Individual captive model and rescue performance. |
| Madnar decoy | — | False identification is the scene function. Confirm the encounter against the target edition. | Test resemblance in the actual lighting and camera before commissioning a separate body. |
| POW informant | — | Directs Snake towards detention. | Browser reuses Fox art. Needs a separate readable identity to prevent players mistaking him for Fox. |
| Outer Heaven guards | — | Patrol, custody, logistics and prison duties. The shared guard prototype supports sidearm and alert behaviours. | One dedicated browser guard. Requires outfit variants, hands for props, aim, radio and alert recovery. |
| Four mercenaries | — | Early group encounter. Individual names, heights and weapon models remain unverified. | Ensemble brief only. Stage four readable positions before commissioning four hero sculpts. |
| Shoot Gunner / Shotmaker | — | Original manual identifies a former Spetsnaz riot-gun specialist guarding solitary. | Dedicated browser boss. Separate weapon and pump source meshes exist. Verify support-hand travel, shoulder contact and reload at gameplay distance. |
| Machinegun Kid | — | Original manual identifies a former SAS machine-gunner. Exact weapon make and model remain open. | Browser guard rig plus a proxy weapon. Unreal uses a guard-derived actor. Needs a distinct ammunition rig, reload performance and a readable silhouette at the lift. |
| Fire Trooper | — | Former GSG 9 operator with a flamethrower in the original manual. | Needs a distinctive carried assembly, hose deformation and a turn animation readable from either shoulder. |
| Coward Duck / Dirty Duck | — | Former Egg Plant leader. Boomerang and hostages define the encounter. | Needs a throwing hand, release/return poses and three individually legible hostage positions. |
| Arnold / Bloody Brad | — | TX-11 artificial combat units, designed by Madnar. | A mechanical body with paired instances. Author hard joint limits and distinct movement timing for each instance. |
| TX-55 Metal Gear | 6.05 m high, 3.25 m long, 2.75 m wide, 20.5 tons, original manual | Nuclear-capable walking tank. Use the original sabotage scene to decide what moves on screen. | Mechanical boss dossier. Build a dimensioned turnaround and articulated legs, then verify player scale against them. |
| Kennel dogs and additional hostages | — | Dogs patrol the service wing. Hostages require distinct safe positions and rescue reactions. | Browser dogs use rigid shapes. Native dogs support a skeletal asset with a greybox fallback. Final quadruped rig and remaining hostage variants are pending. |

## What the exported playtest files establish

| Browser GLB | Skin joints | Exported clips | Body primitive rest bounds |
|---|---:|---|---:|
| Snake | 16 | Idle, walk, run, crouch, prone, down | 183.2 cm |
| Gray Fox | 16 | Same six plus captive | 182.0 cm |
| Guard | 16 | Same six | 181.6 cm |
| Shoot Gunner | 18 | Same six plus address, shoulder, recoil and search | 179.5 cm |

The bounds measure the full vertical extent of the dressed body mesh in model space. Anatomical height needs a separate sole-to-crown landmark on a neutral body. The current exports use vertex colours, body and parent joint weights, and whole-hand and whole-foot joints. Shoot Gunner adds a jaw and a brow. They support blockout movement, encounter timing and sightline tests. Fingers, eye direction, facial expression, twist distribution, textured skin and character-specific proportions need dedicated work for close shots.

The browser repository contains the mesh, skeleton, animation source and GLB exporter. The playtest archive contains the four exported GLBs. The Unreal branch contains actor/animation integration and mesh import paths. This audit found no checked-in production character skeletal mesh or animation blueprint asset. A local Unreal import, retarget, socket inspection and the same recorded scene in each engine are required to establish visual parity.

## Evidence

- Original cast, roles and relationships. [Konami, Metal Gear character archive](https://www.konami.com/mg/archive/mg/chara.html)
- Later-era physical profiles for Solid Snake, Gray Fox and Big Boss. [Konami, Metal Gear 2 character archive](https://www.konami.com/mg/archive/mg2/chara.html)
- Original 1987 manual, weapon table, mercenary descriptions and TX-55 dimensions. [Japanese MSX2 manual scan](https://www.ne.jp/asahi/hzk/kommander/mg1manual.pdf)
- Project measurements and rig inspection. [Greybox dimensions](../../assets/asset_dimensions.json), [body and motion source](../../browser/assets/field_rigs.js), [GLB exporter](../../browser/tools/export_field_rigs.cjs)
