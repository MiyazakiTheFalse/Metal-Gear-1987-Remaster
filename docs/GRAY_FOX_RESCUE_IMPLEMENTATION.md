# Gray Fox prison sequence

The default entry is the last prisoner contact before capture. Completion is the player's exit after Shoot Gunner. The corridor is an intermediate checkpoint. The browser playtest and Unreal source share those story gates. Controller bindings, room dimensions and presentation still differ.

| Beat | Playable action and consequence |
|---|---|
| Intelligence | Speak to the prisoner at the lit desk to learn why entering custody could reach Fox. |
| Approach | Carry the prototype loadout past a patrol. Cover, retreat and confrontation remain available. |
| Arrest | Lower the weapon and volunteer at the post. Snake loses access to his existing inventory. The radio remains available. |
| Solitary | Investigate immediately. Schneider supplies pre-capture context. Big Boss can offer wall guidance inside. |
| Discovery | Strike the repaired panel. Its first two impacts expose damage, with audible and captioned responses. |
| Breakthrough | The third impact opens a traversable hole. Progress persists across reloads. |
| Gray Fox | Approach Fox without a camera cut. Read successive lines about the walking nuclear weapon and Dr. Pettrovich. The intel log preserves the exchange. |
| Second breach | Apply the discovered interaction to a perpendicular wall. The turn leads into the patrol route. |
| Personal effects | Retrieve the inventory and remaining ammunition. The property shutter opens. |
| Shoot Gunner | Use the raised-weapon warning, cover and existing combat inputs. Close combat remains viable with depleted ammunition. Defeat opens the exit. |
| Resolution | Keep control after the fight. Cross the final shutter to checkpoint the objective of finding Pettrovich. |

## Fidelity and presentation

The original MSX2 script and gameplay are comparative evidence for the prison, Fox's revelation, the equipment and the warden. The booth, cover, lighting, radio paraphrases, stun pistol, damage counts, boss tuning and threshold treatment are remake decisions. The J2ME v3.3.0 a1 room graph, loadout, exact triggers and dialogue still require direct source verification. The original equipment transmitter trap is outside this prototype. Records and comparisons are in [the source audit](SOURCE_AUDIT.md), [the MSX script](https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/30618) and [the walkthrough](https://www.neoseeker.com/metal-gear/faqs/105549-script.html).

The browser retains the existing procedural humanoids and optional network-loaded rig. The normal route leaves Fox in the cells; the earlier companion behaviour is available with the browser's escort option. Original synthesized foley covers cell bolts, masonry, gear and gunfire. Browser captions can be accompanied by an installed English system voice. These are temporary performances. Facial acting, final character art and a recorded voice mix remain production work.

## Checkpoints and developer entry

The Unreal slot retains its original name. Schema 2 adds inventory, equipped selections and partial masonry damage. Valid older captured states gain the contact flag and move to a safe landmark in the revised geography. New sessions start in the yard. The development option `-GrayFoxCellStart` starts a fresh mission in the cell. F9 clears the slot.

The browser supports `?cell` for a fresh cell entry, `?escort` for the optional companion prototype, and Continue Checkpoint for saved progress. Old browser saves migrate through a safe landmark. The arena checkpoint permits retries even with an empty pistol.

## Verification

Run `node browser/tests/test_campaign.cjs` and the repository's Python tests and registry validators. The campaign test exercises the real browser movement, navigation, interaction and combat routines in a simulated host. It does not establish frame rate, controller feel or player comprehension. Unreal requires a UE5.7 build host. Human checks should include a muted first run, declining surrender, returning after a fight at the post, a wall reload, a missed Fox line, empty-ammo combat, waiting after the warden falls, and both camera shoulders.
