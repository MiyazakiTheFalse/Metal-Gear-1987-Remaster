#include "MGLaterMission.h"
#include "MGCampaignState.h"
#include "MGGuardActor.h"
#include "MGBodyHidingSpot.h"
#include "MGSecurityCamera.h"
#include "MGPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// Later 1995 mission shells share the same spatial scale and item economy.
// Geometry and exact timings are source-comparative prototypes, NOT verified
// room-by-room against the target Connect2Media J2ME v3.3.0 a1 build.
void AMGLaterMission::BuildDesertAndCowardDuck()
{
    // Mission 6: outer desert -> guarded Building 3 approach -> water/air
    // hazard -> Coward Duck's THREE hostage standoff. The service run and
    // water passage are a connected, traversable physical world.
    Block(TEXT("M06_DesertFloor"),FVector(7600,2200,-65),
          FVector(155,44,1.5f));
    Block(TEXT("M06_NorthPerimeter"),FVector(7600,4400,150),
          FVector(155,.5f,3.8f));
    Block(TEXT("M06_SouthPerimeter"),FVector(7600,0,150),
          FVector(155,.5f,3.8f));
    for (const FVector& P : {FVector(1800,850,90),FVector(3850,3450,90),
                             FVector(6700,950,90),FVector(9200,3550,90)})
        Block(TEXT("M06_DesertRockCover"),P,FVector(2.3f,2.f,1.65f));
    // The false-truck clue is deliberately *optional*. A careful player can
    // reject the commander's advice and choose the real defended route.
    DecisionPost=Block(TEXT("M06_FalseAdviceTruck"),FVector(2450,650,100),
                       FVector(3.f,1.6f,1.9f),false);
    PassageGateA=Block(TEXT("M06_Card7OuterGate"),FVector(5450,2200,150),
                       FVector(.5f,44.f,3.5f));
    Block(TEXT("M06_WaterPassageFloor"),FVector(8500,2200,-40),
          FVector(18,42,.5f),false);
    Block(TEXT("M06_WaterPipeNorth"),FVector(8500,4200,160),
          FVector(18,.5f,3.5f));
    Block(TEXT("M06_WaterPipeSouth"),FVector(8500,200,160),
          FVector(18,.5f,3.5f));
    PassageGateB=Block(TEXT("M06_HostageWingCard7"),
                       FVector(11200,2200,150),FVector(.5f,44.f,3.5f));
    AddPickup(TEXT("L06_COMPASS"),TEXT("COMPASS"),
              FVector(1200,3200,90),1);
    AddPickup(TEXT("L06_OXYGEN"),TEXT("OXYGEN_TANK"),
              FVector(7100,3100,90),1);
    AddPickup(TEXT("L06_RATION"),TEXT("RATION"),
              FVector(9800,900,90),1);
    AddPickup(TEXT("L06_HANDGUN_AMMO"),TEXT("HANDGUN_AMMO"),
              FVector(12100,800,90),24);
    if (AMGGuardActor* Gate=GetWorld()->SpawnActor<AMGGuardActor>(
        FVector(4500,3250,90),FRotator(0.f,190.f,0.f)))
    {
        Gate->ConfigureDuty(TEXT("M06_GATE_WATCH"),
            TEXT("DESERT_CHECKPOINT"),TEXT("CARD7_SECURITY_GATE"),
            FVector(4500,3250,90),2.3f,FVector(5350,2200,90));
        Gate->ConfigurePatrol(TArray<FVector>{FVector(4300,3250,90),
            FVector(5050,3250,90),FVector(5050,2600,90)});
    }
    if (AMGGuardActor* Pump=GetWorld()->SpawnActor<AMGGuardActor>(
        FVector(10200,1400,90),FRotator(0.f,50.f,0.f)))
    {
        Pump->ConfigureDuty(TEXT("M06_PUMP_SENTRY"),
            TEXT("WATER_SERVICE"),TEXT("DRAIN_INTAKE"),
            FVector(10200,1400,90),1.8f,FVector(9700,2250,90));
        Pump->ConfigurePatrol(TArray<FVector>{FVector(9800,1200,90),
            FVector(10800,1200,90),FVector(10800,2000,90)});
    }
    // The hostage silhouettes remain separate and positioned to constrain
    // the line of fire. All three must survive for Jennifer's brother clue.
    JenniferBrother=Block(TEXT("M06_JenniferBrother_Hostage"),
        FVector(13200,2300,90),FVector(.65f,.65f,1.7f),false);
    HostageLeft=Block(TEXT("M06_HostageWest"),
        FVector(13050,1450,90),FVector(.65f,.65f,1.7f),false);
    HostageRight=Block(TEXT("M06_HostageEast"),
        FVector(13050,3150,90),FVector(.65f,.65f,1.7f),false);
    SpawnBoss(TEXT("m06.coward_duck_defeated"),FVector(14000,2300,100),
        TEXT("HANDGUN"),175.f,1100.f,1.25f,15.f);
    if (Story()&&Story()->HasBeat(TEXT("m06.coward_duck_defeated")))
        AddPickup(TEXT("L06_CARD8"),TEXT("CARD8"),
                  FVector(14300,900,90),1);
    if (Story())
    {
        if (Story()->HasBeat(TEXT("brother.escape_ladder"))&&IsValid(JenniferBrother))
            JenniferBrother->Destroy();
        if (Story()->HasBeat(TEXT("m06.hostage_left_rescued"))&&IsValid(HostageLeft))
            HostageLeft->Destroy();
        if (Story()->HasBeat(TEXT("m06.hostage_right_rescued"))&&IsValid(HostageRight))
            HostageRight->Destroy();
    }
    TransitionMarker=Block(TEXT("M06_DeepBunkerEntrance"),
                           FVector(15000,2300,90),FVector(.7f,.7f,1.55f),false);
    SpawnPoint=FVector(700,2200,90);
}
void AMGLaterMission::BuildDeepBunker()
{
    // Mission 7: Card 8 security boundary, prison witness to betrayal,
    // abort order and TWO explainable security countermeasures.
    Block(TEXT("M07_BunkerFloor"),FVector(7600,2200,-65),FVector(155,44,1.5f));
    Block(TEXT("M07_NorthBoundary"),FVector(7600,4400,150),
          FVector(155,.5f,3.6f));
    Block(TEXT("M07_SouthBoundary"),FVector(7600,0,150),
          FVector(155,.5f,3.6f));
    PassageGateA=Block(TEXT("M07_Card8SecurityDoor"),FVector(4550,2200,150),
                       FVector(.5f,44.f,3.5f));
    FinalWitness=Block(TEXT("M07_FinalWitnessPrisoner"),
                       FVector(6100,3000,90),FVector(.7f,.7f,1.7f),false);
    DecisionPost=Block(TEXT("M07_ContradictoryAbortRelay"),
                       FVector(8150,2700,90),FVector(.9f,.9f,1.6f),false);
    // Both methods neutralise this security shutter. Card 8 data-terminal
    // approach = no explosives; weak-service-wall approach = expend one charge.
    PassageGateB=Block(TEXT("M07_FinalSecurityShutter"),
                       FVector(12700,2200,160),FVector(.55f,44.f,3.5f));
    PassageGateD=Block(TEXT("M07_SecurityOverrideTerminal"),
                       FVector(10400,3200,105),FVector(.9f,.7f,1.8f),false);
    WeakWall=Block(TEXT("M07_MaintenanceBypassWall"),
                   FVector(10200,900,160),FVector(.5f,3.8f,3.5f));
    for (const FVector& P : {FVector(7000,1000,100),
                             FVector(9100,3500,100),
                             FVector(11100,2100,100)})
        Block(TEXT("M07_BunkerEquipmentCover"),P,FVector(2.1f,2.8f,2.1f));
    if (AMGGuardActor* Sentry=GetWorld()->SpawnActor<AMGGuardActor>(
        FVector(9200,1200,90),FRotator(0.f,90.f,0.f)))
    {
        Sentry->ConfigureDuty(TEXT("M07_OVERRIDE_WATCH"),
            TEXT("SECURITY_RELAY_WATCH"),TEXT("POWER_CABINET"),
            FVector(9200,1200,90),2.5f,FVector(10400,3200,90));
        Sentry->ConfigurePatrol(TArray<FVector>{FVector(9200,1200,90),
            FVector(9200,2350,90),FVector(10900,2350,90)});
    }
    if (AMGBodyHidingSpot* Locker=GetWorld()->SpawnActor<AMGBodyHidingSpot>(
        FVector(7600,1100,95),FRotator(0.f,180.f,0.f)))
        Locker->SetFacilityLabel(TEXT("LOWER BUNKER / 07"));
    // Earlier pickups keep paying off in 7/8: a stockpile of the real
    // source-critical plastic bombs is available before TX-55.
    AddPickup(TEXT("L07_PLASTIC_EXPLOSIVE"),TEXT("PLASTIC_EXPLOSIVE"),
              FVector(11600,3400,95),12);
    AddPickup(TEXT("L07_ROCKET_AMMO"),TEXT("ROCKET_AMMO"),
              FVector(13400,3100,95),12);
    AddPickup(TEXT("L07_RATION"),TEXT("RATION"),
              FVector(5400,900,95),1);
    TransitionMarker=Block(TEXT("M07_TX55Access"),
                           FVector(14700,2200,90),FVector(.7f,.7f,1.5f),false);
    SpawnPoint=FVector(700,2200,90);
    if (Story())
    {
        if (Story()->HasBeat(TEXT("m07.card8_gate_open"))&&IsValid(PassageGateA))
            PassageGateA->Destroy();
        if (Story()->HasBeat(TEXT("pow.final_reveal"))&&IsValid(FinalWitness))
            FinalWitness->Destroy();
        if (Story()->HasBeat(TEXT("m07.final_security_cleared")))
        {
            if (IsValid(PassageGateB)) PassageGateB->Destroy();
            if (IsValid(WeakWall)) WeakWall->Destroy();
        }
    }
}
void AMGLaterMission::BuildTX55Finale()
{
    // Mission 8: TX-55 remains inert; security cameras defend it. The 16
    // foot-charge demolition starts the self-destruct countdown BEFORE
    // the commander's final fight. Two visible ladders honour the brother's
    // earlier source clue about the LEFT one.
    Block(TEXT("M08_TX55Floor"),FVector(7600,2200,-65),FVector(155,44,1.5f));
    Block(TEXT("M08_NorthBulkhead"),FVector(7600,4400,160),
          FVector(155,.5f,3.7f));
    Block(TEXT("M08_SouthBulkhead"),FVector(7600,0,160),
          FVector(155,.5f,3.7f));
    Block(TEXT("M08_CentralMetalGearTorso"),FVector(4400,2200,370),
          FVector(5.8f,4.8f,6.8f),false);
    TX55LeftFoot=Block(TEXT("M08_TX55_LeftFoot"),
        FVector(3600,3250,110),FVector(2.0f,2.f,1.9f),false);
    TX55RightFoot=Block(TEXT("M08_TX55_RightFoot"),
        FVector(3600,1150,110),FVector(2.f,2.f,1.9f),false);
    for (const FVector& P : {FVector(2600,450,100),
                             FVector(2600,3900,100)})
        Block(TEXT("M08_CameraCover"),P,FVector(2.8f,1.3f,2.f));
    if (!Story() || !Story()->HasBeat(TEXT("tx55.destruction")))
    {
        for (int Index=0;Index<2;++Index)
        {
            const FVector Position(3300,Index==0?600.f:3800.f,300.f);
            if (AMGSecurityCamera* Camera=GetWorld()->SpawnActor<AMGSecurityCamera>(
                    Position,FRotator(0.f,Index==0?110.f:250.f,0.f)))
                Camera->Configure(Index==0 ? TEXT("M08_LASER_CAMERA_N") :
                                              TEXT("M08_LASER_CAMERA_S"),
                                   1,55.f);
        }
    }
    AddPickup(TEXT("L08_DEMOLITION_STOCK"),TEXT("PLASTIC_EXPLOSIVE"),
              FVector(1100,2900,95),18);
    AddPickup(TEXT("L08_ROCKET_REPLENISH"),TEXT("ROCKET_AMMO"),
              FVector(8700,3650,95),15);
    AddPickup(TEXT("L08_RATION"),TEXT("RATION"),
              FVector(8200,900,95),1);
    PassageGateA=Block(TEXT("M08_ReactorSeparationDoor"),
                       FVector(7900,2200,165),FVector(.5f,44.f,3.5f));
    PassageGateB=Block(TEXT("M08_EscapeBulkheadDoor"),
                       FVector(12600,2200,165),FVector(.5f,44.f,3.5f));
    LeftEscapeLadder=Block(TEXT("M08_LeftEscapeLadder"),
                        FVector(14500,1250,115),FVector(.8f,.8f,2.f),false);
    RightEscapeLadder=Block(TEXT("M08_RightEscapeLadder"),
                         FVector(14500,3250,115),FVector(.8f,.8f,2.f),false);
    SpawnPoint=FVector(700,2200,90);
    if (Story())
    {
        if (Story()->HasBeat(TEXT("tx55.destruction")))
        {
            if (IsValid(TX55LeftFoot)) TX55LeftFoot->Destroy();
            if (IsValid(TX55RightFoot)) TX55RightFoot->Destroy();
            if (IsValid(PassageGateA)) PassageGateA->Destroy();
            // Countdown retries resume from a valid post-detonation checkpoint.
            if (Story()->GetEscapeTimeRemaining()<=0.f)
                Story()->SetEscapeTimeRemaining(120.f);
            if (!Story()->HasBeat(TEXT("m08.big_boss_defeated")))
                SpawnBoss(TEXT("m08.big_boss_defeated"),
                          FVector(10700,2200,100),TEXT("ROCKET_LAUNCHER"),
                          260.f,1200.f,1.3f,23.f);
        }
        if (Story()->HasBeat(TEXT("m08.big_boss_defeated"))&&
            IsValid(PassageGateB))
            PassageGateB->Destroy();
    }
}
