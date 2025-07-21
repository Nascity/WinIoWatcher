#include <ntddk.h>
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
	if (!WinWorkItem->WorkItem)
		return;

	IoQueueWorkItem(
		WinWorkItem->WorkItem,
		LoggerHandler,
		DelayedWorkQueue,
		WinWorkItem
	);
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
		if (StackLocation->Parameters.Read.Length <= 0)
			goto end;

		ByteOffset = StackLocation->Parameters.Read.ByteOffset;
		Length = StackLocation->Parameters.Read.Length;

	}
	else
	{
		if (StackLocation->Parameters.Write.Length <= 0)
			goto end;

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

end:
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
	return ReadOrWrite(DeviceObject, Irp, TRUE);
}

NTSTATUS
WINIOWATCHER_DispatchWrite(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
)
{
	return ReadOrWrite(DeviceObject, Irp, FALSE);
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