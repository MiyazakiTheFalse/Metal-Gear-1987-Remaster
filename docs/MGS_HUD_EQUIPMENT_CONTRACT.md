# MGS-Style Equipment HUD — Implementation Contract

Konami's original *Metal Gear Solid* equipment window is hold-to-open, directional choice and release-to-equip. Our modern MGS4/MGSV-like control profile reserves Xbox **LT/RT** for aiming/firing and adopts **LB/RB** for item/weapon windows; keyboard uses **Tab/G**. D-pad Left/Right, mouse wheel or bracket keys cycle the focused slot.

The two windows display all authored slots with counts, availability and a CONFISCATED warning. Equipment is functional data in \`UMGEquipmentComponent\`, not HUD-only painted labels. Captivity makes all but NO ITEM / NO WEAPON unusable, while retaining inventory entries for an eventual gear-recovery sequence. The selectable STUN PISTOL, RATION and KEYCARD slots are labelled **TEST**: they are NOT a statement about what Snake owned during the original J2ME capture.

## UI requirements
- Health and ammunition derive from active gameplay state, not fictional UI numbers.
- Selection requires hold, navigation, release. Locked slots show why equip failed.
- Controller buttons must not conflict with LT aim, RT fire, X interact, B crouch, Y consume, or D-pad Up intel log.
- No game pause while choosing gear. The camera remains free.
- Alert phase drives a radar replacement and intended music state; never infer threat from a narrative timer.
- High-contrast scalable HUD: safe margins, text legibility at 720p, no overlap with subtitles, optional hint/inventory panels.
- Wire actual pickups and persistent equipment in later validated events. Current Gray Fox slice intentionally begins disarmed.

## Verification
- Review confiscated menu, attempt to equip forbidden gun, confirm firing refused.
- During a future verified recovery event, unlock weapons/items and confirm counters change only when used.
- Verify Xbox shoulder menu selection does not accidentally aim/fire, even at 30 FPS.
- Verify selector slot wrapping, repeated button presses, and controller unplug/replug.
- Perform accessibility tests (colour-blind cues, text size, subtitle/radar overlap).
- Current prototype has no verified save migration for all equipment contents or final pixel art.
