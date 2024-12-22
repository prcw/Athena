#pragma once

#include "fortnite_game.hpp"

inline void* ProcessEvent(UObject* This, UFunction* Function, void* Parameters)
{
	auto ThisName = This->GetName();
	auto FunctionName = Function->GetName();

#ifdef _DEBUG
	if (FuncName.contains("BP_PlayButton"))
	{
		SwitchLevel();
	}
#endif

	if (FunctionName.contains("PostLogin")) // Note: impl original
	{
		auto NewPlayer = *reinterpret_cast<UObject**>(Parameters); 
		Debug::Log("GameMode: ", This->GetFullName(), " accepted NewPlayer: ", NewPlayer->GetFullName());
	}

	if (FunctionName.contains("ReadyToStartMatch")) // Note: impl original
	{
		if (!ReadyToStartMatch)
		{
			ReadyToStartMatch = true;

			auto GameMode = This; 
			auto GameState = GameMode->Property("GameState");

			GameState->Property<char>("GamePhase") = 2; // Note: unneeded?
			GameState->Function("OnRep_GamePhase", 0);

			GameState->Property("CurrentPlaylistData") = UObject::Object("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
			GameState->Function("OnRep_CurrentPlaylistData");

			GameMode->Property<bool>("bWorldIsReady") = true; // Note: this is a bitfield
		}
	}

	if (FunctionName.contains("SpawnDefaultPawnFor"))
	{
		auto Original = This->ProcessEvent(Function, Parameters);

		struct SpawnDefaultPawnForParams
		{
			UObject* NewPlayer;
			UObject* StartSpot;
			UObject* ReturnValue;
		};

		auto Params = reinterpret_cast<SpawnDefaultPawnForParams*>(Parameters);

		auto NewPawn = SpawnActor(UObject::Object("/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C"), FVector(35000, 3500, 3500)); if (!NewPawn) { return Original; }
		Params->ReturnValue = NewPawn;

		return Original;
	}

	if (FunctionName.contains("HandleStartingNewPlayer"))
	{
		auto Original = This->ProcessEvent(Function, Parameters);

		auto NewPlayer = *reinterpret_cast<UObject**>(Parameters); if (!NewPlayer) return Original;
		Debug::Log("GameMode: ", This->GetFullName(), " is handling the starting NewPlayer : ", NewPlayer->GetFullName());
 
		NewPlayer->Property<bool>("bHasClientFinishedLoading") = true; // Note: unneeded? 
		NewPlayer->Property<bool>("bHasServerFinishedLoading") = true;
		NewPlayer->Function("OnRep_bHasServerFinishedLoading");

		return Original;
	}

	if (FunctionName.contains("ServerLoadingScreenDropped"))
	{
		auto PlayerController = This;
		auto Original = PlayerController->ProcessEvent(Function, Parameters);

		auto WarmupActors = GameplayStatics()->Function<TArray<UObject*>, 0x10>("GetAllActorsOfClass", World(), UObject::Object("/Script/FortniteGame.FortPlayerStartWarmup"), TArray<UObject*>());
		auto WarmupActor = WarmupActors.Data[rand() % WarmupActors.Num()];
		PlayerController->Property("Pawn")->Function(("K2_TeleportTo"), WarmupActor->Function<FVector>(("K2_GetActorLocation")), WarmupActor->Function<FRotator>("K2_GetActorRotation"));

		return Original;
	}

	return This->ProcessEvent(Function, Parameters);
}
