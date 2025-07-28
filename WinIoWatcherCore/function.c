#include <ntddk.h>
#include <storport.h>
#include <wdm.h>

#include "driver.h"
#include "function.h"
#include "nas_assert.h"
#include "shared_mem.h"

VOID
MakeWorkItem(
	PDEVICE_OBJECT	DeviceObject,
	BOOLEAN			IsRead,
	ULONGLONG		LBA,
	ULONG			Length,
	ULONGLONG		Time
)
{
	PWINIOWATCHER_WORK_ITEM		WinWorkItem;

	WinWorkItem = ExAllocatePool2(
		POOL_FLAG_NON_PAGED,
		sizeof(WINIOWATCHER_WORK_ITEM),
		WORK_ITEM_TAG
	);
	if (!WinWorkItem)
		return;

	WinWorkItem->Log.IsRead = IsRead;
	WinWorkItem->Log.LBA = LBA;
	WinWorkItem->Log.Length = Length;
	WinWorkItem->Log.Time = Time;

	WinWorkItem->Magic = WORK_ITEM_MAGIC;

	WinWorkItem->WorkItem = IoAllocateWorkItem(DeviceObject);

	if (!WinWorkItem->WorkItem) {
		ExFreePoolWithTag(WinWorkItem, WORK_ITEM_TAG);
		return;
	}

	IoQueueWorkItem(
		WinWorkItem->WorkItem,
		LoggerHandler,
		DelayedWorkQueue,
		WinWorkItem
	);
}

VOID
ReadOrWrite(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	PSCSI_REQUEST_BLOCK		SRB;
	PIO_STACK_LOCATION		StackLocation = IoGetCurrentIrpStackLocation(Irp);
	
	LARGE_INTEGER	Time;
	BOOLEAN			IsRead;

	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(IsRead);

	KeQuerySystemTime(&Time);

	SRB = StackLocation->Parameters.Scsi.Srb;
	
	/*
	MakeWorkItem(
		DeviceObject,
		IsRead,
		LBA,
		Length,
		Time.QuadPart
	);
	*/
}

NTSTATUS
WINIOWATCHER_DispatchSCSI(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	PDEVICE_EXTENSION	Ext;

	Ext = DeviceObject->DeviceExtension;

	ReadOrWrite(DeviceObject, Irp);

	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(Ext->LowerDeviceObject, Irp);
}

PIO_WORKITEM	WorkItem;

NTSTATUS
WINIOWATCHER_DispatchIoctl(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	PIO_STACK_LOCATION	StackLocation = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS			Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(DeviceObject);

	switch (StackLocation->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_WINIOWATCHER_INIT:
		WorkItem = IoAllocateWorkItem(DeviceObject);
		if (!WorkItem)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		IoQueueWorkItem(WorkItem, InitializingWorker, DelayedWorkQueue, WorkItem);
		break;
	default:
		Status = STATUS_UNSUCCESSFUL;
		break;
	}

	return Status;
}

NTSTATUS
WINIOWATCHER_DispatchDefault(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	PDEVICE_EXTENSION	Ext;

	Ext = DeviceObject->DeviceExtension;

	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(Ext->LowerDeviceObject, Irp);
}