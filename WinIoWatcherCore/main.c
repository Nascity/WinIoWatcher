#include <ntddk.h>
#include <wdm.h>

#include "driver.h"
#include "function.h"
#include "nas_assert.h"
#include "shared_mem.h"

VOID
DriverUnload(
	PDRIVER_OBJECT 	DriverObject
)
{
	UNICODE_STRING	SymlinkName = RTL_CONSTANT_STRING(SYMLINK_NAME);

	WINIOWATCHER_Cleanup();

	IoDeleteDevice(DriverObject->DeviceObject);
	IoDeleteSymbolicLink(&SymlinkName);

	UNREFERENCED_PARAMETER(DriverObject);
}

NTSTATUS
DriverEntry(
	PDRIVER_OBJECT	DriverObject,
	PUNICODE_STRING	RegistryPath
)
{
	UNICODE_STRING	DeviceName = RTL_CONSTANT_STRING(DEVICE_NAME);
	UNICODE_STRING	SymlinkName = RTL_CONSTANT_STRING(SYMLINK_NAME);
	NTSTATUS		Status = STATUS_SUCCESS;
	
	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrint("[WIW] We're in.\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "[WIW] Please work... For fuck's sake.\n");

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = WINIOWATCHER_DispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_READ] = WINIOWATCHER_DispatchRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = WINIOWATCHER_DispatchWrite;

	DbgPrint("[WIW] Major functions registered.\n");

	Status = IoCreateDevice(
		DriverObject,
		0,
		&DeviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&DriverObject->DeviceObject
	);
	NAS_ASSERT(Status);
	DbgPrint("[WIW] IoCreateDevice.\n");

	Status = WINIOWATCHER_InitSharedMemory();
	NAS_ASSERT(Status);
	DbgPrint("[WIW] InitSharedMemory.\n");

	Status = WINIOWATCHER_InitEvent();
	NAS_ASSERT(Status);
	DbgPrint("[WIW] InitEvent.\n");

	Status = IoCreateSymbolicLink(
		&SymlinkName,
		&DeviceName
	);
	if (!NT_SUCCESS(Status))
	{
		IoDeleteDevice(DriverObject->DeviceObject);
		return Status;
	}

	DbgPrint("[WIW] All set!\n");
	return STATUS_SUCCESS;
}