"""Guard source-level parity; does not replace a compiled UE5.7 playtest."""
from pathlib import Path
import re
import unittest
ROOT=Path(__file__).resolve().parents[1]
RUST=(ROOT/"rust/src/gray_fox.rs").read_text(encoding="utf-8")
HDR=(ROOT/"rust/include/mg_legacy_core.h").read_text(encoding="utf-8")
MISSION=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp").read_text(encoding="utf-8")
EQUIP=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGEquipmentComponent.cpp").read_text(encoding="utf-8")
SAVE=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Public/MGGrayFoxSaveGame.h").read_text(encoding="utf-8")
FLAGS={
    "SHOTMAKER_ENCOUNTERED":1<<6,"GEAR_RECOVERED":1<<7,
    "SHOTMAKER_DEFEATED":1<<8,"SLICE_COMPLETED":1<<9,
    "TRANSMITTER_PLANTED":1<<10,"TRANSMITTER_REMOVED":1<<11,
    "CARD3_RECOVERED":1<<12,"DETENTION_INTELLIGENCE":1<<13,
}
class ExtendedTransactionSourceTests(unittest.TestCase):
    def test_rust_c_abi_bit_parity(self):
        for name, val in FLAGS.items():
            self.assertIn("pub const "+name+": u32 = 1 << "+str(val.bit_length()-1),RUST)
            self.assertIn("#define MG_FOX_"+name+" "+str(val)+"u",HDR)
    def test_gear_capture_and_card_restoration(self):
        self.assertIn('Inventory->SetConfiscated((Flags & MG_FOX_CAPTURED)',MISSION)
        self.assertIn('&& !(Flags & MG_FOX_GEAR_RECOVERED)',MISSION)
        self.assertIn('Inventory->AwardItem(TEXT("CARD3"), 1)',MISSION)
        self.assertIn('FName(TEXT("CARD3"))',EQUIP)
        self.assertIn('flags | CARD3_RECOVERED',RUST)
    def test_save_migration_remains_explicit(self):
        self.assertIn('const bool bLegacy = Save->SchemaVersion == 1',MISSION)
        self.assertIn('Save->SchemaVersion = 4;',MISSION)
        self.assertIn('Save->bEarnedHandgun = bEarlyHandgunAcquired;',MISSION)
        self.assertIn('uint32 SchemaVersion = 4;',SAVE)
        self.assertIn('bool bEarnedHandgun = false;',SAVE)
        self.assertIn('Save->SchemaVersion == 2',MISSION)
        self.assertIn('Save->SchemaVersion == 3 || Save->SchemaVersion == 4',MISSION)
        self.assertIn('16383u',MISSION)
    def test_no_card_three_before_boss_defeat(self):
        self.assertIn('ACTION_COLLECT_CARD3 if flags & SHOTMAKER_DEFEATED != 0',RUST)
        self.assertIn('ACTION_COMPLETE_SLICE if flags & CARD3_RECOVERED != 0',RUST)
    def test_transmitter_and_gear_are_distinct(self):
        self.assertIn('flags | GEAR_RECOVERED | TRANSMITTER_PLANTED',RUST)
        self.assertIn('ACTION_REMOVE_TRANSMITTER if flags & TRANSMITTER_PLANTED != 0',RUST)

if __name__=="__main__":
    unittest.main()
