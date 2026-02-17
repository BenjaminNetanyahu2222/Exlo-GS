#ifndef PCH_H
#define PCH_H

#include <windows.h>
#include <thread>

#include "SDK/MinHook.h"

//SDK
#include "SDK/SDK/GameplayAbilities_classes.hpp"
#include "SDK/SDK/GameplayAbilities_structs.hpp"
#include "SDK/SDK/Engine_classes.hpp"
#include "SDK/SDK/Engine_structs.hpp"
#include "SDK/SDK/CoreUObject_classes.hpp"
#include "SDK/SDK/CoreUObject_structs.hpp"
#include "SDK/SDK/FortniteGame_classes.hpp"
#include "SDK/SDK/FortniteGame_structs.hpp"
#include "SDK/SDK/Basic.hpp"

//Params for ProcessEvent funcs
#include "SDK/SDK/FortniteGame_parameters.hpp"

using namespace SDK;

static TArray<AActor*> Foundations;
static TArray<AActor*> PStarts;


namespace Addresses {
	static inline uint64_t InitListen = InSDKUtils::GetImageBase() + 0x6643A0;
	static inline uint64_t InitHost = InSDKUtils::GetImageBase() + 0x663E50;
	static inline uint64_t SetWorld = InSDKUtils::GetImageBase() + 0x2ed1540;
	static inline uint64_t TickFlush = InSDKUtils::GetImageBase() + 0x2ed22b0;
	static inline uint64_t ServerReplicateActors = InSDKUtils::GetImageBase() + 0x9bfa40;
	static inline uint64_t GetMaxTickRate = InSDKUtils::GetImageBase() + 0x315d2c0;
	static inline uint64_t Wtf = InSDKUtils::GetImageBase() + 0xb10de0;
	static inline uint64_t ApplyCharacterCustomization = InSDKUtils::GetImageBase() + 0x1897D50;
	static inline uint64_t SpawnDefaultPawnFor = InSDKUtils::GetImageBase() + 0x1098E10;
	static inline uint64_t StaticLoadObject = InSDKUtils::GetImageBase() + 0x2003110;
	static inline uint64_t SetPickupTarget = InSDKUtils::GetImageBase() + 0x158A630;
	static inline uint64_t OnRep_ZiplineState = InSDKUtils::GetImageBase() + 0x17D55D0;
};

namespace Funcs {
	static inline auto InitHost = (bool (*) (AOnlineBeaconHost*)) (Addresses::InitHost);
	static inline auto InitListen = (bool (*)(UNetDriver*, UWorld*, FURL&, bool, FString&)) (Addresses::InitListen);
	static inline auto SetWorld = (void (*)(UNetDriver*, UWorld*)) (Addresses::SetWorld);
	static inline auto ServerReplicateActors = (void (*)(UReplicationDriver*, float)) (Addresses::ServerReplicateActors);
	static inline auto ConstructSpec = (void (*)(FGameplayAbilitySpec*, UObject*, int, int, UObject*)) (InSDKUtils::GetImageBase() + 0x8C8410);
	static inline auto GiveAbility = (FGameplayAbilitySpecHandle* (*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec)) (InSDKUtils::GetImageBase() + 0x8A3F30);
	static inline auto InternalTryActivateAbility = (bool (*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle, FPredictionKey, UGameplayAbility**, void*, const FGameplayEventData*)) (InSDKUtils::GetImageBase() + 0x8A5710);
	static inline auto ActivateAbilityOnce = (void (*)(UFortAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec*, void*)) (InSDKUtils::GetImageBase() + 0x8A4050);
	static inline auto StaticLoadObjectOG = (UObject * (*)(UClass*, UObject*, const TCHAR*, const TCHAR*, uint32_t, UObject*, bool, void*)) (Addresses::StaticLoadObject);
	static inline auto CantBuild = (bool (*)(UWorld*, UObject*, FVector, FRotator, bool, TArray<ABuildingSMActor*>*, char*)) (InSDKUtils::GetImageBase() + 0x144D670);
	static inline auto ApplyCharacterCustomization = (void* (*)(void*, void*)) (Addresses::ApplyCharacterCustomization);
	static inline auto ReplaceBuildingActor = (ABuildingSMActor * (*) (ABuildingSMActor*, __int64, TSubclassOf<ABuildingSMActor>, __int64, int, bool, AFortPlayerControllerAthena*)) (InSDKUtils::GetImageBase() + 0x1226C70);
	static inline auto RemoveFromAlivePlayers = (void (*)(AFortGameMode*, AFortPlayerControllerAthena*, AFortPlayerStateAthena*, AFortPlayerPawnAthena*, UFortItemDefinition*, uint8, char)) (InSDKUtils::GetImageBase() + 0x1092830);
	static inline auto SetPickupTarget = (void (*&)(AFortPickup*, AFortPlayerPawnAthena*, float, FVector, bool)) (Addresses::SetPickupTarget);
	static inline auto OnRep_ZiplineState = (void (*)(AFortPlayerPawnAthena*)) (Addresses::OnRep_ZiplineState);
};

namespace OGs {
	static inline void (*TickFlushOG)(UNetDriver*, float);
	static inline void (*ServerLoadingScreenDroppedOG)(AFortPlayerControllerAthena*);
	static inline void (*HandleStartingNewPlayerOG)(AFortGameModeAthena*, AFortPlayerControllerAthena*);
	static inline bool (*ReadyToStartMatchOG)(AFortGameModeAthena*);
	static inline void* (*WtfOG)(void*, void*, int);
	static inline void (*GetPlayerViewpointOG)(AFortPlayerControllerAthena*, FVector&, FRotator&);
	static inline void (*ProcessEventOG)(UObject*, UFunction*, void*);
	static inline bool (*StartAircraftPhaseOG)(AFortGameModeAthena*, char);
	static inline void (*HandlePostSafeZonePhaseChangedOG)(AFortGameModeAthena*, int);
	static inline void (*OnDamageServerOG)(ABuildingSMActor*, float, FGameplayTagContainer, FVector, FHitResult, AFortPlayerControllerAthena*, AActor*, FGameplayEffectContextHandle);
	static inline void (*ClientOnPawnDiedOG)(AFortPlayerControllerAthena*, FFortPlayerDeathReport&);
};

namespace Utils {
	static inline void HookVFT(void* Base, uintptr_t Idx, void* Detour, void** OG = nullptr) {
		auto VTable = (*(void***)Base);
		if (!VTable || !VTable[Idx])
			return;

		if (OG)
			*OG = VTable[Idx];

		DWORD old;
		VirtualProtect(&VTable[Idx], 8, PAGE_EXECUTE_READWRITE, &old);

		VTable[Idx] = Detour;

		DWORD temp;
		VirtualProtect(&VTable[Idx], 8, old, &temp);
	}

	template <typename T>
	static inline void Patch(uintptr_t Ptr, T Byte) {
		DWORD OG;
		VirtualProtect(LPVOID(Ptr), sizeof(T), PAGE_EXECUTE_READWRITE, &OG);

		*(T*)Ptr = Byte;
		VirtualProtect(LPVOID(Ptr), sizeof(T), OG, &OG);
	}

	static inline FQuat FRotToQuat(FRotator Rotation) {
		FQuat Quat;
		const float DEG_TO_RAD = 3.14159f / 180.0f;
		const float DIVIDE_BY_2 = DEG_TO_RAD / 2.0f;

		float SP = sin(Rotation.Pitch * DIVIDE_BY_2);
		float CP = cos(Rotation.Pitch * DIVIDE_BY_2);
		float SY = sin(Rotation.Yaw * DIVIDE_BY_2);
		float CY = cos(Rotation.Yaw * DIVIDE_BY_2);
		float SR = sin(Rotation.Roll * DIVIDE_BY_2);
		float CR = cos(Rotation.Roll * DIVIDE_BY_2);

		Quat.X = CR * SP * SY - SR * CP * CY;
		Quat.Y = -CR * SP * CY - SR * CP * SY;
		Quat.Z = CR * CP * SY - SR * SP * CY;
		Quat.W = CR * CP * CY + SR * SP * SY;

		return Quat;
	}

	struct FActorSpawnParameters
	{
	public:
		FName Name;
		AActor* Template;
		AActor* Owner;
		APawn* Instigator;
		ULevel* OverrideLevel;
		ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride;

	private:
		uint8	bRemoteOwned : 1;
	public:
		uint8	bNoFail : 1;
		uint8	bDeferConstruction : 1;
		uint8	bAllowDuringConstructionScript : 1;
		EObjectFlags ObjectFlags;
	};

	template <class T>
	T* SpawnActor(FVector Location = {}, FRotator Rotation = FRotator{ 0, 0, 0 }, UClass* Class = T::StaticClass(), AActor* Owner = nullptr, FVector Scale3D = { 1,1,1 })
	{
		FActorSpawnParameters addr{};

		addr.Owner = Owner;
		addr.bDeferConstruction = false;
		addr.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FQuat Quat = FRotToQuat(Rotation);
		FTransform Transform{};

		Transform.Rotation = Quat;
		Transform.Scale3D = Scale3D;
		Transform.Translation = Location;

		auto Actor = ((AActor * (*)(UWorld*, UClass*, FTransform const*, FActorSpawnParameters*))(InSDKUtils::GetImageBase() + 0x2e41d80))(UWorld::GetWorld(), Class, &Transform, &addr);
		return (T*)Actor;
	}

	template<typename T>
	static inline T* SpawnBuildingActor(FVector Loc = {}, FRotator Rot = {}, AActor* Owner = nullptr, UClass* Class = T::StaticClass())
	{
		FTransform Transform{};
		Transform.Scale3D = FVector{ 1,1,1 };
		Transform.Translation = Loc;
		Transform.Rotation = FRotToQuat(Rot);

		return (T*)UGameplayStatics::FinishSpawningActor(UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), Class, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, Owner), Transform);
	}

	static inline void Hook(uintptr_t Offset, void* Detour, void** OG = nullptr) {
		MH_CreateHook((LPVOID)Offset, Detour, OG);
		MH_EnableHook((LPVOID)Offset);
	}

	static inline int RetTrue() {
		return 1;
	}

	static inline void RetHook() {
		return;
	}

	template<typename T>
	static inline T* StaticLoadObject(const TCHAR* name) {
		return (T*)Funcs::StaticLoadObjectOG(T::StaticClass(), nullptr, name, nullptr, 0, nullptr, false, nullptr);
	}
};

using namespace Utils;

#include "configuration.h"

inline UFortItemDefinition* Wood = nullptr;
inline UFortItemDefinition* Stone = nullptr;
inline UFortItemDefinition* Metal = nullptr;

inline UFortItemDefinition* LightAmmo = nullptr;
inline UFortItemDefinition* MediumAmmo = nullptr;
inline UFortItemDefinition* HeavyAmmo = nullptr;
inline UFortItemDefinition* Shells = nullptr;

inline UFortItemDefinition* EditTool = nullptr;

#endif