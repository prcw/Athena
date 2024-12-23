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

auto SpawnActor(UObject* Class, FVector Location = FVector(), FRotator Rotation = FRotator(), UObject* Owner = nullptr)
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

	auto Actor = GameplayStatics()->Function<UObject*>("BeginDeferredActorSpawnFromClass", World(), Class, Transform, false, Owner);
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

class InventoryHandler
{
private:
	__int64* Inventory;
	UObject* QuickBars;
	UObject* WorldInventory;
	UObject* PlayerController;

	struct Item
	{
		UObject* ItemDefinition;
		int Slot;
		char QuickBar;
		int Count;
		int Level;
	};
public:
	void Initialize(UObject* PlayerController)
	{
		this->PlayerController = PlayerController;
		Debug::Log("PlayerController: ", PlayerController->GetFullName());

		this->WorldInventory = PlayerController->Property("WorldInventory");
		Debug::Log("WorldInventory: ", WorldInventory->GetFullName());

		this->Inventory = &WorldInventory->Property<__int64>("Inventory");
		Debug::Log("Inventory: ", Inventory);

		this->QuickBars = PlayerController->Property("QuickBars");
		this->QuickBars = SpawnActor(UObject::Object("/Script/FortniteGame.FortQuickBars"), FVector(), FRotator(), PlayerController);
		Debug::Log("QuickBars: ", this->QuickBars->GetFullName());
	}

	std::vector<Item> ItemList;

	void Update()
	{
		for (const auto& Item : ItemList)
		{
			auto ItemInstance = Item.ItemDefinition->Function<UObject*>("CreateTemporaryItemInstanceBP", Item.Count, Item.Level);
			Debug::Log("ItemInstance: ", ItemInstance->GetFullName());
			ItemInstance->Function("SetOwningControllerForTemporaryItem", PlayerController);

			auto ItemEntry = &ItemInstance->Property<void*>("ItemEntry");
			Debug::Log("ItemEntry: ", ItemEntry);

			auto FortItemEntry = UObject::Object("/Script/FortniteGame.FortItemEntry");
			auto FortItemEntryCount = (*reinterpret_cast<int*>(reinterpret_cast<__int64>(ItemEntry) + FortItemEntry->StructPropertyOffset("Count")));
			auto FortItemEntryItemGuid = (*reinterpret_cast<FGuid*>(reinterpret_cast<__int64>(ItemEntry) + FortItemEntry->StructPropertyOffset("ItemGuid")));

			FortItemEntryCount = 1;
			Debug::Log("FortItemEntryCount: ", FortItemEntryCount);

			auto FortItemList = UObject::Object("/Script/FortniteGame.FortItemList");
			auto FortItemListItemInstances = *reinterpret_cast<TArray<UObject*>*>(reinterpret_cast<__int64>(Inventory) + FortItemList->StructPropertyOffset("ItemInstances"));
			auto FortItemListReplicatedEntries = *reinterpret_cast<TArray<UObject*>*>(reinterpret_cast<__int64>(Inventory) + FortItemList->StructPropertyOffset("ReplicatedEntries"));

			FortItemListItemInstances.Add(ItemInstance);
			Debug::Log("FortItemListItemInstances: ", FortItemListItemInstances.Num());

			FortItemListReplicatedEntries.Add(*(int32*)(int64(UObject::Object(("/Script/FortniteGame.FortItemEntry"))) + 0x40), ItemEntry);
			Debug::Log("FortItemListReplicatedEntries: ", FortItemListReplicatedEntries.Num());

			QuickBars->Function("ServerAddItemInternal", FortItemEntryItemGuid, Item.QuickBar, Item.Slot);
		}

		ItemList.clear();
		WorldInventory->Function("HandleInventoryLocalUpdate");
		PlayerController->Function("HandleWorldInventoryLocalUpdate");
		PlayerController->Function("ForceUpdateQuickbar", char(0));
		PlayerController->Function("ForceUpdateQuickbar", char(1));
	}
};

InventoryHandler Inventory;