#include <ntifs.h>

#include "driver.h"
#include "function.h"
#include "nas_assert.h"
#include "shared_mem.h"

VOID
DriverUnload(
	PDRIVER_OBJECT 	DriverObject
)
{
	PDEVICE_EXTENSION	Ext;
	UNICODE_STRING		SymlinkName = RTL_CONSTANT_STRING(SYMLINK_NAME);

	Ext = DriverObject->DeviceObject->DeviceExtension;

	if (Ext->LowerDeviceObject)
		IoDetachDevice(Ext->LowerDeviceObject);
	IoDeleteDevice(DriverObject->DeviceObject);
	IoDeleteSymbolicLink(&SymlinkName);

	WINIOWATCHER_Cleanup();

	UNREFERENCED_PARAMETER(DriverObject);
}

NTSTATUS
DriverEntry(
	PDRIVER_OBJECT	DriverObject,
	PUNICODE_STRING	RegistryPath
)
{
	PDEVICE_EXTENSION	Ext;

	UNICODE_STRING		DeviceName = RTL_CONSTANT_STRING(DEVICE_NAME);
	UNICODE_STRING		SymlinkName = RTL_CONSTANT_STRING(SYMLINK_NAME);
	UNICODE_STRING		TargetDeviceName = RTL_CONSTANT_STRING(TARGET_DEVICE_NAME);

	PDEVICE_OBJECT		DeviceObject;
	PDEVICE_OBJECT		LowerDeviceObject;
	PDEVICE_OBJECT		TopDeviceObject;
	PDEVICE_OBJECT		TargetDeviceObject;

	PFILE_OBJECT		FileObject;
	NTSTATUS			Status = STATUS_SUCCESS;
	
	UNREFERENCED_PARAMETER(RegistryPath);

	// Initializing major functions
	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = WINIOWATCHER_DispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_READ] = WINIOWATCHER_DispatchRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = WINIOWATCHER_DispatchWrite;

	// Retrieving disk's stack
	Status = IoGetDeviceObjectPointer(
		&TargetDeviceName,
		FILE_READ_DATA,
		&FileObject,
		&TopDeviceObject
	);
	NAS_ASSERT(Status);

	TargetDeviceObject = FileObject->DeviceObject;
	ObDereferenceObject(FileObject);

	// Creating device
	Status = IoCreateDevice(
		DriverObject,
		sizeof(DEVICE_EXTENSION),
		&DeviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&DeviceObject
	);
	NAS_ASSERT(Status);

	// Attaching to the stack
	Status = IoAttachDeviceToDeviceStackSafe(
		DeviceObject,
		TargetDeviceObject,
		&LowerDeviceObject
	);
	if (!NT_SUCCESS(Status))
	{
		IoDeleteDevice(DeviceObject);
		return STATUS_NO_SUCH_DEVICE;
	}

	ObDereferenceObject(TopDeviceObject);

	Ext = DeviceObject->DeviceExtension;
	Ext->LowerDeviceObject = LowerDeviceObject;

	DeviceObject->Flags |= DO_BUFFERED_IO;
	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	// Initializing
	Status = WINIOWATCHER_InitSharedMemory();
	NAS_ASSERT(Status);

	Status = WINIOWATCHER_InitEvent();
	NAS_ASSERT(Status);

	// Creating symlinks
	Status = IoCreateSymbolicLink(
		&SymlinkName,
		&DeviceName
	);
	if (!NT_SUCCESS(Status))
	{
		IoDeleteDevice(DeviceObject);
		return Status;
	}

	return STATUS_SUCCESS;
}