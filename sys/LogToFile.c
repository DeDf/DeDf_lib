
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
        DbgPrint("写入源文件失败!!\n - %#X\r\n", Status);
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
        DbgPrint("写入源文件失败!!\n - %#X\r\n", Status);
    }
    return Status;
}

NTSTATUS InitLog()
{
    NTSTATUS status;

    OBJECT_ATTRIBUTES ObjectAttributes;  //OBJECT_ATTRIBUTES结构
    UNICODE_STRING usLogFilePath = RTL_CONSTANT_STRING(L"\\??\\c:\\log.txt");
    IO_STATUS_BLOCK IoStatusBlock;       //返回结果状态结构体

    //初始化OBJECT_ATTRIBUTES结构体
    InitializeObjectAttributes(
        &ObjectAttributes,
        &usLogFilePath,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    //以FILE_OVERWRITE_IF方式打开
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