
#include <ntddk.h>

#ifndef MAX_PATH
#define MAX_PATH          260
#endif

#define DRIVER_NAME         L"IOCTL"
#define NT_DEVICE_NAME      L"\\Device\\"     ## DRIVER_NAME
#define DOS_DEVICE_NAME     L"\\DosDevices\\" ## DRIVER_NAME

#define SIOCTL_TYPE 40000

#define IOCTL_OPENFILE_BUFFERED \
    CTL_CODE( SIOCTL_TYPE, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS )

#if DBG
#define dPrint(_x_) \
    DbgPrint("[IOCTL] ");\
    DbgPrint _x_;
#else
#define dPrint(_x_)
#endif

DRIVER_INITIALIZE DriverEntry;
DRIVER_DISPATCH MJ_CreateClose;
DRIVER_DISPATCH MJ_DeviceControl;
DRIVER_UNLOAD DriverUnload;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, MJ_CreateClose)
#pragma alloc_text( PAGE, MJ_DeviceControl)
#pragma alloc_text( PAGE, DriverUnload)
#endif // ALLOC_PRAGMA

//--------------------------------------------------------

RTL_OSVERSIONINFOW g_OSVerInfo;

NTSTATUS (NTAPI *g_IoSetFileObjectIgnoreSharing) ( PFILE_OBJECT FileObject );

//--------------------------------------------------------

NTSTATUS GetOSVersion()
{
    NTSTATUS status;
    UNICODE_STRING  usFuncName;

    g_OSVerInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    status = RtlGetVersion(&g_OSVerInfo);
    if (!status)
    {
        ULONG dwMajorVersion = g_OSVerInfo.dwMajorVersion;
        ULONG dwMinorVersion = g_OSVerInfo.dwMinorVersion;
        ULONG dwBuildNumber  = g_OSVerInfo.dwBuildNumber;

        dPrint(("winver %d.%d, build : %d\n", dwMajorVersion, dwMinorVersion, dwBuildNumber));
    }

    RtlInitUnicodeString( &usFuncName, L"IoSetFileObjectIgnoreSharing" );
    g_IoSetFileObjectIgnoreSharing = MmGetSystemRoutineAddress(&usFuncName);

    dPrint(("IoSetFileObjectIgnoreSharing : %p\n", g_IoSetFileObjectIgnoreSharing));

    return status;
}

// Get Drive DeviceObject, etc: C:, D:, E:.
NTSTATUS GetDriveObject(PUNICODE_STRING pusDriveName, PDEVICE_OBJECT *ppDeviceObject, PDEVICE_OBJECT *ppReadDevice)
{
    NTSTATUS status;
    HANDLE hDevice = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK ioStatus;

    // parameter check
    if (pusDriveName == NULL|| ppDeviceObject == NULL || ppReadDevice == NULL)
        return STATUS_INVALID_PARAMETER;
    else
        *ppDeviceObject = *ppReadDevice = NULL;

    //  L"\\??\\C:\\"
    InitializeObjectAttributes(&ObjectAttributes, pusDriveName, OBJ_CASE_INSENSITIVE, NULL, NULL);
    status = IoCreateFile(&hDevice,
        SYNCHRONIZE | FILE_ANY_ACCESS,
        &ObjectAttributes,
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

//---------------------------------------------------------------

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

NTSTATUS
MJ_CreateClose (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return STATUS_SUCCESS;
}

NTSTATUS
MJ_DeviceControl (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation( Irp );
    //
    ULONG  inBufLength  = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
    ULONG  outBufLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    PCHAR  inBuf, outBuf;

    switch ( pIrpSp->Parameters.DeviceIoControl.IoControlCode )
    {
    case IOCTL_OPENFILE_BUFFERED:
        //
        // In this method the I/O manager allocates a buffer large enough to
        // to accommodate larger of the user input buffer and output buffer,
        // assigns the address to Irp->AssociatedIrp.SystemBuffer, and
        // copies the content of the user input buffer into this SystemBuffer
        //
        {
             inBuf = Irp->AssociatedIrp.SystemBuffer;
            outBuf = Irp->AssociatedIrp.SystemBuffer;

            //if (outBufLength >= sizeof(HANDLE))
            {
                //Irp->IoStatus.Information = sizeof(HANDLE);
            }
        }
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        dPrint(("MJ_DeviceControl() : Unknown IOCTL %x\n",
            pIrpSp->Parameters.DeviceIoControl.IoControlCode));
        break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}

VOID
DriverUnload (
    PDRIVER_OBJECT DriverObject
    )
{
    UNICODE_STRING usSymbolLink;
    RtlInitUnicodeString( &usSymbolLink, DOS_DEVICE_NAME );
    IoDeleteSymbolicLink( &usSymbolLink );

    if ( DriverObject->DeviceObject )
    {
        IoDeleteDevice( DriverObject->DeviceObject );
    }

    dPrint(("DriverUnload <==\n\n"));
}

NTSTATUS
DriverEntry (
    PDRIVER_OBJECT DriverObject, 
    PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS        status;
    UNICODE_STRING  usDeviceName;
    UNICODE_STRING  usSymbolLink;
    PDEVICE_OBJECT  pDeviceObject = NULL;

    RtlInitUnicodeString( &usDeviceName, NT_DEVICE_NAME );
    status = IoCreateDevice (
        DriverObject,                // Our Driver Object
        0,                           // We don't use a device extension
        &usDeviceName,               // Device name
        FILE_DEVICE_UNKNOWN,         // Device type
        FILE_DEVICE_SECURE_OPEN,     // Device characteristics
        FALSE,                       // Not an exclusive device
        &pDeviceObject );            // Returned ptr to Device Object

    if (status)
    {
        dPrint(("IoCreateDevice() Failed!  status : %x\n", status));
        goto L_Exit;
    }

    RtlInitUnicodeString( &usSymbolLink, DOS_DEVICE_NAME );
    status = IoCreateSymbolicLink(&usSymbolLink, &usDeviceName );
    if (status)
    {
        dPrint(("IoCreateSymbolicLink() Failed!  status : %x\n", status));
        IoDeleteDevice( pDeviceObject );
        goto L_Exit;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = MJ_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = MJ_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MJ_DeviceControl;
    DriverObject->DriverUnload = DriverUnload;

    dPrint(("DriverEntry ==>\n"));
L_Exit:
    return status;
}
