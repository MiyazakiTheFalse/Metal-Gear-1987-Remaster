# CINEMATIC DIRECTION — IN-ENGINE STORYTELLING BIBLE

**Status:** scene/production design only, not rendered Sequencer scenes, implemented cutscenes or Kojima-authored direction. Full-game and prison vertical-slice coverage. Governed by Doctrine 000 (how we build), 002 (events), 003 (pacing), 004 (integrated experience), 005 (spatial structure). Shot-level machine-readable companion: ../data/cinematic_scene_contracts.json.

## Core proposition

An MGS-style cinematic can be expansive, detailed, dramatic, even eccentric—and must *earn* that freedom through the player's preceding choices and new emotional or strategic information. **Not simply long:** every shot changes our knowledge, emotional balance, sense of danger or relationship to space. Pacing is composed with performance, dialogue, silence, sound, camera and eventual return to hands-on play.

Do not simply imitate particular Kojima scenes. The goal is the same integrity between historical source, surprising dramatic rhythm, grounded performance and gameplay, without claiming to know his unmade plans.

## Narrative unit: gameplay → scene → gameplay

- **Before:** player has actually met event prerequisites. Camera, actors, clothing, equipped props, health, knowledge and geography are checked. If an enemy is about to confront the player, show a **credible source of danger**.
- **Entry:** cut only when a new dramatic capability is needed: reaction reading, memorable reveal, changed understanding, confrontation, physical capture. Not to hide missing walking or interaction animation.
- **Middle:** intent-driven blocking. Choose long take, close-up, empty frame, over-the-shoulder or moving shot based on *who controls knowledge*. Voice has source/interpretation labels and genuine human timing. Snake may repeat a newly significant term as a question, with an **acting intention**.
- **Exit:** return control on a safe frame, in the exact same body/posture, location, weapon inventory, camera orientation and ongoing world state. Never damage a helpless player.
- **Skip/interrupt:** mission commits exactly once and independently of Sequencer. Skipped player receives mandatory intel in accessible log. Pausing, reload, death and cutscene restarts never duplicate inventory, loot or state.

## Production structure per scene

Every scene has a unique stable ID, a gameplay trigger, source class, explicit player knowledge, mission transaction owner, skip invariant, authored shot list with purpose, asset gate and known failure modes. See data/cinematic_scene_contracts.json for the Capture, Gray Fox, Shoot Gunner intro and aftermath, Madnar, TX-55 and final Big Boss/Venom scene drafts.

Camera language: favour deliberate geometry and line of action; not constant cuts. Lens and shot movement must express intent rather than advertise technical capability. Frame close emotional performance when a person's withholding or revealing something; hold wide when the *space itself* is plot. Sound is spatial, layered and captionable. No ripped voices or copyright-uncleared orchestral recordings.

Mix/matched motion is approved *only* after retarget, pose and foot contact validation, skeleton/socket agreement, prop alignment, upper/lower-body blend, actor-specific movement identity and tested transitions. Separate Snake, Fox, guards, Shoot Gunner, Big Boss and Venom movement styles. No impressive isolated motion without contextual control.

## Critical beats and intended scope

**Playable lead-in and seizure:** genuinely rescued POW clue, a nonlethal capture plausibly achieved by Snake, guard intelligence bounded by observed evidence, correctly visible confiscation and retained clothes. Preserve agency until real arrest/custody threshold. The intro has enough action to earn a potentially multi-minute capture scene.

**Gray Fox:** close human recognition, tactical Metal Gear/Madnar stakes, cinematic restraint in an oppressive cell, followed by a smooth exit. Do not jump to futuristic Cyborg Ninja. No unearned guard wave or exposition from a character who cannot know the secret.

**Shoot Gunner:** proper spatial encounter and classic recover-your-own-equipment puzzle. During cinematic the boss must not deal unavoidable damage. Weapon ready, fire and recoil are physically consistent. After defeat, Card 3 must still be collected.

**Beyond slice:** Madnar/Ellen scene earns sabotage directions; TX-55's enormity and demise earn the longest *scale* sequence; Venom/Boss aftermath must be more intimate than the machine and leave the 1995 identity ambiguous.

## Definition of done and testing

At present **zero scenes have been captured as finished Sequencer cinematics**. Before claiming completion: verified source dialogue, recorded and facially performed licensed voice, final art and environment, camera blocking and precise in-engine timing, save/skip/load and UX tests, 720p readability, controller accessibility, input-to-feedback measurement, playable continuity and measured scene comprehension. Drafts and C++ skeletons do not qualify.
