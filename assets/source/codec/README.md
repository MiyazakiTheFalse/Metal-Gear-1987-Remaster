# Original 1995 codec portraits — art-source contract

These are **original source-authored illustrations**, traced from NO copyrighted
character sprite, model or actor photograph. The identity details are researched
from Konami's *Metal Gear* (1987) Japanese MSX2 manual character plates on
pages 10–11 (https://www.ne.jp/asahi/hzk/kommander/mg1manual.pdf).

**Current:** eight editable neutral SVGs checked in here; the canonical art
generator lives in `browser/codec_portraits.js`. To regenerate all 24
`Neutral`, `Talk` and `Blink` variants for eight identities, run:

```sh
node tools/export_codec_portraits.cjs
```

For Unreal texture import, run `python tools/build_codec_portraits.py` on a
machine with Node and the optional `cairosvg` Python package, then execute
`unreal/MGRemaster/Content/Python/import_codec_portraits_1995.py` from the
Unreal Editor Python console. The import task preserves already-authored
textures instead of silently replacing them.

The Unreal HUD uses `/Game/UI/Codec/T_Codec_<identity>_<expression>` and
caches those texture assets. The browser renders the **same character-specific
vector source** with CSS-driven idle head sway, blink and approximate viseme
opening during radio speech. No face-tracked voice performance is implied.

**Source-facing reference constraints:**

| Identity | Manual evidence | Current visual cue |
|---|---|---|
| Solid Snake | Young rookie, swept/longer hair, teal field shirt | Young face, loose hair, field shirt, no later-game grey hair |
| Big Boss | Older military commander, beard, eyepatch | Heavy beard, patch over his right eye, field jacket |
| Kyle Schneider | Short-swept hair, resistance engineer | Short spiked hair, functional civilian clothing |
| Diane | Distinctive short sculpted hair, former punk singer | Sculpted swept hairstyle, dark top |
| Jennifer | Cap, wavy longer hair, medical infiltration | Cap, hair and field-medical tunic |
| Gray Fox | Bandaged forehead, tired man, prisoner | Wrap and exhausted features, no later cyborg suit |
| Dr. Pettrovich | Elderly, thinning/bald scalp, spectacles | Glasses, tired face, pale clothing |
| Ellen Pettrovich | Long straight dark hair | Long hair and civilian clothing |

**Not finished:** skin photogrammetry/PBR material maps, approved high-poly
3D portraits, facial rig, genuinely measured phonemes, individual voice-over,
full-body mesh parity and Unreal in-editor footage. Replace the illustrated
source with rigorously approved realistic portraits without changing the
scene/animator interface or character IDs. Never advertise these 2D vectors
as final AAA-quality likenesses.
