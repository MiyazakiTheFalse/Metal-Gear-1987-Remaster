# DOCTRINE 002 — In-Game Events and Player Agency (binding)

Status: ACTIVE. Companion to 2D-to-3D map doctrine. The J2ME a1 archive remains the authoritative **target edition**, while MSX2 scripts are **comparative** until reproduced in J2ME gameplay or decoded event data.

## Decision rule

Preserve each original event's narrative purpose, causal logic, mission-gating consequences, and player information. Reimagine how the player experiences the event to fit third-person interactive stealth. Do not replace player decisions with a cutscene merely because production animation exists. The *gameplay itself* is storytelling.

Research basis:
- Hideo Kojima (2012 BAFTA Annual Games Lecture): https://www.bafta.org/media-centre/press-releases/2012-annual-games-lecture-delivered-by-hideo-kojima/
- Kojima interview (2015): https://gamesbeat.com/hideo-kojima-tries-to-balance-open-world-and-gripping-story-with-metal-gear-solid-v-the-phantom-pain-interview/2/
- Epic Gameplay Tags: https://dev.epicgames.com/documentation/unreal-engine/using-gameplay-tags-in-unreal-engine
- Epic StateTree: https://dev.epicgames.com/documentation/unreal-engine/overview-of-state-tree-in-unreal-engine
- MSX2 comparative script: https://metalgear.dr-sunny.info/?lang=en&page=script

## Every event requires six records

1. **Source/provenance**: exact version, room/scene identifier, screenshot/record/bytecode evidence; mark any MSX2-only claim as unverified for J2ME.
2. **What happened**: trigger (interaction, zone, signal, AI perception, timer, mission transition), and precise preconditions.
3. **State changes**: reversible/irreversible flags, items, room locks, NPC states, journal checkpoint. IDs must be stable.
4. **Player-facing delivery**: ambient, in-world dialogue, short interactive staging, cinematic. State clearly whether camera/control is retained, whether a cutscene is skippable, and how control returns.
5. **Contingencies**: if Snake is spotted, dies, leaves the room, interrupts dialogue, kills an NPC, loads a save, or approaches out of order, what happens?
6. **Proof**: unit tests for flags and gates; playtests for animation, camera collision, continuity and accessibility; save/reload and replay tests.

## Reproduction policy

**Story invariants** are immutable: original objective dependencies, prisoner significance, identity reveals and key items. New opportunities can alter *method*, not cause impossible mission states or eliminate the emotional consequence.

- Low-impact presentation should be seamless: radio while moving, readable cues, closeable and replayable logs.
- Use short unskippable input locks only when the mechanic cannot safely work otherwise. Cinematics should be skippable after their mandatory state transaction commits. Skipping cannot skip the flag change or item award.
- Do not auto-teleport Snake for convenience or lock a doorway behind him without an earned cause and a way forward.
- Every hazard must be perceivable, avoidable or recoverable. No instant punishment without telegraphing.
- A dynamic discovery should remain discoverable via more than one fair route when the mission allows it.
- A cinematic never owns a mission flag; the durable event transaction owns it. Audio/VFX/animation react to that event and may be interrupted.
- Critical clue missed? Provide an appropriate reminder or contextual alternative, not an unexplained broken objective.
- Do not play a rescue/sabotage completion animation twice after load. Replayable optional dialogue is separate from irreversible mission completion.

## Engine boundaries

- **Rust**: evaluate stable event IDs, gates, negative conditions, save snapshot and atomic changes in deterministic order. Initial code is in rust/src/mission_events.rs, independent of UE and J2ME Java.
- **Unreal**: GameInstance/WorldSubsystem routes Gameplay Tag triggers, applies durable flags, and dispatches presentation cues to animation/sequence/audio/HUD. StateTree/AI controllers interpret alert or radio events and memory about last known location. These adapters are NOT yet wired up.
- **Source data**: event ledger is data-driven. When reverse-engineering confirms a J2ME trigger, attach exact source metadata and promote it to verified. Do not let a speculative MSX2 interpretation silently become shipped logic.
- **Persistence**: snapshot format versioned. Snapshot must include flags and fired-once IDs. Save migration is explicit; unknown event IDs are rejected, not silently discarded.

## Required event test matrix

| Input situation | Required result |
|---|---|
| Rescue NPC before prerequisites | Nothing commits; state remains valid |
| Rescue NPC after prerequisites | Flag commits once; dialogue cue plays once |
| Walk out during ambient call | Control remains; core flag retained; repeatable transcript accessible |
| Enter locked door without card | Refusal feedback, not progress flag |
| Reload after card pickup | Inventory and door state coherent, no duplicate award |
| Alert during rescue | AI responds, story action resolves or suspends explicitly |
| Skip cinematic | Same mission state as viewing it |
| Die during script | Restore last checkpoint without half-applied effects |
| Fast re-entry/repeated trigger | No duplicate one-shot event |
| Two events triggered together | All eligibility checks use same pre-event snapshot |

## Specific adaptation proposals (NOT yet proven J2ME events)

- **Gray Fox**: rescue through interactive cell scene, maintain proximity/control until critical information delivered; use short cinematic only for character-reveal impact.
- **Big Boss**: optional radio misinformation and a genuine trap rather than a hardcoded forced detour; preserve the betrayal's late reveal. The MSX2 script documents deliberate misdirections around Building 3. Verify in selected J2ME.
- **Madnar/Ellen**: preserve dependency and consequences, add emotionally grounded interactive exchange, not changed key quest logic. Verify in selected J2ME.
- **TX-55**: maintain sabotage objective and exact prerequisites, modernise physical inspection, staged plant/detonate feedback; *do not* invent a full boss fight that contradicts the source.

## What Kojima might do today — interpretation, NOT a quotation or promise

Based on documented remarks about clear goals with freedom of approach, he might make guards, environmental clues, radio and mission consequences carry more story through player action; use a few deliberately directed dramatic revelations; and let a clever player solve encounters using unexpected tools. This is a design inference, not a factual statement of Kojima's plans.

## Production gate

No event is marked original-edition verified or shippable until we can cite its J2ME trigger and observe save/load, skip, combat interruption, and out-of-order entry behaviour in a packaged build.
