#include "entry_point.hpp"

int32 __stdcall GameThread()
{
	if (!EntryPoint::Begin())
	{
		FreeLibraryAndExitThread(GetModuleHandle(NULL), 0);
	}
}

int32 __stdcall DllMain(HMODULE Module, int32 SwitchCode, int32)
{
	switch (SwitchCode)
	{
	case 1:
		DisableThreadLibraryCalls(Module);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(&GameThread), 0, 0, 0);
	}

	return 1;
}

