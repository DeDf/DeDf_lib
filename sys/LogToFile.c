
#include <ntddk.h>

HANDLE g_hLogFile;

//         char tmp[500];
//         sprintf_s(tmp, sizeof(tmp), "[DrvInjDll] ImageNotifyRoutine Pid : %d, Base: 0x%p, Size: 0x%I64x, %wZ\r\n", 
//             (ULONG)ProcessId, ImageInfo->ImageBase, ImageInfo->ImageSize, FullImageName);
//         WriteLog(tmp);

NTSTATUS WriteLog(UCHAR *p)
{
    NTSTATUS Status;
    ULONG Length = strlen(p);
    IO_STATUS_BLOCK IoStatusBlock;
    Status = ZwWriteFile(
        g_hLogFile,
        NULL,
        NULL,
        NULL,
        &IoStatusBlock,
        p,
        Length,
        NULL,
        NULL);
    if (!NT_SUCCESS(Status))
    {
        DbgPrint("д��Դ�ļ�ʧ��!!\n - %#X\r\n", Status);
    }
    return Status;
}

NTSTATUS WriteData(UCHAR *p, ULONG len)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    Status = ZwWriteFile(
        g_hLogFile,
        NULL,
        NULL,
        NULL,
        &IoStatusBlock,
        p,
        len,
        NULL,
        NULL);
    if (!NT_SUCCESS(Status))
    {
        DbgPrint("д��Դ�ļ�ʧ��!!\n - %#X\r\n", Status);
    }
    return Status;
}

NTSTATUS InitLog()
{
    NTSTATUS status;

    OBJECT_ATTRIBUTES ObjectAttributes;  //OBJECT_ATTRIBUTES�ṹ
    UNICODE_STRING usLogFilePath = RTL_CONSTANT_STRING(L"\\??\\c:\\log.txt");
    IO_STATUS_BLOCK IoStatusBlock;       //���ؽ��״̬�ṹ��

    //��ʼ��OBJECT_ATTRIBUTES�ṹ��
    InitializeObjectAttributes(
        &ObjectAttributes,
        &usLogFilePath,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    //��FILE_OVERWRITE_IF��ʽ��
    status = ZwCreateFile(
        &g_hLogFile,
        GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OVERWRITE_IF,
        FILE_NON_DIRECTORY_FILE |
        FILE_RANDOM_ACCESS |
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0);

    if (!NT_SUCCESS(status))
    {
        KdPrint(("Open Log file fail !! - %#x\r\n", status));
        return status;
    }
}

VOID UninitLog()
{
    if (g_hLogFile)
        ZwClose(g_hLogFile);
}