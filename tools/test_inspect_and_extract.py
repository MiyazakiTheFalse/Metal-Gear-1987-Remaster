"""Dependency-free smoke tests; run: python -m unittest discover -s tools -p 'test_*.py'."""
import io
import tempfile
import unittest
from pathlib import Path
from zipfile import ZipFile

from inspect_and_extract import audit, parse_record, embedded_pngs


def make_game_jar():
    buf = io.BytesIO()
    with ZipFile(buf, "w") as game:
        game.writestr("META-INF/MANIFEST.MF", "MIDlet-Name: MG\nMicroEdition-Profile: MIDP-2.0\nMicroEdition-Configuration: CLDC-1.1\n")
        for n in range(1, 75):
            payload = bytes([n])
            game.writestr(f"f{n}", (0x40000000 | len(payload)).to_bytes(4, "big") + payload)
        game.writestr("theme.mid", b"MThd")
    return buf.getvalue()


class AuditTests(unittest.TestCase):
    def test_length_wrapper(self):
        self.assertEqual(parse_record(b"\x40\x00\x00\x02ab"), b"ab")
        with self.assertRaises(ValueError):
            parse_record(b"\x40\x00\x00\x03ab")

    def test_empty_png_records(self):
        self.assertEqual(list(embedded_pngs(b"abcde")), [])

    def test_synthetic_archive(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "source.zip"
            with ZipFile(path, "w") as outer:
                outer.writestr("Metal Gear (2012)(Connect2Media)(v3.3.0)(a1).jar", make_game_jar())
            result = audit(path, Path(folder) / "output")
            self.assertEqual(result["numbered_records"], 74)
            self.assertEqual(result["midi_tracks"], 1)
            self.assertEqual(result["embedded_pngs"], 0)

    def test_missing_record_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            jar_bytes = make_game_jar()
            source = io.BytesIO()
            with ZipFile(io.BytesIO(jar_bytes)) as jar, ZipFile(source, "w") as edited:
                for name in jar.namelist():
                    if name != "f74":
                        edited.writestr(name, jar.read(name))
            outer_path = Path(folder) / "bad.zip"
            with ZipFile(outer_path, "w") as outer:
                outer.writestr("bad(a1).jar", source.getvalue())
            with self.assertRaises(ValueError):
                audit(outer_path, Path(folder) / "out")


if __name__ == "__main__":
    unittest.main()
