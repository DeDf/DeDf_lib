#include <ntddk.h>

NTSYSCALLAPI LPSTR NTAPI PsGetProcessImageFileName(PEPROCESS Process);
typedef PPEB (NTAPI * pfn_PsGetProcessPeb) (PEPROCESS pEProcess); 
typedef PPEB32 (NTAPI * pfn_PsGetProcessWow64Process) (PEPROCESS Process);

typedef struct _PEB { 
    UCHAR InheritedAddressSpace; 
    UCHAR ReadImageFileExecOptions; 
    UCHAR BeingDebugged; 
    UCHAR Spare; 
    PVOID Mutant; 
    PVOID ImageBaseAddress; 
    PPEB_LDR_DATA Ldr; 
    PRTL_USER_PROCESS_PARAMETERS  ProcessParameters; 
    PVOID SubSystemData; 
} PEB, *PPEB;

//רΪWoW64׼��; 
typedef struct _PEB32 { 
    UCHAR InheritedAddressSpace; 
    UCHAR ReadImageFileExecOptions; 
    UCHAR BeingDebugged; 
    UCHAR Spare; 
    ULONG Mutant; 
    ULONG ImageBaseAddress; 
    ULONG/*PPEB_LDR_DATA32*/ Ldr; 
} PEB32, *PPEB32; 


//ͨ������PID����ȡĿ��ģ��·��
NTSTATUS
GetModulesPathByPID (
    IN HANDLE ProcessId,
    IN WCHAR* ModuleName,
    OUT WCHAR* ModulesPath
    )
{
    NTSTATUS status; 
    KAPC_STATE KAPC = { 0 }; 
    PEPROCESS  pEProcess = NULL; //EPROCESS�ṹָ��; 
    PPEB pPEB = NULL; //PEB�ṹָ��; 
    UNICODE_STRING usFunctionName; //���ҵĺ�������; 
    PLDR_DATA_TABLE_ENTRY pLdrDataEntry = NULL; //LDR�������; 
    PLIST_ENTRY pListEntryStart = NULL; //����ͷ�ڵ㡢β�ڵ�; 
    PLIST_ENTRY pListEntryEnd = NULL;

    pfn_PsGetProcessPeb  PsGetProcessPeb = NULL;
    //��ȡ���̵�EPROCESS�ṹָ��; 
    status = PsLookupProcessByProcessId (ProcessId, &pEProcess); 
    if (status)
        return status;

    //���Һ�����ַ; 
    RtlInitUnicodeString (&usFunctionName, L"PsGetProcessPeb"); 
    PsGetProcessPeb = (pfn_PsGetProcessPeb) MmGetSystemRoutineAddress (&usFunctionName); 

    pPEB = PsGetProcessPeb (pEProcess); 
    KeStackAttachProcess (pEProcess, &KAPC); 

    pListEntryStart = pPEB->Ldr->InMemoryOrderModuleList.Flink; 
    pListEntryEnd   = pPEB->Ldr->InMemoryOrderModuleList.Flink;

    do {//���DLLȫ·��; 
        pLdrDataEntry = (PLDR_DATA_TABLE_ENTRY)CONTAINING_RECORD (pListEntryStart, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks); 
        KdPrint (("module:%wZ\n", &pLdrDataEntry->BaseDllName)); 
        if (_wcsicmp (pLdrDataEntry->BaseDllName.Buffer, ModuleName) == 0)
        { 
            wcscpy (ModulesPath, pLdrDataEntry->FullDllName.Buffer); 
            goto L_Exit; 
        } 
        pListEntryStart = pListEntryStart->Flink; 
    } while (pListEntryStart != pListEntryEnd); 

#ifdef _AMD64_// ��wow64����; 
    PPEB32 pPEB32 = NULL; //PEB�ṹָ��; 
    PLDR_DATA_TABLE_ENTRY32 pLdrDataEntry32 = NULL; //LDR�������; 
    PLIST_ENTRY32 pListEntryStart32 = NULL; //����ͷ�ڵ㡢β�ڵ�; 
    PLIST_ENTRY32 pListEntryEnd32 = NULL; 
    //����ָ��; 
    pfn_PsGetProcessWow64Process PsGetProcessWow64Process = NULL; 
    RtlInitUnicodeString (&usFunctionName, L"PsGetProcessWow64Process"); 
    PsGetProcessWow64Process = (pfn_PsGetProcessWow64Process) (SIZE_T)MmGetSystemRoutineAddress (&usFunctionName); 
    //��ȡPEBָ�� 
    pPEB32 = PsGetProcessWow64Process (pEProcess); 
    pListEntryStart32 = (PLIST_ENTRY32) (((PEB_LDR_DATA32*)pPEB32->Ldr)->InMemoryOrderModuleList.Flink); 
    pListEntryEnd32   = (PLIST_ENTRY32) (((PEB_LDR_DATA32*)pPEB32->Ldr)->InMemoryOrderModuleList.Flink); 
    do {//���DLLȫ·��; 
        pLdrDataEntry32 = (PLDR_DATA_TABLE_ENTRY32)CONTAINING_RECORD (pListEntryStart32, LDR_DATA_TABLE_ENTRY32, InMemoryOrderLinks); 
        KdPrint (("wow64:%ws\n", pLdrDataEntry32->BaseDllName.Buffer)); 
        if (_wcsicmp ((WCHAR*)pLdrDataEntry32->BaseDllName.Buffer, ModuleName) == 0) { 
            wcscpy (ModulesPath, (WCHAR*)pLdrDataEntry32->FullDllName.Buffer); 
            goto end; 
        } 
        pListEntryStart32 = (PLIST_ENTRY32)pListEntryStart32->Flink; 
    } while (pListEntryStart32 != pListEntryEnd32); 
#endif 

L_Exit: 
    KeUnstackDetachProcess (&KAPC);
    ObDereferenceObject (pEProcess); 
    return STATUS_SUCCESS; 
}