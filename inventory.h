#pragma once
#include "pch.h"

namespace Inventory {
	static inline bool CompareGUIDs(FFortItemEntry& Entry, FGuid& ItemGuid) {
		return Entry.ItemGuid.A == ItemGuid.A && Entry.ItemGuid.B == ItemGuid.B && Entry.ItemGuid.C == ItemGuid.C && Entry.ItemGuid.D == ItemGuid.D;
	}

	static inline FFortItemEntry* FindItemEntryByGuid(AFortPlayerControllerAthena* Controller, FGuid& ItemGuid) {
		if (!Controller)
			return nullptr;

		auto& Entries = Controller->WorldInventory->Inventory.ReplicatedEntries;
		FFortItemEntry* Item = nullptr;

		for (int i = 0; i < Entries.Num(); ++i) {
			if (CompareGUIDs(Entries[i], ItemGuid)) {
				Item = &Entries[i];
				break;
			}
		}

		return Item;
	}

	static inline FFortItemEntry* FindItemEntryByDef(AFortPlayerControllerAthena* Controller, UFortItemDefinition* Def) {
		if (!Controller || !Def)
			return nullptr;

		auto& Entries = Controller->WorldInventory->Inventory.ReplicatedEntries;
		FFortItemEntry* Item = nullptr;

		for (int i = 0; i < Entries.Num(); ++i) {
			if (Entries[i].ItemDefinition == Def) {
				Item = &Entries[i];
				break;
			}
		}

		return Item;
	}

	static inline UFortWorldItem* FindWorldItemInstanceByGuid(AFortPlayerControllerAthena* Controller, FGuid& ItemGuid) {
		if (!Controller)
			return nullptr;

		auto& Entries = Controller->WorldInventory->Inventory.ItemInstances;
		UFortWorldItem* Item = nullptr;

		for (int i = 0; i < Entries.Num(); ++i) {
			if (CompareGUIDs(Entries[i]->ItemEntry, ItemGuid)) {
				Item = Entries[i];
				break;
			}
		}

		return Item;
	}

	static inline UFortWorldItem* FindWorldItemInstanceByDef(AFortPlayerControllerAthena* Controller, UFortItemDefinition* Def) {
		if (!Controller || !Def)
			return nullptr;

		auto& Entries = Controller->WorldInventory->Inventory.ItemInstances;
		UFortWorldItem* Item = nullptr;

		for (int i = 0; i < Entries.Num(); ++i) {
			if (Entries[i]->ItemEntry.ItemDefinition == Def) {
				Item = Entries[i];
				break;
			}
		}

		return Item;
	}

	bool IsPrimary(UFortItemDefinition* Def) {
		return Def->IsA(UFortWeaponRangedItemDefinition::StaticClass()) || Def->IsA(UFortConsumableItemDefinition::StaticClass()) || Def->IsA(UFortWeaponMeleeItemDefinition::StaticClass());
	}

	static inline int GetUsedSlots(AFortPlayerControllerAthena* Controller) {
		int ret = 0;
		for (auto& Entry : Controller->WorldInventory->Inventory.ReplicatedEntries)
			if (IsPrimary(Entry.ItemDefinition))
				ret++;
		return ret - 1;
	}

	static inline void UpdateInventory(AFortPlayerControllerAthena* Controller, FFortItemEntry* Entry) {
		if (!Controller || !Entry)
			return;

		AFortInventory* Inventory = Controller->WorldInventory;

		if (Entry)
			Inventory->Inventory.MarkItemDirty(*Entry);
		else
			Inventory->Inventory.MarkArrayDirty();

		Inventory->bRequiresLocalUpdate = true;
		Inventory->HandleInventoryLocalUpdate();
	}

	static inline AFortPickupAthena* SpawnPickup(FVector Loc, FFortItemEntry& Entry)
	{
		auto Ret = SpawnActor<AFortPickupAthena>(Loc);
		Ret->PrimaryPickupItemEntry.Count = Entry.Count;
		Ret->PrimaryPickupItemEntry.ItemDefinition = Entry.ItemDefinition;
		Ret->PrimaryPickupItemEntry.LoadedAmmo = Entry.LoadedAmmo;

		Ret->OnRep_PrimaryPickupItemEntry();
		Ret->TossPickup(Loc, nullptr, -1, true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination);

		return Ret;
	}

	static inline UFortWorldItem* GiveItem(AFortPlayerControllerAthena* Controller, UFortItemDefinition* Def, int Count = 1, int Ammo = 0, int Level = 0, bool bUpdateInv = true) {
		if (!Controller || !Controller->WorldInventory || !Def)
			return nullptr;

		UFortWorldItem* Item = (UFortWorldItem*)Def->CreateTemporaryItemInstanceBP(Count, Level);
		Item->SetOwningControllerForTemporaryItem(Controller);
		Item->ItemEntry.LoadedAmmo = Ammo;
		Item->ItemEntry.ItemDefinition = Def;
		Item->ItemEntry.Count = Count;
		Item->ItemEntry.Level = Level;

		FFortItemEntry* Entry = FindItemEntryByDef(Controller, Def);
		if (Entry) {
			Entry->Count += Count;
			if (Entry->Count > Def->MaxStackSize) {
				if (!Def->bAllowMultipleStacks || GetUsedSlots(Controller) >= 5)
					SpawnPickup(Controller->MyFortPawn->K2_GetActorLocation(), Item->ItemEntry);
				Entry->Count = Def->MaxStackSize;
			}
		}

		if (Entry && Entry->Count <= Def->MaxStackSize && (Def->IsA(UFortAmmoItemDefinition::StaticClass()) || Def->IsA(UFortResourceItemDefinition::StaticClass()))) {
			FFortItemEntryStateValue PickupNotif{};
			PickupNotif.IntValue = true;
			PickupNotif.StateType = EFortItemEntryState::ShouldShowItemToast;
			Item->ItemEntry.StateValues.Add(PickupNotif);
		}

		if (!Entry) {
			Controller->WorldInventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);
			Controller->WorldInventory->Inventory.ItemInstances.Add(Item);
		}

		if (bUpdateInv)
			UpdateInventory(Controller, &Item->ItemEntry);

		return Item;
	}

	static inline void RemoveItem(AFortPlayerControllerAthena* Controller, UFortItemDefinition* Def, int Count, FGuid ItemGuid) {
		if (!Controller || !Def)
			return;

		for (size_t i = 0; i < Controller->WorldInventory->Inventory.ReplicatedEntries.Num(); i++) {
			auto& Item = Controller->WorldInventory->Inventory.ReplicatedEntries[i];
			bool IsSameGuid = true;

			if (UKismetGuidLibrary::IsValid_Guid(ItemGuid) && !CompareGUIDs(Item, ItemGuid))
				IsSameGuid = false;

			if (Item.ItemDefinition == Def && IsSameGuid) {
				Item.Count -= Count;
				if (Item.Count <= 0) {
					for (size_t j = 0; j < Controller->WorldInventory->Inventory.ItemInstances.Num(); j++) {
						if (!Controller->WorldInventory->Inventory.ItemInstances[j])
							continue;

						if (CompareGUIDs(Controller->WorldInventory->Inventory.ItemInstances[j]->ItemEntry, ItemGuid)) {
							Controller->WorldInventory->Inventory.ItemInstances.Remove(j);
							break;
						}
					}

					Controller->WorldInventory->Inventory.ReplicatedEntries.Remove(i);
					UpdateInventory(Controller, &Item);
					return;
				}

				UpdateInventory(Controller, &Item);
				return;
			}
		}
	}

	static inline bool RemoveInventoryItem(IInterface* Interface, FGuid& ItemGuid, int Count, bool bForceRemoval) {
		auto Controller = (AFortPlayerControllerAthena*)(__int64(Interface) - 0x598);
		if (!Controller || !Controller->WorldInventory)
			return false;

		auto ItemP = FindWorldItemInstanceByGuid(Controller, ItemGuid);
		auto itemEntry = FindItemEntryByGuid(Controller, ItemGuid);

		if (ItemP) {
			itemEntry->Count -= Count;
			if (Count < 0 || itemEntry->Count <= 0 || bForceRemoval)
				RemoveItem(Controller, itemEntry->ItemDefinition, itemEntry->Count, ItemGuid);
			else {
				ItemP->ItemEntry.Count = itemEntry->Count;
				UpdateInventory(Controller, itemEntry);
			}

			return true;
		}

		return true;
	}

	static inline EFortQuickBars GetQB(UFortItemDefinition* Def) {  
		if (!Def)
			return EFortQuickBars::Max_None;
		return Def->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) || Def->IsA(UFortResourceItemDefinition::StaticClass()) || Def->IsA(UFortAmmoItemDefinition::StaticClass()) || Def->IsA(UFortTrapItemDefinition::StaticClass()) || Def->IsA(UFortBuildingItemDefinition::StaticClass()) || Def->IsA(UFortEditToolItemDefinition::StaticClass()) || ((UFortWorldItemDefinition*)Def)->bForceIntoOverflow ? EFortQuickBars::Secondary : EFortQuickBars::Primary;
	}
};