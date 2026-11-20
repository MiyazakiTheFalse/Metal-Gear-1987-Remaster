#include "MGPrisonerCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "MGPrototypeAnimationFallback.h"
#include "UObject/ConstructorHelpers.h"

AMGPrisonerCharacter::AMGPrisonerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
    PlaceholderBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrisonerPlaceholder"));
    PlaceholderBody->SetupAttachment(SceneRoot);
    PlaceholderBody->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.78f));
    PlaceholderBody->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PlaceholderBody->SetCollisionResponseToAllChannels(ECR_Ignore);
    PlaceholderBody->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) PlaceholderBody->SetStaticMesh(Cube.Object);

    CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GrayFoxSkeletalBody"));
    CharacterMesh->SetupAttachment(SceneRoot);
    // Actor origin at standing body centre, approximately 89 cm above floor.
    CharacterMesh->SetRelativeLocation(FVector(0.f, 0.f, -89.f));
    CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMGPrisonerCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (!CharacterMesh->GetSkeletalMeshAsset())
        if (USkeletalMesh* Asset = LoadObject<USkeletalMesh>(nullptr,
                TEXT("/Game/Characters/GrayFox/SK_GrayFox_1995_Prisoner.SK_GrayFox_1995_Prisoner")))
            CharacterMesh->SetSkeletalMeshAsset(Asset);
    if (HasAuthoredCharacterArt())
    {
        if (!CharacterMesh->GetAnimClass())
            if (UClass* Class = LoadClass<UAnimInstance>(nullptr,
                    TEXT("/Game/Characters/GrayFox/ABP_GrayFox_1995.ABP_GrayFox_1995_C")))
                CharacterMesh->SetAnimInstanceClass(Class);
        // While the production AnimationBlueprint is still missing, use the
        // checked-in, actually animated breathing Idle source. Never leave a
        // successful skeletal import frozen in its reference T-pose.
        if (!CharacterMesh->GetAnimClass())
            if (UAnimSequence* LivingIdle = LoadObject<UAnimSequence>(nullptr,
                    TEXT("/Game/Characters/GrayFox/Animations/AS_GrayFox_Idle.AS_GrayFox_Idle")))
                CharacterMesh->PlayAnimation(LivingIdle, true);
        PlaceholderBody->SetVisibility(false, true);
        PlaceholderBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CharacterMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CharacterMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
        CharacterMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
        UpdatePrototypeFallbackAnimation();
    }
}

void AMGPrisonerCharacter::UpdatePrototypeFallbackAnimation()
{
    // Captive Fox visibly recovers into wary idle when the REAL rescue commits.
    // His material/actor stays identical; no second Fox or story flag is created.
    MGPrototypeAnimationFallback::Apply(CharacterMesh,
        TEXT("/Game/Characters/GrayFox"), TEXT("GrayFox"),
        bRescued ? TEXT("IdleWary") : TEXT("Captive"),
        PrototypeAnimationRequest);
}

void AMGPrisonerCharacter::SetRescued(bool bInRescued)
{
    bRescued = bInRescued;
    UpdatePrototypeFallbackAnimation();
}

bool AMGPrisonerCharacter::HasAuthoredCharacterArt() const
{
    return CharacterMesh && CharacterMesh->GetSkeletalMeshAsset() != nullptr;
}
