#include "MGEquipmentComponent.h"

UMGEquipmentComponent::UMGEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UMGEquipmentComponent::BeginPlay()
{
    Super::BeginPlay();
    Weapons = {
        {FName(TEXT("NONE")), TEXT("NO WEAPON"), 0, true, false},
        {FName(TEXT("STUN")), TEXT("STUN PISTOL [TEST]"), 8, true, false},
        {FName(TEXT("HANDGUN")), TEXT("HANDGUN [SLICE PROXY]"), 24, true, false}
    };
    Items = {
        {FName(TEXT("NONE")), TEXT("NO ITEM"), 0, true, false},
        {FName(TEXT("RATION")), TEXT("RATION [TEST]"), 1, true, true},
        {FName(TEXT("CARD1")), TEXT("KEYCARD 1 [TEST]"), 0, false, false},
        // Carryover from the earlier infiltration, not a claimed playable acquisition.
        {FName(TEXT("CARD2")), TEXT("KEYCARD 2 [CARRYOVER]"), 0, true, false},
        {FName(TEXT("CARD3")), TEXT("KEYCARD 3 [SLICE]"), 0, false, false},
        // This is placed in Snake's actual inventory when his gear is returned.
        {FName(TEXT("TRANSMITTER")), TEXT("UNKNOWN TRANSMITTER"), 0, false, false}
    };
    // A captured Snake cannot use pre-capture weapons or items. Test entries are
    // deliberately marked provisional rather than claims about J2ME inventory.
    SetConfiscated(true);
}

void UMGEquipmentComponent::TickComponent(
    float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);
    FeedbackSeconds = FMath::Max(0.f, FeedbackSeconds - FMath::Max(0.f, DeltaSeconds));
}

void UMGEquipmentComponent::SetFeedback(const FString& Message)
{
    Feedback = Message;
    FeedbackSeconds = 2.6f;
}

void UMGEquipmentComponent::SetConfiscated(bool bSeized)
{
    bConfiscated = bSeized;
    if (bConfiscated)
    {
        EquippedWeapon = 0;
        EquippedItem = 0;
        WeaponSelection = 0;
        ItemSelection = 0;
        bWeaponWindowOpen = false;
        bItemWindowOpen = false;
    }
}
bool UMGEquipmentComponent::IsAvailable(const FMGEquipmentEntry& Entry) const
{
    return Entry.bOwned && (Entry.Id == FName(TEXT("NONE")) || !bConfiscated)
           && (!Entry.bConsumable || Entry.Quantity > 0);
}

void UMGEquipmentComponent::OpenWeaponWindow()
{
    bItemWindowOpen = false;
    bWeaponWindowOpen = true;
    WeaponSelection = EquippedWeapon;
}
void UMGEquipmentComponent::CloseWeaponWindow()
{
    if (bWeaponWindowOpen) TrySelectEquipped(true);
    bWeaponWindowOpen = false;
}
void UMGEquipmentComponent::OpenItemWindow()
{
    bWeaponWindowOpen = false;
    bItemWindowOpen = true;
    ItemSelection = EquippedItem;
}
void UMGEquipmentComponent::CloseItemWindow()
{
    if (bItemWindowOpen) TrySelectEquipped(false);
    bItemWindowOpen = false;
}
void UMGEquipmentComponent::SelectionPrevious() { MoveSelection(-1); }
void UMGEquipmentComponent::SelectionNext() { MoveSelection(+1); }
void UMGEquipmentComponent::MoveSelection(int32 Delta)
{
    TArray<FMGEquipmentEntry>* List = nullptr;
    int32* Index = nullptr;
    if (bWeaponWindowOpen) { List=&Weapons; Index=&WeaponSelection; }
    else if (bItemWindowOpen) { List=&Items; Index=&ItemSelection; }
    if (!List || List->IsEmpty()) return;
    *Index = (*Index + Delta + List->Num()) % List->Num();
}
bool UMGEquipmentComponent::TrySelectEquipped(bool bWeapon)
{
    const TArray<FMGEquipmentEntry>& List = bWeapon ? Weapons : Items;
    const int32 At = bWeapon ? WeaponSelection : ItemSelection;
    if (!List.IsValidIndex(At)) return false;
    if (!IsAvailable(List[At]))
    {
        SetFeedback(bConfiscated ? TEXT("EQUIPMENT CONFISCATED") : TEXT("NOT IN INVENTORY"));
        return false;
    }
    if (bWeapon) EquippedWeapon = At;
    else EquippedItem = At;
    return true;
}

bool UMGEquipmentComponent::IsStunEquipped() const
{
    return !bConfiscated && Weapons.IsValidIndex(EquippedWeapon)
        && Weapons[EquippedWeapon].Id == FName(TEXT("STUN"))
        && IsAvailable(Weapons[EquippedWeapon]);
}
bool UMGEquipmentComponent::IsHandgunEquipped() const
{
    return !bConfiscated && Weapons.IsValidIndex(EquippedWeapon)
        && Weapons[EquippedWeapon].Id == FName(TEXT("HANDGUN"))
        && IsAvailable(Weapons[EquippedWeapon]);
}
int32 UMGEquipmentComponent::GetHandgunRounds() const
{
    for (const auto& Entry : Weapons)
        if (Entry.Id == FName(TEXT("HANDGUN"))) return Entry.Quantity;
    return 0;
}
bool UMGEquipmentComponent::ConsumeHandgunRound()
{
    if (!IsHandgunEquipped()) return false;
    FMGEquipmentEntry& Slot = Weapons[EquippedWeapon];
    if (Slot.Quantity <= 0) return false;
    --Slot.Quantity;
    return true;
}
void UMGEquipmentComponent::RestoreHandgunRounds(int32 Rounds)
{
    for (FMGEquipmentEntry& E : Weapons)
        if (E.Id == FName(TEXT("HANDGUN")))
            E.Quantity = FMath::Clamp(Rounds, 0, 999);
}
void UMGEquipmentComponent::RestoreStunRounds(int32 Rounds)
{
    for (FMGEquipmentEntry& E : Weapons)
        if (E.Id == FName(TEXT("STUN")))
            E.Quantity = FMath::Clamp(Rounds, 0, 999);
}
int32 UMGEquipmentComponent::GetStunRounds() const
{
    for (const auto& Entry : Weapons)
        if (Entry.Id == FName(TEXT("STUN"))) return Entry.Quantity;
    return 0;
}
bool UMGEquipmentComponent::ConsumeStunRound()
{
    if (!IsStunEquipped()) return false;
    FMGEquipmentEntry& Slot = Weapons[EquippedWeapon];
    if (Slot.Quantity <= 0)
    {
        SetFeedback(TEXT("NO AMMO"));
        return false;
    }
    --Slot.Quantity;
    return true;
}
bool UMGEquipmentComponent::HasPlantedTransmitter() const
{
    for (const FMGEquipmentEntry& E : Items)
        if (E.Id == TEXT("TRANSMITTER"))
            return E.bOwned;
    return false;
}
bool UMGEquipmentComponent::IsTransmitterEquipped() const
{
    return !bConfiscated && Items.IsValidIndex(EquippedItem)
        && Items[EquippedItem].Id == TEXT("TRANSMITTER") && Items[EquippedItem].bOwned;
}
bool UMGEquipmentComponent::DiscardPlantedTransmitter()
{
    if (bConfiscated) return false;
    for (FMGEquipmentEntry& Entry : Items)
        if (Entry.Id == TEXT("TRANSMITTER") && Entry.bOwned)
        {
            Entry.bOwned = false;
            Entry.Quantity = 0;
            if (Items.IsValidIndex(EquippedItem) && Items[EquippedItem].Id == TEXT("TRANSMITTER"))
                EquippedItem = 0;
            if (Items.IsValidIndex(ItemSelection) && Items[ItemSelection].Id == TEXT("TRANSMITTER"))
                ItemSelection = 0;
            return true;
        }
    return false;
}

bool UMGEquipmentComponent::ConsumeRation()
{
    if (bConfiscated || !Items.IsValidIndex(EquippedItem)
        || Items[EquippedItem].Id != FName(TEXT("RATION")))
        return false;
    FMGEquipmentEntry& Ration = Items[EquippedItem];
    if (!IsAvailable(Ration)) return false;
    --Ration.Quantity;
    if (Ration.Quantity == 0) EquippedItem = 0;
    return true;
}
bool UMGEquipmentComponent::AwardItem(FName Id, int32 Count)
{
    if (Count <= 0) return false;
    for (FMGEquipmentEntry& E : Items)
        if (E.Id == Id && Id != FName(TEXT("NONE")))
        {
            E.bOwned = true;
            E.Quantity = FMath::Clamp(E.Quantity + Count, 0, 999);
            return true;
        }
    return false;
}
bool UMGEquipmentComponent::AwardWeapon(FName Id, int32 Ammo)
{
    if (Ammo < 0) return false;
    for (FMGEquipmentEntry& E : Weapons)
        if (E.Id == Id && Id != FName(TEXT("NONE")))
        {
            E.bOwned = true;
            E.Quantity = FMath::Clamp(E.Quantity + Ammo, 0, 999);
            return true;
        }
    return false;
}
