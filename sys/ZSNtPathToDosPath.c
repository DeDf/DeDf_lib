
#include <ntifs.h>

NTSYSCALLAPI NTSTATUS NTAPI ZwQueryInformationProcess (
    __in HANDLE ProcessHandle,
    __in PROCESSINFOCLASS ProcessInformationClass,
    __out_bcount(ProcessInformationLength) PVOID ProcessInformation,
    __in ULONG ProcessInformationLength,
    __out_opt PULONG ReturnLength
    );

NTSTATUS
ZSQueryDosDeviceEx (
                    PWCHAR pwchDeviceName, 
                    PWCHAR TargetPath, 
                    PULONG pTargetPathLen
                    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES oa;
    HANDLE DirectoryHandle = NULL;
    UNICODE_STRING UnicodeDirectory;

    RtlInitUnicodeString(&UnicodeDirectory, L"\\??");
    InitializeObjectAttributes(
        &oa,
        &UnicodeDirectory,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = ZwOpenDirectoryObject(&DirectoryHandle, DIRECTORY_QUERY, &oa);
    if (NT_SUCCESS(Status))
    {
        UNICODE_STRING usDeviceName;
        HANDLE SymbolHandle = NULL;

        RtlInitUnicodeString(&usDeviceName, pwchDeviceName);
        InitializeObjectAttributes(
            &oa, 
            &usDeviceName, 
            OBJ_CASE_INSENSITIVE, 
            DirectoryHandle, 
            NULL);

        Status = ZwOpenSymbolicLinkObject(&SymbolHandle, GENERIC_READ, &oa);
        if (NT_SUCCESS(Status))
        {
            UNICODE_STRING UnicodeTarget;
            UnicodeTarget.Buffer = TargetPath;
            UnicodeTarget.Length = 0;
            UnicodeTarget.MaximumLength = (USHORT)*pTargetPathLen;

            Status = ZwQuerySymbolicLinkObject(SymbolHandle, &UnicodeTarget, pTargetPathLen);
            ZwClose(SymbolHandle);
        }

        ZwClose(DirectoryHandle);
    }

    return Status;
}

typedef struct _VOLUME_NAME {
    ULONG VolumeNameLen;  // In Byte
    WCHAR wchVolumeName[50];

} VOLUME_NAME, *P_VOLUME_NAME;

static VOLUME_NAME l_Volume[26];
static ULONG l_DriveMap;

NTSTATUS ZSNtPathToDosPath (
                            PWCHAR NtPath, 
                            ULONG NtPathLen, 
                            PWCHAR DosPath, 
                            PULONG pDosPathLen
                            )
{
    NTSTATUS Status = STATUS_SUCCESS;
    WCHAR wchVolume[3] = {L"A:"};
    USHORT i;

    if (!l_DriveMap)
    {
#ifdef _X86_
        PROCESS_DEVICEMAP_INFORMATION ProcessDeviceMapInfo;
#else
        PROCESS_DEVICEMAP_INFORMATION_EX ProcessDeviceMapInfo;
#endif

        RtlZeroMemory(&ProcessDeviceMapInfo, sizeof(ProcessDeviceMapInfo));
        Status = ZwQueryInformationProcess (
            NtCurrentProcess(), 
            ProcessDeviceMap, 
            &ProcessDeviceMapInfo, 
            sizeof(ProcessDeviceMapInfo), 
            NULL);

        if (NT_SUCCESS(Status))
        {
            wchVolume[0] = 'A';

            l_DriveMap = ProcessDeviceMapInfo.Query.DriveMap;

            for (i = 0; i < 26; i++)
            {
                if (l_DriveMap & (1<<i))
                {
                    ULONG RetLen = sizeof(l_Volume[i].wchVolumeName);
                    ZSQueryDosDeviceEx(wchVolume, l_Volume[i].wchVolumeName, &RetLen);
                    l_Volume[i].VolumeNameLen = RetLen - 2;

                    KdPrint(("ZSNtPathToDosPath() %ws [%d] %ws\n", wchVolume, RetLen - 2, l_Volume[i].wchVolumeName));
                }

                wchVolume[0]++;
            }
        }
    }

    return Status;
}

NTSTATUS
DriverEntry (
    PDRIVER_OBJECT DriverObject, 
    PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS Status;
    ULONG DrivesLen = 0;

    KdPrint(("[ZSecPort] DriverEntry(), NTDDI_VERSION : 0x%x\n", NTDDI_VERSION));

    ZSNtPathToDosPath(NULL, 0, NULL, NULL);

    return STATUS_UNSUCCESSFUL;
}
