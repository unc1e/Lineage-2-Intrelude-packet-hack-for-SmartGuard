#pragma once

#include "main.h"

void /*__declspec(naked)*/ wrapper_AddNetworkQueue();
void __stdcall handler_AddNetworkQueue(DWORD *stack);

long __stdcall wrapper_SendPacket(PEXCEPTION_POINTERS exInfo);
void __stdcall handler_SendPacket(DWORD *stack);

void /*__declspec(naked)*/ wrapper_SendPacketEnd();
void __stdcall handler_SendPacketEnd();

void /*__declspec(naked)*/ wrapper_Render();
void __stdcall handler_Render();
void /*__declspec(naked)*/ fixupStack_Render();

unsigned int fixupSize;