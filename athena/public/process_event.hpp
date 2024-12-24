#pragma once

#include "fortnite_game.hpp"

inline void* ProcessEvent(UObject* This, UFunction* Function, void* Parameters)
{
	auto ThisName = This->GetName();
	auto FunctionName = Function->GetName();

#ifdef _DEBUG
	if (FunctionName.contains("BP_PlayButton"))
	{
		SwitchLevel();
	}
#endif

	if (FunctionName.contains("ReadyToStartMatch"))
	{
		if (!ReadyToStartMatch)
		{
			ReadyToStartMatch = true;

			auto GameMode = This; 
			auto Original = GameMode->ProcessEvent(Function, Parameters);

			auto GameState = GameMode->Property("GameState"); if (!GameState) return Original;
			GameState->Property<char>("GamePhase") = 2;
			GameState->Function("OnRep_GamePhase", 0);
			
			auto Playlist = UObject::Object("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo"); if (!Playlist) return Original;
			GameState->Property("CurrentPlaylistData") = Playlist;
			GameState->Function("OnRep_CurrentPlaylistData");

			auto bWorldIsReadyOffset = GameMode->PropertyOffset("bWorldIsReady");
			auto bWorldIsReadyBitfield = GameMode->GetAtPointer<Bitfield>(bWorldIsReadyOffset);
			WriteBitfield(bWorldIsReadyBitfield, 1, true);

			return Original;
		}
	}

	if (FunctionName.contains("SpawnDefaultPawnFor"))
	{
		struct SpawnDefaultPawnForParams
		{
			UObject* NewPlayer;
			UObject* StartSpot;
			UObject* ReturnValue;
		};

		auto Params = static_cast<SpawnDefaultPawnForParams*>(Parameters);
		auto Original = This->ProcessEvent(Function, Params);

		auto NewPawn = SpawnActor(UObject::Object("/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C"), FVector(0, 0, 5000)); 
		if (!NewPawn) { return Original; }

		Params->ReturnValue = NewPawn;
		return Original;
	}

	if (FunctionName.contains("HandleStartingNewPlayer"))
	{
		auto GameMode = This;
		auto Original = GameMode->ProcessEvent(Function, Parameters);

		auto NewPlayer = *reinterpret_cast<UObject**>(Parameters); 
		if (!NewPlayer) return Original;
	
		auto bHasInitiallySpawnedOffset = NewPlayer->GetAtPointer<Bitfield>(NewPlayer->PropertyOffset("bHasInitiallySpawned"));
		auto bHasInitiallySpawnedFieldMask = FieldMask(NewPlayer->StaticProperty("bHasInitiallySpawned"));
		WriteBitfield(bHasInitiallySpawnedOffset, bHasInitiallySpawnedFieldMask, true);

		NewPlayer->Property<bool>("bHasClientFinishedLoading") = true;
		NewPlayer->Property<bool>("bHasServerFinishedLoading") = true;
		NewPlayer->Function("OnRep_bHasServerFinishedLoading");

		return Original;
	}

	if (FunctionName.contains("ServerLoadingScreenDropped"))
	{
		auto PlayerController = This;
		auto Original = PlayerController->ProcessEvent(Function, Parameters);

		auto PlayerState = PlayerController->Property("PlayerState");
		if (!PlayerState)
			return Original;

		auto Pawn = PlayerController->Property("Pawn");
		if (!Pawn)
			return Original;

		auto WarmupActors = GameplayStatics()->Function<TArray<UObject*>, 0x10>(
			"GetAllActorsOfClass",
			World(),
			UObject::Object("/Script/FortniteGame.FortPlayerStartWarmup"),
			TArray<UObject*>()
		);
		if (WarmupActors.Num() <= 0)
			return Original;

		auto WarmupActor = WarmupActors.Data[rand() % WarmupActors.Num()];
		if (!WarmupActor)
			return Original;

		Pawn->Function(
			"K2_TeleportTo",
			WarmupActor->Function<FVector>("K2_GetActorLocation"),
			WarmupActor->Function<FRotator>("K2_GetActorRotation")
		);

		Pawn->Function("ServerChoosePart", 0,
			UObject::Object("/Game/Characters/CharacterParts/Female/Medium/Heads/F_Med_Head1.F_Med_Head1", true));
		Pawn->Function("ServerChoosePart", 1,
			UObject::Object("/Game/Characters/CharacterParts/Female/Medium/Bodies/F_Med_Soldier_01.F_Med_Soldier_01", true));
		PlayerState->Function("OnRep_CharacterParts");

		auto Inventory = new InventoryManager(PlayerController);

		auto CustomizationLoadout = &PlayerController->Property<int64*>("CustomizationLoadout");
		Inventory->ItemList.push_back({ GetAtPointer<UObject*>(
			GetAtPointer<UObject*>(
			CustomizationLoadout,
			UObject::Object<UStruct>("/Script/FortniteGame.FortAthenaLoadout")
			->StructPropertyOffset("Pickaxe")
		),
			UObject::Object<UStruct>("/Script/FortniteGame.AthenaPickaxeItemDefinition")
			->StructPropertyOffset("WeaponDefinition")
		), 0, 0, 1 });

		Inventory->Update();

		return Original;
	}

	if (FunctionName.contains("ServerAttemptAircraftJump"))
	{
		auto PlayerController = This;

		auto NewPawn = SpawnActor(UObject::Object("/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C"), 
			PlayerController->Function<UObject*>("GetViewTarget")->Function<FVector>("K2_GetActorLocation"), *reinterpret_cast<FRotator*>(Parameters));
		PlayerController->Function("Possess", NewPawn);

		auto Inventory = InventoryManager(PlayerController);
		Inventory.Equip(Inventory.GetItemInstances().Data[0]->Function<FGuid>("GetItemGuid"));
	}

	if (FunctionName.contains("ServerExecuteInventoryItem"))
	{
		auto PlayerController = This;
		auto Original = PlayerController->ProcessEvent(Function, Parameters);

		auto Inventory = InventoryManager(PlayerController);
		Inventory.Equip(*reinterpret_cast<FGuid*>(Parameters));

		return Original;
	}

	return This->ProcessEvent(Function, Parameters);
}
