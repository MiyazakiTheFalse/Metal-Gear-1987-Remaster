import json
import copy
import unittest
from validate_slice_asset_inventory import validate,ROOT,DATA

class SliceAudioAndInventorySourceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.base=json.loads(DATA.read_text(encoding="utf-8"))
        cls.player=(ROOT/"unreal/MGRemaster/Source/MGRemaster/Private/MGPlayerCharacter.cpp").read_text()
    def test_assets_and_wav_headers(self):
        self.assertEqual(validate(self.base),[])
    def test_radio_cues_are_played_at_real_call_transitions(self):
        for token in ("PlayRadioCue(IncomingRadioSfx","PlayRadioCue(ConnectRadioSfx",
                      "PlayRadioCue(TuneRadioSfx","PlayRadioCue(CloseRadioSfx",
                      "PlayRadioCue(SignalRadioSfx"):
            self.assertIn(token,self.player)
        self.assertIn("IncomingConnectionDelay = -1.f",self.player)
    def test_invalid_shipped_status_rejected(self):
        data=copy.deepcopy(self.base)
        data["entries"][0]["production_approved"]=True
        self.assertTrue(any("false approval" in x for x in validate(data)))
    def test_music_missing_not_falsely_claimed(self):
        music=[e for e in self.base["entries"] if e["category"]=="music"]
        self.assertGreater(len(music),2)
        self.assertTrue(all(e["status"]=="missing" for e in music))
if __name__=="__main__": unittest.main()
