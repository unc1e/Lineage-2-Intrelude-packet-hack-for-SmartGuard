#include "handlers.h"

#include "main.h"

void __declspec(naked) wrapper_AddNetworkQueue()
{
	__asm
	{
		pushad
		pushfd

		sub [unh], 0
		jnz L1
		mov [unh], ecx
		L1:
		lea eax, [esp + 44] //32 (pushad) + 4 (pushfd) + 4 (push 4) + 4 (ret addr)
		push eax
		call [handler_AddNetworkQueue]

		popfd
		popad

		add esi, 0x3c //see disasm
		push 0x1
		jmp [retAddr_AddNetworkQueue]
	}
}

void __stdcall handler_AddNetworkQueue(DWORD *stack)
{
	NetworkPacket_t *pck = (NetworkPacket_t *)*stack;

	if (ShowServerPck)
	{
		printf("s -> c | %02hhX ", pck->id);

		for (int i = 0; i < pck->size; i++)
			printf("%02hhX ", pck->data[i]);

		printf("\n");
	}
}

/////////////////////////////////
/////////////////////////////////

long __stdcall wrapper_SendPacket(PEXCEPTION_POINTERS exInfo)
{
	if (exInfo->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION)
	{
		VirtualProtectEx(hMain, SendPacket, 1, PAGE_EXECUTE, &tmpProtect);

		if (exInfo->ContextRecord->Eip == (DWORD)SendPacket)
		{
			handler_SendPacket((DWORD *)exInfo->ContextRecord->Esp + 3); //4 (ret addr)  + 4 (ret addr) + 4 (1 arg)
		}

		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void __stdcall handler_SendPacket(DWORD *stack)
{
	unsigned int bytesLen;

	char *mask = (char *)*stack;
	stack++;

	printf("c -> s | %s ", mask);

	for (int i = 0; mask[i]; i++)
	{
		switch (mask[i])
		{
		case 'c':
			printf("%02X ", *stack);
			stack++;
			break;
		case 'h':
			printf_s("%04X ", *stack);
			stack++;
			break;
		case 'd':
			printf_s("%08X ", *stack);
			stack++;
			break;
		case 'b':
			printf_s("( ");
			bytesLen = *stack;
			stack++;
			for (unsigned int j = 0; j < bytesLen; j++)
				printf("%02hhX ", *(BYTE *)(*stack + j));
			stack++;
			printf(") ");
			break;
		case 'S':
			printf("%ls ", (wchar_t *)*stack);
			stack++;
			break;
		case 'Q':
			printf_s("%08X%08X ", *(stack + 1), *stack);
			stack += 2;
			break;
		}
	}

	printf("\n");
}

/////////////////////////////////
/////////////////////////////////

void __declspec(naked) wrapper_SendPacketEnd()
{
	__asm
	{
		pushad
		pushfd
		call [handler_SendPacketEnd]
		popfd
		popad

		add esp, 0x2000 //see disasm
		ret
	}
}

void __stdcall handler_SendPacketEnd()
{
	if (ShowClientPck)
		VirtualProtectEx(hMain, SendPacket, 1, PAGE_EXECUTE | PAGE_GUARD, &tmpProtect);
}

/////////////////////////////////
/////////////////////////////////

void __declspec(naked) wrapper_Render()
{
	__asm
	{
		pushad
		pushfd

		call [handler_Render]

		popfd
		popad

		push ebp //see disasm
		lea ebp, [esp - 0x74]
		jmp [retAddr_Render]
	}
}

void __stdcall handler_Render()
{
	if (ShowClientPck)
		VirtualProtectEx(hMain, SendPacket, 1, PAGE_EXECUTE | PAGE_GUARD, &tmpProtect);

	if (doRestart)
	{
		doRestart = !doRestart;

		strcpy_s(pckMsk, 44, "c"); //char pckMsk[44]
		fixupSize = 12; //4 (push eax) + 4 (push [pckMsk]) + 4 (push 0x46)

		__asm
		{
			mov ecx, [unh]
			mov eax, [ecx + 0x48]
			mov ecx, [eax]
			mov edx, [ecx + 0x68] //SendPacket
			push 0x46
			push [pckMsk]
			push eax
			push [retAddr_handler_Render] //trampoline to fixupStack_Render
			jmp edx
		}
	}
}

void __declspec(naked) fixupStack_Render()
{
	__asm
	{
		add esp, [fixupSize] //SendPacket has cdecl convention

		mov esp, ebp //prolog of
		pop ebp ///////handler_Render
		ret //ret to the end of wrapper_Render
	}
}