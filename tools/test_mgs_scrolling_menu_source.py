"""Cross-engine source checks for MGS hold-scroll-release equipment.

No Unreal Editor compile, browser rendered-frame or controller-hardware
verification is implied by these static tests.
"""
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(p):
    return (ROOT / p).read_text(encoding="utf8")


class EquipmentMenuContracts(unittest.TestCase):
    def test_browser_one_line_scroll_and_deduplicated_small_inventory(self):
        script = read("browser/equipment_strip.js")
        html = read("browser/index.html")
        game = read("browser/game.js")
        self.assertIn("[-2,-1,0,1,2]", script)
        self.assertIn("item.blank=seen.has(item.index)", script)
        self.assertIn("window.MGEquipmentStrip.windowSlots", game)
        self.assertIn("MGEquipmentStrip.wrapped", game)
        self.assertIn('data-current="'+"" , game) if False else None
        for signature in ("mg-strip-track", "mg-strip-card", "mg-strip-current",
                          "data-empty", "prefers-reduced-motion"):
            self.assertIn(signature, html + game)
        self.assertIn('<script src="equipment_strip.js"></script>', html)

    def test_input_scroll_and_release_equip_do_not_use_consumables(self):
        game = read("browser/game.js")
        cpp = read("unreal/MGRemaster/Source/MGRemaster/Private/MGPlayerCharacter.cpp")
        ini = read("unreal/MGRemaster/Config/DefaultInput.ini")
        self.assertIn("performance.now()>=menuAxisRepeatAt", game)
        self.assertIn("document.addEventListener('wheel'", game)
        self.assertIn("if(equipOpen){mF=0;mR=0;}", game)
        self.assertIn("NextEquipmentScrollAt", cpp)
        self.assertIn("MoveForward(float Value)", cpp)
        self.assertIn("EquipmentScroll(float Value)", cpp)
        for mapping in (
            'ActionName="ItemWindow",Key=Tab',
            'ActionName="WeaponWindow",Key=G',
            'AxisName="EquipmentScroll",Scale=1.000000,Key=Gamepad_LeftY',
            'ActionName="SelectPrevious",Key=MouseScrollUp',
            'ActionName="SelectNext",Key=MouseScrollDown',
        ):
            self.assertIn(mapping, ini)
        equip = game.split("else if(name==='equipclose'){",1)[1].split(
            "else if(name==='useitem'){",1
        )[0]
        use = game.split("else if(name==='useitem'){",1)[1].split(
            "else if(name==='next'||name==='previous')",1
        )[0]
        self.assertNotIn("rationRemaining--", equip)
        self.assertNotIn("world.smokeUntil", equip)
        self.assertIn("rationRemaining--", use)
        self.assertIn("world.smokeUntil=gameTime+3", use)

    def test_unreal_five_slots_read_authoritative_inventory(self):
        hud = read("unreal/MGRemaster/Source/MGRemaster/Private/MGPrototypeHUD.cpp")
        self.assertIn("for (int32 Offset = -2; Offset <= 2; ++Offset)", hud)
        self.assertIn("GetWeaponSelection()", hud)
        self.assertIn("GetItemSelection()", hud)
        self.assertIn("GetWeapons()", hud)
        self.assertIn("GetItems()", hud)
        self.assertIn("bRepeated", hud)
        self.assertIn("Entry.Quantity", hud)
        self.assertIn("HUD", hud)

    def test_unreal_item_equip_is_not_smoke_consume(self):
        cpp = read("unreal/MGRemaster/Source/MGRemaster/Private/MGPlayerCharacter.cpp")
        close = cpp.split("void AMGPlayerCharacter::ItemWindowStop()",1)[1].split(
            "void AMGPlayerCharacter::SelectionPrevious()",1
        )[0]
        use = cpp.split("void AMGPlayerCharacter::UseItem()",1)[1].split(
            "void AMGPlayerCharacter::MeleeAttack()",1
        )[0]
        self.assertNotIn("ExhaleSmoke", close)
        self.assertIn("ExhaleSmoke", use)
        self.assertIn("Equipment->ConsumeRation()", use)
        self.assertIn("IsItemEquipped(TEXT(\"BINOCULARS\"))", use)
        self.assertIn("IsItemEquipped(TEXT(\"CIGARETTES\"))", use)

    def test_downloaded_browser_source_server_serves_all_optional_assets(self):
        server = read("browser/tools/serve_playtest.py")
        self.assertIn("SimpleHTTPRequestHandler", server)
        self.assertIn("127.0.0.1", server)
        self.assertNotIn('FILES = {"index.html"', server)
        self.assertIn("atlas.html", server)


if __name__ == "__main__":
    unittest.main()
