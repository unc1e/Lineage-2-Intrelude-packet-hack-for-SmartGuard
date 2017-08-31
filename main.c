#include "main.h"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&thread, 0, 0, 0);
		return 1;
	}

	return 0;
}

DWORD WINAPI thread()
{
	CreateConsole();

	HMODULE hEngine = GetModuleHandleA("engine.dll");
	hMain = OpenProcess(PROCESS_VM_OPERATION, FALSE, GetCurrentProcessId());
	DWORD trmpAddr;

	unh = 0;

	ShowClientPck = 0; //do not change until you set exception handler
	ShowServerPck = 0; 
	doRestart = 0;


	//////////////// Get ptr to unused space inside Engine.dll
	//
	BYTE *Remove = (BYTE *)GetProcAddress(hEngine, "?Remove@?$TArray@E@@QAEXHH@Z");
	Remove += *(DWORD *)(Remove + 1) + 5;
	pckMsk = (char *)Remove + 0x74; //max 44 chars with zero (43 without). You can find more.
	VirtualProtectEx(hMain, pckMsk, 1, PAGE_EXECUTE_READWRITE, &tmpProtect); //


	/////////////// Set trampoline inside Engine.dll for fuxup stack after call SendPacket.
	//
	BYTE* RequestRestart = (BYTE *)GetProcAddress(hEngine, "?RequestRestart@UNetworkHandler@@UAEXAAVL2ParamStack@@@Z");
	RequestRestart += *(DWORD *)(RequestRestart + 1) + 5;
	retAddr_handler_Render = RequestRestart + 0x2b;

	trmpAddr = (DWORD)fixupStack_Render - ((DWORD)retAddr_handler_Render + 5);

	VirtualProtectEx(hMain, retAddr_handler_Render, 1, PAGE_READWRITE, &tmpProtect);
	*retAddr_handler_Render = 0xE9;
	*(DWORD *)(retAddr_handler_Render + 1) = trmpAddr;
	VirtualProtectEx(hMain, retAddr_handler_Render, 1, PAGE_EXECUTE, &tmpProtect);


	////////////////////// Hook Render for calling SendPacket and spaw VirtualProtectEx
	//
	BYTE *Render = (BYTE *)GetProcAddress(hEngine, "?Render@FPlayerSceneNode@@UAEXPAVFRenderInterface@@@Z");
	Render += *(DWORD *)(Render + 1) + 5;

	retAddr_Render = (DWORD)Render + 5;
	trmpAddr = (DWORD)wrapper_Render - ((DWORD)Render + 5);

	VirtualProtectEx(hMain, Render, 1, PAGE_EXECUTE_READWRITE, &tmpProtect);
	*Render = 0xE9;
	*(DWORD *)(Render + 1) = trmpAddr;
	VirtualProtectEx(hMain, Render, 1, PAGE_EXECUTE, &tmpProtect);


	//////////////// Hook AddNetworkQueue for sniffing s->c packets and getting unh
	//
	BYTE *AddNetworkQueue = (BYTE *)GetProcAddress(hEngine, "?AddNetworkQueue@UNetworkHandler@@UAEHPAUNetworkPacket@@@Z");
	AddNetworkQueue += *(DWORD *)(AddNetworkQueue + 1) + 5;

	retAddr_AddNetworkQueue = (DWORD)AddNetworkQueue + 0x19;
	trmpAddr = (DWORD)wrapper_AddNetworkQueue - ((DWORD)AddNetworkQueue + 0x14 + 5);

	VirtualProtectEx(hMain, AddNetworkQueue + 0x14, 1, PAGE_EXECUTE_READWRITE, &tmpProtect);
	*(AddNetworkQueue + 0x14) = 0xE9;
	*(DWORD *)(AddNetworkQueue + +0x14 + 1) = trmpAddr;
	VirtualProtectEx(hMain, AddNetworkQueue + 0x14, 1, PAGE_EXECUTE, &tmpProtect);

	while (!unh) Sleep(100);


	//////////////// Hook SendPacket for sniffing c->s packets
	//
	SendPacket = (BYTE *)*(DWORD *)(**(DWORD **)(unh + 0x48) + 0x68); //you can find without unh (by offset or signature)
	SendPacket += *(DWORD *)(SendPacket + 1) + 5;

	trmpAddr = (DWORD)wrapper_SendPacketEnd - ((DWORD)SendPacket + 0xb5 + 5); //first ret inside SendPacket

	VirtualProtectEx(hMain, SendPacket + 0xb5, 1, PAGE_EXECUTE_READWRITE, &tmpProtect);
	*(SendPacket + 0xb5) = 0xE9;
	*(DWORD *)(SendPacket + 0xb5 + 1) = trmpAddr;

	trmpAddr = (DWORD)wrapper_SendPacketEnd - ((DWORD)SendPacket + 0xc5 + 5); //second ret inside SendPacket

	*(SendPacket + 0xc5) = 0xE9;
	*(DWORD *)(SendPacket + 0xc5 + 1) = trmpAddr;
	VirtualProtectEx(hMain, SendPacket + 0xc5, 1, PAGE_EXECUTE, &tmpProtect);

	SendPacket += *(DWORD *)(SendPacket + 1) + 5; //works only
	SendPacket += *(DWORD *)(SendPacket + 1) + 5; //on SmartGuard
	SendPacket += *(DWORD *)(SendPacket + 1) + 5; //End of August 2017, (c) unc1e
                                                  //For tophope.ru and habrahabr.ru
	AddVectoredExceptionHandler(1, wrapper_SendPacket); //set hook
	ShowClientPck = 1;


	///////////////// Small GUI (^~^)
	//
	for(;;)
	{
		if (GetKeyState('1') & 0x8000)
		{
			ShowServerPck = !ShowServerPck;
			Sleep(1000);
		}
		else if (GetKeyState('2') & 0x8000)
		{
			ShowClientPck = !ShowClientPck;
			Sleep(1000);
		}
		else if (GetKeyState('3') & 0x8000)
		{
			doRestart = !doRestart;
			Sleep(1000);
		}
	}

	return 0;
}

inline void CreateConsole()
{
	int   conHandle;
	DWORD stdHandle;
	FILE* f;
	FILE* cout;

	AllocConsole();
	stdHandle = (DWORD)GetStdHandle(STD_OUTPUT_HANDLE);
	conHandle = _open_osfhandle(stdHandle, _O_TEXT);
	f = _fdopen(conHandle, "w");

	*stdout = *f;
	setvbuf(stdout, NULL, _IONBF, 0);
	freopen_s(&cout, "conout$", "w", stdout);
}