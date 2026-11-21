# Outer Heaven 1995 — cross-engine playability pass, 21 November 2026

**Branch:** `feature/character-art-pipeline-1995` · **Draft PR:** #3.
**Doctrine:** 000 (experienced product), 003 (player-led pacing), 004 (coupled experience), 005 (physical structure).
**Status:** real browser/Unreal C++/input/HUD source changes committed, **not a verified real UE build, rendered browser session or user playtest**.

## Why this pass exists

The prior implementation could leave a new player uncertain which object responds to E, make an Xbox player's controls reverse between versions, spend finite consumables when merely looking through an item list, and discard hard-earned world progress when requesting an innocent retry. All four failures make a game feel less playable even when every individual mission flag or animated character exists.

## Implemented, with evidence boundaries

### 1. Progress-preserving retry and death

- Browser `R`, pause Retry, failure Retry and cleared-sector Retry now use the same non-destructive `retryCheckpoint()`. It relaunches a fresh runtime with `#retry-checkpoint` and auto-resumes the existing saved world, inventory, ammunition, body positions and boss states. It cannot issue repeated reloads in successive death frames. There is no programmatic `localStorage.removeItem` on retry.
- Browser Snake death uses that same path, not a distinct title-only reload. A broken or unsupported Continue **never** silently starts New Game or overwrites an unreadable checkpoint. The original bytes remain until the player approves **New Operation** in the already-established safe, cancel-default confirmation dialog.
- Unreal F9, the normal death callback and `AMGGrayFoxMission::RetryFromCheckpoint` now reload the existing SaveGame without deleting it. The level is opened by `GetCurrentLevelName(this, true)`, stripping `UEDPIE_` prefixes so Editor retries resolve correctly. Existing schema V7 is unchanged.
- This does not invent safe-room teleporting, infinite healing or refunded bullets. Every retry returns to **whatever last legitimate save** was recorded. It is not an on-demand save anywhere.

### 2. Common real Xbox controller mapping

| Gameplay action | Browser | Unreal | Keyboard |
|---|---|---|---|
| Interact / pick up / talk | A | A | E |
| Melee / wall strike / wall knock | X | X | F |
| Crouch | B | B | Ctrl |
| Prone | Y | Y | Z |
| Item use (after selecting item) | D-pad Down | D-pad Down | Y |
| Item wheel | LB | LB | Tab |
| Weapon wheel | RB | RB | G |
| Aim / fire | LT / RT | LT / RT | Right / left mouse |
| Intel dossier | D-pad Up | D-pad Up | J |
| Transceiver | View / Back | View / Back | C |
| Skip an active cinematic | B | B or Escape | Escape or Space in browser |
| Retry from checkpoint | R or pause-menu Retry | F9 | Browser R / Unreal F9 |

The source controls are not evidence of physical controller input timings. Test actual XInput and keyboard simultaneously. Unreal's B skip now has a true Input mapping, and `CrouchToggle` checks active cinematic state so skipping one does not also force Snake to crouch.

### 3. Honest, contextual actions

The Unreal HUD now uses `GetContextActionPrompt(Player)`: a display-only query that calls the same real `CanReach` / LOS predicate as interactions. It reveals a pickup only when the player is actually close and facing it and the item has not been earned. It provides actionable E/A for the original three-truck supplies, borrowed carton, prisoner, custody challenge, Gray Fox, gear bag, Card 2/3 reader, parachute, lift, grenade rack and tank tracks, and F/X when physically in reach of the two weak walls or recovery door. No prompt can create intel, see through a wall, auto-pickup a card or change world flags.

Browser local E/A prompts already existed. Both versions preserve broader objectives at a distance and reserve stronger instruction until the player has meaningfully explored.

### 4. Equipment and deliberate item use

Previously the browser could consume a ration, emit cigarette smoke or toggle the binocular camera merely by releasing the equipment selector. It now **selects** an item and separately **uses** it with keyboard Y / controller D-pad Down, matching native Unreal semantics. The selected item is remembered through a checkpoint and remains accessible on reopening the menu; consumed rations remain finite; unused rations never disappear at full health. Binoculars are optical only, and optional smoke does not modify enemy knowledge without actual acoustic/visual evidence. Borrowed freight box selection retains its contextual disguise affordance. This is a control contract, not an item-grant cheat.

### 5. Anti-stall hints respect the player's time

The browser's staged 28/75-second optional guidance previously stopped after early prison beats. `optionalStageHelp` now adds context-specific first and second hints for the custody route, gear recovery, gunner volley, records access cards, service charge and relay, Machinegun Kid, roof Hind D and Building 2 tank. A player actively moving or scanning still delays guidance; genuine enemy alert suppresses it; Help OFF remains respected; the toggle is persisted.

Numbers remain provisional hypotheses pending novice/veteran testing. Explicit hints explain action only **after** the player has had room to observe the environment. They never open doors or adjust AI.

## Regression evidence in this source-authoring turn

- V8-based browser DOM/WebGL-stub campaign and animation tests: **passed**, now **22 logged checkpoints including animation intent**. Actual WebGL render, human look/feel and controller hardware were not exercised by these stubs.
- `browser/tests/test_campaign.cjs` includes lossless R/pause retry, fatal-damage once-only reload, invalid saved-JSON protection, separate item selection/use, finite ration, binocular scope and smoke activation, boss/card guidance without state changes, mission story route, guard interactions, roof/tank progression and older save migration.
- `tools/test_cross_engine_playability_source.py` checks Xbox mapping, skip binding, HUD locally reachable prompts, non-consuming menus, retries, and source invariants. It is automatically included in the tester's standard local Python unittest discovery. **Native Python suite and UE compile not executed in this session.**

## Required actual playtest matrix

1. **First 12 minutes, unfamiliar player:** Can they find and interact with the original three truck cargo props? Do they understand that prisoners and custody/security are different actions, and does a surrender work without fighting? Record every unprompted hesitation and false interaction.
2. **Captured cell:** Strike the wall three times, speak to Fox, strike the second wall, use low cover and escape. Test aiming, sprinting, crouching, prone clearance and shoulder camera while the hint timer is running; actual action prompts must never appear through concrete.
3. **Confiscated gear and Shoot Gunner:** Reach locker, follow audible tracker clue, recover real gear, inspect transmitter, use cover during telegraphed volley, fire during recovery. Test both a successful experienced route and an ammo-limited retry.
4. **Real input test:** Xbox A speaks/loots, X attacks, B crouches, Y goes prone, D-pad Down uses a pre-selected ration/binoculars. Open/close equipment windows rapidly; no extra consumption, double activation or stuck button state. Escape/B skips real cutscenes without Snake ending crouched.
5. **Safety-retry test:** Use R (browser) or F9 (Unreal), choose pause Retry, trigger actual Snake death, die twice rapidly. Confirm same prior checkpoint, same finite inventory, bodies, world doors, security-state reset and no crash. Test corrupted browser Continue; preserve the original bytes and require confirmation to overwrite.
6. **Post-Card 3:** Charge through kennels, Machinegun Kid burst/reload, west lift, Hind D shelter/launcher, Building 2 mine tracks. Move both quickly (speedrunner) and deliberately (novice); no compulsory waiting beyond legible attack/weapon cycles. Compare visual feedback, controls, world transitions and whether a missed grenade/mine leaves a recoverable retry.
7. **Package and performance:** Open the built UE5.7 Windows executable from a clean package and the browser localhost version on target hardware; record input-to-cue latency, softlock attempts, checkpoint restoration, wall clipping, guard fairness, FPS and presentation quality at 720p and 1080p. Ensure cinematic skip works with real Gamepad B/Escape.

## Exit criteria and known limits

A gameplay beat is **source-implemented**, not product-ready, until its actual native/HTML runtime is exercised by a person in all relevant input modes. The original 1987 MSX2 floor atlas, cards 1–8, every Outer Heaven mission, final PBR human models, authored AnimBlueprints, full VO, native Mac/Linux shipping and full side-by-side playtests remain incomplete. The current browser source offers a playable prototype through the tank slice, and Unreal has corresponding source logic awaiting successful 5.7 compilation/PIE. Do not claim either is a fully playable end-to-end remake.
