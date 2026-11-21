#include "MGArtResolver.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    FString MaterialSlug(FName Role)
    {
        const FString Tag = Role.ToString().ToLower();
        if (Tag.Contains(TEXT("floor")) || Tag.StartsWith(TEXT("m01_")))
            return TEXT("garage_floor");
        if (Tag.Contains(TEXT("wall")) || Tag.Contains(TEXT("boundary")) ||
            Tag.Contains(TEXT("divider")) || Tag.Contains(TEXT("concrete")))
            return TEXT("concrete");
        if (Tag.Contains(TEXT("truck")) || Tag.Contains(TEXT("crate")) ||
            Tag.Contains(TEXT("cargo")))
            return TEXT("green_metal_rust");
        if (Tag.Contains(TEXT("bar")) || Tag.Contains(TEXT("gate")))
            return TEXT("painted_rusty_metal_gate");
        if (Tag.Contains(TEXT("electric")) || Tag.Contains(TEXT("hazard")) ||
            Tag.Contains(TEXT("landing")))
            return TEXT("metal_plate");
        if (Tag.Contains(TEXT("door")) || Tag.Contains(TEXT("locker")) ||
            Tag.Contains(TEXT("service")) || Tag.Contains(TEXT("bed")))
            return TEXT("rusty_painted_metal");
        return TEXT("concrete");
    }
}
void FMGArtResolver::ApplySurface(UStaticMeshComponent* Component, FName RoleTag)
{
    if (!IsValid(Component)) return;
    const FString Slug = MaterialSlug(RoleTag);
    const FString ObjectPath = FString::Printf(
        TEXT("/Game/OuterHeaven/Art/Materials/M_%s.M_%s"), *Slug, *Slug);
    UMaterialInterface* Material = LoadObject<UMaterialInterface>(
        nullptr, *ObjectPath);
    if (!Material) return; // Explicitly retain greybox until art is imported.
    Component->SetMaterial(0, Material);
    UMaterialInstanceDynamic* MID = Component->CreateAndSetMaterialInstanceDynamic(0);
    if (MID)
    {
        const FVector Size = Component->GetComponentScale();
        // Import script exposes a UVTiling parameter on every authored surface.
        // Floors/repeated wall sections tile instead of stretching a 2k
        // concrete photograph over a whole 24m/58m modular area.
        const float Tile = FMath::Clamp(
            FMath::Max(FMath::Abs(Size.X), FMath::Abs(Size.Y)) / 2.f,
            1.f, 36.f);
        MID->SetScalarParameterValue(TEXT("UVTiling"), Tile);
    }
    if (Slug.Contains(TEXT("metal")))
        Component->ComponentTags.AddUnique(TEXT("MG_Surface_Metal"));
}
bool FMGArtResolver::TryBindSkeletalVisual(
    USkeletalMeshComponent* Component, const TCHAR* MeshObjectPath,
    const TCHAR* AnimBlueprintClassPath)
{
    if (!IsValid(Component)) return false;
    USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(
        nullptr, MeshObjectPath);
    if (!Mesh) return false;
    Component->SetSkeletalMeshAsset(Mesh);
    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (UClass* AnimClass = LoadObject<UClass>(
        nullptr, AnimBlueprintClassPath))
    {
        if (AnimClass->IsChildOf(UAnimInstance::StaticClass()))
            Component->SetAnimInstanceClass(AnimClass);
    }
    return true;
}
