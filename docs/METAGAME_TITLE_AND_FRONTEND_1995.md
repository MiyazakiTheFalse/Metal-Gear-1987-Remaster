# Metal Gear 1995 remake — metagame experience and title presentation
**20 November 2026 · Active browser source pass, Unreal 5.7 front end outstanding.**

The **metagame is part of the first impression, not a decorative launcher**. It should already communicate tension, intelligence, military secrecy, the year 1995, player agency and the quality of a later in-mission transceiver call. The 1987 MSX2 mission chronology remains the story authority; menus can be inspired by later series presentation without adding out-of-era narrative information.

## Reference practices worth reusing

- **Metal Gear Solid (1998):** decisive logo hierarchy, restrained monochrome military signal language, clipped sound and anticipation before play. A press-to-start cadence should be concise and respect player time, not introduce unskippable boot logos.
- **Metal Gear Solid 2 (2001):** main narrative separated from special/optional content. Mission archives, game records, challenges and replay must not overwrite real campaign progression.
- **Metal Gear Solid 3 (2004):** animated, distinctive title identity that conveys movement and danger without turning the menu into a trailer. The title's living background is an *original* rendition, not copied character screenshots or reused copyrighted music.

Primary references: https://metalgear.konami.net/manual/mc1/mgs2/pc/en/page16.html ; https://www.artofthetitle.com/title/metal-gear-solid-2-sons-of-liberty/ ; https://metalgear.fandom.com/wiki/Title_screen

## Current browser functionality — authored on the active PR

| Screen / system | Present in committed browser source | Not finished |
|---|---|---|
| Main title | Responsive original stylised Metal Gear text hierarchy, Operation Intrude N313 setting, animated tactical radar/industrial illustration, reduced-motion CSS | No final cinematic/title photography, lighting pass, theme music or original mastered UI audio |
| New Game | Starts opening route; when a checkpoint exists opens a **Cancel-default, explicit replace** confirmation; keyboard and pad selectable | No multiple save slots or cloud saves |
| Continue | Disabled if there is no actual local save; resumes existing checkpoint | No save thumbnails / mission summaries / manual slots |
| Briefing | Optional 1987-informed Snake/Big Boss two-head illustrated dossier with direct Start Mission or Return | No final VO, motion-captured facial acting, live 3D scene or canon-approved final wording |
| VR Training | Real, segregated Shoot Gunner practice query and different saved practice context | No full VR mission collection, scoring, ghost or best-time database |
| Options | Working existing comparison rig, context tips, device speech switches; toggle from the title | No full input rebinding, graphics/audio sliders, colourblind, safe-area/subtitle-size settings |
| Keyboard/controller | Arrow/W-S & D-pad navigation, Enter/A to select, confirm left/right controls, no accidental checkbox-triggered Start | Needs real pad focus and accessibility research |
| Pause and archive | Existing functional pause, intelligence archive and replay actions retained | Require full menu-skin cohesion, confirm-reset safety, playable recap and saved records |
| Codec | Separate 1995 transceiver with optional channels, subtitle/line events, animated eight-character original portrait source | Real PBR facial models, full VO, lip sync, complete cast and in-engine art review |

**Evidence:** static source parse and 15/15 V8 simulated-DOM interaction checks passed in this authoring pass. Browser Node test assertions are committed, but the complete Node suite and real WebGL/UE test have *not* been executed here. No complete Unreal front-end menu is being claimed.

## The front-end interaction rules

1. **Access in a heartbeat.** From launcher to New Game or Continue in one or two deliberate inputs. Extra lore lives in a voluntary briefing or archive.
2. **Protect player time.** Cancel-first confirmation before overwriting an existing save; the menu never silently changes mission flags.
3. **Fast menus, slow atmosphere.** Animations run behind legible static text. An atmospheric sweep can run at a low frame rate; it never delays input.
4. **Meaningful choices only.** Hide locked future menus or explain them plainly; no decorative button linking to a fake chapter. VR rehearsals never affect authentic campaign progression.
5. **Accessible by default.** Keyboard, controller, focus indication, reduced-motion CSS, readable captions and subtitles. Future support for rebinding, subtitle size, contrast and sound-balance.
6. **Save/pause fidelity.** Mission timestamp, difficulty, earned weapons/cards and location should eventually be visible in Continue. No spoilers in the save thumbnail beyond player-earned knowledge.
7. **Original authorship.** Original techno-espionage sound design, procedural military display and authored art. Never lift the MGS soundtrack, title animation or Konami logo into an unlicensed product.
8. **One visual identity.** Main menu → mission dossier → transceiver → pause → records → completion must share consistent motion, audio, hierarchy, and speaker/mission terminology.
9. **Unskippable actions: none.** Earning the first cutscene should be impactful, but title idents and background art must be skippable / reduced motion; optional cutscenes do not gate required controls.
10. **No future-spoiler graphics.** The static facility silhouette is labelled a tactical visualization rather than the factual 1987 floor plan. Later Metal Gear, Big Boss and identity twists aren't displayed in the opening menu.

## Ordered remaining implementation

**N01 — First complete Building 1 mission:** Fix proper first-floor layout, all physically earned mission items/POWs/Card 1/2 and four mercenaries. Use actual gameplay capture to decide whether the attract background should show a live camera tracking a guard's routine rather than a purely abstract visual.

**N02 — Unreal native front end:** Build an Unreal GameInstance/saves + UMG (or Slate) menu, authored title level/backdrop, screen reader/text handling, pad navigation and save-confirm actions. No premature claim of a working title until a real editor PIE launch demonstrates it. Import approved browser-face identity source as interim texture assets, replace with actual realistic 3D when ready.

**N03 — Metagame convergence:** Unified mission archive with earned chronology only, earned rank and no-spoiler records, actual audio mix, title stingers and optional short debrief VO. Challenge rehearsal as separate scenario namespace.

**N04/N05 — Whole game additions:** New mission cards and contextual menu imagery as the user earns them, original finale return-to-title credits and New Game+ only if its exact rules are verified and deliberately separated from original progression.

**N06 — Release:** Performance, transition polish, safe-area support, subtitles, rebindable inputs, reduced motion, localised scalable text, accessibility playtest, memory/asset-profile evidence and signed Windows/Linux/macOS release packages.

## Acceptance evidence required

- From clean save, New Game starts original opening; Continue disabled.
- From existing save, New Game requires explicit replace; Cancel leaves the bytes unchanged; Confirm resets predictably.
- An optional briefing does not change stage, inventory, alert or checkpoint.
- VR rehearsal never mutates standard story save.
- Keyboard and two common gamepads navigate the title, modal, options and pause with observable focus.
- Animation disabled by OS remains operable and readable.
- Zero mandatory network calls: offline title, basic controls and original local rigs remain playable; optional comparison art may need network.
- A real screenshot/video of the title at 720p, 1080p, ultrawide and mobile portrait; check input lag and focus order.
- The full Node tests and UE5.7 compile/PIE pass before calling anything production-ready.

Related: [whole-game status](FULL_GAME_CONVERSION_AND_BOSS_STATUS_2026_11_20.md), [governing doctrine](DOCTRINE_000_HOW_WE_BUILD.md) and [front-end quality tracker](https://github.com/MiyazakiTheFalse/Metal-Gear-1987-Remaster/issues/9).
