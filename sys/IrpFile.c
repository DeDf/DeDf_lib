#include <ntddk.h>

#ifndef MAX_PATH
#define MAX_PATH          260
#endif

NTSTATUS
ObOpenObjectByPointer (
    PVOID Object,
    ULONG HandleAttributes,
    PACCESS_STATE PassedAccessState,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE ObjectType,
    KPROCESSOR_MODE AccessMode,
    PHANDLE Handle
    );

NTSTATUS
ObCreateObject (
    KPROCESSOR_MODE ProbeMode,
    POBJECT_TYPE ObjectType,
    POBJECT_ATTRIBUTES ObjectAttributes,
    KPROCESSOR_MODE OwnershipMode,
    PVOID ParseContext,
    ULONG ObjectBodySize,
    ULONG PagedPoolCharge,
    ULONG NonPagedPoolCharge,
    PVOID *Object
    );

NTSTATUS
SeCreateAccessState (
    PACCESS_STATE AccessState,
    PVOID AuxData,
    ACCESS_MASK DesiredAccess,
    PGENERIC_MAPPING GenericMapping
    );

typedef struct _AUX_ACCESS_DATA {
    PPRIVILEGE_SET PrivilegesUsed;
    GENERIC_MAPPING GenericMapping;
    ACCESS_MASK AccessesToAudit;
    ACCESS_MASK MaximumAuditMask;
    ULONG Unknown[256];
} AUX_ACCESS_DATA, *PAUX_ACCESS_DATA;

//获取设备对象
NTSTATUS GetDriveObject(PUNICODE_STRING pDriveName, PDEVICE_OBJECT *DeviceObject, PDEVICE_OBJECT *ReadDevice);

//IRP删除文件
NTSTATUS IrpDeleteFileForce(PFILE_OBJECT pFileObject);

//IRP删除文件
NTSTATUS IrpDeleteFileEx(PFILE_OBJECT pFileObject);

//IRP删除文件
NTSTATUS IrpDeleteFile(PFILE_OBJECT pFileObject);

//删除文件
NTSTATUS ZwDeleteFile(PFILE_OBJECT pFileObject);

//复制文件
NTSTATUS CopyFileEx(PUNICODE_STRING lpExistingFileName, PUNICODE_STRING lpNewFileName);

//IRP设置文件
NTSTATUS irpSetFileAttributes(PFILE_OBJECT pFileObject, PIO_STATUS_BLOCK  pIoStatusBlock,PVOID FileInformation,ULONG Length,FILE_INFORMATION_CLASS  FileInformationClass,BOOLEAN  ReplaceIfExists);

//IRP打开文件
NTSTATUS IrpCreateFile(PUNICODE_STRING pFilePath, ACCESS_MASK DesiredAccess, PIO_STATUS_BLOCK pIoStatusBlock, PFILE_OBJECT *pFileObject);

//IRP读取文件
NTSTATUS IrpReadFile(PFILE_OBJECT pFileObject, PLARGE_INTEGER pByteOffset, ULONG Length, PVOID pBuffer, PIO_STATUS_BLOCK pIoStatusBlock);

//IRP写文件
NTSTATUS IrpFileWrite(PFILE_OBJECT pFileObject, PLARGE_INTEGER ByteOffset, ULONG Length, PVOID Buffer, PIO_STATUS_BLOCK pIoStatusBlock);

//IRP关闭文件
NTSTATUS IrpClose(PFILE_OBJECT  pFileObject);


VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock = {0};
    PFILE_OBJECT pFileObject = NULL;

    UNICODE_STRING usFilePathName;
    RtlInitUnicodeString(&usFilePathName, L"\\??\\c:\\notepad.exe_");

    __debugbreak();

    //打开文件
    status = IrpCreateFile(&usFilePathName, STANDARD_RIGHTS_READ, &IoStatusBlock, &pFileObject);
    if (NT_SUCCESS(status))
    {
        HANDLE FileHandle = NULL;
        IO_STATUS_BLOCK IoStatus = { 0 };
        LARGE_INTEGER Offset = { 0 };
        char buf[100];

        KdPrint(("pFileObject : %p\n", pFileObject));
        // 获取文件句柄
        status = ObOpenObjectByPointer(pFileObject, 0, NULL, STANDARD_RIGHTS_READ, *IoFileObjectType, KernelMode, &FileHandle);
        if (!NT_SUCCESS(status))
            return status;
        
        status = ZwReadFile(FileHandle, NULL, NULL, NULL, &IoStatus, buf, sizeof(buf), &Offset, NULL);
        ZwClose(FileHandle);

        //删除文件
        //status = ZwDeleteFile(pFileObject);
        //status = IrpDeleteFile(pFileObject);
        //status = IrpDeleteFileForce(pFileObject);
        //KdPrint(("OK\n"));

        //关闭文件
        status = IrpClose(pFileObject);
    }

    DriverObject->DriverUnload = DriverUnload;
    return status;
}

// 完成历程
NTSTATUS IoCompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
    *Irp->UserIosb = Irp->IoStatus;

    if (Irp->UserEvent)
        KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, 0);

    if (Irp->MdlAddress)
    {
        IoFreeMdl(Irp->MdlAddress);
        Irp->MdlAddress = NULL;
    }

    IoFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

//IRP写文件
NTSTATUS IrpFileWrite(PFILE_OBJECT pFileObject, PLARGE_INTEGER ByteOffset, ULONG Length, PVOID Buffer, PIO_STATUS_BLOCK pIoStatusBlock)
{
    //定义变量
    NTSTATUS status;
    KEVENT event;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject;

    //参数效验
    if (pIoStatusBlock == NULL || pFileObject == NULL || Length <= 0 || Buffer == NULL)return STATUS_INVALID_PARAMETER;

    //参数效验
    if (ByteOffset == NULL)
    {
        if (!(pFileObject->Flags & FO_SYNCHRONOUS_IO))
            return STATUS_INVALID_PARAMETER;
        ByteOffset = &pFileObject->CurrentByteOffset;
    }

    //获取原始设备对象
    deviceObject = IoGetRelatedDeviceObject(pFileObject);
    if (deviceObject == NULL || deviceObject->StackSize <= 0)return STATUS_UNSUCCESSFUL;


    //分配IRP
    irp = IoAllocateIrp(deviceObject->StackSize, FALSE);
    if (irp == NULL)return STATUS_INSUFFICIENT_RESOURCES;

    //分配MDL缓冲区
    irp->MdlAddress = IoAllocateMdl(Buffer, Length, FALSE, FALSE, NULL);
    if (irp->MdlAddress == NULL)
    {
        IoFreeIrp(irp);
        return STATUS_INSUFFICIENT_RESOURCES;;
    }
    //更新MDL
    MmBuildMdlForNonPagedPool(irp->MdlAddress);

    //设置变量
    irp->Flags = IRP_WRITE_OPERATION;            //IRP写操作
    irp->RequestorMode = KernelMode;
    irp->UserIosb = pIoStatusBlock;
    irp->UserEvent = &event;
    irp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
    irp->Tail.Overlay.OriginalFileObject = pFileObject;

    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->MajorFunction = IRP_MJ_WRITE;
    irpSp->MinorFunction = IRP_MN_NORMAL;
    irpSp->DeviceObject = deviceObject;
    irpSp->FileObject = pFileObject;
    irpSp->Parameters.Write.Length = Length;
    irpSp->Parameters.Write.ByteOffset = *ByteOffset;

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);
    IoSetCompletionRoutine(irp, IoCompletionRoutine, NULL, TRUE, TRUE, TRUE);
    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING)
        status = KeWaitForSingleObject(&event, Executive, KernelMode, TRUE, NULL);
    status = pIoStatusBlock->Status;

    KdPrint(("写入文件长度%d,  status : %x\n", pIoStatusBlock->Information, pIoStatusBlock->Status));
    return status;
}


//IRP删除文件
NTSTATUS IrpDeleteFileEx(PFILE_OBJECT pFileObject)
{
    //定义变量
    NTSTATUS                status;
    PIRP                    pIrp;
    PIO_STACK_LOCATION      irpSp;
    PDEVICE_OBJECT          DeviceObject;
    IO_STATUS_BLOCK         IoStatusBlock;
    KEVENT                  SycEvent;
    PSECTION_OBJECT_POINTERS        pSectionObjectPointer;
    FILE_DISPOSITION_INFORMATION    FileInformationDelete;
    static FILE_BASIC_INFORMATION   FileInformationAttribute;


    //参数效验
    if (pFileObject == NULL)return STATUS_INVALID_PARAMETER;

    //设置文件属性
    memset(&FileInformationAttribute, 0, sizeof(FILE_BASIC_INFORMATION));
    FileInformationAttribute.FileAttributes = FILE_ATTRIBUTE_NORMAL;
    status = irpSetFileAttributes(pFileObject, &IoStatusBlock, &FileInformationAttribute, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation, TRUE);
    if (!NT_SUCCESS(status))return status;


    //获取原始设备对象
    DeviceObject = IoGetRelatedDeviceObject(pFileObject);
    if (DeviceObject == NULL || DeviceObject->StackSize <= 0)return STATUS_UNSUCCESSFUL;

    // 创建IRP
    pIrp = IoAllocateIrp(DeviceObject->StackSize, TRUE);
    if (pIrp == NULL)
    {
        //ObDereferenceObject(pFileObject);
        return STATUS_UNSUCCESSFUL;
    }


    //初始化同步事件对象
    KeInitializeEvent(&SycEvent, SynchronizationEvent, FALSE);

    //设置变量
    FileInformationDelete.DeleteFile = TRUE;

    //初始化IRP
    pIrp->AssociatedIrp.SystemBuffer = &FileInformationDelete;
    pIrp->UserEvent = &SycEvent;
    pIrp->UserIosb = &IoStatusBlock;
    pIrp->Tail.Overlay.OriginalFileObject = pFileObject;
    pIrp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
    pIrp->RequestorMode = KernelMode;

    // 设置IRP堆栈
    irpSp = IoGetNextIrpStackLocation(pIrp);
    irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    irpSp->DeviceObject = DeviceObject;
    irpSp->FileObject = pFileObject;
    irpSp->Parameters.SetFile.Length = sizeof(FILE_DISPOSITION_INFORMATION);
    irpSp->Parameters.SetFile.FileInformationClass = FileDispositionInformation;
    irpSp->Parameters.SetFile.FileObject = pFileObject;

    // 设置完成例程
    IoSetCompletionRoutine(pIrp, IoCompletionRoutine, NULL, TRUE, TRUE, TRUE);

    //如果没有这3行，就无法删除正在运行的文件
    pSectionObjectPointer = pFileObject->SectionObjectPointer;
    pSectionObjectPointer->ImageSectionObject = 0;
    pSectionObjectPointer->DataSectionObject = 0;

    // 派发IRP
    status=IoCallDriver(DeviceObject, pIrp);
    if (status == STATUS_PENDING)
        KeWaitForSingleObject(&SycEvent, Executive, KernelMode, TRUE, NULL);
    status = IoStatusBlock.Status;
    // 递减引用计数
    //ObDereferenceObject(pFileObject);

    return status;

}

//IRP删除文件
NTSTATUS IrpDeleteFileForce(PFILE_OBJECT pFileObject)
{

    //定义变量
    NTSTATUS                status;
    PIRP                    pIrp;
    PIO_STACK_LOCATION      irpSp;
    PDEVICE_OBJECT          DeviceObject;
    IO_STATUS_BLOCK         IoStatusBlock;
    KEVENT                  SycEvent;
    PVOID pImageSectionObject = NULL;
    PVOID pDataSectionObject = NULL;
    PVOID pSharedCacheMap = NULL;
    PSECTION_OBJECT_POINTERS        pSectionObjectPointer;
    FILE_DISPOSITION_INFORMATION    FileInformationDelete;
    static FILE_BASIC_INFORMATION   FileInformationAttribute;


    //参数效验
    if (pFileObject == NULL)return STATUS_INVALID_PARAMETER;

    //设置文件属性
    memset(&FileInformationAttribute, 0, sizeof(FILE_BASIC_INFORMATION));
    FileInformationAttribute.FileAttributes = FILE_ATTRIBUTE_NORMAL;
    status = irpSetFileAttributes(pFileObject, &IoStatusBlock, &FileInformationAttribute, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation, TRUE);
    if (!NT_SUCCESS(status))return status;


    //获取原始设备对象
    DeviceObject = IoGetRelatedDeviceObject(pFileObject);
    if (DeviceObject == NULL || DeviceObject->StackSize <= 0)return STATUS_UNSUCCESSFUL;

    // 创建IRP
    pIrp = IoAllocateIrp(DeviceObject->StackSize, TRUE);
    if (pIrp == NULL)return STATUS_UNSUCCESSFUL;

    //初始化同步事件对象
    KeInitializeEvent(&SycEvent, SynchronizationEvent, FALSE);

    //设置变量
    FileInformationDelete.DeleteFile = TRUE;

    //初始化IRP
    pIrp->AssociatedIrp.SystemBuffer = &FileInformationDelete;
    pIrp->UserEvent = &SycEvent;
    pIrp->UserIosb = &IoStatusBlock;
    pIrp->Tail.Overlay.OriginalFileObject = pFileObject;
    pIrp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
    pIrp->RequestorMode = KernelMode;

    // 设置IRP堆栈
    irpSp = IoGetNextIrpStackLocation(pIrp);
    irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    irpSp->DeviceObject = DeviceObject;
    irpSp->FileObject = pFileObject;
    irpSp->Parameters.SetFile.Length = sizeof(FILE_DISPOSITION_INFORMATION);
    irpSp->Parameters.SetFile.FileInformationClass = FileDispositionInformation;
    irpSp->Parameters.SetFile.FileObject = pFileObject;

    // 设置完成例程
    IoSetCompletionRoutine(pIrp, IoCompletionRoutine, NULL, TRUE, TRUE, TRUE);


    //设置变量
    if (pFileObject->SectionObjectPointer)
    {
        //一个空值表明，可执行图像目前不在内存中
        //一个空值表示数据流当前不在内存中
        //空值的数据流是
        pImageSectionObject = pFileObject->SectionObjectPointer->ImageSectionObject;
        pDataSectionObject = pFileObject->SectionObjectPointer->DataSectionObject;
        pSharedCacheMap = pFileObject->SectionObjectPointer->SharedCacheMap;

        pFileObject->SectionObjectPointer->ImageSectionObject = NULL;
        pFileObject->SectionObjectPointer->DataSectionObject = NULL;
        pFileObject->SectionObjectPointer->SharedCacheMap = NULL;
    }


    // 派发IRP
    status = IoCallDriver(DeviceObject, pIrp);
    if (status == STATUS_PENDING)
        KeWaitForSingleObject(&SycEvent, Executive, KernelMode, TRUE, NULL);
    status = IoStatusBlock.Status;

    //还原变量
    if (pFileObject->SectionObjectPointer)
    {
        pFileObject->SectionObjectPointer->ImageSectionObject = pImageSectionObject;
        pFileObject->SectionObjectPointer->DataSectionObject = pDataSectionObject;
        pFileObject->SectionObjectPointer->SharedCacheMap = pSharedCacheMap;
    }

    return status;
}

//IRP删除文件
NTSTATUS IrpDeleteFile(PFILE_OBJECT pFileObject)
{
    //定义变量
    NTSTATUS           ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT DeviceObject;
    PIRP                        pIrp;
    KEVENT                  SycEvent;
    FILE_DISPOSITION_INFORMATION    FileInformation;
    IO_STATUS_BLOCK                 ioStatus;
    PIO_STACK_LOCATION           irpSp;

    //参数效验
    if (pFileObject == NULL)return STATUS_INVALID_PARAMETER;

    //获取原始设备对象
    DeviceObject = IoGetRelatedDeviceObject(pFileObject);
    if (DeviceObject == NULL || DeviceObject->StackSize <= 0)return STATUS_UNSUCCESSFUL;

    //分配IRP
    pIrp = IoAllocateIrp(DeviceObject->StackSize, TRUE);
    if (pIrp == NULL)
    {
        //ObDereferenceObject(pFileObject);
        return ntStatus;
    }

    // 初始化同步事件对象
    KeInitializeEvent(&SycEvent, SynchronizationEvent, FALSE);

    //指示操作系统文件是否应在关闭该文件时删除该文件。将此成员设置为在关闭时删除该文件
    FileInformation.DeleteFile = TRUE;

    // 初始化IRP
    pIrp->AssociatedIrp.SystemBuffer = &FileInformation;
    pIrp->UserEvent = &SycEvent;
    pIrp->UserIosb = &ioStatus;
    pIrp->Tail.Overlay.OriginalFileObject = pFileObject;
    pIrp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
    pIrp->RequestorMode = KernelMode;

    // 设置IRP堆栈
    irpSp = IoGetNextIrpStackLocation(pIrp);
    irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    irpSp->DeviceObject = DeviceObject;
    irpSp->FileObject = pFileObject;
    irpSp->Parameters.SetFile.Length = sizeof(FILE_DISPOSITION_INFORMATION);
    irpSp->Parameters.SetFile.FileInformationClass = FileDispositionInformation;
    irpSp->Parameters.SetFile.FileObject = pFileObject;

    IoSetCompletionRoutine(pIrp, IoCompletionRoutine, NULL, TRUE, TRUE, TRUE);

    // 派发IRP
    ntStatus = IoCallDriver(DeviceObject, pIrp);
    if (ntStatus == STATUS_PENDING)
        KeWaitForSingleObject(&SycEvent, Executive, KernelMode, TRUE, 0);
    ntStatus = ioStatus.Status;

    return ntStatus;
}

//删除文件
NTSTATUS ZwDeleteFile(PFILE_OBJECT pFileObject)
{
    //定义变量
    PDEVICE_OBJECT DeviceObject;
    HANDLE FileHandle;
    NTSTATUS ntStatus;
    IO_STATUS_BLOCK                 ioStatus;
    FILE_DISPOSITION_INFORMATION    FileInformation;

    //参数效验
    if (pFileObject == NULL)
        return STATUS_INVALID_PARAMETER;

    // 获取文件句柄
    ntStatus = ObOpenObjectByPointer(pFileObject,0,NULL,DELETE,*IoFileObjectType,KernelMode,&FileHandle);
    if (!NT_SUCCESS(ntStatus))
        return ntStatus;
    ObDereferenceObject(pFileObject);

    FileInformation.DeleteFile = TRUE;
    ntStatus = ZwSetInformationFile(FileHandle, &ioStatus, &FileInformation, sizeof(FILE_DISPOSITION_INFORMATION), FileDispositionInformation);
    return ntStatus;
}

//IRP设置文件
NTSTATUS irpSetFileAttributes(PFILE_OBJECT pFileObject, PIO_STATUS_BLOCK  pIoStatusBlock, PVOID pFileInformation, ULONG FileInformationLength, FILE_INFORMATION_CLASS  FileInformationClass, BOOLEAN  ReplaceIfExists)
{
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT          DeviceObject;
    PIRP                    Irp;
    KEVENT                  SycEvent;
    PIO_STACK_LOCATION      irpSp;

    //参数效验
    if (pFileObject == NULL || pIoStatusBlock==NULL || pFileInformation ==NULL|| FileInformationLength <=0)return STATUS_INVALID_PARAMETER;

    //获取原始设备对象
    DeviceObject = IoGetRelatedDeviceObject(pFileObject);
    if (DeviceObject == NULL || DeviceObject->StackSize <= 0)return STATUS_UNSUCCESSFUL;

    // 创建IRP
    Irp = IoAllocateIrp(DeviceObject->StackSize, TRUE);
    if (Irp == NULL)
    {
        //ObDereferenceObject(pFileObject);
        return STATUS_UNSUCCESSFUL;
    }

    // 初始化同步事件对象
    KeInitializeEvent(&SycEvent, SynchronizationEvent, FALSE);


    // 初始化IRP
    Irp->AssociatedIrp.SystemBuffer = pFileInformation;                  //设置属性
    Irp->UserEvent = &SycEvent;
    Irp->UserIosb = pIoStatusBlock;
    Irp->Tail.Overlay.OriginalFileObject = pFileObject;
    Irp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
    Irp->RequestorMode = KernelMode;


    // 设置IRP堆栈信息
    irpSp = IoGetNextIrpStackLocation(Irp);
    irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    irpSp->DeviceObject = DeviceObject;
    irpSp->FileObject = pFileObject;
    irpSp->Parameters.SetFile.ReplaceIfExists = ReplaceIfExists;         //是否替换
    irpSp->Parameters.SetFile.Length = FileInformationLength;                //长度
    irpSp->Parameters.SetFile.FileInformationClass = FileInformationClass;   //类型
    irpSp->Parameters.SetFile.FileObject = pFileObject;


    IoSetCompletionRoutine(Irp, IoCompletionRoutine, NULL, TRUE, TRUE, TRUE);
    ntStatus=IoCallDriver(DeviceObject, Irp);
    if (ntStatus == STATUS_PENDING)
        KeWaitForSingleObject(&SycEvent, Executive, KernelMode, TRUE, NULL);
    ntStatus = pIoStatusBlock->Status;
    //ObDereferenceObject(pFileObject);
    return ntStatus;

}

// 获取设备对象
NTSTATUS GetDriveObject(PUNICODE_STRING pusDriveName, PDEVICE_OBJECT *ppDeviceObject, PDEVICE_OBJECT *ppReadDevice)
{
    NTSTATUS status;
    HANDLE hDevice = NULL;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatus;

    //参数效验
    if (pusDriveName == NULL|| ppDeviceObject == NULL || ppReadDevice == NULL)
        return STATUS_INVALID_PARAMETER;
    else
        *ppDeviceObject = *ppReadDevice = NULL;

    //  "\\??\\C:\\"
    InitializeObjectAttributes(&objectAttributes, pusDriveName, OBJ_CASE_INSENSITIVE, NULL, NULL);
    status = IoCreateFile(&hDevice,
        SYNCHRONIZE | FILE_ANY_ACCESS,
        &objectAttributes,
        &ioStatus,
        NULL,
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
        NULL,
        0,
        CreateFileTypeNone,
        NULL,
        IO_NO_PARAMETER_CHECKING);

    if (!status)
    {
        PFILE_OBJECT pFileObject = NULL;

        status = ObReferenceObjectByHandle(hDevice, FILE_READ_DATA, *IoFileObjectType, KernelMode, &pFileObject, NULL);
        if (!status)
        {
            if (pFileObject && pFileObject->Vpb)
            {
                *ppDeviceObject = pFileObject->Vpb->DeviceObject;
                *ppReadDevice   = pFileObject->Vpb->RealDevice;
            }

            ObDereferenceObject(pFileObject);
        }

        ZwClose(hDevice);
    }

    return status;
}

//IRP打开文件

#define  SYMBOLICLINKLEN   7     // L"\\??\\c:\\"

NTSTATUS IrpCreateFile(PUNICODE_STRING pusFilePathName, ACCESS_MASK DesiredAccess, PIO_STATUS_BLOCK pIoStatusBlock, PFILE_OBJECT *ppFileObject)
{
    NTSTATUS status;
    PFILE_OBJECT  pFileObject;
    //
    PIRP pIrp;
    PIO_STACK_LOCATION IrpSp;
    KEVENT kEvent;
    //
    IO_SECURITY_CONTEXT SecurityContext;
    ACCESS_STATE AccessState;
    AUX_ACCESS_DATA AuxData;
    //
    PDEVICE_OBJECT pDeviceObject = NULL;
    PDEVICE_OBJECT pReadDevice = NULL;

    //参数效验
    if (pusFilePathName==NULL || pIoStatusBlock==NULL || ppFileObject==NULL ||
        pusFilePathName->Length < SYMBOLICLINKLEN*2 ||
        pusFilePathName->Length > MAX_PATH * 2 - 2)
        return STATUS_INVALID_PARAMETER;
    else
        *ppFileObject = NULL;

    {
        UNICODE_STRING usDriveName;
        wchar_t wchDriveName[SYMBOLICLINKLEN + 1];        

        //  "\\??\\c:\\"， "X:\\"的"\\"很重要，vpb才能有值
        RtlCopyMemory(wchDriveName, pusFilePathName->Buffer, SYMBOLICLINKLEN*2);
        wchDriveName[SYMBOLICLINKLEN] = 0;
        RtlInitUnicodeString(&usDriveName, wchDriveName);

        //获取设备对象
        status = GetDriveObject(&usDriveName, &pDeviceObject, &pReadDevice);
        if (status)
            return status;
    }

    //参数效验
    if (pDeviceObject == NULL || pReadDevice == NULL || pDeviceObject->StackSize <= 0)
        return STATUS_UNSUCCESSFUL;

    //----------------------------------------------------------
    {
        OBJECT_ATTRIBUTES ObjectAttributes;
        ULONG FileObject_FileNameLen = pusFilePathName->Length - (SYMBOLICLINKLEN*2 - 2);
        WCHAR *pwchFileNameBuf =
            ExAllocatePoolWithTag(NonPagedPool, FileObject_FileNameLen+2, 'DeDf');

        if (pwchFileNameBuf == NULL)
            return STATUS_INSUFFICIENT_RESOURCES;

        RtlCopyMemory(pwchFileNameBuf, &pusFilePathName->Buffer[SYMBOLICLINKLEN - 1], FileObject_FileNameLen);
        pwchFileNameBuf[FileObject_FileNameLen/2] = 0;

        InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, NULL);
        status = ObCreateObject(KernelMode, *IoFileObjectType, &ObjectAttributes, KernelMode, NULL, sizeof(FILE_OBJECT), 0, 0, &pFileObject);
        if (!NT_SUCCESS(status))
        {
            ExFreePool(pwchFileNameBuf);
            return status;
        }

        RtlZeroMemory(pFileObject, sizeof(FILE_OBJECT));
        pFileObject->Type = IO_TYPE_FILE;
        pFileObject->Size = sizeof(FILE_OBJECT);
        pFileObject->DeviceObject = pReadDevice;
        pFileObject->Flags = FO_SYNCHRONOUS_IO;
        RtlInitUnicodeString(&pFileObject->FileName, pwchFileNameBuf);       //地址不能是局部变量地址
        KeInitializeEvent(&pFileObject->Lock, SynchronizationEvent, FALSE);
        KeInitializeEvent(&pFileObject->Event, NotificationEvent, FALSE);
    }
    //----------------------------------------------------------

    status = SeCreateAccessState(&AccessState, &AuxData, FILE_ALL_ACCESS, IoGetFileObjectGenericMapping());
    if (!NT_SUCCESS(status))
    {
        ObDereferenceObject(pFileObject);
        return status;
    }

    SecurityContext.SecurityQos = NULL;
    SecurityContext.AccessState = &AccessState;
    SecurityContext.DesiredAccess = DesiredAccess;
    SecurityContext.FullCreateOptions = 0;

    //----------------------------------------------------------

    pIrp = IoAllocateIrp(pDeviceObject->StackSize, FALSE);
    if (!pIrp)
    {
        ObDereferenceObject(pFileObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);
    pIrp->MdlAddress = NULL;
    pIrp->AssociatedIrp.SystemBuffer = NULL;
    pIrp->Flags = IRP_CREATE_OPERATION | IRP_SYNCHRONOUS_API;
    pIrp->RequestorMode = KernelMode;
    pIrp->UserIosb = pIoStatusBlock;
    pIrp->UserEvent = &kEvent;
    pIrp->PendingReturned = FALSE;
    pIrp->Cancel = FALSE;
    pIrp->CancelRoutine = NULL;
    pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
    pIrp->Tail.Overlay.AuxiliaryBuffer = NULL;
    pIrp->Tail.Overlay.OriginalFileObject = pFileObject;

    IrpSp = IoGetNextIrpStackLocation(pIrp);
    IrpSp->MajorFunction = IRP_MJ_CREATE;
    IrpSp->DeviceObject = pDeviceObject;
    IrpSp->FileObject = pFileObject;
    IrpSp->Parameters.Create.SecurityContext = &SecurityContext;
    IrpSp->Parameters.Create.Options = (FILE_OPEN_IF << 24) | 0;
    IrpSp->Parameters.Create.FileAttributes = (USHORT)FILE_ATTRIBUTE_NORMAL;
    IrpSp->Parameters.Create.ShareAccess = 0; //(USHORT)FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    IrpSp->Parameters.Create.EaLength = 0;

    IoSetCompletionRoutine(pIrp, IoCompletionRoutine, 0, TRUE, TRUE, TRUE);
    status = IoCallDriver(pDeviceObject, pIrp);
    if (status == STATUS_PENDING)
        KeWaitForSingleObject(&kEvent, Executive, KernelMode, TRUE, 0);
    status = pIoStatusBlock->Status;

    if (!NT_SUCCESS(status))
    {
        pFileObject->DeviceObject = NULL;
        ObDereferenceObject(pFileObject);
    }
    else
    {
        InterlockedIncrement(&pFileObject->DeviceObject->ReferenceCount);

        if (pFileObject->Vpb)
            InterlockedIncrement(&pFileObject->Vpb->ReferenceCount);
        *ppFileObject = pFileObject;
    }

    return status;
}

//IRP读取文件
NTSTATUS
IrpReadFile (
    PFILE_OBJECT pFileObject,
    PLARGE_INTEGER pByteOffset,
    ULONG Length,
    PVOID pBuffer,
    PIO_STATUS_BLOCK pIoStatusBlock
    )
{
    NTSTATUS status;
    PDEVICE_OBJECT DeviceObject;
    PIRP pirp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;

    //参数效验
    if (pFileObject == NULL|| pByteOffset ==NULL || Length==0 || pBuffer ==NULL|| pIoStatusBlock==NULL)
        return STATUS_INVALID_PARAMETER;

    //获取原始设备对象
    DeviceObject = IoGetRelatedDeviceObject(pFileObject);
    if (DeviceObject == NULL || DeviceObject->StackSize <= 0)
        return STATUS_UNSUCCESSFUL;

    //分配IRP
    pirp = IoAllocateIrp(DeviceObject->StackSize, FALSE);
    if (pirp == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    //分配MDL
    pirp->MdlAddress = IoAllocateMdl(pBuffer, Length, FALSE, FALSE, NULL);
    if (pirp->MdlAddress == NULL)
    {
        IoFreeIrp(pirp);
        return STATUS_INSUFFICIENT_RESOURCES;;
    }
    //更新MDL
    MmBuildMdlForNonPagedPool(pirp->MdlAddress);

    pirp->Flags = IRP_READ_OPERATION;
    pirp->RequestorMode = KernelMode;
    pirp->UserIosb = pIoStatusBlock;
    pirp->UserEvent = &event;
    pirp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
    pirp->Tail.Overlay.OriginalFileObject = pFileObject;

    irpSp = IoGetNextIrpStackLocation(pirp);
    irpSp->MajorFunction = IRP_MJ_READ;
    irpSp->MinorFunction = IRP_MN_NORMAL;
    irpSp->DeviceObject = DeviceObject;
    irpSp->FileObject = pFileObject;
    irpSp->Parameters.Read.Length = Length;
    irpSp->Parameters.Read.ByteOffset = *pByteOffset;

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);
    IoSetCompletionRoutine(pirp, IoCompletionRoutine, NULL, TRUE, TRUE, TRUE);
    status = IoCallDriver(DeviceObject, pirp);
    if (status == STATUS_PENDING)
        status = KeWaitForSingleObject(&event, Executive, KernelMode, TRUE, NULL);
    status = pIoStatusBlock->Status;
    return status;
}

//复制文件
NTSTATUS CopyFileEx(PUNICODE_STRING lpExistingFileName, PUNICODE_STRING lpNewFileName)  // done!
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    char *buf;

    //参数效验
    if (lpExistingFileName == NULL || lpExistingFileName->Length <= 0 ||
        lpNewFileName == NULL || lpNewFileName->Length <= 0)
        return STATUS_INVALID_PARAMETER;

    buf = ExAllocatePoolWithTag(PagedPool, PAGE_SIZE, 'DeDf');
    if (buf)
    {
        HANDLE hFileSource = NULL;
        HANDLE hFileDest = NULL;
        OBJECT_ATTRIBUTES ObAttr = { 0 };
        IO_STATUS_BLOCK IoStatus = { 0 };

        InitializeObjectAttributes(&ObAttr, lpExistingFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
        status = ZwOpenFile(&hFileSource, FILE_READ_DATA, &ObAttr, &IoStatus, FILE_SHARE_READ, 
            FILE_NON_DIRECTORY_FILE | FILE_RANDOM_ACCESS | FILE_SYNCHRONOUS_IO_NONALERT);
        if (!status)
        {
            InitializeObjectAttributes(&ObAttr, lpNewFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
            status = ZwCreateFile(&hFileDest, GENERIC_WRITE, &ObAttr, &IoStatus, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OVERWRITE_IF,
                FILE_NON_DIRECTORY_FILE | FILE_RANDOM_ACCESS | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
            if (!status)
            {
                ULONG len = 0;
                LARGE_INTEGER Offset = { 0 };

                do
                {
                    status = ZwReadFile(hFileSource, NULL, NULL, NULL, &IoStatus, buf, PAGE_SIZE, &Offset, NULL);
                    if (NT_ERROR(status))
                    {
                        if (STATUS_END_OF_FILE == status)
                        {
                            status = STATUS_SUCCESS;
                        }
                        break;
                    }
                    len = (ULONG)IoStatus.Information;
                    status = ZwWriteFile(hFileDest, NULL, NULL, NULL, &IoStatus, buf, len, &Offset, NULL);
                    if (len != IoStatus.Information)
                    {
                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }
                    Offset.QuadPart += IoStatus.Information;

                } while (NT_SUCCESS(status));

                ZwClose(hFileDest);
            }

            ZwClose(hFileSource);
        }

        ExFreePool(buf);
    }

    return status;
}

//IRP关闭文件
NTSTATUS IrpClose(PFILE_OBJECT pFileObject)
{
    NTSTATUS status;
    PDEVICE_OBJECT DeviceObject;
    IO_STATUS_BLOCK  IoStatusBlock;
    PIRP pIrp;
    PIO_STACK_LOCATION IrpSp;
    KEVENT kEvent;

    //参数效验
    if (pFileObject == NULL)
        return STATUS_INVALID_PARAMETER;

    //获取原始设备对象
    DeviceObject = IoGetRelatedDeviceObject(pFileObject);
    if (DeviceObject == NULL || DeviceObject->StackSize <= 0)
        return STATUS_UNSUCCESSFUL;

    //分配IRP
    pIrp = IoAllocateIrp(DeviceObject->StackSize, FALSE);
    if (pIrp == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);
    pIrp->UserEvent = &kEvent;
    pIrp->UserIosb = &IoStatusBlock;
    pIrp->RequestorMode = KernelMode;
    pIrp->Flags = IRP_CLOSE_OPERATION | IRP_SYNCHRONOUS_API;
    pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
    pIrp->Tail.Overlay.OriginalFileObject = pFileObject;

    IrpSp = IoGetNextIrpStackLocation(pIrp);
    IrpSp->MajorFunction = IRP_MJ_CLEANUP;
    IrpSp->FileObject = pFileObject;

    status = IoCallDriver(DeviceObject, pIrp);
    if (status == STATUS_PENDING)
        KeWaitForSingleObject(&kEvent, Executive, KernelMode, FALSE, NULL);

    status = IoStatusBlock.Status;
    if (!NT_SUCCESS(status))
    {
        IoFreeIrp(pIrp);
        return status;
    }

    KeClearEvent(&kEvent);
    IoReuseIrp(pIrp, STATUS_SUCCESS);

    pIrp->UserEvent = &kEvent;
    pIrp->UserIosb = &IoStatusBlock;
    pIrp->Tail.Overlay.OriginalFileObject = pFileObject;
    pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
    pIrp->AssociatedIrp.SystemBuffer = (PVOID)NULL;
    pIrp->Flags = IRP_CLOSE_OPERATION | IRP_SYNCHRONOUS_API;

    IrpSp = IoGetNextIrpStackLocation(pIrp);
    IrpSp->MajorFunction = IRP_MJ_CLOSE;
    IrpSp->FileObject = pFileObject;

    if (pFileObject->Vpb && !(pFileObject->Flags & FO_DIRECT_DEVICE_OPEN))
    {
        InterlockedDecrement(&pFileObject->Vpb->ReferenceCount);
        pFileObject->Flags |= FO_FILE_OPEN_CANCELLED;
    }

    status = IoCallDriver(DeviceObject, pIrp);
    if (status == STATUS_PENDING)
        KeWaitForSingleObject(&kEvent, Executive, KernelMode, FALSE, NULL);

    IoFreeIrp(pIrp);

    status = IoStatusBlock.Status;
    return status;
}