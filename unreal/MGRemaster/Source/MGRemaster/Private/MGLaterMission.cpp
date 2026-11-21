#include "MGLaterMission.h"
#include "MGSetpieceBoss.h"
#include "MGPlayerCharacter.h"
#include "MGCampaignState.h"
#include "MGEquipmentComponent.h"
#include "MGMissionOneSaveGame.h"
#include "MGGuardActor.h"
#include "MGBodyHidingSpot.h"
#include "MGArtResolver.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AMGLaterMission::AMGLaterMission()
{
    PrimaryActorTick.bCanEverTick = true;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    Cube = Mesh.Succeeded() ? Mesh.Object : nullptr;
}
UMGCampaignState* AMGLaterMission::Story() const
{
    return GetWorld() && GetWorld()->GetGameInstance() ?
        GetWorld()->GetGameInstance()->GetSubsystem<UMGCampaignState>() : nullptr;
}
AStaticMeshActor* AMGLaterMission::Block(FName Id, FVector Center,
                                        FVector Scale, bool bSolid)
{
    if (!Cube || !GetWorld()) return nullptr;
    AStaticMeshActor* A = GetWorld()->SpawnActor<AStaticMeshActor>(Center,
        FRotator::ZeroRotator);
    if (!A) return nullptr;
    A->Tags.AddUnique(Id);
    UStaticMeshComponent* Component = A->GetStaticMeshComponent();
    Component->SetMobility(EComponentMobility::Movable);
    Component->SetStaticMesh(Cube);
    Component->SetWorldScale3D(Scale);
    Component->SetCollisionProfileName(bSolid ? TEXT("BlockAll") : TEXT("NoCollision"));
    FMGArtResolver::ApplySurface(Component,Id);
    return A;
}
void AMGLaterMission::AddPickup(FName Id, FName Kind,
                                FVector Location, int32 Amount, FName Dependency)
{
    FWorldPickup Item;
    Item.Id = Id;
    Item.Kind = Kind;
    Item.Quantity = Amount;
    Item.Prerequisite = Dependency;
    const UMGMissionOneSaveGame* Save = Cast<UMGMissionOneSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SavedSlot,0));
    Item.bTaken = Save && Save->LaterCollectedPickupIds.Contains(Id);
    if (!Item.bTaken)
    {
        Item.Marker = Block(Id,Location,FVector(.8f,.8f,.55f),false);
        if (IsValid(Item.Marker) && !Dependency.IsNone() &&
            (!Story() || !Story()->HasBeat(Dependency)))
            Item.Marker->SetActorHiddenInGame(true);
    }
    Pickups.Add(Item);
}
void AMGLaterMission::SpawnBoss(FName Id,FVector Position,FName Weakness,
                                float HP,float Radius,float Telegraph,float Damage)
{
    if (!GetWorld() || (Story() && Story()->HasBeat(Id))) return;
    if (AMGSetpieceBoss* Boss = GetWorld()->SpawnActor<AMGSetpieceBoss>(
        Position,FRotator(0.f,180.f,0.f)))
        Boss->ConfigureEncounter(this,Id,Weakness,HP,Radius,Telegraph,
                                 Damage,Position);
}
bool AMGLaterMission::IsWithin(const AMGPlayerCharacter* Player,
                               const AActor* Target,float Range) const
{
    if (!IsValid(Player) || !IsValid(Target)) return false;
    const FVector Delta=Player->GetActorLocation()-Target->GetActorLocation();
    return Delta.Size2D()<=Range && FMath::Abs(Delta.Z)<=155.f;
}
void AMGLaterMission::SetSubtitle(const FString& Message,float Seconds)
{
    Dialogue=Message;
    DialogueSeconds=Seconds;
}
void AMGLaterMission::BeginPlay()
{
    Super::BeginPlay();
    UMGCampaignState* State=Story();
    Mission=State ? State->GetMissionNumber() : 3;
    if (Mission==3) BuildExterior();
    else if (Mission==4) BuildBuildingTwo();
    else if (Mission==5) BuildEllenReturn();
    RemoveDefeatedGates();
    if (AMGPlayerCharacter* Player=Cast<AMGPlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(this,0)))
        Player->SetActorLocation(SpawnPoint,false,nullptr,
                                 ETeleportType::TeleportPhysics);
}
void AMGLaterMission::BuildExterior()
{
    // Source-comparative order: Machinegun Kid -> Hind D -> mine-resistant tank.
    // Linear three-encounter exterior is a provisional traversal silhouette.
    Block(TEXT("M03_ExteriorRoad"),FVector(6800,2200,-65),FVector(140,44,1.5f));
    Block(TEXT("M03_NorthBoundary"),FVector(6800,4400,170),FVector(140,.5f,4.7f));
    Block(TEXT("M03_SouthBoundary"),FVector(6800,0,170),FVector(140,.5f,4.7f));
    for (float X : {1400.f,3900.f,6100.f,8500.f,10300.f})
    {
        Block(TEXT("M03_ConcreteCover"),FVector(X,1200,75),FVector(1.8f,5.f,2.8f));
        Block(TEXT("M03_IndustrialCover"),FVector(X+400,3300,75),FVector(2.4f,3.f,2.8f));
    }
    PassageGateA = Block(TEXT("M03_MachinegunGate"),FVector(4250,2200,155),
                         FVector(.6f,44.f,3.7f));
    PassageGateB = Block(TEXT("M03_HindGate"),FVector(8300,2200,155),
                         FVector(.6f,44.f,3.7f));
    PassageGateC = Block(TEXT("M03_TankGate"),FVector(12400,2200,155),
                         FVector(.6f,44.f,3.7f));
    SpawnBoss(TEXT("m03.machinegun_kid_defeated"),FVector(2800,2200,90),
              TEXT("HANDGUN"),175.f,1100.f,1.15f,15.f);
    SpawnBoss(TEXT("m03.hind_d_defeated"),FVector(6600,2100,110),
              TEXT("GRENADE_LAUNCHER"),170.f,1200.f,1.65f,20.f);
    SpawnBoss(TEXT("m03.tank_defeated"),FVector(10800,2250,90),
              TEXT("MINE"),155.f,1200.f,2.f,28.f);
    AddPickup(TEXT("L03_HANDGUN_AMMO"),TEXT("HANDGUN_AMMO"),
              FVector(1100,2400,95),18);
    AddPickup(TEXT("L03_GRENADE_LAUNCHER"),TEXT("GRENADE_LAUNCHER"),
              FVector(4700,1400,95),6);
    AddPickup(TEXT("L03_GRENADE_AMMO"),TEXT("GRENADE_AMMO"),
              FVector(5300,3000,95),7);
    AddPickup(TEXT("L03_MINE_RESTOCK"),TEXT("MINES"),
              FVector(8900,1200,95),6);
    AddPickup(TEXT("L03_RATION"),TEXT("RATION"),
              FVector(10400,3400,95),1);
    // Security staffing follows actual posts. No sudden random reinforcement
    // appears just because a boss has been damaged.
    if (AMGGuardActor* Guard=GetWorld()->SpawnActor<AMGGuardActor>(
        FVector(9400,3800,90),FRotator(0.f,-100.f,0.f)))
    {
        Guard->ConfigureDuty(TEXT("M03_CHECKPOINT_AUX"),
            TEXT("EXTERIOR_GATE_PATROL"),TEXT("TANK_ROAD_ACCESS"),
            FVector(9400,3800,90),2.0f,FVector(10300,2400,90));
        Guard->ConfigurePatrol(TArray<FVector>{
            FVector(9400,3800,90),FVector(10600,3800,90),
            FVector(10600,3100,90)});
    }
    if (AMGBodyHidingSpot* Locker=GetWorld()->SpawnActor<AMGBodyHidingSpot>(
            FVector(5300,3500,95),FRotator(0.f,180.f,0.f)))
        Locker->SetFacilityLabel(TEXT("ROOFTOP SERVICE / 03"));
    TransitionMarker=Block(TEXT("M03_BuildingTwoCheckpoint"),
                            FVector(13300,2200,90),FVector(.8f,.8f,1.7f),false);
    Block(TEXT("M03_ReturnToBuildingOne"),FVector(350,2200,90),
          FVector(.8f,.8f,1.7f),false);
    SpawnPoint=FVector(650,2200,90);
}
void AMGLaterMission::BuildBuildingTwo()
{
    // Still a contiguous, traversable proto-fortress: no magical door after
    // a boss, only doors whose operator/staff duty logically depends on them.
    Block(TEXT("M04_ResearchFloor"),FVector(7300,2200,-65),FVector(150,44,1.5f));
    Block(TEXT("M04_NorthBoundary"),FVector(7300,4400,170),FVector(150,.5f,4.7f));
    Block(TEXT("M04_SouthBoundary"),FVector(7300,0,170),FVector(150,.5f,4.7f));
    for (float X : {1400.f,3000.f,5600.f,8700.f,10300.f})
        Block(TEXT("M04_ArmourCover"),FVector(X,1500,90),FVector(2.5f,2.1f,2.8f));
    PassageGateA=Block(TEXT("M04_BulldozerGarageDoor"),
                       FVector(4300,2200,165),FVector(.6f,44.f,3.7f));
    PassageGateB=Block(TEXT("M04_FireWingDoor"),
                       FVector(8000,2200,165),FVector(.6f,44.f,3.7f));
    PassageGateC=Block(TEXT("M04_TX11SecurityShutter"),
                       FVector(13200,2200,165),FVector(.6f,44.f,3.7f));
    // The rocket store has its own Card 6 door. Jennifer arranges the weapon
    // but Snake still has to enter a physically secured room and collect it.
    Block(TEXT("M04_RocketStoreNorthWall"),FVector(10400,3800,150),
          FVector(22,.5f,3.5f));
    Block(TEXT("M04_RocketStoreWestWall"),FVector(9300,3400,150),
          FVector(.5f,8.f,3.5f));
    Block(TEXT("M04_RocketStoreEastWall"),FVector(11500,3400,150),
          FVector(.5f,8.f,3.5f));
    Block(TEXT("M04_RocketStoreSouthSegmentWest"),FVector(9700,3000,150),
          FVector(8,.5f,3.5f));
    Block(TEXT("M04_RocketStoreSouthSegmentEast"),FVector(11100,3000,150),
          FVector(8,.5f,3.5f));
    RocketRoomDoor=Block(TEXT("M04_RocketStoreCard6Door"),
          FVector(10400,3000,150),FVector(6.f,.5f,3.5f));
    SpawnBoss(TEXT("m04.bulldozer_defeated"),FVector(2700,2200,95),
              TEXT("GRENADE_LAUNCHER"),160.f,1100.f,1.8f,22.f);
    SpawnBoss(TEXT("m04.fire_trooper_defeated"),FVector(6600,2300,95),
              TEXT("HANDGUN"),170.f,1060.f,1.35f,18.f);
    // Fake doctor is encountered before the real Madnar and cannot yield the
    // TX-55 sequence or create a false shortcut to the conclusion.
    ResearchPrisoner=Block(TEXT("M04_FALSE_DOCTOR_CELL"),
                           FVector(8400,3100,90),FVector(.7f,.7f,1.7f),false);
    JenniferPrisoner=Block(TEXT("M04_JENNIFER_CONTACT_POW"),
                           FVector(9250,3000,90),FVector(.7f,.7f,1.7f),false);
    // A source-provisional second POW supports the relationship/rank loop.
    Block(TEXT("M04_OCCUPIED_HOLDING_BAY"),FVector(8850,3850,100),
          FVector(1.5f,.3f,1.8f),true);
    AddPickup(TEXT("L04_GRENADE_AMMO"),TEXT("GRENADE_AMMO"),
              FVector(1000,3150,95),6);
    AddPickup(TEXT("L04_RATION"),TEXT("RATION"),
              FVector(5000,1300,95),1);
    AddPickup(TEXT("L04_CARD6"),TEXT("CARD6"),
              FVector(9000,1300,95),1);
    // Only a Jennifer radio call can provision this real physical cache;
    // the old source ranks/stars system is still pending a full POW census.
    AddPickup(TEXT("L04_ROCKET_LAUNCHER"),TEXT("ROCKET_LAUNCHER"),
              FVector(10400,3200,95),0,TEXT("jennifer.rocket_help"));
    AddPickup(TEXT("L04_ROCKET_AMMO"),TEXT("ROCKET_AMMO"),
              FVector(10900,3100,95),12,TEXT("jennifer.rocket_help"));
    SpawnBoss(TEXT("M04_TX11_A"),FVector(11900,1450,95),
              TEXT("ROCKET_LAUNCHER"),125.f,1050.f,1.3f,20.f);
    SpawnBoss(TEXT("M04_TX11_B"),FVector(12450,3150,95),
              TEXT("ROCKET_LAUNCHER"),125.f,1050.f,1.3f,20.f);
    if (Story() && Story()->HasBeat(TEXT("tx11.rocket_defeat")))
        AddPickup(TEXT("L04_CARD7"),TEXT("CARD7"),
                  FVector(12600,2200,95),1);
    if (AMGGuardActor* Warden=GetWorld()->SpawnActor<AMGGuardActor>(
        FVector(8650,2450,90),FRotator(0.f,70.f,0.f)))
    {
        Warden->ConfigureDuty(TEXT("M04_RESEARCH_WARDEN"),
            TEXT("PRISONER_CUSTODY"),TEXT("BUILDING_TWO_MEDICAL_CELL"),
            FVector(8650,2450,90),2.2f,FVector(9250,3000,90));
        Warden->ConfigurePatrol(TArray<FVector>{
            FVector(8650,2450,90),FVector(9500,2450,90),
            FVector(9450,2750,90)});
    }
    if (AMGGuardActor* Custodian=GetWorld()->SpawnActor<AMGGuardActor>(
        FVector(10300,2550,90),FRotator(0.f,90.f,0.f)))
    {
        Custodian->ConfigureDuty(TEXT("M04_WEAPON_CUSTODIAN"),
            TEXT("SECURE_AMMUNITION"),TEXT("ROCKET_STORE_CARD6_DOOR"),
            FVector(10300,2550,90),1.8f,FVector(10400,3000,90));
        Custodian->ConfigurePatrol(TArray<FVector>{
            FVector(10300,2550,90),FVector(11300,2500,90)});
    }
    if (AMGBodyHidingSpot* Locker=GetWorld()->SpawnActor<AMGBodyHidingSpot>(
        FVector(8300,1200,95),FRotator(0.f,90.f,0.f)))
        Locker->SetFacilityLabel(TEXT("BUILDING TWO / UTILITY"));
    TransitionMarker=Block(TEXT("M04_ResearchInnerGate"),
                           FVector(14000,2200,90),FVector(.8f,.8f,1.7f),false);
    SpawnPoint=FVector(650,2200,90);
}
void AMGLaterMission::BuildEllenReturn()
{
    // Main lab / connected return tunnel / Building One basement. A correct
    // two-building underground map remains source reconstruction work.
    Block(TEXT("M05_ResearchLabFloor"),FVector(2000,2100,-65),
          FVector(40,42,1.5f));
    Block(TEXT("M05_ReturnPassage"),FVector(7900,1600,-65),
          FVector(85,20,1.5f));
    Block(TEXT("M05_BuildingOneBasement"),FVector(13000,1600,-65),
          FVector(35,30,1.5f));
    Block(TEXT("M05_PassageNorthWall"),FVector(7900,2650,150),
          FVector(85,.5f,3.5f));
    Block(TEXT("M05_PassageSouthWall"),FVector(7900,600,150),
          FVector(85,.5f,3.5f));
    // No arbitrary guard stationed inside the daughter's hidden room.
    Block(TEXT("M05_ServiceObstacles"),FVector(6600,1350,90),FVector(2.0f,2.f,2.8f));
    Block(TEXT("M05_HiddenCellPartitionLower"),FVector(11800,825,165),
          FVector(.4f,16.5f,3.7f));
    Block(TEXT("M05_HiddenCellPartitionUpper"),FVector(11800,2475,165),
          FVector(.4f,10.5f,3.7f));
    WeakWall=Block(TEXT("M05_WeakWall_Ellen"),FVector(11800,1800,155),
                   FVector(.5f,3.f,3.3f));
    EllenProxy=Block(TEXT("M05_EllenPrisoner"),FVector(12800,1800,90),
                     FVector(.7f,.7f,1.7f),false);
    MadnarProxy=Block(TEXT("M05_Madnar"),FVector(1850,2900,90),
                      FVector(.7f,.7f,1.7f),false);
    AddPickup(TEXT("L05_PLASTIC_EXPLOSIVE"),TEXT("PLASTIC_EXPLOSIVE"),
              FVector(10600,1100,95),3);
    AddPickup(TEXT("L05_RATION"),TEXT("RATION"),
              FVector(9200,1000,95),1);
    // Same passage is traversed back to Madnar, whose dialogue now reacts
    // to Ellen's rescue. No invented in-person reunion scene is forced.
    SpawnPoint=FVector(650,1800,90);
    if (Story() && Story()->HasBeat(TEXT("m05.ellen_wall_breached")) &&
        IsValid(WeakWall))
        WeakWall->Destroy();
    if (Story() && Story()->HasBeat(TEXT("ellen.rescue")) && IsValid(EllenProxy))
        EllenProxy->Destroy();
}
void AMGLaterMission::RemoveDefeatedGates()
{
    if (!Story()) return;
    if (Mission==3)
    {
        if (Story()->HasBeat(TEXT("m03.machinegun_kid_defeated")) && IsValid(PassageGateA))
            PassageGateA->Destroy();
        if (Story()->HasBeat(TEXT("m03.hind_d_defeated")) && IsValid(PassageGateB))
            PassageGateB->Destroy();
        if (Story()->HasBeat(TEXT("m03.tank_defeated")) && IsValid(PassageGateC))
            PassageGateC->Destroy();
    }
    if (Mission==4)
    {
        if (Story()->HasBeat(TEXT("m04.rocket_room_open")) &&
            IsValid(RocketRoomDoor)) RocketRoomDoor->Destroy();
        if (Story()->HasBeat(TEXT("m04.bulldozer_defeated")) && IsValid(PassageGateA))
            PassageGateA->Destroy();
        if (Story()->HasBeat(TEXT("m04.fire_trooper_defeated")) && IsValid(PassageGateB))
            PassageGateB->Destroy();
        if (Story()->HasBeat(TEXT("tx11.rocket_defeat")) && IsValid(PassageGateC))
            PassageGateC->Destroy();
        if (Story()->HasBeat(TEXT("jennifer.contact")) && IsValid(JenniferPrisoner))
            JenniferPrisoner->Destroy();
        if (Story()->HasBeat(TEXT("fake_madnar.basement_lure")) && IsValid(ResearchPrisoner))
            ResearchPrisoner->Destroy();
    }
}
void AMGLaterMission::OnBossDefeated(FName Id)
{
    UMGCampaignState* State=Story();
    if (!State) return;
    State->RecordBeat(Id,FString::Printf(TEXT("ENCOUNTER CLEARED: %s"),*Id.ToString()));
    if (Id==TEXT("M04_TX11_A") || Id==TEXT("M04_TX11_B"))
        if (State->HasBeat(TEXT("M04_TX11_A")) && State->HasBeat(TEXT("M04_TX11_B")))
        {
            State->RecordBeat(TEXT("tx11.rocket_defeat"),
                TEXT("TX-11 DEFENDERS DISABLED. Card Seven found in their security cabinet."));
            SetSubtitle(TEXT("SNAKE: The machines are down. A new access card is exposed."));
            AddPickup(TEXT("L04_CARD7"),TEXT("CARD7"),FVector(12600,2200,95),1);
        }
    RemoveDefeatedGates();
}
void AMGLaterMission::NotifyWrongWeapon(FName BossId,FName Needed)
{
    if (!GetWorld() || WrongWeaponCooldown>0.f) return;
    if (AMGPlayerCharacter* Player=Cast<AMGPlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(this,0)))
        Player->NotifyFeedback(
            FString::Printf(TEXT("%s RESISTS THAT WEAPON / TRY %s"),
                            *BossId.ToString(),*Needed.ToString()),TEXT("denied"));
    WrongWeaponCooldown=3.0f;
}
void AMGLaterMission::Tick(float Dt)
{
    Super::Tick(Dt);
    DialogueSeconds=FMath::Max(0.f,DialogueSeconds-Dt);
    WrongWeaponCooldown=FMath::Max(0.f,WrongWeaponCooldown-Dt);
    UMGCampaignState* State=Story();
    for (FWorldPickup& Pickup:Pickups)
        if (IsValid(Pickup.Marker) && !Pickup.Prerequisite.IsNone())
            Pickup.Marker->SetActorHiddenInGame(
                !State || !State->HasBeat(Pickup.Prerequisite));
}
bool AMGLaterMission::SavePickupState(FName Id,AMGPlayerCharacter* Player)
{
    UMGMissionOneSaveGame* Save=Cast<UMGMissionOneSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SavedSlot,0));
    if (!Save)
        Save=Cast<UMGMissionOneSaveGame>(UGameplayStatics::CreateSaveGameObject(
            UMGMissionOneSaveGame::StaticClass()));
    if (!Save) return false;
    Save->Version=FMath::Max(7,Save->Version);
    if (!Id.IsNone()) Save->LaterCollectedPickupIds.AddUnique(Id);
    if (Player && Player->GetEquipment())
    {
        for (const FMGEquipmentEntry& Entry:Player->GetEquipment()->GetWeapons())
            if (Entry.Id!=TEXT("NONE"))
                Save->WeaponRounds.Add(Entry.Id,Entry.Quantity);
        for (const FMGEquipmentEntry& Entry:Player->GetEquipment()->GetItems())
            if (Entry.Id!=TEXT("NONE"))
                Save->ItemQuantities.Add(Entry.Id,Entry.Quantity);
        Save->HandgunRounds=Player->GetEquipment()->GetHandgunRounds();
        Save->RationsRemaining=Player->GetEquipment()->GetItemQuantity(TEXT("RATION"));
    }
    return UGameplayStatics::SaveGameToSlot(Save,SavedSlot,0);
}
void AMGLaterMission::PersistPlayerEquipment(AMGPlayerCharacter* Player)
{
    if (!Player || !Player->GetEquipment()) return;
    if (!SavePickupState(NAME_None,Player))
        UE_LOG(LogTemp,Warning,TEXT("Later mission inventory persistence failed"));
}
void AMGLaterMission::RestorePlayerEquipment(AMGPlayerCharacter* Player)
{
    if (!Player || !Player->GetEquipment()) return;
    UMGEquipmentComponent* E=Player->GetEquipment();
    E->InitialiseMissionOneLoadout();
    const UMGMissionOneSaveGame* Save=Cast<UMGMissionOneSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SavedSlot,0));
    if (!Save) return;
    for (const FName Id:Save->CollectedPickupIds)
    {
        if (Id==TEXT("P01")) E->AwardItem(TEXT("RATION"),1);
        else if (Id==TEXT("P02")) E->AwardItem(TEXT("CARD1"),1);
        else if (Id==TEXT("P03")) E->AwardItem(TEXT("BINOCULARS"),1);
        else if (Id==TEXT("P04")) E->AwardItem(TEXT("GASMASK"),1);
        else if (Id==TEXT("P05")) E->AwardWeapon(TEXT("HANDGUN"),0);
        else if (Id==TEXT("P06")) E->AwardItem(TEXT("MINES"),1);
        else if (Id==TEXT("P07")) E->AwardAmmunition(TEXT("HANDGUN"),12);
        else if (Id==TEXT("P08")) E->AwardItem(TEXT("CARD2"),1);
        else if (Id==TEXT("P09")) E->AwardWeapon(TEXT("RC_MISSILE"),4);
        else if (Id==TEXT("P10")) E->AwardWeapon(TEXT("GRENADE_LAUNCHER"),6);
    }
    for (const FName Item:Save->ExtraOwnedItems)
        E->AwardItem(Item,1);
    for (const FName Weapon:Save->ExtraOwnedWeapons)
        E->AwardWeapon(Weapon,0);
    for (const FName Id:Save->LaterCollectedPickupIds)
    {
        if (Id==TEXT("L03_GRENADE_LAUNCHER"))
            E->AwardWeapon(TEXT("GRENADE_LAUNCHER"),6);
        else if (Id==TEXT("L04_ROCKET_LAUNCHER"))
            E->AwardWeapon(TEXT("ROCKET_LAUNCHER"),0);
        else if (Id==TEXT("L04_CARD6")) E->AwardItem(TEXT("CARD6"),1);
        else if (Id==TEXT("L04_CARD7")) E->AwardItem(TEXT("CARD7"),1);
        else if (Id==TEXT("L05_PLASTIC_EXPLOSIVE")) E->AwardItem(TEXT("PLASTIC_EXPLOSIVE"),3);
        else if (Id==TEXT("L03_MINE_RESTOCK")) E->AwardItem(TEXT("MINES"),6);
        else if (Id==TEXT("L03_HANDGUN_AMMO")) E->AwardAmmunition(TEXT("HANDGUN"),18);
        else if (Id==TEXT("L03_GRENADE_AMMO")) E->AwardAmmunition(TEXT("GRENADE_LAUNCHER"),7);
        else if (Id==TEXT("L04_GRENADE_AMMO")) E->AwardAmmunition(TEXT("GRENADE_LAUNCHER"),6);
        else if (Id==TEXT("L04_ROCKET_AMMO")) E->AwardAmmunition(TEXT("ROCKET_LAUNCHER"),12);
        else if (Id==TEXT("L03_RATION") || Id==TEXT("L04_RATION") ||
                 Id==TEXT("L05_RATION")) E->AwardItem(TEXT("RATION"),1);
    }
    if (Save->Version>=6)
    {
        for (const TPair<FName,int32>& Entry:Save->WeaponRounds)
            E->RestoreWeaponAmmo(Entry.Key,Entry.Value);
        for (const TPair<FName,int32>& Entry:Save->ItemQuantities)
            E->RestoreItemQuantity(Entry.Key,Entry.Value);
    }
    else
    {
        E->RestoreHandgunRounds(FMath::Max(0,Save->HandgunRounds));
        E->RestoreItemQuantity(TEXT("RATION"),FMath::Max(0,Save->RationsRemaining));
    }
}
bool AMGLaterMission::ClaimPickup(FWorldPickup& Pickup,AMGPlayerCharacter* Player)
{
    if (!Player || !Player->GetEquipment() || Pickup.bTaken ||
        !IsValid(Pickup.Marker) || !IsWithin(Player,Pickup.Marker)) return false;
    if ((Pickup.Kind==TEXT("ROCKET_LAUNCHER") ||
         Pickup.Kind==TEXT("ROCKET_AMMO")) &&
        (!Story() || !Story()->HasBeat(TEXT("m04.rocket_room_open"))))
    {
        Player->NotifyFeedback(TEXT("ENTER THE CARD 6 WEAPON STORE"),TEXT("denied"));
        return true;
    }
    if (!Pickup.Prerequisite.IsNone() &&
        (!Story() || !Story()->HasBeat(Pickup.Prerequisite)))
    {
        Player->NotifyFeedback(TEXT("SUPPLY NOT YET DELIVERED"),TEXT("denied"));
        return true;
    }
    UMGEquipmentComponent* E=Player->GetEquipment();
    const bool bWeapon=Pickup.Kind==TEXT("GRENADE_LAUNCHER") ||
                       Pickup.Kind==TEXT("ROCKET_LAUNCHER");
    bool bGranted=false;
    if (bWeapon) bGranted=E->AwardWeapon(Pickup.Kind,Pickup.Quantity);
    else if (Pickup.Kind==TEXT("GRENADE_AMMO"))
        bGranted=E->AwardAmmunition(TEXT("GRENADE_LAUNCHER"),Pickup.Quantity);
    else if (Pickup.Kind==TEXT("ROCKET_AMMO"))
        bGranted=E->AwardAmmunition(TEXT("ROCKET_LAUNCHER"),Pickup.Quantity);
    else if (Pickup.Kind==TEXT("HANDGUN_AMMO"))
        bGranted=E->AwardAmmunition(TEXT("HANDGUN"),Pickup.Quantity);
    else bGranted=E->AwardItem(Pickup.Kind,Pickup.Quantity);
    if (!bGranted) return true;
    if (!SavePickupState(Pickup.Id,Player))
        Player->NotifyFeedback(TEXT("INVENTORY SAVE FAILED / RETRY ON CHECKPOINT"),
                               TEXT("denied"));
    Pickup.bTaken=true;
    Pickup.Marker->Destroy();
    Player->NotifyFeedback(FString::Printf(TEXT("%s / +%d"),
        *Pickup.Kind.ToString(),Pickup.Quantity),TEXT("select"));
    return true;
}
bool AMGLaterMission::RescuePOW(AStaticMeshActor*& Marker,FName Beat,
                               const FString& Intel,AMGPlayerCharacter* Player)
{
    if (!IsWithin(Player,Marker)) return false;
    if (Story() && Story()->RecordBeat(Beat,Intel))
    {
        Marker->Destroy();
        Marker=nullptr;
        SetSubtitle(Intel);
        Player->NotifyFeedback(TEXT("PRISONER RESCUED / INTEL ARCHIVED"),TEXT("rescue"));
    }
    return true;
}
void AMGLaterMission::AdvanceAndReload(int32 NextMission)
{
    if (!Story() || !Story()->AdvanceMission(NextMission)) return;
    UGameplayStatics::OpenLevel(this,FName(*GetWorld()->GetName()));
}
bool AMGLaterMission::TryInteract(AMGPlayerCharacter* Player)
{
    if (!Player || !Player->GetEquipment() || !Story()) return false;
    UMGCampaignState* State=Story();
    for (FWorldPickup& P:Pickups)
        if (ClaimPickup(P,Player)) return true;
    if (Mission==3 &&
        FVector::Dist2D(Player->GetActorLocation(),FVector(350,2200,90)) < 165.f)
    {
        if (State->SetMissionThreeExteriorActive(false))
            UGameplayStatics::OpenLevel(this,FName(*GetWorld()->GetName()));
        else Player->NotifyFeedback(TEXT("RETURN CHECKPOINT FAILED"),TEXT("denied"));
        return true;
    }
    if (Mission==3 && IsWithin(Player,TransitionMarker))
    {
        if (!State->HasBeat(TEXT("m03.machinegun_kid_defeated")) ||
            !State->HasBeat(TEXT("m03.hind_d_defeated")) ||
            !State->HasBeat(TEXT("m03.tank_defeated")))
            Player->NotifyFeedback(TEXT("EXTERIOR SECURITY STILL ACTIVE"),TEXT("denied"));
        else AdvanceAndReload(4);
        return true;
    }
    if (Mission==4)
    {
        if (IsWithin(Player,RocketRoomDoor,250.f))
        {
            if (!Player->GetEquipment()->HasItem(TEXT("CARD6")))
            {
                Player->NotifyFeedback(TEXT("RESEARCH ARMOURY / CARD 6 REQUIRED"),
                                       TEXT("denied"));
                return true;
            }
            if (State->RecordBeat(TEXT("m04.rocket_room_open"),
                TEXT("Card 6 opened the physical research weapons store.")))
            {
                RocketRoomDoor->Destroy();
                Player->NotifyFeedback(TEXT("CARD 6 / WEAPON ROOM OPEN"),TEXT("select"));
            }
            return true;
        }
        if (IsWithin(Player,ResearchPrisoner))
            return RescuePOW(ResearchPrisoner,TEXT("fake_madnar.basement_lure"),
                TEXT("PRISONER: This is not Dr. Pettrovich. Search the protected laboratory."),
                Player);
        if (IsWithin(Player,JenniferPrisoner))
            return RescuePOW(JenniferPrisoner,TEXT("jennifer.contact"),
                TEXT("PRISONER: Jennifer is working undercover. Call 120.48 for supplies."),
                Player);
        if (IsWithin(Player,TransitionMarker))
        {
            if (!State->HasBeat(TEXT("tx11.rocket_defeat")))
                Player->NotifyFeedback(TEXT("TX-11 DEFENDERS STILL ACTIVE"),TEXT("denied"));
            else AdvanceAndReload(5);
            return true;
        }
    }
    if (Mission==5)
    {
        if (IsWithin(Player,MadnarProxy))
        {
            if (!State->HasBeat(TEXT("madnar.condition")))
            {
                State->RecordBeat(TEXT("madnar.discovered"),
                    TEXT("DR. PETTROVICH MADNAR: Alive in Building Two research custody."));
                State->RecordBeat(TEXT("madnar.condition"),
                    TEXT("MADNAR: I cannot explain the TX-55 weakness until Ellen is safe."));
                State->RecordBeat(TEXT("ellen.mentioned"),
                    TEXT("ELLEN: Scientist's daughter kept in Building One basement."));
                SetSubtitle(TEXT("MADNAR: Save Ellen first. Then I'll help you stop Metal Gear."));
                Player->NotifyFeedback(TEXT("RETURN TO BUILDING ONE BASEMENT"),TEXT("select"));
            }
            else if (State->HasBeat(TEXT("ellen.rescue")) &&
                     !State->HasBeat(TEXT("madnar.weakness")))
            {
                State->RecordBeat(TEXT("madnar.reunion_confirmation"),
                    TEXT("MADNAR: Snake confirms Ellen is safe, without inventing an unseen reunion."));
                State->RecordBeat(TEXT("madnar.weakness"),
                    TEXT("MADNAR: The TX-55 feet require a precise demolition sequence."));
                State->RecordBeat(TEXT("madnar.tx55_sequence"),
                    TEXT("METAL GEAR DESTRUCTION: A foot-by-foot plastic explosive sequence is required; confirm source order."));
                SetSubtitle(TEXT("MADNAR: The weak points are in its feet. Plant explosives in sequence."));
                Player->NotifyFeedback(TEXT("TX-55 EXPLOSIVE INTELLIGENCE SAVED"),TEXT("rescue"));
            }
            else Player->NotifyFeedback(
                State->HasBeat(TEXT("madnar.weakness")) ?
                TEXT("TECHNICAL INTELLIGENCE ARCHIVED / MISSION 06 NOT YET BUILT") :
                TEXT("MADNAR WILL TALK ONLY AFTER ELLEN IS SAFE"),TEXT("denied"));
            return true;
        }
        if (IsWithin(Player,WeakWall))
        {
            if (!State->HasBeat(TEXT("madnar.condition")))
            {
                Player->NotifyFeedback(TEXT("LEARN WHO IS HELD HERE FIRST"),TEXT("denied"));
                return true;
            }
            if (!Player->GetEquipment()->ConsumeItem(TEXT("PLASTIC_EXPLOSIVE"),1))
            {
                Player->NotifyFeedback(TEXT("PLASTIC EXPLOSIVE REQUIRED"),TEXT("denied"));
                return true;
            }
            PersistPlayerEquipment(Player);
            if (!State->RecordBeat(TEXT("m05.ellen_wall_breached"),
                TEXT("A charge opened an old hidden prison wall in Building One basement.")))
                return true;
            WeakWall->Destroy();
            SetSubtitle(TEXT("SNAKE: There's someone behind this repaired wall."));
            Player->NotifyFeedback(TEXT("HIDDEN CELL OPEN / CHECK FOR PRISONER"),TEXT("breach"));
            return true;
        }
        if (IsWithin(Player,EllenProxy))
        {
            if (!State->HasBeat(TEXT("m05.ellen_wall_breached")))
            {
                Player->NotifyFeedback(TEXT("A CLOSED WALL STILL SEPARATES THE CELL"),TEXT("denied"));
                return true;
            }
            const bool Worked=RescuePOW(EllenProxy,TEXT("ellen.rescue"),
                TEXT("ELLEN: Madnar's daughter is alive. Snake promises to bring word to her father."),
                Player);
            if (Worked)
            {
                State->RecordBeat(TEXT("ellen.knowledge"),
                    TEXT("ELLEN: Confirms she has been held to coerce the scientist."));
                State->RecordBeat(TEXT("ellen.safety"),
                    TEXT("ELLEN: Released from the hidden detention cell."));
            }
            return true;
        }
    }
    return false;
}
FString AMGLaterMission::GetObjectiveText() const
{
    const UMGCampaignState* State=Story();
    if (!State) return TEXT("RESTORE CAMPAIGN STORY STATE");
    if (Mission==3)
    {
        if (!State->HasBeat(TEXT("m03.machinegun_kid_defeated")))
            return TEXT("EXTERIOR / COUNTER MACHINEGUN KID / HANDGUN + COVER");
        if (!State->HasBeat(TEXT("m03.hind_d_defeated")))
            return TEXT("HELIPAD / FIND GRENADES / COUNTER HIND D");
        if (!State->HasBeat(TEXT("m03.tank_defeated")))
            return TEXT("CHECKPOINT / LAY MINES BEFORE THE TANK");
        return TEXT("THREE ENCOUNTERS CLEARED / ENTER BUILDING TWO");
    }
    if (Mission==4)
    {
        if (!State->HasBeat(TEXT("m04.bulldozer_defeated")))
            return TEXT("GARAGE / BULLDOZER / GRENADES AND COVER");
        if (!State->HasBeat(TEXT("m04.fire_trooper_defeated")))
            return TEXT("FIRE WING / REACT TO FIRE TROOPER TELEGRAPHS");
        if (!State->HasBeat(TEXT("jennifer.contact")))
            return TEXT("FIND THE PRISONER WHO KNOWS JENNIFER / 120.48");
        if (!State->HasBeat(TEXT("jennifer.rocket_help")))
            return TEXT("CALL JENNIFER ON 120.48 FOR THE ROCKET CACHE");
        if (!State->HasBeat(TEXT("tx11.rocket_defeat")))
            return TEXT("RECOVER ROCKETS / TWO TX-11 UNITS / CARD 7");
        return TEXT("RECOVER CARD 7 / ENTER THE RESEARCH INNER LAB");
    }
    if (!State->HasBeat(TEXT("madnar.condition")))
        return TEXT("FIND THE REAL DR. PETTROVICH MADNAR");
    if (!State->HasBeat(TEXT("ellen.rescue")))
        return TEXT("RETURN VIA SERVICE PASSAGE / FREE ELLEN WITH EXPLOSIVES");
    if (!State->HasBeat(TEXT("madnar.weakness")))
        return TEXT("RETURN TO MADNAR / REPORT ELLEN SAFE");
    return TEXT("TX-55 WEAKNESS KNOWN / MISSION 06 REQUIRES FURTHER IMPLEMENTATION");
}
