#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MGEquipmentComponent.generated.h"

USTRUCT(BlueprintType)
struct FMGEquipmentEntry
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName Id = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString Label;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Quantity = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bOwned = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bConsumable = false;
};

/** Invariant: capture disarms Snake, but does not erase his recoverable inventory. */
UCLASS(ClassGroup=(MetalGear), meta=(BlueprintSpawnableComponent))
class MGREMASTER_API UMGEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMGEquipmentComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaSeconds, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    const TArray<FMGEquipmentEntry>& GetWeapons() const { return Weapons; }
    const TArray<FMGEquipmentEntry>& GetItems() const { return Items; }
    int32 GetWeaponSelection() const { return WeaponSelection; }
    int32 GetItemSelection() const { return ItemSelection; }
    int32 GetEquippedWeaponIndex() const { return EquippedWeapon; }
    int32 GetEquippedItemIndex() const { return EquippedItem; }
    bool IsWeaponWindowOpen() const { return bWeaponWindowOpen; }
    bool IsItemWindowOpen() const { return bItemWindowOpen; }
    bool IsConfiscated() const { return bConfiscated; }
    // Actor BeginPlay order is unspecified. The mission must not restore
    // saved ammo before the equipment component has built its slot arrays.
    bool HasInitializedInventory() const { return bInventorySlotsInitialized; }
    FString GetEquipmentFeedback() const { return FeedbackSeconds > 0.f ? Feedback : FString(); }

    void SetConfiscated(bool bSeized);
    void OpenWeaponWindow();
    void CloseWeaponWindow();
    void OpenItemWindow();
    void CloseItemWindow();
    void SelectionPrevious();
    void SelectionNext();
    bool IsStunEquipped() const;
    bool IsHandgunEquipped() const;
    bool IsFirearmEquipped() const { return IsStunEquipped() || IsHandgunEquipped(); }
    bool ConsumeHandgunRound();
    int32 GetHandgunRounds() const;
    void RestoreHandgunRounds(int32 Rounds);
    void RestoreStunRounds(int32 Rounds);
    bool ConsumeStunRound();
    bool ConsumeRation();
    bool IsItemEquipped(FName ItemId) const;
    bool IsTransmitterEquipped() const;
    bool HasPlantedTransmitter() const;
    bool DiscardPlantedTransmitter();
    int32 GetStunRounds() const;

    // Off by default and not registered at all in shipping builds.
    // This is a *test harness*, never 1987 campaign equipment.
    UPROPERTY(EditDefaultsOnly, Category="MG|Development")
    bool bEnableNonCanonicalTestWeapons = false;

    // Designed for eventual room/item pickup and mission SaveGame integration.
    bool AwardItem(FName ItemId, int32 Count);
    bool RevokeTemporaryItem(FName ItemId); // Only the borrowed dispatch carton.
    bool AwardWeapon(FName WeaponId, int32 Ammo);
    void SetFeedback(const FString& Message);

private:
    UPROPERTY()
    TArray<FMGEquipmentEntry> Weapons;
    UPROPERTY()
    TArray<FMGEquipmentEntry> Items;

    int32 WeaponSelection = 0;
    int32 ItemSelection = 0;
    int32 EquippedWeapon = 0;
    int32 EquippedItem = 0;
    bool bWeaponWindowOpen = false;
    bool bItemWindowOpen = false;
    bool bConfiscated = true;
    bool bInventorySlotsInitialized = false;
    FString Feedback;
    float FeedbackSeconds = 0.f;

    void MoveSelection(int32 Delta);
    bool IsAvailable(const FMGEquipmentEntry& Entry) const;
    bool TrySelectEquipped(bool bWeapon);
};
