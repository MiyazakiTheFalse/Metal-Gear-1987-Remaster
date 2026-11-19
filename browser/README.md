# Gray Fox browser playtest

Extract the entire package and open **index.html** in a desktop browser with WebGL. Choose **Start infiltration** for the expanded mission or **Continue checkpoint** to resume. Snake, Gray Fox and patrol guards use original skinned models rendered locally in WebGL. The game audio also works offline. The comparison checkbox loads the older generic soldier over the network. Attribution is in assets/ASSET_CREDITS.md.

Move with WASD, look with the mouse, and interact with E. Ctrl crouches, Z goes prone, V enters cover, and F strikes or knocks from cover. Hold the right mouse button to aim and use the left to fire. G opens the weapon selector, brackets select, and releasing G confirms. C calls on the transceiver. J opens the intelligence log.

On a standard gamepad, the sticks control movement and view. A interacts, B crouches, Y goes prone, X strikes, and the triggers aim and fire. The bumpers open equipment. Back calls on the radio.

The source target is the Connect2Media J2ME edition. The prison sequence follows comparative MSX2 evidence. Geometry, dialogue, stun equipment, lighting and timing are provisional. For isolated testing, append ?cell to the file URL before starting. Optional ?escort enables the earlier companion behaviour.


Editable GLBs are included in assets/models in the playtest ZIP. Rebuild them from the browser mesh and motion sources with `node tools/export_field_rigs.cjs`. Each includes 16 bones and idle, walk, run, crouch, prone and down clips. Fox also has a captive pose. These are stylised playtest models. Unreal import and production character art are pending.
