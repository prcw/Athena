#pragma once

#include "athena/utilities/macros.hpp"
#include "athena/public/process_event.hpp"

class EntryPoint
{
public:
	static inline bool Begin()
	{
		Debug::AllocateConsole();

		Decl(FMemory::FreeInternal, Memory::SignatureScan("48 85 C9 74 1D 4C 8B 05 ? ? ? ? 4D 85 C0"));
		Decl(FMemory::ReallocInternal, Memory::SignatureScan("E8 ? ? ? ? 48 89 03 48 8B 5C 24 ? 48 83 C4 20", true, 1));

#ifdef _DEBUG
		Debug::Log("FMemory::Free: ", FMemory::FreeInternal);
		Debug::Log("FMemory::Realloc: ", FMemory::ReallocInternal);
#endif
		
		Decl(UObject::StaticFindObjectInternal, Memory::SignatureScan("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8B EC 48 83 EC 60 80 3D ? ? ? ? ? 45 0F B6"));
		Decl(UObject::StaticLoadObjectInternal, Memory::SignatureScan("4C 89 4C 24 ? 48 89 54 24 ? 48 89 4C 24 ? 55 53 56 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 33 D2 48 8D 05 ? ? ? ? 38 15"));

#ifdef _DEBUG
		Debug::Log("UClass::StaticFindObject: ", UObject::StaticFindObjectInternal);
		Debug::Log("UClass::StaticLoadObject: ", UObject::StaticLoadObjectInternal);
#endif

		Decl(FName::ToStringInternal, Memory::SignatureScan("48 89 5C 24 ? 57 48 83 EC 40 83 79 04 00 48 8B DA 48 8B F9"));
		Decl(UObject::ProcessEventInternal, UObject::Object("/Script/CoreUObject.Default__Object")->VTable[0x40]);

#ifdef _DEBUG
		Debug::Log("FName::ToString: ", FName::ToStringInternal);
		Debug::Log("UObject::ProcessEvent: ", UObject::ProcessEventInternal);
#endif

		FString URL;
#ifdef _DEBUG
		URL = FString(L"Athena_Faceoff");
#else _RELEASE
		URL = FString(L"Athena_Terrain");
#endif
		PlayerController()->Function("SwitchLevel", URL);
		return true;
	}
};