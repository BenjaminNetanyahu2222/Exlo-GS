#include "pch.h"

#include "misc.h"
#include "gamemode.h"
#include "player.h"
#include "abilities.h"
#include "building.h"

void Main() {
    AllocConsole();
    FILE* F;
    freopen_s(&F, "CONOUT$", "w", stdout);
    SetConsoleTitleA("Exlo 9.10 | Loading");

    Sleep(5000);

    MH_Initialize();

    Patch<uint8_t>(InSDKUtils::GetImageBase() + 0x29A2100, 0xC3);
    Patch<uint8_t>(InSDKUtils::GetImageBase() + 0x1403040, 0xC3);

    Patch<uint32_t>(InSDKUtils::GetImageBase() + 0x2D9AE90, 0xC0FFC031);
    Patch<uint32_t>(InSDKUtils::GetImageBase() + 0x890540, 0xC0FFC031);
    Patch<uint32_t>(InSDKUtils::GetImageBase() + 0x1FAA590, 0xC0FFC031);

    Patch<uint8_t>(InSDKUtils::GetImageBase() + 0x2D9AE90 + 4, 0xC3);
    Patch<uint8_t>(InSDKUtils::GetImageBase() + 0x890540 + 4, 0xC3);
    Patch<uint8_t>(InSDKUtils::GetImageBase() + 0x1FAA590 + 4, 0xC3);

    Patch<uint8_t>(InSDKUtils::GetImageBase() + 0x10D9CCC, 0x85);
    Patch<uint8_t>(InSDKUtils::GetImageBase() + 0x31AEBCA, 0x74);

    Hook(InSDKUtils::GetImageBase() + 0x31AC180, Misc::GetNetMode);
    Hook(InSDKUtils::GetImageBase() + Offsets::ProcessEvent, Misc::ProcessEvent, (void**)&OGs::ProcessEventOG);

    Hook(Addresses::GetMaxTickRate, Misc::GetMaxTickRate);
    Hook(Addresses::TickFlush, Misc::TickFlush, (void**)&OGs::TickFlushOG);
    Hook(Addresses::Wtf, Misc::Wtf, (void**)&OGs::WtfOG);

    Hook(Addresses::SpawnDefaultPawnFor, GameMode::SpawnDefaultPawnFor);

    Hook(InSDKUtils::GetImageBase() + 0x109C520, Misc::StartAircraftPhase, (void**)&OGs::StartAircraftPhaseOG);
    Hook(InSDKUtils::GetImageBase() + 0x109D9D0, Misc::HandlePostSafeZonePhaseChanged, (void**)&OGs::HandlePostSafeZonePhaseChangedOG);

    Hook(InSDKUtils::GetImageBase() + 0x18172A0, Player::GetPlayerViewpoint, (void**)&OGs::GetPlayerViewpointOG);

    HookVFT(AFortGameModeAthena::GetDefaultObj(), 0xFC, GameMode::ReadyToStartMatch, (void**)&OGs::ReadyToStartMatchOG);
    HookVFT(AFortGameModeAthena::GetDefaultObj(), 0xC9, GameMode::HandleStartingNewPlayer, (void**)&OGs::HandleStartingNewPlayerOG);

    HookVFT(UAbilitySystemComponent::GetDefaultObj(), 0xF4, Abilities::InternalServerTryActivateAbility);
    HookVFT(UFortAbilitySystemComponent::GetDefaultObj(), 0xF4, Abilities::InternalServerTryActivateAbility);
    HookVFT(UFortAbilitySystemComponentAthena::GetDefaultObj(), 0xF4, Abilities::InternalServerTryActivateAbility);

    HookVFT(UFortWorldItem::GetDefaultObj(), 0x94, Player::SetLoadedAmmo);

    HookVFT(AFortPlayerControllerAthena::GetDefaultObj(), 0x260, Player::ServerLoadingScreenDropped, (void**)&OGs::ServerLoadingScreenDroppedOG);
    HookVFT(AFortPlayerControllerAthena::GetDefaultObj(), 0x108, Player::ServerAcknowledgePossession);
    HookVFT(AFortPlayerControllerAthena::GetDefaultObj(), 0x223, Building::ServerCreateBuildingActor);
    HookVFT(AFortPlayerControllerAthena::GetDefaultObj(), 0x22A, Building::ServerBeginEditingBuildingActor);
    HookVFT(AFortPlayerControllerAthena::GetDefaultObj(), 0x228, Building::ServerEndEditingBuildingActor);
    HookVFT(AFortPlayerControllerAthena::GetDefaultObj(), 0x225, Building::ServerEditBuildingActor);

    Hook(InSDKUtils::GetImageBase() + 0x1837A60, Inventory::RemoveInventoryItem);

    Hook(InSDKUtils::GetImageBase() + 0x1A49CB0, Building::OnDamageServer, (void**)&OGs::OnDamageServerOG);

    Hook(InSDKUtils::GetImageBase() + 0x1C61050, Player::ClientOnPawnDied, (void**)&OGs::ClientOnPawnDiedOG);

    *(bool*)(InSDKUtils::GetImageBase() + 0x5BE45F3) = false;
    *(bool*)(InSDKUtils::GetImageBase() + 0x5BE45F3 + 1) = true;

    UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"open Athena_Terrain", nullptr);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        std::thread(Main).detach();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

