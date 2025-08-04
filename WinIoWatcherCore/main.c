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
	UNICODE_STRING	SymbolicLinkName = RTL_CONSTANT_STRING(SYMBOLIC_LINK_NAME);
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

	Status = IoCreateSymbolicLink(
		&SymbolicLinkName,
		&DeviceName
	);
	NAS_ASSERT(Status);

	LowerDeviceObject = IoAttachDeviceToDeviceStack(
		DeviceObject,
		PhysicalDeviceObject
	);
	if (!LowerDeviceObject)
		return STATUS_NO_SUCH_DEVICE;

	((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->LowerDeviceObject = LowerDeviceObject;

	DeviceObject->Flags = LowerDeviceObject->Flags;

	return STATUS_SUCCESS;
}

NTSTATUS
DriverEntry(
	PDRIVER_OBJECT	DriverObject,
	PUNICODE_STRING	RegistryPath
)
{
	INT		i;
	
	UNREFERENCED_PARAMETER(RegistryPath);

	RtlZeroMemory(DriverObject->MajorFunction, sizeof(DriverObject->MajorFunction));

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->DriverExtension->AddDevice = AddDevice;
	DriverObject->MajorFunction[IRP_MJ_SCSI] = WINIOWATCHER_DispatchSCSI;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = WINIOWATCHER_DispatchIoctl;

	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		if (DriverObject->MajorFunction[i] == NULL)
			DriverObject->MajorFunction[i] = WINIOWATCHER_DispatchDefault;
	}

	return STATUS_SUCCESS;
}