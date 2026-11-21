#pragma once

#include "CoreMinimal.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;

/**
 * Bridges source-authored, collision-first level geometry to licensable art.
 * Never invents a loaded asset. Once tools/unreal_import_pbr.py imports
 * CC0 maps into /Game/OuterHeaven/Art/Materials, existing mission actors
 * immediately gain real tiled PBR surfaces on their next PIE session.
 */
struct MGREMASTER_API FMGArtResolver
{
    static void ApplySurface(UStaticMeshComponent* Component, FName RoleTag);
    static bool TryBindSkeletalVisual(USkeletalMeshComponent* Component,
                                      const TCHAR* MeshObjectPath,
                                      const TCHAR* AnimBlueprintClassPath);
};
