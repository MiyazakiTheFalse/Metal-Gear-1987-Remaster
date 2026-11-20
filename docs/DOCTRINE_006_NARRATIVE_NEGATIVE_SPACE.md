# Doctrine 006 — Narrative Negative Space: What Outer Heaven Shows and What It Withholds

**Binding under Doctrine 000; source status: 1987 MSX2 primary, `The Phantom Pain` a 1984–1995 thematic/identity continuity layer. Updated 20 November 2026.** This is a design/implementation contract, not a claim of final art, licensed performance or approved story.

> **Connect the place; do not prematurely connect every truth.** The player should understand where guards eat, why a corridor is patrolled and how an item enters a storeroom. The player should not understand exactly who controls Outer Heaven before the original story permits it.

## Why this doctrine exists

The original MSX2 game is deliberately terse. There are two kinds of missing information:

1. **Production/screen-abstraction gaps** — a door joins two rooms with no plausible 3D threshold, security guards have no guard shifts, prisoners have no transfer routes, weapons appear without stores, maintenance has no workers. These deserve consistent spatial and physical interpretation because modern players will otherwise feel the game is unfinished.
2. **Dramatic gaps** — Big Boss's shifting orders, Schneider's genuine blind spots, Gray Fox's limited testimony, the final identity and the nature of command, abrupt named opponents, incomplete knowledge of the weapon. These must remain withheld or deliberately fragmented. An exposition cutscene or collectible that explains them too early is a **narrative bug**.

Not all apparent gaps are blanks. The original Japanese MSX2 has Schneider/Steve contact details missing or truncated in certain English releases: cross-check Japanese source rather than taking an abridged English version as evidence that a character never existed. The notorious *turn off the MSX* intervention is a deliberate fourth-wall tonal dislocation. Do not smooth that into an ordinary orderly radio mission cancel.

**Edition control:** `metalgear.dr-sunny.info/?lang=en&page=script` transcribes the **NES** release. It cannot substantiate the precise 1987 MSX2 wording. Use NekuraHoka's MSX script dump and TCRF's Japanese/MSX2 comparisons; source-verify exact room timing against MSX2 footage/maps. `The Phantom Pain` later establishes the body-double resolution, but the young protagonist in 1995 must not know it.

## Where we can fill blanks (remake interpretation, never assert them as 1987 verbatim)

| Space / story gap | New **showable** evidence / systemic gameplay | What must remain unsaid |
|---|---|---|
| **Outer perimeter / supply yard** | Actual delivery paths, a guard watching for vehicles, radios carrying changes of duty, marked field rations and medic shipments, one naturally unguarded approach. One small cargo chit is currently placed and readable. | Ownership of the organisation, who's personally responsible for its ideology, or a signed secret plan for the invasion |
| **Custody transport / secret prison** | Arrest chain of custody: guards challenge and secure a compliant Snake, gear is logged, trucks travel to the cell, isolation compromises normal medical/maintenance processes. A duty slate and cell repair slip are placed. | The hidden cell's exact location in resistance intelligence; Big Boss's knowledge cannot become player-proof of betrayal |
| **Gray Fox cell** | Human exhaustion, breath, hesitation, footsteps beyond the door, damaged bedding; his selective technical intelligence points the *player* at Dr Pettrovich/Madnar. | Imagined full memories of capture, a confession about future events, cyborg ninja imagery |
| **Shotmaker prison domain** | Storage & security chain: confiscated bag, shell supplies, loading stool, an official property receipt, staff retreat from a frightening weapon, a reachable service door. The receipt is currently inspectable. | Invented childhood, loyalty oath to the phantom, or self-conscious boss speech proving Big Boss' actual identity |
| **Card 3 / kennels** | Dog feeding/water, animal handlers' chore board, physical patrol and kennel cover, optional low service/crawl route (marked as a **new 3D adaptation**). Existing source builds dogs/duct and a duty board. | A surprise explanation of who every dog obeys; an invented mandatory dog death to earn a card |
| **Later barracks / medical / recreation** | Mess trays, fatigue, old pictures from an earlier unit with scratched emblems, incompatible opinions expressed by soldiers who know different things, deferrals in pay and medical treatment. | An unambiguous Diamond Dogs alumni roster or an unexplained participant from 1984 presented as established canon |
| **Industrial TX-55 programme** | Genuine electrical load, transport rails, test facilities, engineers' messes, warm machinery, production budgets, no entire war machine revealed through a single signpost. | Final destroy method before the scientist explains it; a complete secret boss blueprint in the first building |
| **Resistance network** | Intact and damaged lines; conditional frequencies and conflicting estimates, Diane's and Steve's recognisable lightness when appropriate to the relevant edition, isolated help among ordinary people. | Schneider's unexplained omniscience, guaranteed contact everywhere, or a unified cutscene briefing that removes the game's discovery |

The implementation should prefer **evidence the player sees while moving**: a poorly repaired wall changes footstep reverberation, a kennel's rations create dog-care plausibility, and guard placement reflects the location of supplies, cover and entrances. A new line of dialogue is a last resort when a physical cue is stronger.

## Where we intentionally preserve disjointedness

1. **Commands vs observed reality** — Big Boss is initially useful, then subtly unreliable, then openly desperate at the original late-story moments. The radio is neither a universal quest marker nor a narrator who already knows every room. His voice must be **one voice** across all calls, not a suspicious villainous performance from minute one.
2. **Human memory and evidence** — a prisoner knows one piece of the route; Schneider does not know where Fox's secret cell is; Fox knows about Metal Gear but directs Snake toward the scientist for its vulnerability. Respect whose first-hand knowledge supports a statement. Players assemble the mosaic.
3. **Tonal whiplash** — tense stealth punctuated by a strange guard boast, sometimes absurd equipment/radio humour, then a dangerous corridor. Avoid Hollywood exposition that pre-explains every eccentric boss; make the boss physically credible and let their entrance retain some ridiculous abruptness.
4. **Silence, isolation and scale** — long unscored service tunnels, dead channels, distant machinery, an empty room after an encounter. No compulsory radio or collectible every thirty seconds; these intervals are deliberate recovery and unease.
5. **The Phantom Pain's identity split** — place faint thematic evidence of an older military culture and the 1984 legacy for returning fans, but never a labelled confession, leaked photograph or a guard calling the commander's double by a revealing name. The 1987 player-character's experience remains self-contained. The retcon becomes a second reading, not a different mission.
6. **Fourth-wall rupture** — preserve the extraordinary system-level panic in the late Big Boss intervention. Its 1987 specificity needs a faithful yet platform-aware treatment; do not turn it into a technical quit instruction that actually exits or deletes progress.
7. **Brief encounters** — Shoot Gunner and other named defenders are allowed to arrive with insufficient biographical explanation. Their tactical setup, command over space and body language should provide characterization without a pre-boss monologue about their biography.

**Disjointedness that should *not* be preserved:** arbitrary player-opaque keycards, unclear floor-to-floor connections, a stealth guard who teleports after a player, untelegraphed hits, broken radio audio, random dropped ammunition, missing subtitles, mislabelled identities and invisible puzzle affordances. Those were never essential mysteries.

## Concrete scene design tests for every 3D addition

- What does Snake actually **perceive**, before knowing who is right?
- Why would a staff member/guard/NPC be **here, now**, without a plot director telling them Snake is approaching?
- Does the optional object teach a useful environmental truth without rendering a secret flag or keycard? Is it readable with sound muted?
- Can the player rush past it without losing required information?
- Does the item have a believable owner, provenance and route into the room?
- Does the scene have a quiet and a high-threat variation, with believable guard line of sight and cover?
- Is the evidence's wording safe from the 1987 mystery and 2015 retrospective spoiler?
- Is it a validated MSX2 event, later canon context, or a **new 1995 3D interpretation**? Do not blur classifications.
- Does it still read if the user has never played `The Phantom Pain`?
- Has source-only work been clearly separated from imported rig, animation, VO, lighting and 720p playtests?

## Current source implementation (not an editor-quality claim)

`AMGGrayFoxMission` now spawns **five optional physically located evidence props**, in cargo, guard-duty, cell, confiscated-gear and kennel locations. They require actual range/facing/line-of-sight before the player can read a restrained subtitle. They do **not** mutate mission flags, grant inventory, trigger boss/capture, change AI knowledge or require completion. Every text is **original remake-written flavour**, not copied from a Konami script.

The evidence is currently **tiny cube geometry with subtitle strings**, not paper textures, readable prop labels, animation or recorded voices. In the next authored-art pass, substitute legal 1995 paper/metal/CRT material, physically scaled notes and optional pickup camera, with a safe skip path. No graphic claims of Diamond Dogs service history before source review.

## Source authority (not licence to copy)

- 1987 MSX script transcription (verify exact scene): https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/30618
- Japanese MSX2 differences, missing English Schneider/Steve lines: https://tcrf.net/Metal_Gear_(MSX2)
- Movement/room order: https://thesnakesoup.org/metal-gear-msx-walkthrough/
- MSX2 event guide: https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/65155
- Official franchise timeline from Konami: https://eu-support.konami.com/hc/en-gb/articles/9822379926423-Backstory-What-is-the-timeline-of-the-events-that-precede-Metal-Gear-Solid-V-Ground-Zeroes-and-Metal-Gear-Solid-V-The-Phantom-Pain

## Our intended player experience

**They should know where they are, but not necessarily know where they stand.** They feel physical cause and effect, uncover a place that existed before their arrival, and wonder how much of the mission story is true. A repeat player who knows 1984 catches layered details; a first-time player simply experiences a coherent, unnerving 1995 stealth adventure.
