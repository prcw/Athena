#pragma once

#include "unreal_engine.hpp"

bool ReadyToStartMatch = false;

auto FortEngine()
{
	auto FortEngine = UObject::Object("/Engine/Transient.FortEngine_0");
	Debug::Log("FortEngine: ", FortEngine->GetFullName());
	return FortEngine;
}

auto GameViewport()
{
	auto GameViewport = FortEngine()->Property("GameViewport");
	Debug::Log("GameViewport: ", GameViewport->GetFullName());
	return GameViewport;
}

auto World()
{
	auto World = GameViewport()->Property("World");
	Debug::Log("World: ", World->GetFullName());
	return World;
}

auto GameInstance()
{
	auto GameInstance = GameViewport()->Property("GameInstance");
	Debug::Log("GameInstance: ", GameInstance->GetFullName());
	return GameInstance;
}

auto LocalPlayers()
{
	auto LocalPlayers = GameInstance()->Property<TArray<UObject*>>("LocalPlayers");
	Debug::Log("LocalPlayers: ", LocalPlayers.Num());
	return LocalPlayers;
}

auto LocalPlayer()
{
	auto LocalPlayer = LocalPlayers().Data[0];
	Debug::Log("LocalPlayer: ", LocalPlayer->GetFullName());
	return LocalPlayer;
}

auto PlayerController()
{
	auto PlayerController = LocalPlayer()->Property("PlayerController");
	Debug::Log("PlayerController: ", PlayerController->GetFullName());
	return PlayerController;
}

auto GameplayStatics()
{
	return UObject::Object("/Script/Engine.Default__GameplayStatics");
}

auto SpawnActor(UObject* Class, FVector Location = FVector(), FRotator Rotation = FRotator())
{
	FQuat Quat;
	FTransform Transform;

	auto DEG_TO_RAD = 3.14159 / 180;
	auto DIVIDE_BY_2 = DEG_TO_RAD / 2;

	auto SP = sin(Rotation.Pitch * DIVIDE_BY_2);
	auto CP = cos(Rotation.Pitch * DIVIDE_BY_2);

	auto SY = sin(Rotation.Yaw * DIVIDE_BY_2);
	auto CY = cos(Rotation.Yaw * DIVIDE_BY_2);

	auto SR = sin(Rotation.Roll * DIVIDE_BY_2);
	auto CR = cos(Rotation.Roll * DIVIDE_BY_2);

	Quat.X = CR * SP * SY - SR * CP * CY;
	Quat.Y = -CR * SP * CY - SR * CP * SY;
	Quat.Z = CR * CP * SY - SR * SP * CY;
	Quat.W = CR * CP * CY + SR * SP * SY;

	Transform.Rotation = Quat;
	Transform.Scale3D = FVector{ 1,1,1 };
	Transform.Translation = Location;

	auto Actor = GameplayStatics()->Function<UObject*>("BeginDeferredActorSpawnFromClass", World(), Class, Transform, false, nullptr);
	return GameplayStatics()->Function<UObject*>("FinishSpawningActor", Actor, Transform);
}

auto SwitchLevel()
{
	FString URL;
#ifdef _DEBUG
	URL = FString(L"Athena_Faceoff");
#else _RELEASE
	URL = FString(L"Athena_Terrain");
#endif
	PlayerController()->Function("SwitchLevel", URL);
}