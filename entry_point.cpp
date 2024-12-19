#include "entry_point.hpp"

int32 __stdcall game_thread()
{
	if (!entry_point::begin())
	{
		FreeLibraryAndExitThread(GetModuleHandle(NULL), 0);
	}
}

int32 __stdcall DllMain(HMODULE module, int32 switch_code, int32)
{
	switch (switch_code)
	{
	case 1:
		DisableThreadLibraryCalls(module);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(&game_thread), 0, 0, 0);
	}

	return 1;
}

