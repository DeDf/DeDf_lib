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

//专为WoW64准备; 
typedef struct _PEB32 { 
    UCHAR InheritedAddressSpace; 
    UCHAR ReadImageFileExecOptions; 
    UCHAR BeingDebugged; 
    UCHAR Spare; 
    ULONG Mutant; 
    ULONG ImageBaseAddress; 
    ULONG/*PPEB_LDR_DATA32*/ Ldr; 
} PEB32, *PPEB32; 


//通过进程PID来获取目标模块路径
NTSTATUS
GetModulesPathByPID (
    IN HANDLE ProcessId,
    IN WCHAR* ModuleName,
    OUT WCHAR* ModulesPath
    )
{
    NTSTATUS status; 
    KAPC_STATE KAPC = { 0 }; 
    PEPROCESS  pEProcess = NULL; //EPROCESS结构指针; 
    PPEB pPEB = NULL; //PEB结构指针; 
    UNICODE_STRING usFunctionName; //查找的函数名称; 
    PLDR_DATA_TABLE_ENTRY pLdrDataEntry = NULL; //LDR链表入口; 
    PLIST_ENTRY pListEntryStart = NULL; //链表头节点、尾节点; 
    PLIST_ENTRY pListEntryEnd = NULL;

    pfn_PsGetProcessPeb  PsGetProcessPeb = NULL;
    //获取进程的EPROCESS结构指针; 
    status = PsLookupProcessByProcessId (ProcessId, &pEProcess); 
    if (status)
        return status;

    //查找函数地址; 
    RtlInitUnicodeString (&usFunctionName, L"PsGetProcessPeb"); 
    PsGetProcessPeb = (pfn_PsGetProcessPeb) MmGetSystemRoutineAddress (&usFunctionName); 

    pPEB = PsGetProcessPeb (pEProcess); 
    KeStackAttachProcess (pEProcess, &KAPC); 

    pListEntryStart = pPEB->Ldr->InMemoryOrderModuleList.Flink; 
    pListEntryEnd   = pPEB->Ldr->InMemoryOrderModuleList.Flink;

    do {//输出DLL全路径; 
        pLdrDataEntry = (PLDR_DATA_TABLE_ENTRY)CONTAINING_RECORD (pListEntryStart, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks); 
        KdPrint (("module:%wZ\n", &pLdrDataEntry->BaseDllName)); 
        if (_wcsicmp (pLdrDataEntry->BaseDllName.Buffer, ModuleName) == 0)
        { 
            wcscpy (ModulesPath, pLdrDataEntry->FullDllName.Buffer); 
            goto L_Exit; 
        } 
        pListEntryStart = pListEntryStart->Flink; 
    } while (pListEntryStart != pListEntryEnd); 

#ifdef _AMD64_// 或wow64进程; 
    PPEB32 pPEB32 = NULL; //PEB结构指针; 
    PLDR_DATA_TABLE_ENTRY32 pLdrDataEntry32 = NULL; //LDR链表入口; 
    PLIST_ENTRY32 pListEntryStart32 = NULL; //链表头节点、尾节点; 
    PLIST_ENTRY32 pListEntryEnd32 = NULL; 
    //函数指针; 
    pfn_PsGetProcessWow64Process PsGetProcessWow64Process = NULL; 
    RtlInitUnicodeString (&usFunctionName, L"PsGetProcessWow64Process"); 
    PsGetProcessWow64Process = (pfn_PsGetProcessWow64Process) (SIZE_T)MmGetSystemRoutineAddress (&usFunctionName); 
    //获取PEB指针 
    pPEB32 = PsGetProcessWow64Process (pEProcess); 
    pListEntryStart32 = (PLIST_ENTRY32) (((PEB_LDR_DATA32*)pPEB32->Ldr)->InMemoryOrderModuleList.Flink); 
    pListEntryEnd32   = (PLIST_ENTRY32) (((PEB_LDR_DATA32*)pPEB32->Ldr)->InMemoryOrderModuleList.Flink); 
    do {//输出DLL全路径; 
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