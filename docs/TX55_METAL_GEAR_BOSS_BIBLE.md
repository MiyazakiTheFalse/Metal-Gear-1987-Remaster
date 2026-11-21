# TX-55 METAL GEAR — ENGINEERING CATASTROPHE / BOSS BIBLE

**Version:** 1.0, 19 November 2026. **Status:** design/implementation contract, NOT a finished rig, set, cinematic or functional Unreal boss. Separate from Shoot Gunner vertical slice. Authority: global Doctrine 000, 002 event, 003 pacing, 004 integrated experience, 005 spatial structure and 1995 continuity treatment.

## Core identity and canon

The original Metal Gear TX-55 is the strategic threat of 1995 Outer Heaven. **In 1987 MSX2 play, it is stationary in the confrontation:** the danger is a sophisticated, security-laser-guarded facility and the precision with which Snake must neutralise a world-changing machine. Do **not** replace the 1987 original with a generic walking mech firefight. The physical destruction of TX-55 is the technical climax; **the subsequent 'Big Boss' battle is the human climax**. The hidden production truth about Venom Snake does not become overt in Snake's 1995 narrative.

Canon classification: **C87:** stationary sabotage target, exactly sixteen leg explosives in a precise order; Madnar's first fifteen directions and forgotten sixteenth; security laser hazards; destruction precedes final human confrontation. **CL:** later-series Big Boss/Venom identity. **I:** high-resolution materials, plausible damage, industrial ambience and camera/art direction. **U:** exact Connect2Media J2ME implementation, detailed engineering dimensions, activation technology.

## The signature puzzle — unaltered design spine

Sixteen accepted explosive placements:
**RIGHT, RIGHT, LEFT, RIGHT, LEFT, LEFT, RIGHT, LEFT, LEFT, RIGHT, RIGHT, LEFT, RIGHT, LEFT, RIGHT, RIGHT**.

Madnar tells Snake **only the first 15**; his incomplete memory makes number 16 an earned decision. This sequence is source-checked with two independent MSX2 guides. The player can replay the fifteen in a log; critical intel is never locked behind a cutscene that can be skipped. A modern HUD must not auto-indicate the unknown sixteenth.

Each leg has a real, spatially credible service/structural weak point. The character **places** a charge by hand with a responsive, contact-verified action. The authoritative state machine validates sequence and inventory once; cosmetic blast/armature states follow it. Invalid action gives unmistakable, fair feedback and does not silently consume a correct-progress slot. Exact reset/damage policy is a tuning hypothesis pending playtests.

**No softlocks:** explosives required for a valid solve must be available or recoverable; a player must not permanently lose the encounter by trialing the unknown last placement. Failure, reload, skip, inventory and pause must not duplicate or corrupt progress.

## Visual and spatial direction: a boss that hardly moves

**Approach:** workers' marks and supply manifests reveal a tremendous industrial programme. Light from the hanger reveals armour underside and heavy gantry structures before Snake can see the full silhouette. Use restraint: a still giant is more frightening if the world is built to contain it.

**Reveal:** one justified, patient camera move traces Snake's scale against TX-55's feet, then up to the machine's head. Show the laser scanners, the weak-point neighbourhoods and a credible player route. End at a camera match to gameplay, not by teleporting Snake or waiting for a fake 'boss starts' flag.

**Sabotage:** the player studies repeated but changing security sweeps, crosses lanes, places charges, retreats as damaging energy moves through joints/actuators. Visually accumulate structural deterioration by real accepted hits; no model-wide arbitrary colour flash. Audio conveys cable tension, unstable power, real spatial sound and imminent laser discharge. Respect mute/caption alternatives and colourblind leg distinction.

**Last charge:** maintain narrative uncertainty and player control. The final direction is not handed out by an off-screen companion to avoid challenge. The game acknowledges the engineering consequence immediately after a valid placement.

**Catastrophe:** make the damage *physical*. Important components lose alignment, hydraulics fail, massive weight shifts, gantry responds, alarm lights switch to emergency. The explosion is the earned culmination, not a random volume spike or twenty-second video that stops the game being interactive. Then begin the escape/final confrontation sequence without confusing the player about who betrayed them.

## Art, engineering and movement tools: asset vetoes

Accept original/licensed **TX-55-specific** silhouette only—not Metal Gear D, REX, RAY, ZEKE, Sahelanthropus or a future suit. Use modular 3D rigged/rigid subassemblies, separate LODs, metal/paint/rubber/hydraulic material breakup, actual panel geometry and sockets for 2 charge anchors. Mechanical motion is justified only by functioning parts: sensors, weight transfer, failure and deformation **do not imply** the original machine could walk here.

Minimum asset package: hero TX-55 mesh and lower LODs; charge-placement anchors; engineering material library; service gantry; laser-camera tracking hardware; accessible laser charge telegraphs; reversible visible damage stages; heavy collapse and smaller debris effects; machine ambience and alarms; lighting/exposure map; interaction poses; save-resumable destruction timeline; UI log. Every source asset carries provenance, reuse permission, scale/rig validation, material integration, 720p recognisability and measured budget.

Technical design must be deterministic and independent of cinematic timeline: inactive → discovered → sabotage progress 0..15 → destroyed → alarm → human confrontation. Asset movement never writes authoritatively to progress. UE Sequencer reads state and plays matching presentation. If Unreal isn't compiled, this remains unverified regardless of authored source quality.

## High-stakes interaction and pacing contract

The player should always have an observable safe response to approaching lasers, including when entering/exiting inventory and camera framing. The exact safe window is tested, not guessed. Forcing damage during a cutscene is prohibited. Responsive placement, interrupt handling, cover, direction and timing are as important as visual quality. No automatic aim, QTE-only boss, unexplained random sequence, 16-hit traditional health bar or an unearned final revelation that Venom is a double.

Metrics to collect in a true UE5 test: charge input-to-contact feedback (initial candidate under 120 ms), visible leg/charge registration, failed placements per stage, laser hit fairness, player ability to access Madnar notes, safe recovery when out of ammo, skip parity, camera occlusion, 720p identity, framerate and player satisfaction. **Targets are hypotheses, not measured proof.**

## Story and art quality gates

- Independent first-time players understand why this machine matters before entering the room.
- The original sabotage sequence is respected; only a correct sixteenth placement destroys TX-55.
- Madnar's fifteen-step clue is earnable, persisted and readable without hearing dialogue.
- All hazard geometry, cast animation, lighting, impacts and audio communicate the same state.
- The model is unmistakably TX-55 in anonymous gameplay footage without label.
- The physical climax, then Venom/Big Boss's personal climax, retain distinct dramatic meanings.
- The final sequence survives save/load, restart, cinema skip, multiple difficulty settings and low-spec 720p.
- Unlicensed art and contradictory later-era costume/technology are rejected before production.

**Evidence:** https://www.konami.com/mg/archive/mg/chara.html ; https://thesnakesoup.org/metal-gear-msx-walkthrough/ ; https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/65155 ; https://www.youtube.com/watch?v=ehlAMTh42Ko . The source order is a design baseline for target J2ME only pending direct archive inspection.
