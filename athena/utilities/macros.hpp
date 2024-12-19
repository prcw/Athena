#pragma once

#include "../../resources/memory.hpp"
#include "../../resources/minhook/minhook.h"

#define Decl(target, source) target = decltype(target)(source);

#define Detour(target, detour, original) MH_Initialize(); \
 MH_CreateHook((LPVOID*)(target), detour, (LPVOID*)(&original)); \
 MH_EnableHook((LPVOID*)(target));