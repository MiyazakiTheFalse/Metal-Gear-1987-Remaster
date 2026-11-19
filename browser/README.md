# Gray Fox browser playtest

Extract the ZIP. On Windows, open **Play-Windows.cmd** to launch a standalone browser tab on localhost. Keep the launcher window open while playing. The opening screen and pause menu show the connected controller name, live stick values and pressed buttons. Focus the playtest and press A. Directly opening **index.html** also supports keyboard play. Localhost and file-based launches use separate browser checkpoints. To use an existing file-based checkpoint, open the same extracted index.html at its previous location. On macOS or Linux, python3 tools/serve_playtest.py starts the localhost version. Choose **Start infiltration** for the expanded mission or **Continue checkpoint** to resume. Snake, Gray Fox, patrol guards and Shoot Gunner use original skinned models rendered locally in WebGL. The game audio also works offline. The comparison checkbox loads the older generic soldier over the network. Attribution is in assets/ASSET_CREDITS.md.

Move with WASD, look with the mouse, and interact with E. Ctrl crouches, Z goes prone, V enters cover, and F strikes or knocks from cover. Hold the right mouse button to aim and use the left to fire. G opens the weapon selector, brackets select, and releasing G confirms. C calls on the transceiver. J opens the intelligence log.

On a standard gamepad, the sticks control movement and view. A interacts, B crouches, Y goes prone, X strikes, and the triggers aim and fire. The bumpers open equipment. Back calls on the radio.

The source target is the Connect2Media J2ME edition. The prison sequence follows comparative MSX2 evidence. Geometry, dialogue, stun equipment, lighting and timing are provisional. For isolated testing, append ?cell to the file URL before starting. Optional ?escort enables the earlier companion behaviour.


Editable GLBs are included in assets/models in the playtest ZIP. Rebuild them from the browser mesh and motion sources with `node tools/export_field_rigs.cjs`. The body rigs have 16 joints and idle, walk, run, crouch, prone and down clips. Fox also has a captive pose. Shoot Gunner adds jaw and brow joints and four performance clips. These are stylised playtest models. Unreal import and production character art are pending.

Choose **Rehearse Shoot Gunner** on the opening menu to enter the property hall with recovered gear and a separate checkpoint. Its low cart and tall freight trunks offer different sightlines.

After Shoot Gunner falls, recover Card 3 from the floor. Face the reader beside the east shutter and press E or gamepad A, then cross when ready. At the far threshold you can review the debrief, reopen Fox’s intelligence or return to play. Older saves with an open exit retain access. The badge, reader and shutter choreography are original playtest assets.

The processing desk and recovered kit share an audible and captioned two-tone cue. To inspect the kit, hold Tab or LB, cycle items, and use E or A on the unlisted tag to set it down. Releasing the equipment menu keeps it. The boss reacts to individual signals in the property hall. This interaction and its dialogue are remake additions.
