# DOCTRINE 000 — HOW WE BUILD (supreme development doctrine)

**Status: BINDING, GAME-WIDE AND ABOVE ALL PROJECT DOCTRINES.**

> It is not only what we build. It is how we build it — and how we know it works for the player.

## Intent

The product is the complete experience the player actually encounters, not a checklist of features, files or attractive screenshots. Research, narrative truth, physical coherence, sound, image, animation, timing, responsiveness, level structure, accessibility, implementation quality and validation must be designed as a coupled system. How a feature is made determines whether it belongs in the game.

## Decision process — mandatory for every meaningful feature

1. **Establish truth before assumption.** Identify authentic 1987/J2ME edition evidence, later-series comparison, and remake invention. Mark uncertainty clearly. Do not invent franchise lore to make a mechanic convenient.
2. **State the player experience before the code.** Describe what a newcomer sees, understands, chooses, hears and feels, in the moment and after it. Include fast players, curious players and players who struggle; account for accessibility.
3. **Design interactions and dependencies together.** For each beat describe geometry, sightlines, audio source/mix, animation, camera, input, latency, AI knowledge, world consequences and durable mission flags in one contract. The environment must do real narrative and tutorial work.
4. **Choose the smallest coherent implementation.** Prefer reusable state machines, explicit contracts, deterministic mission logic, source-controlled data, ownership boundaries, measured budgets and clear failure cases. Don't solve a local problem with a global hack or add polish that masks broken mechanics.
5. **Make it respond to player intent.** Fast solvers should not wait for a timer; exploration should defer intrusive hints. Actions give prompt, honest feedback. AI reacts to evidence, not concealed developer knowledge.
6. **Verify in layers and report status accurately.** Source review, unit tests, game-engine compile, integration tests, packaged capture and unfamiliar-player tests are separate evidence levels. Never call a rendered mockup a functioning feature, an authored test an executed test, or a greybox a finished animation.
7. **Learn and iterate.** Define falsifiable hypotheses and measurable observations (comprehension, input-to-cue latency, stall points, camera collisions, encounter success and recovery). Change timing or layout using player evidence, not arbitrary universal 'best' numbers.
8. **Do not introduce avoidable debt.** Preserve compatibility with existing flags/saves/controls; document exceptions and migration. Reuse infrastructure, ensure tests can run locally without paid CI, and retain readable diffs/reviewable scope.

## Precedence and resolution of conflicts

**Non-negotiable constraints:** law, legitimate asset/licensing permissions, safety, accessibility of critical information, source-truth labelling and honest claims of verification. These cannot be traded away for cinematic effect.

**Within those constraints:**
- Preserve original story/mission causality and edition-specific evidence before introducing remake embellishment (Doctrines 001/002).
- Protect player agency, correct state transitions and input reliability before fixed presentation timing (Doctrines 002/003).
- Resolve actual experienced flow across sight, sound, action, timing and consequences rather than maximizing individual departments (Doctrine 004).
- Use spatial structure/legibility to create pacing and choices; scripted prompts are secondary (Doctrine 005).
- Performance, maintainability and technical constraints are part of experience quality; do not silently degrade one platform to optimize another.

When two requirements conflict, add a short Architectural Decision Record in docs/decisions/ that describes the user-facing conflict, the source evidence, considered options, accessibility/technical cost, chosen compromise, test plan and explicit owner. **No undocumented exception and no stealth rewrite of canon.** Where uncertainty remains, implement a reversible prototype behind a clearly named tuneable, not a falsely definitive rule.

## Definitions of ready and done

**Ready:** source status known; intended player experience and spatial flow described; a Doctrine 004 beat contract exists with material/VO/visual, timing, AI and interruption behaviour; source, build, animation and sound dependencies identified; test scenarios specified.

**Done:** implementation compiled, has integrated rather than isolated audio/visual/action state, survives interrupts/skip/save/load, meets performance/accessibility targets on relevant hardware, passes documented player tests, and has authentic or permission-cleared assets. Missing components or unevaluated timings remain explicitly **in progress**, even if source unit tests pass.

## Anti-patterns and review questions

- A feature added because it sounds exciting but does not reinforce the game loop or source.
- An environment that merely hosts a scripted hint instead of conveying one through layout and material.
- Arbitrary fixed delays, omniscient guards, unexplained controls, silent feedback, forced camera takeover or cutscenes that own mission flags.
- Separate art/audio/AI sign-offs when their combined experience has never been tested.
- Disguising missing licensed rigs, audio or UE builds behind aspirational labels.

Before approval ask: **What is the player trying to do? What can they reasonably perceive and infer? Does every response causally agree? Is the flow respectful of both novice and expert play? Which source supports it? What did we actually test? What will break when the player does something unexpected?**

## Implementation and review enforcement

- All future doctrines must reference Doctrine 000, describe their scope and not silently weaken its gates.
- Docs 002, 003, 004 and 005 supply specialized rules, not alternative definitions of 'done'.
- Reviews identify affected beat and area contract IDs, evidence level, deviations and unverified assumptions.
- Validators can check contract completeness and format. They **cannot** substitute for Unity/Unreal playtesting, subjective pacing studies or licensed final art. The build gate must report this honestly.

**Adoption note (2026-11-18):** Established as a project-wide engineering and game-design policy. Existing source-only systems are candidates for audit, not automatically compliant or completed.
