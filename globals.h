#pragma once

#include "main.h"

typedef struct NetworkPacket
{
	unsigned char id, unk2, unk3, unk4;
	unsigned short size, unk6;
	unsigned char* data;
} NetworkPacket_t;

HANDLE hMain;
DWORD tmpProtect;

DWORD unh;

BYTE *SendPacket;

DWORD retAddr_AddNetworkQueue;
DWORD retAddr_RequestManorList;
DWORD retAddr_Render;

char* pckMsk;

BYTE *retAddr_handler_Render;

char ShowClientPck;
char ShowServerPck;
char doRestart;