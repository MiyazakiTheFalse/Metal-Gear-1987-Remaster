# J2ME runtime tracing — initial disassembly findings

The input is the user's licensed `Metal-Gear_J2ME_EN_v330.zip` archive,
with the `Metal Gear (2012)(Connect2Media)(v3.3.0)(a1).jar` build chosen as reference.
Findings below are from local `javap -c -p` inspection of that exact JAR;
no decompiled code or game assets are published here.

## Confirmed entry points

- The archive has 18 Java class files. `Main` extends `javax.microedition.midlet.MIDlet` and launches an instance of obfuscated class `j`.
- Class `j` extends `javax.microedition.lcdui.Canvas` and is the largest class (~157 KB), containing the main graphical/game logic surface.
- `j.b(int)` (private static) concatenates the literal `f` with the numeric argument and calls `Main.b.getResourceAsStream(...)`. This resolves the packaged numbered `f1` through `f74` records.
- `j.c(int)` reads a four-byte big-endian resource length, strips the upper two flag bits with `0x3fffffff`, optionally reads another length header when invoked with a negative parameter and appropriate flags, and delegates body reading to `j.d(int)`.
- A method `j.b(boolean)` has a state branch that reads a file number from an indexed array (`cq[53 + cq[1]]`) and invokes `j.b(int)` to open that record. This is evidence the resource loader is driven by game-state arrays, not merely by a fixed list of textures.
- The source JAR includes 74 numbered resources, 56 root MIDI tracks, and other supporting files such as `boxal.inf`.

## Structural interpretation

The prefix `0x40000000 | payload_size` on every numbered record is compatible
with the disassembled byte-length logic, but the *inner* formats are different.
For example, `f22` begins with a second `0x40000307` header after its outer wrapper, whereas `f72` begins with bytes `0x000002b2`.
Do **not** assume these inner bytes describe a level without tracing the callers.

## Reproduce locally

1. Run `python tools/inspect_and_extract.py <original-zip> --output content`.
2. Run `javap -classpath content/reference/source.jar -private j` to inspect methods.
3. Run `javap -classpath content/reference/source.jar -c -p j` for code.
4. Search for `getResourceAsStream`, `Method b:(I)V`, and `Method c:(I)[B`.

## Next reverse-engineering milestone

Trace writes into the `cq`, `cr`, `cs`, `ct` arrays around the resource-reading
state machine, identify room dimensions and event flags with repeatable test
vectors, then export **data-only, versioned JSON level manifests** for Unreal.
This must be validated by comparisons with running J2ME gameplay before
claiming the imported rooms are faithful.

All original assets and source bytecode stay out of the public repository.
