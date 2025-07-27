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

	Ext = DriverObject->DeviceObject->DeviceExtension;

	if (Ext->LowerDeviceObject)
		IoDetachDevice(Ext->LowerDeviceObject);
	IoDeleteDevice(DriverObject->DeviceObject);

	WINIOWATCHER_Cleanup();
}

NTSTATUS
AddDevice(
	PDRIVER_OBJECT	DriverObject,
	PDEVICE_OBJECT	PhysicalDeviceObject
)
{
	UNICODE_STRING	DeviceName = RTL_CONSTANT_STRING(DEVICE_NAME);
	PDEVICE_OBJECT	DeviceObject;
	PDEVICE_OBJECT	LowerDeviceObject;
	NTSTATUS		Status;

	Status = IoCreateDevice(
		DriverObject,
		sizeof(DEVICE_EXTENSION),
		&DeviceName,
		FILE_DEVICE_DISK,
		0,
		FALSE,
		&DeviceObject
	);
	NAS_ASSERT(Status);

	LowerDeviceObject = IoAttachDeviceToDeviceStack(
		DeviceObject,
		PhysicalDeviceObject
	);

	((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->LowerDeviceObject = LowerDeviceObject;

	// Initializing
	Status = WINIOWATCHER_InitSharedMemory();
	NAS_ASSERT(Status);

	Status = WINIOWATCHER_InitEvent();
	NAS_ASSERT(Status);

	DeviceObject->Flags |= DO_POWER_PAGABLE;
	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}

NTSTATUS
DriverEntry(
	PDRIVER_OBJECT	DriverObject,
	PUNICODE_STRING	RegistryPath
)
{
	INT			i;
	
	UNREFERENCED_PARAMETER(RegistryPath);

	RtlZeroMemory(DriverObject->MajorFunction, sizeof(DriverObject->MajorFunction));

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->DriverExtension->AddDevice = AddDevice;
	DriverObject->MajorFunction[IRP_MJ_SCSI] = WINIOWATCHER_DispatchSCSI;

	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		if (DriverObject->MajorFunction[i] == NULL)
			DriverObject->MajorFunction[i] = WINIOWATCHER_DispatchDefault;
	}

	return STATUS_SUCCESS;
}