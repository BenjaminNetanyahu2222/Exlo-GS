#pragma once
#include "pch.h"

#include "abilities.h"
#include "inventory.h"
#include "lootpool.h"
#include "1v1.h"

namespace Player {
    static inline void ServerAcknowledgePossession(AFortPlayerController* Controller, APawn* Pawn) {
        Controller->AcknowledgedPawn = Pawn;
    }

    static inline void SetLoadedAmmo(UFortWorldItem* Item, int LoadedAmmo) {
        auto Controller = (AFortPlayerControllerAthena*)Item->GetOwningController();
        if (!Controller)
            return;

        auto* repEnt = Inventory::FindItemEntryByGuid(Controller, Item->ItemEntry.ItemGuid);
        if (!repEnt)
            return;

        repEnt->LoadedAmmo = LoadedAmmo;
        Item->ItemEntry.LoadedAmmo = LoadedAmmo;

        Inventory::UpdateInventory(Controller, repEnt);
    }

    static inline void ServerLoadingScreenDropped(AFortPlayerControllerAthena* Controller) {
        if (!Controller->bLoadingScreenDropped) {
            auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;

            if (!LightAmmo)
                LightAmmo = StaticLoadObject<UFortItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
            if (!MediumAmmo)
                MediumAmmo = StaticLoadObject<UFortItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
            if (!HeavyAmmo)
                HeavyAmmo = StaticLoadObject<UFortItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
            if (!Shells)
                Shells = StaticLoadObject<UFortItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");

            if (!Wood)
                Wood = StaticLoadObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
            if (!Stone)
                Stone = StaticLoadObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
            if (!Metal)
                Metal = StaticLoadObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

            if (!EditTool)
                EditTool = StaticLoadObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/EditTool.EditTool");

            Abilities::GiveAbilitySet(Controller, UObject::FindObject<UFortAbilitySet>("FortAbilitySet GAS_AthenaPlayer.GAS_AthenaPlayer"));

            Controller->MyFortPawn->CosmeticLoadout = Controller->CosmeticLoadoutPC;
            Controller->MyFortPawn->OnRep_CosmeticLoadout();
            Funcs::ApplyCharacterCustomization(Controller->PlayerState, Controller->MyFortPawn);

            Controller->WorldInventory->Inventory.ReplicatedEntries.Clear();
            Controller->WorldInventory->Inventory.ItemInstances.Clear();

            Inventory::GiveItem(Controller, Controller->CosmeticLoadoutPC.Pickaxe->WeaponDefinition);

            for (auto& StartingItem : GameMode->StartingItems)
                Inventory::GiveItem(Controller, StartingItem.Item, StartingItem.Count);

            if (Configuration::bIs1v1)
                Map_1v1::Give1v1Items(Controller);
        }

		return OGs::ServerLoadingScreenDroppedOG(Controller);
	}

    static inline void ServerExecuteInventoryItem(AFortPlayerControllerAthena* Controller, FGuid ItemGuid)
    {
        if (!Controller->WorldInventory)
            return;

        for (auto& Entry : Controller->WorldInventory->Inventory.ReplicatedEntries) {
            if (Inventory::CompareGUIDs(Entry, ItemGuid)) {
                Controller->MyFortPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, ItemGuid);
                return;
            }
        }
    }

   static inline void GetPlayerViewpoint(AFortPlayerControllerAthena* Controller, FVector& Loc, FRotator& Rot)
    {
       if (auto Pawn = Controller->MyFortPawn) {
           Loc = Pawn->K2_GetActorLocation();
           Rot = Controller->GetControlRotation();
           return;
       }

       if (OGs::GetPlayerViewpointOG)
           OGs::GetPlayerViewpointOG(Controller, Loc, Rot);
       else {
           if (Controller->StateName == UKismetStringLibrary::Conv_StringToName(L"Spectating")) {
               Loc = Controller->LastSpectatorSyncLocation;
               Rot = Controller->LastSpectatorSyncRotation;
           }
           else {
               auto ViewTarget = Controller->GetViewTarget();
               if (ViewTarget) {
                   Loc = ViewTarget->K2_GetActorLocation();
                   Rot = ViewTarget->K2_GetActorRotation();
               }
           }
       }
    }

    static inline void ServerAttemptAircraftJump(AFortPlayerControllerAthena* Controller, FRotator& Rotation) {
        static auto ServerAttemptAircraftJumpOG = reinterpret_cast<void(*)(AFortPlayerControllerAthena*, FRotator&)>((*(void***)AFortPlayerControllerAthena::GetDefaultObj())[0x284]);
        ServerAttemptAircraftJumpOG(Controller, Rotation);

        if (Configuration::bLateGame) {
            if (Controller && Controller->Pawn && Controller->MyFortPawn) {
                Controller->MyFortPawn->SetShield(100.f);

                Controller->WorldInventory->Inventory.ReplicatedEntries.Clear();
                Controller->WorldInventory->Inventory.ItemInstances.Clear();

                Inventory::GiveItem(Controller, Controller->CosmeticLoadoutPC.Pickaxe->WeaponDefinition);

                for (auto& StartingItem : ((AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode)->StartingItems)
                    Inventory::GiveItem(Controller, StartingItem.Item, StartingItem.Count);

                Inventory::GiveItem(Controller, Wood, 500); 
                Inventory::GiveItem(Controller, Stone, 500);
                Inventory::GiveItem(Controller, Metal, 500);

                Inventory::GiveItem(Controller, Shells, 30);
                Inventory::GiveItem(Controller, LightAmmo, 200);
                Inventory::GiveItem(Controller, MediumAmmo, 250);
                Inventory::GiveItem(Controller, HeavyAmmo, 20);

                Lootpool::ItemEntry Shotgun = Lootpool::GetShotgun();
                Lootpool::ItemEntry AR = Lootpool::GetAR();
                Lootpool::ItemEntry Movement = Lootpool::GetMovement();
                Lootpool::ItemEntry Heal = Lootpool::GetHeal();
                Lootpool::ItemEntry Heal2 = Lootpool::GetHeal();

                while (Heal.Item == Heal2.Item)
                    Heal2 = Lootpool::GetHeal();

                Inventory::GiveItem(Controller, Shotgun.Item, Shotgun.Count, Shotgun.Ammo, false);
                Inventory::GiveItem(Controller, AR.Item, AR.Count, AR.Ammo, false);
                Inventory::GiveItem(Controller, Movement.Item, Movement.Count, Movement.Ammo, Movement.Item->MaxStackSize > 1);
                Inventory::GiveItem(Controller, Heal.Item, Heal.Count, Heal.Ammo, Movement.Item->MaxStackSize > 1);
                Inventory::GiveItem(Controller, Heal2.Item, Heal2.Count, Heal2.Ammo, Movement.Item->MaxStackSize > 1);
            }
        }
    }

    static inline void ClientOnPawnDied(AFortPlayerControllerAthena* Controller, FFortPlayerDeathReport& DeathReport) {
        if (!Controller)
            return OGs::ClientOnPawnDiedOG(Controller, DeathReport);

        auto GameMode = (AFortGameMode*)UWorld::GetWorld()->AuthorityGameMode;
        auto GameState = (AFortGameStateAthena*)GameMode->GameState;

        auto DeadState = (AFortPlayerStateAthena*)Controller->PlayerState;
        auto KillerState = (AFortPlayerStateAthena*)DeathReport.KillerPlayerState;
        auto KillerPawn = (AFortPlayerPawnAthena*)DeathReport.KillerPawn;

        if (Controller->WorldInventory && Controller->MyFortPawn)
            for (auto& Item : Controller->WorldInventory->Inventory.ReplicatedEntries)
                if (((UFortWorldItemDefinition*)Item.ItemDefinition)->bCanBeDropped == 1)
                    Inventory::SpawnPickup(Controller->MyFortPawn->K2_GetActorLocation(), Item);

        if (DeadState && KillerState && KillerState != DeadState) {
            KillerState->KillScore++;
            KillerState->OnRep_Kills();

            KillerState->TeamKillScore++;
            KillerState->OnRep_TeamKillScore();

            KillerState->ClientReportKill(DeadState);
            KillerState->ClientReportTeamKill(KillerState->TeamKillScore);

            if (KillerPawn) {
                static FGameplayTag EarnedElim = { UKismetStringLibrary::Conv_StringToName(TEXT("Event.EarnedElimination")) };
                FGameplayEventData Data{};
                Data.EventTag = EarnedElim;
                Data.ContextHandle = KillerState->AbilitySystemComponent->MakeEffectContext();
                Data.Instigator = KillerState->Owner;
                Data.Target = DeadState;
                Data.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(DeadState);
                UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(KillerPawn, EarnedElim, Data);
            }
        }

        if (DeadState) {
            auto DeathInfo = DeadState->DeathInfo;

            DeadState->PawnDeathLocation = Controller->MyFortPawn ? Controller->MyFortPawn->K2_GetActorLocation() : FVector();
            DeathInfo.DeathLocation = DeadState->PawnDeathLocation;
            DeathInfo.DeathTags = Controller->MyFortPawn ? *(FGameplayTagContainer*)(__int64(&Controller->MyFortPawn->MoveSoundStimulusBroadcastInterval) + 0x10) : FGameplayTagContainer();
            DeathInfo.FinisherOrDowner = KillerPawn ? KillerPawn : nullptr;
            DeathInfo.DeathCause = AFortPlayerStateAthena::ToDeathCause(DeathInfo.DeathTags, false);
            DeathInfo.Distance = Controller->MyFortPawn ? (DeadState->DeathInfo.DeathCause != EDeathCause::FallDamage ? (KillerPawn ? KillerPawn->GetDistanceTo(Controller->MyFortPawn) : 0) : ((AFortPlayerPawnAthena*)Controller->MyFortPawn)->LastFallDistance) : 0;
            DeathInfo.bDBNO = false;
            DeathInfo.bInitialized = true;

            DeadState->OnRep_DeathInfo();

            DeadState->Place = GameState->PlayersLeft;
            DeadState->OnRep_Place();

            AFortWeapon* DamageCauser = nullptr;
            if (DeathReport.DamageCauser ? DeathReport.DamageCauser->IsA(AFortProjectileBase::StaticClass()) : false) {
                auto Owner = DeathReport.DamageCauser->Owner;
                if (Owner->IsA(AFortWeapon::StaticClass()))
                    DamageCauser = (AFortWeapon*)Owner;
                else if (Owner->IsA(AFortPlayerControllerAthena::StaticClass()))
                    DamageCauser = (AFortWeapon*)((AFortPlayerControllerAthena*)Owner)->MyFortPawn->CurrentWeapon;
                else if (Owner->IsA(AFortPlayerPawnAthena::StaticClass()))
                    DamageCauser = (AFortWeapon*)((AFortPlayerPawnAthena*)Owner)->CurrentWeapon;
            }
            else if (auto Weapon = DeathReport.DamageCauser ? (AFortWeapon*)DeathReport.DamageCauser : nullptr)
                DamageCauser = Weapon;

            Funcs::RemoveFromAlivePlayers(GameMode, Controller, KillerState == DeadState ? nullptr : KillerState, KillerPawn, DamageCauser->IsA(AFortWeapon::StaticClass()) ? DamageCauser->WeaponData : nullptr, (uint8)DeathInfo.DeathCause, 0);

            if (Controller->MyFortPawn && KillerState && KillerState != DeadState && KillerState->Place == 1) {
                auto KillerWeapon = DamageCauser ? DamageCauser->WeaponData : nullptr;

                GameState->WinningTeam = KillerState->TeamIndex;
                GameState->OnRep_WinningTeam();

                GameState->WinningPlayerState = KillerState;
                GameState->OnRep_WinningPlayerState();
            }
        }

        return OGs::ClientOnPawnDiedOG(Controller, DeathReport);
    }
};