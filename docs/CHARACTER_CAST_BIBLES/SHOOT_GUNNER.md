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

## Source-confirmed boss durability and performance direction — 19 November 2026

**Observed original MSX2 difficulty (guides):** 10 accepted handgun hits or approximately 4 direct remote-controlled missiles defeat Shoot Gunner; one guide also lists a single rocket. See [The Snake Soup's MSX2 walkthrough](https://thesnakesoup.org/metal-gear-msx-walkthrough/) and [Wilson Lau's MSX guide](https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/7524). The full original attack is 3 damaging spread blasts per firing sequence. These sources report *hit counts*, not an official underlying health integer.

**Gameplay production contract:** retain 100 normalised health, **10 effective health damage per accepted handgun hit against this boss only**; guards retain their normal 26-damage prototype hits. Future guided remote missile direct hit would do 25 effective damage (four successful hits), **NOT YET IMPLEMENTED**. No automatic scaling that makes players spend excess ammunition to satisfy an arbitrary scene duration. A shot counts only when the physical muzzle ray actually hits the boss; no cinematic invulnerability or hard 'three-phase lock'. Raw hit-count equivalence is a design default pending controlled UE5 playtests, not an invariant across every difficulty or headshot/alternative weapon design.

**Canonical character evidence:** original MSX script introduction identified in [teflon's boss guide](https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/35361) is a first-person boast that nobody has escaped the prison. What is **not** established: his civilian name, family, ideological convictions, childhood, exact rank/age, romance, canonical accent, exact modern shotgun/KS-23 model or detailed inner monologue. Do not invent these as canon. Original 1987 presentation is outsized and almost amusingly direct; faithfully keep its memorable self-identification rather than making him a generic silent elite soldier.

**Our proposed personality (I; remake authored):** fierce professional pride in an **apparently inescapable detention system**. He is self-important and unembarrassed about proclaiming his own name, yet his body language demonstrates genuine competence. This is the first truly memorable humiliation of Outer Heaven's institutions: the rookie has reached and reclaimed the very equipment he confiscated. His arrogance comes from confidence in procedure, range control, and the gun he uses every day—not supernatural awareness. He treats Snake as an escaped captive with nothing but nerve; **he must not know Snake intended to get captured**.

**Performing the emotional arc:** before attack he has a practised, almost ritualistic stance; after a clean dodge he tries to correct the angle, after the recovered handgun causes the first wound his gaze briefly flicks toward the breached recovery room, and below ~70% / ~35% health the *presentation* can communicate mild irritation followed by unwanted disbelief. **Do not secretly speed up untelegraphed attacks based on health**, and never announce a later-era retcon in his dialogue. The exact health thresholds are provisional acting triggers, not mechanical lockouts.

**Voice and timing:** the boss intro can be theatrical and deliberately slightly ridiculous, then gun handling sharply controlled and brutal. Avoid scripted cheap shots during an uninterruptible introduction. The player should laugh at the audacity of his introductory boast, then respect his dangerous three-wide-shot pattern. Dry humour comes from the contradiction between his professional confidence and Snake's survival, not a clown costume or exposition. The camera can hold on his rechecking an unexpectedly damaged door rather than giving him three paragraphs explaining his feelings.

**Unverified source mechanic:** speedrun-community discussion reports that killing Shoot Gunner is not mandatory to finish the original MSX mission; current slice prototype **does require his defeat for Card 3**, which may be a fidelity problem. Review TAS / original game evidence and exact J2ME source before classifying mandatory boss defeat as canonical. Preserve as a source audit issue, not a new fact asserted by the remake.
