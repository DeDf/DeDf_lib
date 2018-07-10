
#include <ntddk.h>

#define _In_
#define _Inout_opt_

#define ProcessNotify_IsSubsystemProcess 2

typedef enum _PSCREATEPROCESSNOTIFYTYPE { 
    PsCreateProcessNotifySubsystems  = 0
} PSCREATEPROCESSNOTIFYTYPE;

typedef NTSTATUS (*P_CREATE_PROC_NOTIFY2)(
                           _In_ PSCREATEPROCESSNOTIFYTYPE  NotifyType,
                           _In_ PVOID                     NotifyInformation,
                           _In_ BOOLEAN                   Remove
                           );

P_CREATE_PROC_NOTIFY2 PsSetCreateProcessNotifyRoutineEx2;

NTKERNELAPI HANDLE PsGetProcessInheritedFromUniqueProcessId( __in PEPROCESS Process );

void
ProcessNotifyRoutineEx2(
                        _In_        HANDLE                 ParentId,
                        _In_        HANDLE                 ProcessId,
                        _Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
                        )
{
    PEPROCESS pEProcess = ParentId;
    HANDLE Pid  = PsGetProcessId((PEPROCESS)ParentId);
    HANDLE PPid = PsGetProcessInheritedFromUniqueProcessId((PEPROCESS)ParentId);

    if (KeGetCurrentIrql() == PASSIVE_LEVEL)
    {
        if (CreateInfo)
        {
            //KdPrint(("[ProcessNotify] ~~~~~ FLAG : %x ~~~~~\n", CreateInfo->Flags));

            if (CreateInfo->Flags & ProcessNotify_IsSubsystemProcess)
            {
                KdPrint(("[ProcessNotify] CurPID : %4I64d, EProcess : %p, PPid/Pid : %4I64d/%4I64d - %wZ : %wZ\n",
                    PsGetCurrentProcessId(), pEProcess,
                    PPid, Pid,
                    CreateInfo->ImageFileName,
                    CreateInfo->CommandLine));
            }
        }
        else
        {
            KdPrint(("[ProcessNotify] CurPID : %4I64d, EProcess : %p, PPid/Pid : %4I64d/%4I64d - Exit\n",
                PsGetCurrentProcessId(), pEProcess,
                PPid, Pid));
        }
    }
}

VOID
DriverUnload(
             PDRIVER_OBJECT  driverObject
             )
{
    if (PsSetCreateProcessNotifyRoutineEx2)
        PsSetCreateProcessNotifyRoutineEx2(PsCreateProcessNotifySubsystems, ProcessNotifyRoutineEx2, TRUE);
}

NTSTATUS
DriverEntry(
            IN  PDRIVER_OBJECT      pDriverObject,
            IN  PUNICODE_STRING     pusRegistryPath
            )
{
    NTSTATUS status  = STATUS_UNSUCCESSFUL;
    PWCHAR pFuncName = L"PsSetCreateProcessNotifyRoutineEx2";
    UNICODE_STRING usFuncName;

    // pass IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY,
    //   or you should set IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY in OptionalHeader->DllCharacteristics;
    //      and recalculate PE checksum.
    *(PULONG)((PCHAR)pDriverObject->DriverSection + 13 * sizeof(void*)) |= 0x20;

    RtlInitUnicodeString(&usFuncName, pFuncName);
    PsSetCreateProcessNotifyRoutineEx2 = (P_CREATE_PROC_NOTIFY2)MmGetSystemRoutineAddress(&usFuncName);

    if (PsSetCreateProcessNotifyRoutineEx2)
        status = PsSetCreateProcessNotifyRoutineEx2(PsCreateProcessNotifySubsystems, ProcessNotifyRoutineEx2, FALSE);
    else
        KdPrint(("[ProcessNotify] Not find PsSetCreateProcessNotifyRoutineEx2 !\n"));

    pDriverObject->DriverUnload = DriverUnload;
    return status;
}