# Solid Snake 1995 — Three-Strike Melee Implementation

**Implementation status: Unreal source committed, no UE editor compile / animation asset bake / playable build validation.** The MGS1-style **left punch → right punch → right roundhouse kick** is a modernized movement-language addition to the 1987 fan-remake; it is *not* an assertion that the J2ME or original 1987 MSX2 game already had this combo.

## Actual input and damage rules

One melee button starts the left punch. A second press buffers the right punch; a third press buffers the roundhouse finisher. Two rapid presses can be buffered during the first animation, so reasonable triple tapping still works without requiring frame-perfect rhythm. Four or more presses do **not** produce an infinite combo. Stopping after either punch is allowed. Wall cover retains the contextual wall-knock action instead of punching straight through a wall. Choke, body drag, aiming and the remotely controlled missile cannot execute simultaneously with the melee attack.

| Stage | Visual intention | Total duration | Contact window | Damage | Enemy reaction |
|---|---|---:|---:|---:|---|
| 1 | Left straight punch, small torso twist | 0.34 s | 0.105–0.190 s | 24 | Leftward stagger |
| 2 | Right straight punch, contrasting torso twist | 0.36 s | 0.125–0.210 s | 26 | Rightward stagger |
| 3 | Hip-driven right roundhouse kick | 0.56 s | 0.235–0.340 s | 36 | Physical 65 cm swept shove, 1.25 s knockdown |

Only one hit can resolve in each stage. The contact is checked when the actual stage time passes its authored contact start, never at button press, and survives dropped frames. A missed punch/kick causes no damage; the close-range test rejects wrong-floor enemies, enemies behind Snake and enemies behind geometry. A finishing kick knocks a conscious guard down and interrupts its fire briefly; it does not magically kill every opponent. Knocked-out bodies remain physically in the world and can still be dragged or discovered. The first two attacks have a small asymmetric guard response rather than the same generic reaction.

**Movement & player control:** Melee slows Snake temporarily to 100 cm/s and blocks contradictory sprint, aim and gunshot input. Short recovery follows a completed beat (0.17 s) or kick (0.25 s). No artificial steering/animation jitter; the player regains movement afterwards. Times are authoring defaults and must be tuned with frame-rate, gamepad, animation and latency testing.

## Animation and visual implementation

Current C++ fallback adds four articulated prototype limb meshes to Snake's capsule, with independent left/right punches, torso twist, bracing leg and a sweeping kick. The guard prototype flinches left/right, falls and recovers on its own temporary timer. **These blockout motions are not finished full-body animation.** No licensed realistic Snake model, skeletal attack assets or animation blueprint has yet been imported/verified.

When the real 1995 rig is available, the existing `AMGPlayerCharacter` exposes `GetMeleeStrikeIndex`, `GetMeleeStrikeAlpha`, `IsMeleeComboActive` and `IsMeleeHitWindow` to the animation blueprint and provides editable montage slots `LeftPunchMontage`, `RightPunchMontage`, `FinisherKickMontage`. Link the imported authored montages to those slots; the prototype limbs automatically hide when a valid skeletal mesh is assigned.

**Animation production brief:** Two different planted-foot punches, authentic alternating shoulder and hip mechanics, a grounded pivot into the kick, foot contact at the gameplay active frame, separate small recoil and large finisher knockdown for guards, no floating root, no accidental rotation through cover, consistent first-person and third-person hit reactions. Add hand/foot socket alignment, facial and breathing layers, appropriate clothing dynamics, audio and impact reaction clips before declaring a production-ready animation pass. Keep all assets provenance-checked under the art catalogue.

## Regression and acceptance

`python -m unittest tools/test_three_strike_melee_contract.py` performs source contract and timing checks; it is not a UE compile. A UE tester must verify:
- quick triple tap at 30, 60 and 120 FPS is three attacks, not one or a fourth;
- spaced first/second inputs allow a two-hit sequence; stage 3 is always a kick;
- tap outside guard reach, into solid wall, or while prone/dragging does not produce phantom damage;
- only hit-window frames deal damage; hit-stop/recovery don't lock control;
- third kick displaces and knocks down *only* when it really lands;
- physical back-against-wall cover retains wall knock; no overlap with drag/choke;
- animated rig foot plant, body rotation, weapon carry, audio cue, and HUD response match contact timing.

Sources for **MGS1** combo structure: the *Metal Gear Solid* PlayStation manual describes punch/kick input; period MGS1 guides document left punch, right punch, right kick. https://www.gamesdatabase.org/Media/SYSTEM/Sony_Playstation/manual/Formated/Metal_Gear_Solid_-_1998_-_Konami.pdf ; https://gamefaqs.gamespot.com/ps/197909-metal-gear-solid/faqs/10010
