# SOLID SNAKE — 1995 STEALTH-ESPIONAGE PROTAGONIST BIBLE

**Status:** BINDING design/production contract, **not** a claim of finished 3D assets, animation or a playable build. Version 1.0 — 19 November 2026.
**Owner:** Character direction + gameplay + animation + engineering + QA, reviewed together.
**Story:** *Metal Gear* (1987), Operation Intrude N313 / Outer Heaven in 1995; first reference target is the supplied Connect2Media J2ME edition, still awaiting complete direct extraction/verification.
**Authority:** [Doctrine 000](DOCTRINE_000_HOW_WE_BUILD.md) is supreme. Also binding: [Doctrine 002](DOCTRINE_002_EVENTS.md), [Doctrine 003](DOCTRINE_003_PACING.md), [Doctrine 004](DOCTRINE_004_INTEGRATED_EXPERIENCE.md), [Doctrine 005](DOCTRINE_005_SPATIAL_STRUCTURE.md). This bible may not override those doctrines.
**Machine-checkable companion:** [data/solid_snake_character_contract.json](../data/solid_snake_character_contract.json). Run `python tools/validate_solid_snake_contract.py`.

## 0. The doctrine governing Snake

> It is not only *what* he does; it is *how* the player makes him do it, how he carries it out, and how the world responds.

Snake must be unmistakably Solid Snake, a capable but relatively inexperienced FOXHOUND infiltrator in **1995**. The player must believe his body occupies space, his actions have physical causes, his equipment is real, and guards respond to evidence rather than game logic. Everything visible, audible or controllable must agree with the mission state. Hero animations are never merely decoration. Their timing, silhouettes, contact surfaces, sounds, camera and consequences must form one action.

**Definition of “finished protagonist”:** Authored, distinguishable face/outfit/rig; 8-way locomotion; credible start/stop/turn, crouch, prone, aim, weapon grip, cover, interaction and melee; skin/fabric/metal PBR materials; IK and collision; animations integrated to gameplay events; source/permission provenance; three quality presets including a performant 720p path; browser character identity where applicable; tested on real hardware with captures and representative new players. A code path that can load a skeletal mesh is **not** completion.

## 1. Truth, continuity, and interpretation hierarchy

Every art or animation choice gets a tag:
- **C — Canon/source-supported:** source-game or official Konami reference verified; cite URL/game/file and what it proves.
- **R — Later-series comparative reference:** useful for identity, directing, camera grammar or production quality; **not** evidence of a 1995 costume or technique.
- **I — Remake interpretation:** historically plausible and gameplay-consistent but not attested in the target release. Easy to revise without breaking code.
- **P — Prototype:** expedient greybox or debug representation. Forbidden from a “complete” character presentation.
- **U — Unverified:** missing or contradictory evidence; preserve the uncertainty in the asset ledger.

### 1.1 Canon anchors

| Anchor | Classification | Production implication |
|---|---|---|
| The original *Metal Gear* takes place in 1995; Solid Snake, Big Boss and Gray Fox are the principal character identities | C — [Konami official history](https://www.konami.com/mg/history/jp/ja/mg) | Model **young Solid Snake**, not Old Snake, Big Boss or the Cyborg Ninja |
| FOXHOUND operative enters Outer Heaven to investigate its threat; Gray Fox is missing/captive | C — original Metal Gear history and story; compare [Konami character archive](https://www.konami.com/mg/archive/mg/chara.html) | Reconnaissance and rescue are credible priorities |
| Big Boss is Snake's superior and later becomes his adversary | C — [Konami archival timeline](https://img.konami.com/mg/mc2/s/img/top/book_sample_mgs4_en.pdf) | The early Snake can respect him; avoid prematurely revealing the later betrayal |
| Prison rescue, two weak barriers and present cell layout | P/U — current project adaptation and source greybox | Never call current geometry confirmed J2ME original |
| Later MGS camera, mocap, animation fidelity, head expressiveness | R | Benchmark craftsmanship without importing later-era equipment or character age |
| Specific belt, fabric weave, headband, pouch count, boot tread, firearms, suit seams | I/U unless verified individually | Design language subject to reference review |

**Do not silently import:** 2005 Shadow Moses sneaking suit, 2007 Tanker kit, 2009 Pliskin guise, 2014 OctoCamo/Solid Eye, Cyborg Ninja exoskeleton, MGSV Diamond Dogs insignia or later-period electronics. No default supernatural foreknowledge. The protagonist's future exploits are *not* a licence to make 1995 Snake omniscient.

### 1.2 Who is the playable Snake?

- **Identity:** Solid Snake, the sole playable protagonist of this vertical slice. A labelled HUD alone does not establish identity: neutral portrait, readable face, hair silhouette, uniform, posture and cutscene framing must corroborate it.
- **Career:** Trained FOXHOUND operative, effective at stealth and improvisation; not a bored superhuman veteran. His restraint, economical foot placement and cautious scanning should distinguish him from an ordinary sentry.
- **Mindset:** Alert, analytical, persistent, resourceful. Capable of mistakes; player mistakes must yield understandable consequences, not contemptuous character barks.
- **Dialogue/directing:** Calm and concise; asks focused questions. No late-series catchphrases inserted merely for familiarity. Respect early-game source and the player's own deductions.
- **Relationship to Gray Fox:** He rescues a more experienced captured operative, listens and protects mission continuity. Fox's condition must be evident without forcing a long exposition lock.
- **Relationship to Big Boss:** Initial chain of command matters. Subtle direction can seed later dramatic irony without spoiling it within the rescue slice.
- **Physical proportions:** 178 cm current **provisional later-game reference**, not verified target-game measurement. Match collision capsule and physically grounded hands/feet.
- **Age/appearance:** young adult in 1995. Source-reference head and hair study required. Facial hair, eye specifics, exact haircut and individual features are an **art-direction sign-off**, never assumed canon from distant sprites.

## 2. Visual identity and art direction

**Silhouette first:** At gameplay distance, distinguish Snake from a guard via face/hair outline, uniform tailoring, head/neck position, stance and body language. At portrait distance, build a specific young Snake face that passes a blind-recognition test without visible names. Avoid a generic soldier model with a Snake label.

**1995 costume:** Muted functional olive/green field clothing, period-plausible webbing, compact gear and durable field boots; all precise cuts, fastening, insignia and pouch arrangements are interpretive until source-confirmed. Disarmed/captured Snake is *the same person in the same base uniform* minus confiscated kit. Dirt, sweat, creasing, bruise and wear may accumulate through events; do not invent injuries or blood state not caused in play.

**Gray Fox comparison:** Fox is visibly a different person, not the same face shader tinted blue. In the prison: compromised fatigue, restrained movement, distinct blue-toned detention wear as a comparative source interpretation, no cyborg armour. **Guards:** occupational uniforms, varied silhouettes and plausible equipment without confusing the hero, no universal palette-swapped Snake.

**Texture and shader specification:** Anatomically plausible skin with appropriate subsurface scattering; normal and roughness breakup without wax/plastic effect; fabric micro-weave follows scale and seam direction; stitching and wear placed causally around knees, elbows, cuff edges and gear friction; non-metal materials use non-metallic response; metal fasteners and weapon surfaces use restrained roughness/edge wear, not uniform chrome. Prioritise source fidelity, legibility at 720p and temporal stability over indiscriminate 4K maps.

**Art selection rule:** Download/reuse/modify the best source only after checking (1) silhouette potential, (2) facial topology, (3) body proportions, (4) skin weighting, (5) UV coverage, (6) texture and material range, (7) animation interoperability, (8) right-hand/left-hand socket viability, (9) LOD potential, (10) exact author/asset licence, provenance and allowed modification/distribution. Reject a high-resolution mesh if it deforms badly or cannot act. Record acquisition URL, creator, version, hashes, changes and source class. Non-commercial status does not waive permission review.

## 3. Model, skeleton and technical asset contract

| Property | Target / how |
|---|---|
| Real character geometry | Import a **USkeletalMesh**, not a StaticMesh cylinder or procedural limb chain |
| Character variants | `snake_mg1_1995_infiltration`, `snake_mg1_1995_disarmed`; share compatible skeleton and animation identity |
| Visible hand socket | `weapon_r` oriented for an attached prop; true relative transform measured in engine |
| Support joints | Pelvis/root, spine segments, clavicles, neck/head, shoulders/elbows/wrists, finger bones, legs/knees/ankles/toes; optional twist joints |
| IK | Left/right hand and feet effectors or retarget chains, floor contact, two-hand weapon alignment, hand-on-wall alignment |
| Facial performance | Minimum distinct brow/eye/jaw/mouth shapes or facial controls and look-at; mouth motion tied to actual dialogue, not random looping |
| Import space | Centimetres; root forward axis and humanoid rest pose documented; actual capsule remains authoritative |
| Mesh/collision | Capsule controls gameplay; skeletal visual obeys appropriate visibility traces but never creates new gameplay collision advantage |
| Clothing | Consistent weight paint across shoulders, pelvis, knees and elbows; no clipping during crouch/prone/aim/cover/KO |
| LODs | Hero portrait/close gameplay/mid-range/720p; author LOD rather than sacrifice readability; budget based on captured worst-case frame |
| Material channels | Base colour, normal, packed occlusion/roughness/metallic; transparent or subsurface maps only where warranted |
| Animation interchange | Retarget source animations to a stable skeleton, preserve source curve/notifies and licence; same identity in Unreal and browser-optimised GLB |
| Hero variant swap | Disarm/gear removal preserves actor, health, flags, equipment, input and rig motion continuity |
| Engine art location | `/Game/Characters/Snake/`, with authored `SK_Snake_1995_Infiltration`, `SK_Snake_1995_Disarmed` and `ABP_Snake_1995` |
| Truth rule | If the asset, AnimBP, socket or pose is absent, status stays UNVERIFIED; visual fallback may keep gameplay debug-playable |

**Working code status (2026-11-19):** This branch wires UE skeletal mesh loading for Snake, guard and Fox; presents anim-state values; adds optional right-hand weapon attachment and aim pitch/yaw/recoil values. There are **no confirmed imported hero meshes, pose assets, IK rig, functional upper-body aim graph, hand/weapon grip animations or tested UE executable**. Existing feedback and visual proxies are prototypes. Never mark any of these as delivered because the bridge compiles in theory.

## 4. Input philosophy and arbitration

An input is a **player intention**, then a contextual affordance, then an authoritative action, then a synchronised audiovisual response. Controls must be discoverable, predictable, remappable and forgiving. Different physical controls may express the same intention; distinct intentions must not trigger each other unexpectedly.

**Current source bindings** (confirm through `unreal/MGRemaster/Config/DefaultInput.ini` before revising):

| Intent | Keyboard/mouse | Current gamepad | Priority and feedback |
|---|---|---|---|
| Move / camera | WASD / mouse | Left / right stick | Independent camera control; acceleration is physical, never a menu delay |
| Aim / fire | RMB hold / LMB | LT / RT | Character upper-body and camera respond together; fire only with valid equipped weapon |
| Sprint | Shift | Left stick press | Suppressed by aim/crouch/prone and disengages cover |
| Crouch | Ctrl | B/Circle | Smooth stance with checked capsule and uninterrupted camera |
| Prone | Z | D-pad down | Body clearance before entry/exit, no passage through walls |
| Cover | V or contextual | Contextual | Non-breachable walls only; must never steal E/X breach intent |
| Knock / melee | F | A/face-bottom | At valid wall cover: knock; otherwise melee, each with own spatial cause |
| Interact / breach / rescue | E | X/face-left | Eligible mission interaction beats optional cover handling |
| Shoulder swap / radio tuning | Q | Right stick press | In radio: channel; outside: shoulder; signposted |
| Transceiver | C | Start/left special | Non-pausing private communication; no magical guard awareness |
| Equipment windows / use | G, Tab, Y | RB, LB, Y | Cannot invent a carried weapon while confiscated |
| Log / restart / graphics | J, F9, F6 | D-pad up / debug / menu pending | Debug restart not a normal player-facing feature |

**Input arbitration:** User's explicit interact on a weak mission wall takes precedence over automatic wall cover. In cover, melee-button may knock; outside cover, it is melee. No invisible weapon fire when inventory is confiscated. A button during a pose transition is queued or rejected with clear feedback according to transaction eligibility—not dropped silently. A player's quick reaction may finish a puzzle before a designer's cutscene finishes speaking.

**Remapping/accessibility:** Support hold/toggle aim, hold/toggle crouch where feasible, stick dead zone and sensitivity, inversion, camera-side preference, input buffering, auto-repeat resistance, subtitles for audible cues and reduced shake. Alternative controls must not change AI detection or narrative outcomes.

## 5. Core locomotion — full-body specification

The character root moves through the movement/physics model; animation then conveys that movement truth. Avoid silent speed cheats, pose popping, reverse-foot skating, turning before feet can support it, camera lurches, impossible stops or unintentional target loss.

| State | Visual animation | Player-facing rule | Collision and audio |
|---|---|---|---|
| Relaxed grounded idle | Resting shoulders, subtle breath, weight transfer, contextual head scan | Alert but not exaggerated | Feet planted; ambient cloth/gear subtle only |
| Quiet walk | Heel-to-toe controlled gait, short arm counter-swing, gaze scanning | Precise stealth, responsive stops | Footstep material and true foot contact |
| Normal run | More forward pitch and foot clearance; weapon/gear secondary motion | Distinct from sprint | Loudness depends on actual gait and surface |
| Sprint | Athletic full-body stride, stable head, readable sudden deceleration | No sprint while prone/aiming; cannot wall phase | Foot placement, collision and breath cues |
| Direction changes | Starts, stops, pivots, strafe and diagonals, feet plant under centre of mass | Camera never secretly rotates Snake to cheat aim | Turn in place when needed, anti-skate |
| Crouch | Hand/torso/pelvis lower in one continuous supported transition | Can aim and move; no silent pose-capsule mismatch | Knee clearance and slower footstep envelopes |
| Prone | Grounded torso/forearms, controlled forward/back/lateral crawl | Cannot crawl through narrow gaps unsupported by full body sweep | Swept body volume, clothing friction |
| Wall cover | Hand-wall contact, shoulder-to-surface distance, no magnetic snap | Learnable approach, fast release | Geometry and wall normal own the pose |
| Cover shuffle | Heel/hip repositioning tangent to wall, head checks angle | Left/right input maps consistently | Surface friction/shuffle audio not gunfire |
| Corner peek | Pelvis near cover; shoulders/eyes lean around; camera collision checked | Aim and peek reveal only what is physically visible | Exposure and guard LOS agree; elbows don't pass wall |
| Broken-wall crossing | Clear foot placement and obstacle adaptation | Fast solver retains agency | True collision opening after breach, persistent rubble |

**Directional locomotion:** Base 8-way blend, additive turn-in-place and start/stop transitions. Test 45°, 90° and 180° changes; quick reversals and diagonals; variable FPS, shoulder switching, near-wall pivots, stair inclines, doorway frames, low ceiling and uneven debris.

**Movement data vs animation:** Engine speed is not authoritative art timing. Current source uses approximate movement speeds (walk 350cm/s, crouch 170, prone 95, cover 120, sprint 600). Those are tuning candidates, not natural gait measurements; compare capsule displacement, stride cycles and player tests before approval. Footsteps are dispatched by contact not arbitrary per-second timers. At 720p, pose must remain legible.

## 6. Aiming — full-body + hands + weapon + camera contract

Aiming is an integrated *physical* action, not just a lower FOV and central reticle.

1. **Input:** LT/RMB begins aim at once; no forced stationary delay. If unarmed, show an honest ready/unarmed pose, never an invented pistol. Equipped hand prop depends on the actual inventory state.
2. **Pose onset:** Hips keep supporting locomotion; clavicles/torso/neck rotate coherently toward camera line; weapon hand goes into a deliberate grip; left hand supports where appropriate. Avoid aiming without moving the shoulder girdle.
3. **Aim direction:** Camera control rotation feeds smoothed local pitch and yaw. Additive aim offset layers above locomotion, with yaw limits tighter when moving. At an extreme angle, rotate the feet/body instead of corkscrewing the spine.
4. **Weapon:** Render an actual weapon mesh socketed to right hand (`weapon_r`) with measured muzzle transform and sensible grip scale. **Left-hand IK locks to the support/contact point** where the weapon requires it; fingers have a credible trigger grip. Pistol and unarmed variants are distinct.
5. **Sighting:** Over-the-shoulder view keeps weapon, hand and reticle legible without hiding close cover; shoulder switching respects collision; no camera through solid geometry.
6. **Line of fire:** Camera ray finds intended target; muzzle/character ray verifies actual unobstructed shot. A near-wall weapon must lower/offset or flag “obstructed” rather than shoot through concrete. Weapon orientation, muzzle flash, hit point and audio must agree.
7. **Movement while aimed:** Strafe, diagonal/backward locomotion, crouch, cover/lean and stop/turn all retain coherent upper-body pose and hand grip. No torsional jumps when changing shoulder or crossing aim yaw limits.
8. **Fire:** Only an equipped weapon with ammunition commits shot/ammo event; visual recoil, muzzle cue, sound, impact and AI hearing follow **that** event. Trigger recoil is not a fake muzzle shot when no weapon exists. The current test “stun pistol” is a **P** prototype, not source-attested kit.
9. **Release/interrupt:** Aim-out returns the arms smoothly without dropping pose, even during sprint request, knock, emergency evasion, stun, codec opening, save/load or camera collision.
10. **Timing:** Target first visible pose response under ~120 ms on reference hardware (hypothesis, not measured). Capture input-to-animation, barrel alignment and camera hand occlusion with 30/60/120fps instrumentation. Do not sacrifice snappy steering merely to show the full raise animation.

**Animation graph plan:** Base locomotion state machine → additive upper-body mask → mesh-space Aim Offset for pitch/yaw → hand and weapon IK → foot IK → dynamic additive impacts (recoil, knock, breathing) → final pose; gate layers by actual inventory/stance/cover state. [Epic's Aim Offset guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/aim-offset-in-unreal-engine) specifically recommends additive aim poses and limiting yaw extremes in motion; linked animation layers can isolate weapon-specific logic [Epic animation blueprint linking](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-linking-in-unreal-engine).

**Accept:** From over either shoulder, a player can *see both hands behaving credibly*, see the actual equipped weapon (if any), and maintain the same point of aim while moving. Test unarmed captured Snake separately. A boolean `bAiming` without a validated blended pose fails this gate.

## 7. Stealth movement, awareness and meaningful guards

- Low silhouette works by measured sightline, cover geometry, illumination and reported visibility—not arbitrary “invisible because crouched”.
- Patrols have real facing, hearing and investigation locations; a wall knock records the impact location, never a telepathic player location.
- A guard should be able to notice what a person could notice: footsteps and environment noise when not occluded, a body crossing a visible opening, changed cover, a body or impact. If the system knows more than the guard, it must not act on secret information.
- A wary guard stops, looks, chooses a route/search arc, checks a plausible disturbance and resumes patrol after bounded investigation. Alert escalation includes readable voice/posture/audio without a fabricated instant response.
- Environmental art must cooperate: shadows and silhouette convey cover; cloth/boots/metal distinguish audio; material contact aligns with guard perception and player captions.
- Stealth success can be quiet. Do not force detection because a cinematic needs excitement.

## 8. Physical interaction, violence and environmental agency

| Action | Authored motion | State transaction | Playtest failure cases |
|---|---|---|---|
| Inspect a wall | Lean/head/hand at contact; subtle material test | May reveal clue; never grants unrelated mission flag | Muted audio, colour-blind cue, looking from wrong side |
| Knock in cover | Brief elbow/knuckle extension, retraction to cover | World-space acoustic stimulus; actual guards hear it as plausible | Repeat spam; blocked sound; active alert; corridor occlusion |
| Break weakened wall | Multiple distinct contact beats, accumulated cracking/resistance, decisive rupture | Wall breach flag authoritative; collision/mesh opening persists after real commit | Immediate solver, repeated E, moving away, save/reload, 30fps |
| Rescue Fox | Unthreatening approach, grounded eye line, responsive listening | Rescue commits once; neither face animation nor subtitles own flag | Player walks off, interrupts, reloads, radio opens |
| Melee | Distinct 3-hit chain; torso rotation, planted feet, contact and recoil | Damage should originate on collision/notifies when montage system exists | No phantom hits behind wall; no attack through guard; no lost inputs |
| Incapacitation | Reactions appropriate to impulse/stance; fall follows physics or staged authored safe pose | AI/health incapacitation commits once | Prop/body collision, navigation blocking, no sudden animation reset |
| Pick up/holster | Reach/grip and gear state aligned with actually owned inventory | Authoritative inventory governs prop visibility | Captured state, rapid select, zero ammo, save/load |
| Radio receive | Restrained listening gesture if safe; free navigation continues | No mandatory mission progress | Under attack, moving, non-audio user, changed channels |

**Wall breach pacing:** Not necessarily first-hit destruction. Damage can accumulate visibly and audibly with each legal action *provided gameplay never fabricates a forced timer or violates the original story gate*. Prototype currently removes a wall on first valid interaction; staged damage requires dedicated mission and asset work. A false crack animation on a wall that cannot be damaged is worse than a truthful proxy.

## 9. Micro-animation and acting library

A human-readable library, with corresponding stable IDs in the companion JSON:

**Idle:** neutral alert, combat-ready, breath exertion, injured fatigue, equipment check, audible listening, subtle eye scan, shoulder reposition, footing adjustment. Do not loop exaggerated scanning in a way that exposes player position without input.

**Start/stop:** forward/back/left/right/diagonal initiations, turn-in-place, 45/90/180 pivots, hard stop, careful stop at ledge/obstacle, sprint recovery.

**Standing locomotion:** 8-way walk, aim-walk, run and sprint, strafe, controlled backwards retreat; with distinct light/heavy equipment additive layers only if inventory actually affects mass.

**Low locomotion:** crouch down/up/idle/8-way locomotion; prone entry/exit/idle/crawl and turn; careful stand when overhead clearance fails. Avoid simulating “prone” with only a horizontal cylinder.

**Wall behaviour:** attach, detach, left/right shuffle, left/right peek, corner return, wall knock variants, wall surface inspect, two or more escalating breach contacts and rupture follow-through.

**Equipment:** unarmed idle/ready, acquire/draw/holster, one-handed and two-handed aim where actually suitable, support-hand IK, fire/recoil, reload with actual ammo state, out-of-ammo reaction, equipment confiscation animation.

**Combat:** unarmed strikes 1/2/3, feint/windup, connect/whiff, stagger, vulnerable posture, regain balance, KO; not MGSV-era trademark fighting choreography by default.

**Narrative/acting:** capture/disarm, inventory-confiscated check, confined-space exploration, first sight of Fox, approach/rescue, recognition/active listening, cautious retreat, transceiver moment, post-breach reaction, quiet recovery.

**Face/look-at:** eye targeting, voluntary gaze, conversational listening, eyelids/blink, nuanced jaw/cheek/brow, fatigue. Facial rig must not produce visible lip movement without timed speech.

**Animation event notifies:** stance contact, footstep material, hand-wall contact, weapon draw/holster, magazine insert, recoil start, muzzle discharge, strike contact, break impact, debris collision, eye-line target hand-off. Each notify must be tied to an event ID and may not double-commit mission actions.

## 10. Camera, framing, cinematography and control

- Third person is primary. Visualise arms, face and 1995 silhouette while preserving forward visibility. Close framing is never a licence to hide obstacles from the player.
- Camera follows intention: grounded smoothing, spring-arm geometry, measured shoulder offsets and collision sweeps; minimise camera pop on cover contact, falling, stance switch, aim in/out and corner peeks.
- Gameplay camera should provide *meaningful hand visibility* during aim; camera can shift across shoulders, never reveal a guard through an intact wall. Apply environmental audio occlusion and subtitles.
- Close-ups are deliberately selected: opening capture (face and confiscation), first Fox identification (gaze and recognition), mission briefing and inventory portrait. No long forced look at the protagonist when player should act.
- In optional narrative scenes, maintain subtitles, skip/interrupt recovery, no duplicate mission flags, no second “capture” when loading a checkpoint.
- Camera distance and FOV targets are data-driven and performance tested, not sacred constants. Preserve a functional aiming crosshair at all aspect ratios and 720p.
- For cinematic animation, accept control limits only where necessary to communicate a meaningful one-time event, with clear onset/exit and no action loss. Prefer playable directed beats.

## 11. Audio, haptics, material interaction and accessibility

Sound is a *physical event*: fabric rustle, breath, landing, steps, impacts, scrape and muzzle report originate from the responsible material and position. No guard “hearing” non-diegetic HUD cues. A headset user must distinguish a hollow wall, an approaching guard, and a private radio message. The same clues must be available without sound through readable seams, subtitles/captions, guard body language and controller feedback options.

- Footsteps: authored sync to true contact; gait/ground material and volume convey player decisions. Never fake stealth by globally muting footsteps when player holds crouch.
- Equipment: fabric and buckles react to posture and actual worn kit; loss of equipment removes audible jingle.
- Dialogue: source-supported character identity and plausible delivery; script source status on every line; voice priority over incidental foley without muting genuine danger.
- Transceiver: internal/private, not spatially broadcast; subtitle transcript and channel text always present; freely closable.
- Accessibility: camera sensitivity/invert/shoulder preference, subtitles, visual sound-caption option, remapping, adjustable camera shake, aim toggle where supported, readable silhouettes without colour reliance.
- A critical clue must have at least two channels, one not relying solely on sound and one not solely on colour.

## 12. Release gates — what actually proves quality

All gates are **UNVERIFIED** until there is archived evidence.

| Gate | Required evidence |
|---|---|
| A. Recognisable identity | Blind 3/4 head/body recognition in 1995 uniform at 1080p and 720p; comparisons to source references; correct disarmed variation |
| B. Imported rig | UE asset browser import logs, skeleton hierarchy, animations actually replayed in engine, verified licence/author and texture provenance |
| C. Locomotion | Captures of 8-way start/stop/pivots/crouch/prone at 30/60/120fps; foot slide and clipping report |
| D. Aim and weapons | Aiming/strafe/crouch/peek with visible arms/hands/prop/reticle, both camera shoulders, actual muzzle obstruction; confiscated state has **no** weapon |
| E. Contact and collision | Wall cover, knock, weak wall, Fox rescue and guard line of sight across full mission; no penetration or phantom hits |
| F. Narrative/cinematic | Unspoiled 1995 identity, Gray Fox visible and reactive, capture/disarm continuity, interruption, replays and saves |
| G. Audio/accessibility | Matched material events, subtitles for every critical clue, quiet mode pass, controller mapping and remap tests |
| H. Performance parity | Performance/Balanced/High with unchanged mission flags, detection logic and playable control; 720p 60fps target assessed honestly |
| I. Browser parity | Same recognisable characters and event/animation semantics, optimised GLB where available; no separate unrelated protagonist |
| J. Player study | Newcomer, experienced fan and accessibility passes; record disorientation, intent comprehension, reaction delay, stealth fairness and completion |

**Risk register:** Missing actual licensed/approved source meshes and textures; missing AnimBP graph and blend/IK pose assets; unproven UE5.7 compilation; browser prototype not synced; unverified J2ME geometry; original weapon and uniform details pending comparison; missing facial and voice work; foot sliding and prone capsule/cover desync; low-end VRAM budget. No claims of “AAA delivered” until these risks close with evidence.

## 13. Implementation order and dependency graph

1. Freeze identity/reference ledger for young Snake, Fox, guards and gear; differentiate C/R/I/P/U.
2. Acquire candidate rigs with inspectable editable licences and source assets; record SHA-256 and mesh/texture provenance. Reject unsuitable rigs early.
3. Model/modify head, haircut, 1995 clothing and disarmed appearance; maintain compatible skeleton with motion retargeting and socket `weapon_r`.
4. Establish neutral movement as real 8-way animation; validate feet, aim movement and wall contact before extensive custom cutscenes.
5. Build **AnimBP** pose graph and aim offset; two-hand IK, camera-relative aim, muzzle and impact alignment. Add reliable recoil, draw/holster and reload.
6. Add cautious stealth, crouch, prone, cover, knock and wall-contact poses. Verify collision and guard evidence end to end.
7. Add Fox's rescue acting, hand contacts, cinematics, facial controls and asynchronous subtitling with explicit event ownership.
8. Add PBR textures and staged wear, appropriate LODs/optimisation; light at 720p before approving colour or detail.
9. Export matching character identities for browser GLB, preserve common action IDs and compare first-person input-to-event and third-person readability.
10. Compile Unreal; record footage with input/audio timestamps; run tests at 30/60/120fps, new-user comprehension study and regression cases; then mark gates as passed.

**Blocked dependencies cannot be hidden behind polish.** Changes must append evidence (revision, linked asset, captured test, owner and issue) to the data/quality ledger. Every new animation and asset must meet Doctrine 000's *how* criterion.

## 14. References and authority

- [Konami, official Metal Gear (1987) history / 1995 roster](https://www.konami.com/mg/history/jp/ja/mg) — historical anchor.
- [Konami, original Metal Gear character archive](https://www.konami.com/mg/archive/mg/chara.html) — role and identity comparison.
- [Konami, Metal Gear archival character timeline (PDF)](https://img.konami.com/mg/mc2/s/img/top/book_sample_mgs4_en.pdf) — later-canon cross-reference; do not treat all later details as 1995-specific.
- [Epic, Unreal Aim Offset documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/aim-offset-in-unreal-engine) — additive aiming poses.
- [Epic, Animation Blueprint linking](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-linking-in-unreal-engine) — shared locomotion and weapon-specific animation layers.
- Internal: `docs/ASSET_ACQUISITION_AND_ANIMATION_PLAN.md`, `docs/GRAY_FOX_VERTICAL_SLICE_QUALITY_BAR.md`, `docs/DOCTRINE_004_INTEGRATED_EXPERIENCE.md`, `data/beat_contracts.json`, `unreal/MGRemaster/Config/DefaultInput.ini`.

**Update protocol:** Any change to physical identity, rig, source interpretation, input priority, pose state, aim/camera, weapon attachment or required cinematic must update this bible and the machine-checkable contract in the same PR. Never raise a gate from unverified without actual recorded evidence.


## Addendum — Snake's distinctive echo question and 1995 viewpoint (19 November 2026)

Snake often repeats a term he has just heard **as a question**. Preserve this recognisable dialogue rhythm, including dry humour, while directing each instance as a *different action*: asking for confirmation, expressing disbelief, buying a tactical second, compelling a witness to explain, or puncturing pompous exposition. It does **not** indicate he is incapable of reading a room. A question must expose an important missing fact or character relationship; a camera hold/face reaction can be more effective than adding another line. Capture the intention in the dialogue direction track.

His identity as a talented FOXHOUND rookie is never overruled by later MGSV audience knowledge: he cannot know about Venom's double or his own later legend in 1995. The real Big Boss's helpful radio guidance must remain credible early; Snake's doubts grow through **discovered contradictions**, not retroactive personality rewriting. In-engine cinema must retain the exact active Snake mesh/gear/sockets and player camera handoff. See [1995 continuity](OUTER_HEAVEN_1995_CONTINUITY_TREATMENT.md) and [cinematic shot contracts](../data/cinematic_scene_contracts.json).
