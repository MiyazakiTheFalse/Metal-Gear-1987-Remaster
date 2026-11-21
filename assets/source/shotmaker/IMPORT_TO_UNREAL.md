# Original Shotmaker OBJ/WAV → Unreal Engine 5.7

Enable Python Editor Script Plugin and required Interchange/OBJ mesh import. In the **open Editor console**, run:

```python
exec(open(r"/YOUR_REPO/unreal/MGRemaster/Content/Python/import_shotmaker_source_assets.py", encoding="utf-8").read())
```

It imports `riot_gun_game_mesh.obj` as `/Game/Weapons/Shotmaker/SM_RiotShotgun_1995` and three original WAV effects as `/Game/Audio/Shotmaker/Shotmaker_AimCue`, `Shotmaker_Blast` and `Shotmaker_Cycle`. The boss C++ code loads these exact asset paths. It refuses silent overwrites and checks imported types. OBJ import may require an Editor plugin or manual FBX conversion depending on configuration; the task fails explicitly if unavailable.

The multi-object `riot_gun_proxy.obj` remains individually editable; the single-object game export uses named groups for the same parts, but it is not a skeletal mesh. **This is original author-created prototype source**, not a historical KS-23 identification or finished PBR MGS4-quality art. Verify dimensions, orientation, shoulder/hand contact, muzzle position, rig/animations, spatial audio, performance and 720p capture before approving it.
