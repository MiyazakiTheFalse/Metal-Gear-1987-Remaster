# SHOOT GUNNER / SHOTMAKER — CHARACTER / PERFORMANCE BIBLE

**Scope:** 1995 Outer Heaven, full-game production; source-authored and unverified in-game. This is neither a complete cinematic nor a claim of finished character assets. **Priority hierarchy:** Doctrine 000, continuity treatment, then character intent. Source labels C87 / CL / I / U as defined in the cast index.

## Who this character is and what they can know
Original early shotgun boss; original MSX2 name Shoot Gunner, later alias Shotmaker.

**Dramatic function / internal conflict:** Intimidating security professional who teaches survival, tactical withdrawal and reclaimed-gear combat.

## Art and assets: visual recognition, materials and rig
A distinct recognisable 1995 mercenary, proper two-handed shotgun rig, reload and muzzle sockets.

**Production acceptance:** face and costume provenance; era-correct distinct silhouette at 720p; high/low LOD and PBR integration; suitable rig; correct size and equipment; animation-source licensing; no ripped or unverifiable fan asset. Decide against fashionable but incongruent animation, even where impressive in isolation.

## Acting, movement, voice and cinematic treatment
Controlled stride, forearm/shoulder recoil, readable pumping and recovery; avoids stand-and-fire turret behaviour.

**Source-informed scene priorities:** Full boss intro after prison escape, physically dangerous fight, equipment room and Card 3 aftermath.
Blocking, silence, eye lines, gestures and camera grammar must tell the same story. Snake's repeated question is an authentic voice trait; use it with distinct intent (surprise, doubt, information gathering, dry humour), not as filler. Every new line must be tagged as direct-source or new remake writing.

## Gameplay authority and vetoes
Do not spawn gun in confiscated Snake's hands or shoot Snake during a forced cutscene; transmitter must be addressed.
Animation never fabricates gameplay state or overrides validated mission progression. Cutscene skip and interruption must leave identical playable inventory, location, guard knowledge and flags.

## Actual review and completion gates
A first-time player identifies safe windows and the route to recover gear without brute-force deaths.
**Not yet met:** source-edition line-by-line confirmation, physical face/rig/weapon render, cinematic blocking test, animation/voice sample, UE5.7 compile, captured 720p comparison, save/skip failure test and unfamiliar-player comprehension test. Remain **unverified** until evidence exists.


## 19 November source audit — signature weapon and boss combat

MSX2 Japanese manual background: former Soviet Spetsnaz operative, specialising in **riot guns**; exact make/model **not canonical**. Reference: https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/17103 and scanned Konami 1987 manual: https://www.ne.jp/asahi/hzk/kommander/mg1manual.pdf .

**Verified 1987 gameplay via MSX2 guide:** boss makes **three spread shotgun blasts in a row** and moves about the room. The original blast projectiles are large and damaging; the source does not mandate a real-world reload interval, specific number of pellets or a concrete weapon weight. https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/35361 and https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/65155 .

**Redesign hypotheses:** 5 physically traced pellets at up to ±6 degrees, a 0.78-second aim cue, three shots 0.52 seconds apart, a 1.85-second burst recovery. All are **playtesting candidates**, not reconstructed 1987 numeric values. Do not give the boss instantaneous aim tracking through walls, or unblockable damage during cinematics. The hero handgun is regained in a breached side room; original remote-controlled missiles remain a later, desirable fully playable equipment feature. Source actor: Unreal / MGShootGunnerActor. Original editable weapon candidate: [riot_gun_proxy.obj](../../assets/source/shotmaker/riot_gun_proxy.obj).

**Community response:** fans often joke about his literal original name, but others want a remake to turn him into a punishing large-gauge breacher who forces range control. Treat that as a creative suggestion rather than canon. https://www.reddit.com/r/metalgearsolid/comments/1b3scn6 .


### Physical weapon-cycle addendum (source prototype)

A single animation/3D firearm must make each shot, recoil and pump cycle spatially consistent. The proposed interval is now 0.52 seconds (playtest variable, **not** 1987 cadence), allowing the authored 0.38-second pump audio after each blast. A Blueprint-implementable OnPumpAction cue marks the completed shot. A short muzzle-origin point light is authored directly on the actor; actual mesh muzzle/hand/socket agreement and recoil animation are outstanding. Enemy muzzle must physically face the committed aim point before its spread rays can fire; a turn can extend a telegraph instead of snapping a shot through the side of the model. All numeric dimensions remain interpretation. 
