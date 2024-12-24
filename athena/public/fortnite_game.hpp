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

class InventoryManager {
private:
    UObject* QuickBars;
    UObject* WorldInventory;
    UObject* PlayerController;

    struct ItemBatch {
        UObject* ItemDefinition;
        int Slot;
        char InQuickBar;
        int Count;
        int Level;
    };

public:
    InventoryManager(UObject* PlayerController) {
        this->PlayerController = PlayerController;
        this->QuickBars = SpawnActor(UObject::Object("/Script/FortniteGame.FortQuickBars"));
        this->PlayerController->Property("QuickBars") = this->QuickBars;
        this->QuickBars->Function("SetOwner", PlayerController);
        this->WorldInventory = GetWorldInventory();
    }

    std::vector<ItemBatch> ItemList;

    // Retrieve the WorldInventory property
    UObject* GetWorldInventory() {
        return PlayerController->Property("WorldInventory");
    }

    // Retrieve the Inventory pointer from WorldInventory
    __int64* GetInventory() {
        return &WorldInventory->Property<__int64>("Inventory");
    }

    // Retrieve ItemInstances from the Inventory
    TArray<UObject*> GetItemInstances() {
        auto Inventory = GetInventory();
        auto FortItemList = UObject::Object<UStruct>("/Script/FortniteGame.FortItemList");
        return GetAtPointer<TArray<UObject*>>(Inventory, FortItemList->StructPropertyOffset("ItemInstances"));
    }

    void Equip(FGuid Guid) {
        auto ItemInstances = GetItemInstances();

        for (auto ItemInstance : ItemInstances) {
            if (!ItemInstance) continue;

            auto ItemEntry = &ItemInstance->Property<void*>("ItemEntry");
            auto FortItemEntry = UObject::Object<UStruct>("/Script/FortniteGame.FortItemEntry");
            auto ItemGuid = GetAtPointer<FGuid>(ItemEntry, FortItemEntry->StructPropertyOffset("ItemGuid"));

            if (!UObject::Object("/Script/Engine.Default__KismetGuidLibrary")->Function<bool>("EqualEqual_GuidGuid", ItemGuid, Guid))
                continue;

            auto Pawn = PlayerController->Property("Pawn");
            auto ItemDefinition = ItemInstance->Function("GetItemDefinitionBP");

            if (Pawn && ItemDefinition) {
                auto Weapon = Pawn->Function<UObject*>("EquipWeaponDefinition", ItemDefinition, Guid);
                Weapon->Property<int32>("AmmoCount") = ItemInstance->Function<int32>("GetLoadedAmmo");
                break;
            }
        }
    }

    void Update() {
        for (const auto& Item : ItemList) {
            auto ItemInstance = Item.ItemDefinition->Function<UObject*>("CreateTemporaryItemInstanceBP", Item.Count, Item.Level);
            ItemInstance->Function("SetOwningControllerForTemporaryItem", PlayerController);

            auto ItemEntry = &ItemInstance->Property<void*>("ItemEntry");
            auto FortItemEntry = UObject::Object<UStruct>("/Script/FortniteGame.FortItemEntry");
            GetAtPointer<int>(ItemEntry, FortItemEntry->StructPropertyOffset("Count")) = 1;

            auto Inventory = GetInventory();
            auto FortItemList = UObject::Object<UStruct>("/Script/FortniteGame.FortItemList");
            GetAtPointer<TArray<UObject*>>(Inventory, FortItemList->StructPropertyOffset("ItemInstances")).Add(ItemInstance);
            GetAtPointer<TArray<UObject*>>(Inventory, FortItemList->StructPropertyOffset("ReplicatedEntries")).Add(FortItemEntry->Size(), ItemEntry);

            PlayerController->Property("QuickBars")->Function(
                "ServerAddItemInternal",
                GetAtPointer<FGuid>(ItemEntry, FortItemEntry->StructPropertyOffset("ItemGuid")),
                Item.InQuickBar,
                Item.Slot
            );
        }

        ItemList.clear();
        WorldInventory->Function("HandleInventoryLocalUpdate");
        PlayerController->Function("OnRep_QuickBar");
    }
};
