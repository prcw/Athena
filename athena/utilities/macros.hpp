#pragma once

#include "../../resources/memory.hpp"
#include "../../resources/minhook/minhook.h"

#define Decl(Target, Source) Target = decltype(Target)(Source);

#define Detour(Target, Detour, Original) MH_Initialize(); \
 MH_CreateHook((LPVOID*)(Target), Detour, (LPVOID*)(&Original)); \
 MH_EnableHook((LPVOID*)(Target));