#pragma once
#include "pch.h"

#include "inventory.h"

namespace Building {
	static inline void ServerCreateBuildingActor(AFortPlayerControllerAthena* Controller, FCreateBuildingActorData BData) {
		if (!Controller || !Controller->MyFortPawn)
			return;

		TArray<ABuildingSMActor*> Buildings;
		char Out = 0;

		auto BuildingClass = Controller->BroadcastRemoteClientInfo->RemoteBuildableClass;

		if (Funcs::CantBuild(UWorld::GetWorld(), BuildingClass, BData.BuildLoc, BData.BuildRot, BData.bMirrored, &Buildings, &Out))
			return;

		ABuildingSMActor* Building = SpawnBuildingActor<ABuildingSMActor>(BData.BuildLoc, BData.BuildRot, nullptr, BuildingClass);
		if (!Building)
			return;

		Building->bPlayerPlaced = true;
		Building->InitializeKismetSpawnedBuildingActor(Building, Controller, true);

		Building->Team = EFortTeam(((AFortPlayerStateAthena*)Controller->PlayerState)->TeamIndex);
		Building->TeamIndex = (uint8)Building->Team;

		for (auto& Build : Buildings)
			Build->K2_DestroyActor();
		Buildings.Clear();

		auto* Resource = UFortKismetLibrary::K2_GetResourceItemDefinition(((ABuildingSMActor*)BuildingClass->DefaultObject)->ResourceType);
		if (!Resource)
			return;

		auto* repEnt = Inventory::FindItemEntryByDef(Controller, Resource);
		if (!repEnt)
			return;

		Inventory::RemoveItem(Controller, Resource, 10, repEnt->ItemGuid);
	}

	static inline void ServerBeginEditingBuildingActor(AFortPlayerControllerAthena* Controller, ABuildingSMActor* Building) {
		if (!Controller || !Building)
			return;

		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)Controller->PlayerState;
		Building->SetNetDormancy(ENetDormancy::DORM_DormantAll);
		Building->EditingPlayer = PlayerState;

		auto* Item = Inventory::FindItemEntryByDef(Controller, EditTool);
		if (Item)
			Controller->MyFortPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)EditTool, Item->ItemGuid);

		if (!Controller->MyFortPawn->CurrentWeapon || !Controller->MyFortPawn->CurrentWeapon->IsA(AFortWeap_EditingTool::StaticClass()))
			return;

		AFortWeap_EditingTool* EditTool = (AFortWeap_EditingTool*)Controller->MyFortPawn->CurrentWeapon;
		EditTool->EditActor = Building;
		EditTool->OnRep_EditActor();

		EditTool->ForceNetUpdate();
		Building->ForceNetUpdate();
	}

	static inline void ServerEndEditingBuildingActor(AFortPlayerControllerAthena* Controller, ABuildingSMActor* Building) {
		if (!Controller || !Building || !Controller->MyFortPawn || Building->bDestroyed)
			return;
		
		Building->SetNetDormancy(ENetDormancy::DORM_DormantAll);
		Building->EditingPlayer = nullptr;

		auto* Item = Inventory::FindItemEntryByDef(Controller, EditTool);
		if (Item)
			Controller->MyFortPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)EditTool, Item->ItemGuid);

		if (!Controller->MyFortPawn->CurrentWeapon || !Controller->MyFortPawn->CurrentWeapon->WeaponData || !Controller->MyFortPawn->CurrentWeapon->IsA(AFortWeap_EditingTool::StaticClass()))
			return;

		AFortWeap_EditingTool* EditTool = (AFortWeap_EditingTool*)Controller->MyFortPawn->CurrentWeapon;
		EditTool->EditActor = nullptr;
		EditTool->OnRep_EditActor();

		EditTool->ForceNetUpdate();
		Building->ForceNetUpdate();
	}

	static inline void ServerEditBuildingActor(AFortPlayerControllerAthena* Controller, ABuildingSMActor* Building, TSubclassOf<ABuildingSMActor> NewBuildingClass, uint8 RotationIterations, bool bMirrored) {
		if (!Controller || !Building || !NewBuildingClass || Building->bDestroyed)
			return;

		Building->SetNetDormancy(ENetDormancy::DORM_DormantAll);
		Building->EditingPlayer = nullptr;

		ABuildingSMActor* EditedBuildingActor = Funcs::ReplaceBuildingActor(Building, 1, NewBuildingClass, 0, RotationIterations, bMirrored, Controller);
		if (EditedBuildingActor)
			EditedBuildingActor->bPlayerPlaced = true;

		EditedBuildingActor->ForceNetUpdate();
	}

	static inline void OnDamageServer(ABuildingSMActor* Actor, float Dmg, FGameplayTagContainer DmgTags, FVector Momentum, FHitResult HitInfo, AFortPlayerControllerAthena* InstigatedBy, AActor* DmgCauser, FGameplayEffectContextHandle EffectCtx) {
		auto GameState = ((AFortGameStateAthena*)UWorld::GetWorld()->GameState);

		if (!InstigatedBy || Actor->bPlayerPlaced || Actor->GetHealth() == 1 || Actor->IsA(UObject::FindClassFast("B_Athena_VendingMachine_C")) || Actor->IsA(GameState->MapInfo->LlamaClass)) 
			return OGs::OnDamageServerOG(Actor, Dmg, DmgTags, Momentum, HitInfo, InstigatedBy, DmgCauser, EffectCtx);
		
		if (!DmgCauser || !DmgCauser->IsA(AFortWeapon::StaticClass()) || !((AFortWeapon*)DmgCauser)->WeaponData->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
			return OGs::OnDamageServerOG(Actor, Dmg, DmgTags, Momentum, HitInfo, InstigatedBy, DmgCauser, EffectCtx);

		static auto PickaxeTag = UKismetStringLibrary::Conv_StringToName(L"Weapon.Melee.Impact.Pickaxe");

		FGameplayTag* Entry = nullptr;
		for (auto& Tag : DmgTags.GameplayTags)
			if (Tag.TagName.ComparisonIndex == PickaxeTag.ComparisonIndex)
				Entry = &Tag;

		if (!Entry)
			return OGs::OnDamageServerOG(Actor, Dmg, DmgTags, Momentum, HitInfo, InstigatedBy, DmgCauser, EffectCtx);

		auto* Resource = UFortKismetLibrary::K2_GetResourceItemDefinition(Actor->ResourceType);
		if (!Resource)
			return OGs::OnDamageServerOG(Actor, Dmg, DmgTags, Momentum, HitInfo, InstigatedBy, DmgCauser, EffectCtx);

		auto MaxMat = 500;
		int ResCount = 0;
		FCurveTableRowHandle& BuildingResourceAmountOverride = Actor->BuildingResourceAmountOverride;

		if (Actor->BuildingResourceAmountOverride.RowName.ComparisonIndex > 0) {
			float Out;
			UDataTableFunctionLibrary::EvaluateCurveTableRow(Actor->BuildingResourceAmountOverride.CurveTable, Actor->BuildingResourceAmountOverride.RowName, 0.f, nullptr, &Out, FString());
			float RC = Out / (Actor->GetMaxHealth() / Dmg);
			ResCount = (int)round(RC);
		}

		if (ResCount > 0)
		{
			FFortItemEntry* Entry2 = nullptr;
			for (auto& Entry_ : InstigatedBy->WorldInventory->Inventory.ReplicatedEntries)
				if (Entry_.ItemDefinition == Resource)
					Entry2 = &Entry_;

			if (Entry2)
			{
				Entry2->Count += ResCount;
				if (Entry2->Count > MaxMat)
				{
					FFortItemEntry idk;
					idk.Count = ResCount;
					idk.ItemDefinition = Resource;
					idk.LoadedAmmo = 0;

					Entry2->Count = MaxMat;

					Inventory::SpawnPickup(InstigatedBy->Pawn->K2_GetActorLocation(), idk);
				}
				Inventory::UpdateInventory(InstigatedBy, Entry2);
			}
			else
			{
				if (ResCount > MaxMat)
				{
					FFortItemEntry idk;
					idk.Count = ResCount;
					idk.ItemDefinition = Resource;
					idk.LoadedAmmo = 0;

					ResCount = MaxMat;

					Inventory::SpawnPickup(InstigatedBy->Pawn->K2_GetActorLocation(), idk);
				}

				Inventory::GiveItem(InstigatedBy, Resource, ResCount, 0, false);
			}
		}
		
		InstigatedBy->ClientReportDamagedResourceBuilding(Actor, ResCount == 0 ? EFortResourceType::None : Actor->ResourceType, ResCount, false, Dmg == 100.f);
		return OGs::OnDamageServerOG(Actor, Dmg, DmgTags, Momentum, HitInfo, InstigatedBy, DmgCauser, EffectCtx);
	}
};