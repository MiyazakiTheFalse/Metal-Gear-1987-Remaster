#include "MGLaterMission.h"
#include "MGCampaignState.h"
#include "MGGuardActor.h"
#include "MGBodyHidingSpot.h"
#include "MGSecurityCamera.h"
#include "MGPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "MGEquipmentComponent.h"
#include "MGSetpieceBoss.h"
#include "EngineUtils.h"

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
    TX55Torso=Block(TEXT("M08_CentralMetalGearTorso"),
                    FVector(4400,2200,370),FVector(5.8f,4.8f,6.8f),false);
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
            if (IsValid(TX55Torso)) TX55Torso->Destroy();
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


void AMGLaterMission::HandleMissionHazards(float DeltaSeconds)
{
    UMGCampaignState* State=Story();
    AMGPlayerCharacter* Snake=Cast<AMGPlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(this,0));
    if (!State || !Snake || Snake->IsGameOver()) return;
    if (Mission==6)
    {
        // The passage has a physical footprint. An oxygen tank found earlier
        // changes it from hazardous to traversable; it does not warp Snake.
        const FVector P=Snake->GetActorLocation();
        const bool bInWater=P.X>=7750.f && P.X<=9250.f &&
            P.Y>250.f && P.Y<4150.f;
        if (bInWater && !Snake->GetEquipment()->HasItem(TEXT("OXYGEN_TANK")))
        {
            WaterDamageAccumulator+=DeltaSeconds;
            if (WaterDamageAccumulator>=1.0f)
            {
                WaterDamageAccumulator=FMath::Fmod(WaterDamageAccumulator,1.f);
                Snake->NotifyFeedback(TEXT("UNDERWATER / FIND OXYGEN TANK"),
                                      TEXT("denied"));
                UGameplayStatics::ApplyDamage(Snake,17.f,nullptr,this,
                                              UDamageType::StaticClass());
            }
            if (!State->HasBeat(TEXT("m06.water_hazard_discovered")))
                State->RecordBeat(TEXT("m06.water_hazard_discovered"),
                    TEXT("Water passage requires breathing equipment."));
        }
        else WaterDamageAccumulator=0.f;
    }
    if (Mission!=8 || !State->HasBeat(TEXT("tx55.destruction")) ||
        State->HasBeat(TEXT("snake.escaped_outer_heaven"))) return;
    // Countdown starts at TX-55 demolition, persists over reloads, and only
    // advances when the player is in this real mission world and alive.
    EscapeSaveAccumulator+=FMath::Max(0.f,DeltaSeconds);
    if (EscapeSaveAccumulator>=.5f)
    {
        const float Remaining=FMath::Max(0.f,
            State->GetEscapeTimeRemaining()-EscapeSaveAccumulator);
        EscapeSaveAccumulator=0.f;
        if (!State->SetEscapeTimeRemaining(Remaining)) return;
        if (Remaining<=0.f)
        {
            Snake->NotifyFeedback(TEXT("OUTER HEAVEN SELF-DESTRUCT / TOO LATE"),
                                  TEXT("game_over_sting"));
            UGameplayStatics::ApplyDamage(Snake,10000.f,nullptr,this,
                                          UDamageType::StaticClass());
        }
        else if (FMath::FloorToInt(Remaining)==30 ||
                FMath::FloorToInt(Remaining)==10)
        {
            Snake->NotifyFeedback(FString::Printf(
                TEXT("SELF-DESTRUCT / %d SECONDS LEFT"),
                FMath::CeilToInt(Remaining)),TEXT("alarm"));
        }
    }
}
bool AMGLaterMission::TryFinaleInteraction(AMGPlayerCharacter* Player)
{
    if (!Player || !Player->GetEquipment() || !Story()) return false;
    UMGCampaignState* State=Story();
    UMGEquipmentComponent* E=Player->GetEquipment();
    if (Mission==5 && IsWithin(Player,TransitionMarker))
    {
        if (!State->HasBeat(TEXT("madnar.weakness")))
            Player->NotifyFeedback(
                TEXT("RETURN TO THE DOCTOR / ELLEN MUST BE SAFE FIRST"),
                TEXT("denied"));
        else
        {
            PersistPlayerEquipment(Player);
            AdvanceAndReload(6);
        }
        return true;
    }
    if (Mission==6)
    {
        if (IsWithin(Player,DecisionPost,265.f))
        {
            State->RecordBeat(TEXT("boss.false_truck"),
                TEXT("The commander's truck directions were inconsistent with the real route."));
            SetSubtitle(TEXT("BIG BOSS: Check that truck.\n"
                "SNAKE: A dead end? The security road goes another way."));
            Player->NotifyFeedback(TEXT("COMMAND ROUTE CONTRADICTION"),
                                   TEXT("radio"));
            return true;
        }
        if (IsWithin(Player,PassageGateA,250.f))
        {
            if (!E->HasItem(TEXT("CARD7")))
                Player->NotifyFeedback(TEXT("DESERT SECURITY GATE / CARD 7"),
                                       TEXT("denied"));
            else if (State->RecordBeat(TEXT("m06.card7_gate_open"),
                TEXT("Card 7 unlocked the Building Three desert checkpoint.")))
            {
                PassageGateA->Destroy();
                Player->NotifyFeedback(TEXT("CARD 7 / GATE UNLOCKED"),
                                       TEXT("unlock"));
            }
            return true;
        }
        if (IsWithin(Player,PassageGateB,250.f))
        {
            if (!E->HasItem(TEXT("CARD7")))
                Player->NotifyFeedback(TEXT("HOSTAGE HOLDING WING / CARD 7"),
                                       TEXT("denied"));
            else if (State->RecordBeat(TEXT("m06.hostage_wing_open"),
                TEXT("Building Three's hostage wing security unlocked.")))
            {
                PassageGateB->Destroy();
                Player->NotifyFeedback(TEXT("CARD 7 / HOSTAGE WING OPEN"),
                                       TEXT("unlock"));
            }
            return true;
        }
        if (IsWithin(Player,JenniferBrother))
        {
            if (!State->HasBeat(TEXT("m06.coward_duck_defeated")))
            {
                Player->NotifyFeedback(
                    TEXT("COWARD DUCK IS HOLDING THE PRISONERS AT GUNPOINT"),
                    TEXT("denied"));
                return true;
            }
            return RescuePOW(JenniferBrother,TEXT("brother.escape_ladder"),
                TEXT("JENNIFER'S BROTHER: During the escape, take the LEFT ladder."),
                Player);
        }
        if (IsWithin(Player,HostageLeft))
        {
            if (!State->HasBeat(TEXT("m06.coward_duck_defeated")))
            {
                Player->NotifyFeedback(TEXT("HOSTAGE STILL THREATENED"),
                                       TEXT("denied"));
                return true;
            }
            return RescuePOW(HostageLeft,TEXT("m06.hostage_left_rescued"),
                TEXT("PRISONER: The commander keeps secrets deeper than this wing."),
                Player);
        }
        if (IsWithin(Player,HostageRight))
        {
            if (!State->HasBeat(TEXT("m06.coward_duck_defeated")))
            {
                Player->NotifyFeedback(TEXT("HOSTAGE STILL THREATENED"),
                                       TEXT("denied"));
                return true;
            }
            return RescuePOW(HostageRight,TEXT("m06.hostage_right_rescued"),
                TEXT("PRISONER: A final detention sector guards the research shaft."),
                Player);
        }
        if (IsWithin(Player,TransitionMarker,300.f))
        {
            if (!State->HasBeat(TEXT("brother.escape_ladder")) ||
                !State->HasBeat(TEXT("m06.hostage_left_rescued")) ||
                !State->HasBeat(TEXT("m06.hostage_right_rescued")) ||
                !State->HasBeat(TEXT("m06.card8_acquired")))
                Player->NotifyFeedback(
                    TEXT("RESCUE ALL THREE / TAKE CARD 8 / THEN ENTER"),
                    TEXT("denied"));
            else
            {
                PersistPlayerEquipment(Player);
                AdvanceAndReload(7);
            }
            return true;
        }
    }
    if (Mission==7)
    {
        if (IsWithin(Player,PassageGateA,250.f))
        {
            if (!E->HasItem(TEXT("CARD8")))
                Player->NotifyFeedback(TEXT("INNER SECURITY / CARD 8 REQUIRED"),
                                       TEXT("denied"));
            else if (State->RecordBeat(TEXT("m07.card8_gate_open"),
                TEXT("Card 8 opened the final interrogation wing.")))
            {
                PassageGateA->Destroy();
                Player->NotifyFeedback(TEXT("CARD 8 / ACCESS GRANTED"),
                                       TEXT("unlock"));
            }
            return true;
        }
        if (IsWithin(Player,FinalWitness))
            return RescuePOW(FinalWitness,TEXT("pow.final_reveal"),
                TEXT("FINAL PRISONER: The man behind Outer Heaven is Big Boss."),
                Player);
        if (IsWithin(Player,DecisionPost,260.f))
        {
            if (!State->HasBeat(TEXT("pow.final_reveal")))
            {
                Player->NotifyFeedback(TEXT("INVESTIGATE THE PRISONER'S CLAIM"),
                                       TEXT("denied"));
                return true;
            }
            if (!State->HasBeat(TEXT("boss.abort_order")))
            {
                State->RecordBeat(TEXT("boss.abort_order"),
                    TEXT("BIG BOSS: Abandon the mission at once."));
                SetSubtitle(TEXT("BIG BOSS: Abort the operation. Immediately."));
                Player->NotifyFeedback(TEXT("ABORT ORDER / CONTRADICTS EVIDENCE"),
                                       TEXT("radio"));
            }
            else if (!State->HasBeat(TEXT("snake.disobeys_abort")))
            {
                State->RecordBeat(TEXT("snake.disobeys_abort"),
                    TEXT("SNAKE: No. I've seen what this place is doing."));
                SetSubtitle(TEXT("SNAKE: I can't follow that order. Not now."));
                Player->NotifyFeedback(TEXT("SNAKE CHOOSES TO CONTINUE"),
                                       TEXT("radio"));
            }
            return true;
        }
        if (IsWithin(Player,PassageGateD,270.f))
        {
            if (!State->HasBeat(TEXT("snake.disobeys_abort")))
            {
                Player->NotifyFeedback(TEXT("CHECK YOUR RADIO EVIDENCE FIRST"),
                                       TEXT("denied"));
                return true;
            }
            if (State->RecordBeat(TEXT("m07.final_security_cleared"),
                TEXT("CARD 8 OVERRIDE: Snake disabled the final bunker shutter.")))
            {
                if (IsValid(PassageGateB)) PassageGateB->Destroy();
                SetSubtitle(TEXT("SCHNEIDER'S PLANS: The relay has a service override."));
                Player->NotifyFeedback(TEXT("SECURITY RELAY DISABLED"),
                                       TEXT("select"));
            }
            return true;
        }
        if (IsWithin(Player,WeakWall,250.f))
        {
            if (!State->HasBeat(TEXT("snake.disobeys_abort")))
            {
                Player->NotifyFeedback(TEXT("NO REASON TO BREAK THE SERVICE WALL YET"),
                                       TEXT("denied"));
                return true;
            }
            if (E->GetItemQuantity(TEXT("PLASTIC_EXPLOSIVE"))<1)
            {
                Player->NotifyFeedback(TEXT("MAINTENANCE BYPASS / EXPLOSIVE REQUIRED"),
                                       TEXT("denied"));
                return true;
            }
            if (State->RecordBeat(TEXT("m07.final_security_cleared"),
                TEXT("SERVICE BYPASS: Snake spent a charge rather than use the watched relay.")))
            {
                E->ConsumeItem(TEXT("PLASTIC_EXPLOSIVE"),1);
                PersistPlayerEquipment(Player);
                WeakWall->Destroy();
                if (IsValid(PassageGateB)) PassageGateB->Destroy();
                Player->NotifyFeedback(TEXT("MAINTENANCE ROUTE OPENED"),
                                       TEXT("breach"));
            }
            return true;
        }
        if (IsWithin(Player,TransitionMarker,290.f))
        {
            if (!State->HasBeat(TEXT("m07.final_security_cleared")) ||
                !State->HasBeat(TEXT("pow.final_reveal")) ||
                !State->HasBeat(TEXT("snake.disobeys_abort")))
                Player->NotifyFeedback(TEXT("INNER SECURITY STILL ACTIVE"),
                                       TEXT("denied"));
            else
            {
                PersistPlayerEquipment(Player);
                AdvanceAndReload(8);
            }
            return true;
        }
    }
    if (Mission!=8) return false;
    if (!State->HasBeat(TEXT("tx55.destruction")))
    {
        for (int32 Foot=0;Foot<2;++Foot)
        {
            AStaticMeshActor* Target=Foot==0?TX55RightFoot:TX55LeftFoot;
            if (!IsWithin(Player,Target,230.f)) continue;
            if (ChargePlacementRecovery>0.f)
            {
                Player->NotifyFeedback(TEXT("CHARGE IS STILL ARMING"),
                                       TEXT("denied"));
                return true;
            }
            if (!State->HasBeat(TEXT("madnar.tx55_sequence")))
            {
                Player->NotifyFeedback(TEXT("MADNAR'S FOOT-SEQUENCE INTEL REQUIRED"),
                                       TEXT("denied"));
                return true;
            }
            if (E->GetItemQuantity(TEXT("PLASTIC_EXPLOSIVE"))<1)
            {
                Player->NotifyFeedback(TEXT("NO PLASTIC EXPLOSIVES / CHECK THE ENTRY CACHE"),
                                       TEXT("denied"));
                return true;
            }
            const bool bRight=Foot==0;
            if (!State->RecordTX55Charge(bRight))
            {
                Player->NotifyFeedback(TEXT("WRONG FOOT / RECALL MADNAR'S SEQUENCE"),
                                       TEXT("denied"));
                return true;
            }
            E->ConsumeItem(TEXT("PLASTIC_EXPLOSIVE"),1);
            PersistPlayerEquipment(Player);
            ChargePlacementRecovery=.75f;
            Player->NotifyFeedback(FString::Printf(
                TEXT("TX-55 CHARGE %d/16 / %s FOOT"),
                State->GetTX55CorrectCharges(),bRight?TEXT("RIGHT"):TEXT("LEFT")),
                TEXT("mine"));
            if (State->GetTX55CorrectCharges()>=16)
            {
                if (State->RecordBeat(TEXT("tx55.destruction"),
                    TEXT("The sixteenth foot charge destroyed TX-55. Outer Heaven self-destruct has started.")))
                {
                    if (IsValid(TX55LeftFoot)) TX55LeftFoot->Destroy();
                    if (IsValid(TX55RightFoot)) TX55RightFoot->Destroy();
                    if (IsValid(TX55Torso)) TX55Torso->Destroy();
                    if (IsValid(PassageGateA)) PassageGateA->Destroy();
                    for (TActorIterator<AMGSecurityCamera> Camera(GetWorld());Camera;++Camera)
                        Camera->DisableForService();
                    SpawnBoss(TEXT("m08.big_boss_defeated"),
                        FVector(10700,2200,100),TEXT("ROCKET_LAUNCHER"),
                        260.f,1200.f,1.3f,23.f);
                    SetSubtitle(TEXT("BIG BOSS: Snake... this isn't over.\n"
                        "SNAKE: You've been giving me orders all along."));
                    Player->NotifyFeedback(TEXT("TX-55 DESTROYED / SELF-DESTRUCT ACTIVE"),
                                           TEXT("alarm"));
                }
            }
            return true;
        }
    }
    if (IsWithin(Player,RightEscapeLadder,270.f))
    {
        Player->NotifyFeedback(TEXT("RIGHT LADDER / DEAD END — REMEMBER THE HOSTAGE"),
                               TEXT("denied"));
        return true;
    }
    if (IsWithin(Player,LeftEscapeLadder,270.f))
    {
        if (!State->HasBeat(TEXT("tx55.destruction")) ||
            !State->HasBeat(TEXT("m08.big_boss_defeated")))
        {
            Player->NotifyFeedback(TEXT("THE CONFRONTATION IS NOT OVER"),
                                   TEXT("denied"));
            return true;
        }
        if (!State->HasBeat(TEXT("brother.escape_ladder")))
        {
            Player->NotifyFeedback(TEXT("LADDER ROUTE NOT VERIFIED"),
                                   TEXT("denied"));
            return true;
        }
        if (State->GetEscapeTimeRemaining()<=0.f)
        {
            Player->NotifyFeedback(TEXT("COUNTDOWN EXPIRED"),TEXT("denied"));
            return true;
        }
        if (State->RecordBeat(TEXT("snake.escaped_outer_heaven"),
            TEXT("Snake used the left escape ladder Jennifer's brother described and survived.")))
        {
            SetSubtitle(TEXT("SNAKE: Outer Heaven's finished.\n"
                "RADIO: ...Snake? Can you hear me?"));
            Player->NotifyFeedback(TEXT("OPERATION INTRUDE N313 / COMPLETE"),
                                   TEXT("win"));
        }
        return true;
    }
    return false;
}
