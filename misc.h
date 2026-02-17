#pragma once
#include "pch.h"

#include "player.h"
#include "configuration.h"

namespace Misc {
	static inline void TickFlush(UNetDriver* Driver, float Delta) {
		if (Driver->ClientConnections.Num() > 0)
			Funcs::ServerReplicateActors(Driver->ReplicationDriver, Delta);

		return OGs::TickFlushOG(Driver, Delta);
	}

	static inline float GetMaxTickRate() {
		return Configuration::bLateGame ? 60.f : 30.f;
	}

	static inline int GetNetMode() {
		return 1;
	}

	static inline void* Wtf(void* a1, void* a2, int) {
		return OGs::WtfOG(a1, a2, 3);
	}

	static inline bool Listen() {
		auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
		auto GameNetDriver = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");

		AFortOnlineBeaconHost* Beacon = SpawnActor<AFortOnlineBeaconHost>();
		if (!Beacon)
			return false;

		if (!Funcs::InitHost(Beacon))
			return false;

		if (!Beacon->NetDriver)
			return false;

		Beacon->NetDriverName = FName(282);
		Beacon->NetDriver->NetDriverName = FName(282);
		Beacon->NetDriver->World = UWorld::GetWorld();

		UWorld::GetWorld()->NetDriver = Beacon->NetDriver;
		UWorld::GetWorld()->NetDriver->NetDriverName = GameNetDriver;
		UWorld::GetWorld()->NetDriver->World = UWorld::GetWorld();

		FURL URL;
		URL.Port = 7777;

		FString Error;

		Funcs::InitListen(UWorld::GetWorld()->NetDriver, UWorld::GetWorld(), URL, false, Error);
		Funcs::SetWorld(UWorld::GetWorld()->NetDriver, UWorld::GetWorld());

		for (auto& Level : UWorld::GetWorld()->LevelCollections)
			Level.NetDriver = UWorld::GetWorld()->NetDriver;

		SetConsoleTitleA("Exlo 9.10 | Listening");

		return true;
	}

	static inline bool StartAircraftPhase(AFortGameModeAthena* GameMode, char a2) {
		auto OG = OGs::StartAircraftPhaseOG(GameMode, a2);

		auto* GameState = (AFortGameStateAthena*)GameMode->GameState;
		auto* Aircraft = GameState->Aircrafts[0];

		if (Configuration::bLateGame) {
			auto Aircraft = GameState->GetAircraft(0);
			auto& FlightInfo = Aircraft->FlightInfo;

			FVector Loc = GameMode->SafeZoneLocations[4];
			Loc.Z = 12000.f;

			FlightInfo.FlightSpeed = 0.f;
			FlightInfo.FlightStartLocation = FVector_NetQuantize100(Loc);
			FlightInfo.TimeTillDropEnd = 2.f;
			FlightInfo.TimeTillDropStart = 0.f;
			FlightInfo.TimeTillFlightEnd = 10.f;

			Aircraft->DropEndTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + 2.f;
			Aircraft->DropStartTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
			Aircraft->FlightEndTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + 10.f;

			//GameState->bAircraftIsLocked = false;
			GameState->SafeZonesStartTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + 2.f;
		}

		SetConsoleTitleA("Exlo 9.10 | Running");

		return OG;
	}

	static inline void HandlePostSafeZonePhaseChanged(AFortGameModeAthena* GameMode, int NewSafeZonePhase_Inp) {
		if (!GameMode->SafeZoneIndicator)
			return;

		auto GameState = (AFortGameStateAthena*)GameMode->GameState;
		auto NewSafeZonePhase = NewSafeZonePhase_Inp >= 0 ? NewSafeZonePhase_Inp : (GameMode->SafeZonePhase + 1);
		auto SafeZoneDefinition = &GameState->MapInfo->SafeZoneDefinition;

		OGs::HandlePostSafeZonePhaseChangedOG(GameMode, NewSafeZonePhase_Inp);

		if (Configuration::bLateGame && GameMode->SafeZonePhase < 4)
		{
			float TimeSeconds = (float)UGameplayStatics::GetTimeSeconds(GameState);
			GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = TimeSeconds;
			GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime + 0.15f;
			return;
		}
		else if (Configuration::bLateGame && GameMode->SafeZonePhase == 4)
			if (Configuration::bLateGame && Configuration::bRespawn)
				GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = 676767.f;
	}

	static inline void ProcessEvent(UObject* Obj, UFunction* Func, void* Params) { // too lazy to find vfts
		std::string FuncName = Func->GetName();

		if (FuncName == "ServerReturnToMainMenu") {
			((APlayerController*)Obj)->ClientReturnToMainMenu(L"");
			return;
		}
		else if (FuncName == "ServerChangeName") {
			auto Controller = (AFortPlayerControllerAthena*)Obj;
			if (!Controller || !Controller->MyFortPawn)
				return;
			Controller->MyFortPawn->K2_DestroyActor();
			return;
		}
		else if (FuncName == "ServerExecuteInventoryItem") {
			auto prms = (Params::FortPlayerController_ServerExecuteInventoryItem*)Params;
			auto Controller = (AFortPlayerControllerAthena*)Obj;
			if (!Controller || !Controller->MyFortPawn)
				return;
			Player::ServerExecuteInventoryItem(Controller, prms->ItemGuid);
			return;
		}
		else if (FuncName == "ServerAttemptAircraftJump") {
			auto prms = (Params::FortPlayerController_ServerAttemptAircraftJump*)Params;
			Player::ServerAttemptAircraftJump((AFortPlayerControllerAthena*)Obj, prms->ClientRotation);
			return;
		}
		else if (FuncName == "ServerExecuteInventoryWeapon") { // fuck s9
			auto prms = (Params::FortPlayerController_ServerExecuteInventoryWeapon*)Params;
			auto Controller = (AFortPlayerControllerAthena*)Obj;
			if (!Controller || !Controller->MyFortPawn)
				return;
			Player::ServerExecuteInventoryItem(Controller, prms->Weapon->ItemEntryGuid);
			return;
		}
		else if (FuncName == "OnAircraftExitedDropZone") {
			auto GameMode = (AFortGameModeAthena*)Obj;
			auto GameState = (AFortGameStateAthena*)GameMode->GameState;

			for (auto Player : GameMode->AlivePlayers)
				if (Player->IsInAircraft())
					Player->ServerAttemptAircraftJump(FRotator());

			if (Configuration::bLateGame) {
				GameState->GamePhase = EAthenaGamePhase::SafeZones;
				GameState->GamePhaseStep = EAthenaGamePhaseStep::StormHolding;
				GameState->OnRep_GamePhase(EAthenaGamePhase::Aircraft);
			}
		}
		else if (FuncName == "ServerRepairBuildingActor") {
			auto prms = (Params::FortPlayerController_ServerRepairBuildingActor*)Params;
			auto Controller = (AFortPlayerControllerAthena*)Obj;

			auto Price = std::floor((10.f * (1.f - prms->BuildingActorToRepair->GetHealthPercent())) * 0.75f);
			auto Resource = UFortKismetLibrary::K2_GetResourceItemDefinition(prms->BuildingActorToRepair->ResourceType);
			auto Entry = Inventory::FindItemEntryByDef(Controller, Resource);

			Inventory::RemoveItem(Controller, Resource, Price, Entry->ItemGuid);

			prms->BuildingActorToRepair->RepairBuilding(Controller, Price);
			return;
		}
		else if (FuncName == "ServerHandlePickup") {
			auto prms = (Params::FortPlayerPawn_ServerHandlePickup*)Params;
			auto* Pawn = (AFortPlayerPawnAthena*)Obj;
			if (Pawn) {
				auto Controller = (AFortPlayerControllerAthena*)Pawn->Controller;
				auto ItemEntry = prms->Pickup->PrimaryPickupItemEntry;
				Funcs::SetPickupTarget(prms->Pickup, Pawn, prms->InFlyTime / Pawn->PickupSpeedMultiplier, prms->InStartDirection, prms->bPlayPickupSound);
				Inventory::GiveItem((AFortPlayerControllerAthena*)Pawn->Controller, ItemEntry.ItemDefinition, ItemEntry.Count, ItemEntry.LoadedAmmo, ItemEntry.Level);
			}
			return;
		}
		else if (FuncName == "OnCapsuleBeginOverlap") {
			auto prms = (Params::FortPlayerPawn_OnCapsuleBeginOverlap*)Params;
			auto* Pawn = (AFortPlayerPawnAthena*)Obj;

			if (!Pawn || prms->OtherActor->IsA(AFortPickup::StaticClass()))
				return OGs::ProcessEventOG(Obj, Func, Params);

			auto* Pickup = (AFortPickup*)prms->OtherActor;
			if (!Pickup)
				return OGs::ProcessEventOG(Obj, Func, Params);

			auto* ItemDefinition = Pickup->PrimaryPickupItemEntry.ItemDefinition;
			if (!ItemDefinition)
				return OGs::ProcessEventOG(Obj, Func, Params);

			auto* Controller = (AFortPlayerControllerAthena*)Pawn->Controller;
			if (!Controller)
				return OGs::ProcessEventOG(Obj, Func, Params);

			auto* repEnt = Inventory::FindItemEntryByDef(Controller, ItemDefinition);
			if (Pickup->PawnWhoDroppedPickup != Pawn)
				if ((!repEnt && Pickup->PrimaryPickupItemEntry.ItemDefinition->bForceAutoPickup) || (repEnt && repEnt->Count < ItemDefinition->MaxStackSize))
					Pawn->ServerHandlePickup(Pickup, 0.4f, FVector(), true);

			return;
		}
		else if (FuncName == "ServerAttemptInventoryDrop") {
			auto prms = (Params::FortPlayerController_ServerAttemptInventoryDrop*)Params;
			auto* Controller = (AFortPlayerControllerAthena*)Obj;
			if (!Controller || !Controller->MyFortPawn)
				return;

			auto* repEnt = Inventory::FindItemEntryByGuid(Controller, prms->ItemGuid);
			if (!repEnt || repEnt->Count < prms->Count)
				return;

			FVector DropLoc = Controller->MyFortPawn->K2_GetActorLocation() + Controller->MyFortPawn->GetActorForwardVector() * 70.f + FVector(0,0,50);

			Inventory::SpawnPickup(DropLoc, *repEnt);
			Inventory::RemoveItem(Controller, repEnt->ItemDefinition, prms->Count, prms->ItemGuid);

			return;
		}
		else if (FuncName == "ServerSendZiplineState") {
			auto prms = (Params::FortPlayerPawn_ServerSendZiplineState*)Params;
			auto* Pawn = (AFortPlayerPawnAthena*)Obj;
			if (Pawn) {
				Pawn->ZiplineState = prms->InZiplineState;
				Funcs::OnRep_ZiplineState(Pawn);
				if (Pawn->ZiplineState.bJumped) {
					auto VelocityX = Pawn->CharacterMovement->Velocity.X * -0.5f;
					auto VelocityY = Pawn->CharacterMovement->Velocity.Y * -0.5f;
					Pawn->LaunchCharacterJump({ VelocityX >= -750 ? min(VelocityX, 750) : -750, VelocityY >= -750 ? min(VelocityY, 750) : -750, 1200 }, false, false, true, true);
				}
			}
			return;
		}
		else if (FuncName == "ServerPlayEmoteItem") {
			auto prms = (Params::FortPlayerController_ServerPlayEmoteItem*)Params;
			auto Controller = (AFortPlayerControllerAthena*)Obj;
			if (!prms->EmoteAsset || !Controller->MyFortPawn || Controller->MyFortPawn->IsDead())
				return;

			if (prms->EmoteAsset->IsA(UAthenaDanceItemDefinition::StaticClass())) {
				auto EmoteAbility = StaticLoadObject<UBlueprintGeneratedClass>(L"/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C");
				auto DanceAsset = (UAthenaDanceItemDefinition*)prms->EmoteAsset;

				Controller->MyFortPawn->bMovingEmote = DanceAsset->bMovingEmote;
				Controller->MyFortPawn->EmoteWalkSpeed = DanceAsset->WalkForwardSpeed;

				FGameplayAbilitySpecHandle Handle{};
				FGameplayAbilitySpec Spec{};
				Funcs::ConstructSpec(&Spec, EmoteAbility->DefaultObject, 1, -1, (UObject*)prms->EmoteAsset);
				Funcs::ActivateAbilityOnce(((AFortPlayerStateAthena*)Controller->PlayerState)->AbilitySystemComponent, &Handle, &Spec, nullptr);
			}

			return;
		}

		return OGs::ProcessEventOG(Obj, Func, Params);
	}
};