#pragma once
#include "pch.h"

namespace GameMode {
	static inline APawn* SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AController* Controller, AActor* StartSpot) {
		return GameMode->SpawnDefaultPawnAtTransform(Controller, StartSpot->GetTransform());
	}

	static inline void HandleStartingNewPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Controller) {
		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)Controller->PlayerState;
		if (!PlayerState)
			return OGs::HandleStartingNewPlayerOG(GameMode, Controller);

		PlayerState->SquadId = PlayerState->TeamIndex - 3;
		PlayerState->OnRep_SquadId();

		FGameMemberInfo Member;
		Member.MostRecentArrayReplicationKey = -1;
		Member.ReplicationID = -1;
		Member.ReplicationKey = -1;

		Member.TeamIndex = PlayerState->TeamIndex;
		Member.SquadId = PlayerState->SquadId;
		Member.MemberUniqueId = PlayerState->UniqueId;

		auto GameState = (AFortGameStateAthena*)GameMode->GameState;
		GameState->GameMemberInfoArray.Members.Add(Member);
		GameState->GameMemberInfoArray.MarkItemDirty(Member);

		if (!Controller->MatchReport)
			Controller->MatchReport = reinterpret_cast<UAthenaPlayerMatchReport*>(UGameplayStatics::SpawnObject(UAthenaPlayerMatchReport::StaticClass(), Controller));
	
		return OGs::HandleStartingNewPlayerOG(GameMode, Controller);
	}

	static bool ReadyToStartMatch(AFortGameModeAthena* GameMode) {
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)GameMode->GameState;

		if (!GameState->MapInfo)
			return false;

		UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");

		if (GameMode->CurrentPlaylistId == -1) {
			GameMode->CurrentPlaylistId = Playlist->PlaylistId;
			GameMode->CurrentPlaylistName = Playlist->PlaylistName;

			GameState->CurrentPlaylistId = Playlist->PlaylistId;

			Playlist->GarbageCollectionFrequency = 9999999999.f;

			GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
			GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;
			GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
			GameState->CurrentPlaylistInfo.MarkArrayDirty();

			GameState->OnRep_CurrentPlaylistId();
			GameState->OnRep_CurrentPlaylistInfo();

			GameMode->GameSession->MaxPlayers = Playlist->MaxPlayers;
			GameMode->WarmupRequiredPlayerCount = 1;

			for (auto& Level : GameState->CurrentPlaylistInfo.BasePlaylist->AdditionalLevels) {
				bool Success = false;
				ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), Level, FVector(), FRotator(), &Success);

				if (Success)
					GameState->AdditionalPlaylistLevelsStreamed.Add(Level.ObjectID.AssetPathName);
			}

			GameState->OnRep_AdditionalPlaylistLevelsStreamed();
		}

		if (!GameMode->bWorldIsReady) {
			if (!Misc::Listen())
				printf("Failed to listen!");

			GameMode->DefaultPawnClass = StaticLoadObject<UClass>(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");

			GameMode->bWorldIsReady = true;
		}

		return GameMode->AlivePlayers.Num() > 0;
	}
};