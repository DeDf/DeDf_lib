#include "DrvInjDll_Arm.h"

// get the ImageBase of a system dll
ULONG_PTR GetSystemModuleHandle(PCSTR ModuleName)
{
	ULONG_PTR ImageBase = 0;
	ULONG size = 0;
    PULONG_PTR buf;

	ZwQuerySystemInformation(11 /*SystemModuleInformation*/, &buf, 4, &size);
	if (size)
	{
		buf = ExAllocatePool(PagedPool, size * 2);
		if (buf)
		{
            NTSTATUS status = ZwQuerySystemInformation(11 /*SystemModuleInformation*/, buf, size * 2, NULL);
			if (!status)
			{
				// we got information about the loaded system modules
				PSYSTEM_MODULE_INFORMATION modules = (PVOID)(buf + 1);
				ULONG i;

				for (i = 0; i < *((ULONG*)buf); i++)
				{
					if (!_stricmp(ModuleName, &modules[i].ImageName[modules[i].ModuleNameOffset]))
					{
						// found it!
						ImageBase = (ULONG_PTR)modules[i].ImageBase;
						break;
					}
				}
			}
			ExFreePool(buf);
		}
	}

	return ImageBase;
}

USHORT GetPEMachine(HANDLE ProcessId)
{
    PEPROCESS pEProcess = NULL;
    KAPC_STATE ApcState;
    USHORT PEMachine = 0;

	__try
	{
		PsLookupProcessByProcessId(ProcessId, &pEProcess);
		if (pEProcess)
		{
			PUCHAR peb = NULL;
			KeStackAttachProcess(pEProcess, &ApcState);
			peb = *(PUCHAR*)((PUCHAR)pEProcess + 0x3A0);
			if (peb)
			{
				PUCHAR ImageBase = *(PUCHAR*)(peb + 0x10);

				PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)ImageBase;
				if (pDosHdr->e_magic == IMAGE_DOS_SIGNATURE)
				{
					ImageBase += pDosHdr->e_lfanew;
					if (*(PULONG)ImageBase == IMAGE_NT_SIGNATURE)
					{
						ImageBase += 4;
						PEMachine = *(PUSHORT)ImageBase;
					}
				}
			}

			KeUnstackDetachProcess(&ApcState);
			ObDereferenceObject(pEProcess);
		}
	}
	__finally {}

    return PEMachine;
}