# What this copy of ArmorPaint adds

This repository is a copy of [ArmorPaint](https://github.com/armory3d/armorpaint), the open-source 3D texture
painting app, kept by the collective that uses Singerie. Its `main` branch is upstream's, unchanged. The branch
`sng/main` adds one folder, [`layer/`](layer/README.md), that makes ArmorPaint feel more like Substance 3D Painter:
Painter's shortcuts and a Painter-like dark theme as the defaults, export presets that name the textures the way the
collective's pipeline expects, and new script functions plugins can use to build layer stacks, masks and groups,
undo and redo, set the bake settings and export. Nothing in upstream's own files is changed; the new functions are
compiled in through a hook upstream provides for exactly that. Run `git diff main...sng/main` to see everything this
copy adds.

Builds made from `sng/main` are altered versions of ArmorPaint, "based on ArmorPaint", and say so
(`ALTERED_VERSION.txt` beside the program, `+sng.` in the version shown in the About box). ArmorPaint's licence
(zlib, `license.md`) applies to ArmorPaint; the layer's own files are zlib too (`layer/LICENSE.md`). ArmorPaint's
author funds his work by selling ready-made builds at [armorpaint.org](https://armorpaint.org/download).
