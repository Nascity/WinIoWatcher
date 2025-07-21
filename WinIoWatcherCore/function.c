#include <ntddk.h>
#include <wdm.h>

#include "driver.h"
#include "function.h"
#include "nas_assert.h"
#include "shared_mem.h"

NTSTATUS
WINIOWATCHER_DispatchCreate(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

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

	WinWorkItem->WorkItem = IoAllocateWorkItem(DeviceObject);

	IoQueueWorkItem(
		WinWorkItem->WorkItem,
		LoggerHandler,
		DelayedWorkQueue,
		WinWorkItem
	);

	DbgPrint("[WIW] Queueing work item...\n");
}

NTSTATUS
ReadOrWrite(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp,
	BOOLEAN			IsRead
)
{
	PIO_STACK_LOCATION	StackLocation = IoGetCurrentIrpStackLocation(Irp);
	LARGE_INTEGER		ByteOffset;
	LARGE_INTEGER		Time;
	ULONGLONG			LBA;
	ULONG				Length;

	KeQuerySystemTime(&Time);

	if (IsRead)
	{
		ByteOffset = StackLocation->Parameters.Read.ByteOffset;
		Length = StackLocation->Parameters.Read.Length;

	}
	else
	{
		ByteOffset = StackLocation->Parameters.Write.ByteOffset;
		Length = StackLocation->Parameters.Write.Length;
	}
	LBA = ByteOffset.QuadPart / 512;

	MakeWorkItem(
		DeviceObject,
		IsRead,
		LBA,
		Length,
		Time.QuadPart
	);

	IoSkipCurrentIrpStackLocation(Irp);

	return IoCallDriver(
		((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->LowerDeviceObject,
		Irp
	);
}

NTSTATUS
WINIOWATCHER_DispatchRead(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	DbgPrint("[WIW] DispatchRead.\n");

	return ReadOrWrite(DeviceObject, Irp, TRUE);
}

NTSTATUS
WINIOWATCHER_DispatchWrite(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	DbgPrint("[WIW] DispatchWrite.\n");

	return ReadOrWrite(DeviceObject, Irp, FALSE);
}