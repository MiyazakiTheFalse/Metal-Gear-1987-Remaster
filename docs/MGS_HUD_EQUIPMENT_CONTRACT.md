# MGS-style scrolling weapon and equipment windows — two-engine implementation contract

**Updated 21 November 2026. State:** browser and Unreal source implemented, browser JavaScript simulation checked; **actual Unreal 5.7 compile/PIE and visual/controller UX review not performed here**.

### Franchise mechanic vs remake input

Konami's *Metal Gear Solid* (Master Collection) manual explains hold weapon/equipment, move selection, then **release to equip**; a `NO ITEM` choice allows unequipping. Ammo/item counts appear below icons. Evidence: https://metalgear.konami.net/manual/mc1/mgs1/pc/en/page06.html . Konami's original 1987 MSX2 manual has separate weapon/equipment sub-screens and an explicit action requirement for some items; see https://www.ne.jp/asahi/hzk/kommander/mg1manual.pdf . The **visual slot arrangement, glyphs, code and audio in this project are original assets**, not extracted Konami UI.

We preserve the recognizable **hold → scroll → release** tactical rhythm, with a modern controller mapping that keeps LT aiming / RT firing. We do **not** restore obsolete source-specific controller conflicts, such as treating Xbox Y as simultaneous prone and use.

| Step | Browser | Unreal 5.7 source | Expected result |
|---|---|---|---|
| Open items | Hold LB or Tab | Hold LB or Tab | Item five-card strip; world at project 0.2× time scale |
| Open weapons | Hold RB or G | Hold RB or G | Separate weapon five-card strip |
| Scroll | D-pad ←/→, left-stick vertical, mouse wheel, `[ / ]` | D-pad ←/→, left-stick vertical, mouse wheel, `[ / ]` | Moving highlight wraps; weak audio/menu cue; never spends inventory |
| Equip | Release held LB/RB or Tab/G | Release held input | Exactly one valid item or weapon is committed; name/counter updates |
| Explicit use | D-pad ↓ or keyboard Y | D-pad ↓ or keyboard Y | A selected ration heals once; binoculars toggle optical FOV; smoke only when actually requested |
| Unequip | `NO ITEM` / `BARE HANDS` | `NO ITEM` / `NO WEAPON` | Safe state, no invisible attack privileges |
| Denied item | Highlight visible, unavailable label, no state transaction | Highlight visible, locked color, denied feedback | Selection cannot unlock or spend an unowned source weapon |

### Rendering doctrine

- **Five-card one-line centered carousel** instead of vertical scrolling over the whole inventory. The center is brighter and full size; flanking cards shrink; outer cards are faint. No licensed art assets or extracted fonts.
- Weapon panel appears on the right; item panel appears on the left. Both work at 720p and widescreen, with an accessibility reduced-motion override.
- Source data (owned/quantity/currently equipped/confiscation) drives both. **The UI cannot determine ownership or award ammunition**. Browser uses `browser/equipment_strip.js` for a pure view model. Unreal reads its existing `UMGEquipmentComponent` in `MGPrototypeHUD.cpp`.
- Selection wraps across all slots. Locked/unowned entries are visible for clarity, but selecting one cannot equip or change save state.
- **No inventory griefing:** selecting rations never consumes them, and selecting cigarettes never creates smoke. Only UseItem can perform the action. Scouting through binoculars remains an optical mechanic and does not reveal occluded enemies. The box disguise is an equipped contextual effect, not an auto-used heal.
- At normal scale, item selection must neither accelerate guard AI nor teleport Snake. During open windows gameplay clocks run at **0.20×** as a tunable project default; horizontal travel is suppressed while the player chooses (Unreal left stick and browser WASD/stick). Camera remains available. An enemy might already be reacting, so never confuse the slow-time effect with a paused or invulnerable state.
- Original menu opening/scroll/confirm audio uses the existing own-authored cues with audio-less text fallback. Only actually owned slots produce a successful equip cue.
- Do not confuse the **limited player inventory** with the entire future asset list: locked Handgun/SMG/Rocket Launcher slots are implementation placeholders until earned as genuine MSX2 pickups; noncanonical stun remains dev-only and excluded from normal runs.
- When Snake's kit is confiscated, only `NONE` is available. Pre-capture collected items remain in storage for actual earned recovery.

### Source files

Browser: `browser/equipment_strip.js`, `browser/index.html`, `browser/game.js`, `browser/tests/test_equipment_strip.cjs`, `browser/tests/test_campaign.cjs`.

Unreal: `unreal/MGRemaster/Source/MGRemaster/Private/MGPrototypeHUD.cpp`, `Public/MGEquipmentComponent.h`, `Private/MGEquipmentComponent.cpp`, `Public/MGPlayerCharacter.h`, `Private/MGPlayerCharacter.cpp`, `Config/DefaultInput.ini`.

### Acceptance: what to actually watch in both builds

1. Hold LB with gamepad, move left stick vertically: the highlighted card changes **one at a time** with ~180–185ms held repeat, Snake does not walk away; release LB to equip. Repeat with RB and right-hand weapon strip.
2. Move through locked cards and back to `NO ITEM`; confirm there is no weapon unlock, no quantity change and no invisible pistol shot.
3. Equip ration while injured then quickly reopen the menu: it must still exist. Press Y / D-pad down **once** to heal and decrement one unit; at full health the ration must not be consumed.
4. Equip cigarettes and shut the window: no smoke. Press explicit UseItem: one nearby smoke cue. Equip binoculars and toggle only through UseItem; switching to a different item lowers them.
5. Confirm wheel directions and D-pad direction agree across both engines. Test pause, title, cinematic skip, controller unplug/replug and lossless checkpoint save/reload while a menu has just closed.
6. Test 720p, 1080p, ultrawide, captions/subtitles, reduced motion, muted audio, hardware Xbox pad and keyboard. No menu overlap with alert or quick mission objective that blocks situational awareness.
7. **Unreal: compile UE5.7 and launch real PIE before sign-off.** C++ UI code here is source-level evidence only. High-quality custom icon art, final selector animation and final sample-based sound design remain production tasks.
