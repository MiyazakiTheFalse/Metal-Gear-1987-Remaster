#include "MGBodyHidingSpot.h"
#include "MGGuardActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

AMGBodyHidingSpot::AMGBodyHidingSpot()
{
    PrimaryActorTick.bCanEverTick = true;
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("LockerRoot"));
    SetRootComponent(Root);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(
        TEXT("/Engine/BasicShapes/Cube.Cube"));

    auto MakePart = [this, &Cube](const TCHAR* Name, USceneComponent* Parent,
                                FVector Position, FVector Scale) -> UStaticMeshComponent*
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Part->SetupAttachment(Parent);
        if (Cube.Succeeded()) Part->SetStaticMesh(Cube.Object);
        Part->SetRelativeLocation(Position);
        Part->SetRelativeScale3D(Scale);
        Part->SetCollisionProfileName(TEXT("BlockAll"));
        return Part;
    };

    // A utilitarian two-metre tall steel cabinet: framing, recess, separate
    // hinged face and an actual grip. No floating interaction marker.
    ShellRear = MakePart(TEXT("SteelRear"), Root,
                         FVector(-39,0,0), FVector(.08f,.86f,1.96f));
    ShellLeft = MakePart(TEXT("SteelSideL"), Root,
                         FVector(0,-44,0), FVector(.82f,.06f,1.96f));
    ShellRight = MakePart(TEXT("SteelSideR"), Root,
                         FVector(0,44,0), FVector(.82f,.06f,1.96f));
    ShellTop = MakePart(TEXT("SteelRoof"), Root,
                         FVector(0,0,99), FVector(.82f,.92f,.06f));
    BasePlate = MakePart(TEXT("RaisedBase"), Root,
                         FVector(0,0,-97), FVector(.82f,.92f,.08f));
    DoorHinge = CreateDefaultSubobject<USceneComponent>(TEXT("DoorHinge"));
    DoorHinge->SetupAttachment(Root);
    DoorHinge->SetRelativeLocation(FVector(40,-42,0));
    DoorPlate = MakePart(TEXT("HingedDoor"), DoorHinge,
                         FVector(0,42,0), FVector(.07f,.83f,1.88f));
    // Handle is on the free edge, out from the steel front.
    DoorHandle = MakePart(TEXT("Grip"), DoorHinge,
                          FVector(8,76,0), FVector(.08f,.09f,.32f));
    DoorPlate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DoorHandle->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FacilitySign = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ServiceMarking"));
    FacilitySign->SetupAttachment(DoorHinge);
    FacilitySign->SetRelativeLocation(FVector(9,12,62));
    FacilitySign->SetRelativeRotation(FRotator(0,0,90));
    FacilitySign->SetWorldSize(14.f);
    FacilitySign->SetHorizontalAlignment(EHTA_Center);
    FacilitySign->SetText(FText::FromString(TEXT("SERVICE / STORAGE")));
    FacilitySign->SetTextRenderColor(FColor(196,208,190,255));

    // Industrial palette rather than a naked engine-grey cube. Engine basic
    // material has a color parameter in UE's default BasicShapes asset.
    if (UStaticMesh* Mesh = Cube.Object)
    {
        if (UMaterialInterface* Base = ShellRear->GetMaterial(0))
        {
            auto Tint = [this, Base](UStaticMeshComponent* Part, const FLinearColor& Color)
            {
                UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
                MID->SetVectorParameterValue(TEXT("Color"), Color);
                Part->SetMaterial(0, MID);
            };
            for (UStaticMeshComponent* Part : {ShellRear,ShellLeft,ShellRight,ShellTop,BasePlate})
                Tint(Part, FLinearColor(.18f,.24f,.23f,1.f));
            Tint(DoorPlate, FLinearColor(.32f,.40f,.36f,1.f));
            Tint(DoorHandle, FLinearColor(.72f,.71f,.56f,1.f));
        }
    }
}
void AMGBodyHidingSpot::SetFacilityLabel(const FString& Text)
{
    FacilitySign->SetText(FText::FromString(Text));
}
bool AMGBodyHidingSpot::TryStow(AMGGuardActor* Body)
{
    if (!IsAccessible() || !IsValid(Body) || !Body->IsIncapacitated())
        return false;
    StoredBody = Body;
    bOccupied = true; // reserve before door animation to prevent duplication
    bOpening = true;
    LoadingHold = 0.f;
    return true;
}
void AMGBodyHidingSpot::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bOpening)
    {
        DoorSwing = FMath::FInterpConstantTo(DoorSwing, 108.f, DeltaSeconds, 240.f);
        if (DoorSwing >= 107.f)
        {
            bOpening = false;
            if (IsValid(StoredBody))
            {
                StoredBody->SetBodyDragged(false);
                StoredBody->ConcealBody(this);
            }
            LoadingHold = 0.65f;
        }
    }
    else if (LoadingHold > 0.f)
    {
        LoadingHold = FMath::Max(0.f, LoadingHold - DeltaSeconds);
        if (LoadingHold <= 0.f) bClosing = true;
    }
    else if (bClosing)
    {
        DoorSwing = FMath::FInterpConstantTo(DoorSwing, 0.f, DeltaSeconds, 190.f);
        if (DoorSwing <= 0.2f)
        {
            DoorSwing = 0.f;
            bClosing = false;
        }
    }
    DoorHinge->SetRelativeRotation(FRotator(0,DoorSwing,0));
}
