#pragma once

#include "unreal_engine.hpp"

auto FortEngine()
{
	auto FortEngine = UObject::Object("/Engine/Transient.FortEngine_0");
#ifdef DEBUG
	Debug::Log("FortEngine: ", FortEngine->GetFullName());
#endif
	return FortEngine;
}

auto GameViewport()
{
	auto GameViewport = FortEngine()->Property("GameViewport");
#ifdef DEBUG
	Debug::Log("GameViewport: ", GameViewport->GetFullName());
#endif
	return GameViewport;
}

auto World()
{
	auto World = GameViewport()->Property("World");
#ifdef DEBUG
	Debug::Log("World: ", World->GetFullName());
#endif
	return World;
}

auto GameInstance()
{
	auto GameInstance = GameViewport()->Property("GameInstance");
#ifdef DEBUG
	Debug::Log("GameInstance: ", GameInstance->GetFullName());
#endif
	return GameInstance;
}

auto LocalPlayers()
{
	auto LocalPlayers = GameInstance()->Property<TArray<UObject*>>("LocalPlayers");
#ifdef DEBUG
	Debug::Log("LocalPlayers: ", LocalPlayers.Num());
#endif
	return LocalPlayers;
}

auto LocalPlayer()
{
	auto LocalPlayer = LocalPlayers().Data[0];
#ifdef DEBUG
	Debug::Log("LocalPlayer: ", LocalPlayer->GetFullName());
#endif
	return LocalPlayer;
}

auto PlayerController()
{
	auto PlayerController = LocalPlayer()->Property("PlayerController");
#ifdef DEBUG
	Debug::Log("PlayerController: ", PlayerController->GetFullName());
#endif
	return PlayerController;
}