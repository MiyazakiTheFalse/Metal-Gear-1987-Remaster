#pragma once

// Lightweight, source-controlled animation fallback while final AnimBPs are
// missing. This NEVER decides collision, awareness, weapon hits or mission flags.
// Source names match Content/Python/import_field_character_glbs.py exactly.
// Single-node transitions are functional but NOT motion-matched/production blends.
// Authored AnimationBlueprints take precedence without modification.
#include "CoreMinimal.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/UObjectGlobals.h"

namespace MGPrototypeAnimationFallback
{
    inline bool IsLoop(FName Name)
    {
        return Name != TEXT("Down") && Name != TEXT("Hit") &&
               Name != TEXT("Melee") && Name != TEXT("Reaching") &&
               Name != TEXT("Recoil");
    }

    inline UAnimSequence* FindClip(const FString& Folder,
                                  const FString& Prefix, FName Clip)
    {
        const FString Stem = FString::Printf(TEXT("%s/Animations/AS_%s_%s"),
            *Folder, *Prefix, *Clip.ToString());
        return LoadObject<UAnimSequence>(nullptr, *(Stem + TEXT(".") +
            FString::Printf(TEXT("AS_%s_%s"), *Prefix, *Clip.ToString())));
    }

    inline void Apply(USkeletalMeshComponent* Mesh,
                      const FString& Folder, const FString& Prefix,
                      FName Requested, FName& LastRequest, float PlayRate = 1.f)
    {
        if (!Mesh || !Mesh->GetSkeletalMeshAsset()) return;
        // Never overwrite a working, artist-authored AnimBlueprint.
        if (Mesh->GetAnimationMode() == EAnimationMode::AnimationBlueprint &&
            Mesh->GetAnimClass() != nullptr) return;
        if (Requested.IsNone()) Requested = TEXT("Idle");
        if (Requested == LastRequest &&
            Mesh->GetAnimationMode() == EAnimationMode::AnimationSingleNode)
        {
            Mesh->SetPlayRate(FMath::Clamp(PlayRate, .6f, 1.6f));
            return;
        }
        UAnimSequence* Clip = FindClip(Folder, Prefix, Requested);
        if (!Clip) Clip = FindClip(Folder, Prefix, TEXT("Idle"));
        if (!Clip) return; // Missing import keeps visible greybox/known state.
        Mesh->PlayAnimation(Clip, IsLoop(Requested));
        Mesh->SetPlayRate(FMath::Clamp(PlayRate, .6f, 1.6f));
        // Cache requested, not fallback key: a missing clip must not trigger
        // asset lookups each frame. A skeletal mesh swap resets this state.
        LastRequest = Requested;
    }
}
