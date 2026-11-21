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
        {FName(TEXT("HANDGUN")), TEXT("HANDGUN"), 0, false, false},
        {FName(TEXT("RC_MISSILE")), TEXT("REMOTE-CONTROLLED MISSILE"), 0, false, false},
        {FName(TEXT("ROCKET_LAUNCHER")), TEXT("ROCKET LAUNCHER"), 0, false, false},
        {FName(TEXT("GRENADE_LAUNCHER")), TEXT("GRENADE LAUNCHER"), 0, false, false}
    };
    Items = {
        {FName(TEXT("NONE")), TEXT("NO ITEM"), 0, true, false},
        {FName(TEXT("RATION")), TEXT("RATION [TEST]"), 1, true, true},
        {FName(TEXT("CARD1")), TEXT("CARD 1"), 0, false, false},
        {FName(TEXT("CARD2")), TEXT("CARD 2"), 0, false, false},
        {FName(TEXT("CARD3")), TEXT("CARD 3"), 0, false, false},
        {FName(TEXT("CARD4")), TEXT("CARD 4"), 0, false, false},
        {FName(TEXT("CARD5")), TEXT("CARD 5"), 0, false, false},
        {FName(TEXT("CARD6")), TEXT("CARD 6"), 0, false, false},
        {FName(TEXT("CARD7")), TEXT("CARD 7"), 0, false, false},
        {FName(TEXT("CARD8")), TEXT("CARD 8"), 0, false, false},
        {FName(TEXT("COMPASS")), TEXT("COMPASS"), 0, false, false},
        {FName(TEXT("PLASTIC_EXPLOSIVE")), TEXT("PLASTIC EXPLOSIVE"), 0, false, true},
        {FName(TEXT("BINOCULARS")), TEXT("BINOCULARS"), 0, false, false},
        {FName(TEXT("GASMASK")), TEXT("GAS MASK"), 0, false, false},
        {FName(TEXT("MINES")), TEXT("LAND MINES"), 0, false, true}
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
bool UMGEquipmentComponent::ConsumeItem(FName ItemId, int32 Amount)
{
    if (bConfiscated || Amount <= 0) return false;
    for (FMGEquipmentEntry& Entry : Items)
        if (Entry.Id == ItemId && Entry.bOwned && Entry.bConsumable)
        {
            if (Entry.Quantity < Amount) return false;
            Entry.Quantity -= Amount;
            if (Entry.Quantity <= 0 && Items.IsValidIndex(EquippedItem) &&
                Items[EquippedItem].Id == ItemId)
                EquippedItem = 0;
            return true;
        }
    return false;
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

bool UMGEquipmentComponent::HasItem(FName ItemId) const
{
    for (const FMGEquipmentEntry& Entry : Items)
        if (Entry.Id == ItemId && Entry.bOwned && (!Entry.bConsumable || Entry.Quantity > 0))
            return true;
    return false;
}

void UMGEquipmentComponent::InitialiseMissionOneLoadout()
{
    bConfiscated = false;
    EquippedWeapon = EquippedItem = WeaponSelection = ItemSelection = 0;
    bWeaponWindowOpen = bItemWindowOpen = false;
    for (FMGEquipmentEntry& Entry : Weapons)
    {
        if (Entry.Id == TEXT("NONE")) continue;
        Entry.bOwned = false;
        Entry.Quantity = 0;
    }
    for (FMGEquipmentEntry& Entry : Items)
    {
        if (Entry.Id == TEXT("NONE")) continue;
        Entry.bOwned = false;
        Entry.Quantity = 0;
    }
    SetFeedback(TEXT("MISSION START — NO WEAPONS OR EQUIPMENT"));
}

bool UMGEquipmentComponent::AwardAmmunition(FName WeaponId, int32 Count)
{
    if (Count <= 0) return false;
    for (FMGEquipmentEntry& Entry : Weapons)
        if (Entry.Id == WeaponId)
        {
            // Ammunition is stored even if the actual firearm is not yet owned.
            Entry.Quantity = FMath::Clamp(Entry.Quantity + Count, 0, 999);
            return true;
        }
    return false;
}

bool UMGEquipmentComponent::IsItemEquipped(FName ItemId) const
{
    return !bConfiscated && Items.IsValidIndex(EquippedItem) &&
        Items[EquippedItem].Id == ItemId && IsAvailable(Items[EquippedItem]);
}
bool UMGEquipmentComponent::IsHandgunEquipped() const
{
    return !bConfiscated && Weapons.IsValidIndex(EquippedWeapon) &&
        Weapons[EquippedWeapon].Id == FName(TEXT("HANDGUN")) &&
        IsAvailable(Weapons[EquippedWeapon]);
}
int32 UMGEquipmentComponent::GetHandgunRounds() const
{
    for (const FMGEquipmentEntry& E : Weapons)
        if (E.Id == FName(TEXT("HANDGUN"))) return E.Quantity;
    return 0;
}
bool UMGEquipmentComponent::ConsumeHandgunRound()
{
    if (!IsHandgunEquipped()) return false;
    FMGEquipmentEntry& Slot = Weapons[EquippedWeapon];
    if (Slot.Quantity <= 0) { SetFeedback(TEXT("HANDGUN EMPTY")); return false; }
    --Slot.Quantity;
    return true;
}
void UMGEquipmentComponent::RestoreHandgunRounds(int32 Rounds)
{
    for (FMGEquipmentEntry& E : Weapons)
        if (E.Id == FName(TEXT("HANDGUN")))
        {
            E.Quantity = FMath::Clamp(Rounds, 0, 999);
            return;
        }
}

int32 UMGEquipmentComponent::GetItemQuantity(FName Id) const
{
    for (const FMGEquipmentEntry& E : Items)
        if (E.Id == Id) return E.Quantity;
    return 0;
}
void UMGEquipmentComponent::RestoreItemQuantity(FName Id, int32 Quantity)
{
    for (FMGEquipmentEntry& E : Items)
        if (E.Id == Id)
        {
            E.Quantity = FMath::Clamp(Quantity, 0, 999);
            return;
        }
}

bool UMGEquipmentComponent::IsWeaponEquipped(FName WeaponId) const
{
    return !bConfiscated && Weapons.IsValidIndex(EquippedWeapon) &&
        Weapons[EquippedWeapon].Id == WeaponId && IsAvailable(Weapons[EquippedWeapon]);
}
int32 UMGEquipmentComponent::GetWeaponAmmo(FName WeaponId) const
{
    for (const FMGEquipmentEntry& Entry : Weapons)
        if (Entry.Id == WeaponId) return Entry.Quantity;
    return 0;
}
bool UMGEquipmentComponent::ConsumeWeaponAmmo(FName WeaponId)
{
    if (!IsWeaponEquipped(WeaponId)) return false;
    FMGEquipmentEntry& Entry = Weapons[EquippedWeapon];
    if (Entry.Quantity <= 0)
    {
        SetFeedback(TEXT("NO AMMUNITION"));
        return false;
    }
    --Entry.Quantity;
    return true;
}
void UMGEquipmentComponent::RestoreWeaponAmmo(FName WeaponId, int32 Amount)
{
    for (FMGEquipmentEntry& Entry : Weapons)
        if (Entry.Id == WeaponId)
        {
            Entry.Quantity = FMath::Clamp(Amount, 0, 999);
            return;
        }
}
