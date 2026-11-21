"""Unreal source contracts for honest muzzle/camera/weapon presentation.

PIE, animation alignment and real wall-penetration tests still required.
"""
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "unreal/MGRemaster/Source/MGRemaster/Private"
read = lambda name: (ROOT / name).read_text(encoding="utf-8")


class UnrealWeaponPresentationTests(unittest.TestCase):
    def test_player_trace_origin_matches_visible_flash_and_proxy_scale(self):
        src = read("MGPlayerCharacter.cpp")
        self.assertIn('PlayerMuzzleFlashLight->GetComponentLocation()', src)
        self.assertIn('WeaponBodyVisual->GetSocketLocation(TEXT("muzzle"))', src)
        self.assertIn('Authored ? FVector(20.f, 0.f, 1.f) : FVector(52.f, 0.f, 0.f)', src)
        self.assertIn('WeaponBodyVisual->GetAttachParent() != GetCapsuleComponent()', src)
        self.assertIn('WeaponBodyVisual->SetVisibility(bCanHoldProp, true)', src)
        self.assertIn('const bool bCanHoldProp = HasReadyWeapon()', src)

    def test_guard_trace_origin_matches_rendered_sidearm_and_flash(self):
        src = read("MGGuardActor.cpp")
        self.assertIn('SecuritySidearm->SetStaticMesh(PistolProxy.Object)', src)
        self.assertIn('SidearmMuzzleLight->GetComponentLocation()', src)
        self.assertIn('SidearmMuzzleLight->SetRelativeLocation(FVector(52.f, 0.f, 0.f))', src)
        self.assertIn('SidearmMuzzleLight->SetRelativeLocation(FVector(21.f, 0.f, 3.f))', src)
        self.assertIn('SecuritySidearm->GetSocketLocation(TEXT("muzzle"))', src)
        self.assertIn('skinned art lacks hand_R/weapon_r', src)

    def test_machinegun_kid_uses_flash_transform_for_authoritative_ray(self):
        src = read("MGMachinegunKid.cpp")
        self.assertIn('Flash->GetComponentLocation()', src)
        self.assertIn('Beat == EBeat::Burst', src)
        self.assertIn('MGKidLane', src)
        self.assertIn('UGameplayStatics::ApplyDamage(Player', src)
        self.assertIn('Receiver->GetRelativeScale3D()', src)

    def test_shotmaker_keeps_real_bore_obstruction_and_pump_cycle(self):
        src = read("MGShootGunnerActor.cpp")
        self.assertIn('FVector AMGShootGunnerActor::GetActualMuzzleLocation()', src)
        self.assertIn('WeaponBody->GetComponentTransform()', src)
        self.assertIn('GetActualMuzzleLocation()', src)
        self.assertIn('PendingPumpShotIndex', src)
        self.assertIn('FMath::Clamp(PelletDamage', src)


if __name__ == "__main__":
    unittest.main()
