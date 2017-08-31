#pragma once

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include "globals.h"
#include "handlers.h"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved);
DWORD WINAPI thread();
inline void CreateConsole();