#include <ntddk.h>
#include <storport.h>
#include <wdm.h>

#include "driver.h"
#include "function.h"
#include "nas_assert.h"
#include "shared_mem.h"

VOID
GetOpcodeAndLBA(
	PSCSI_REQUEST_BLOCK	SRB,
	PBOOLEAN			IsRead,
	PUINT64				LBA,
	PUINT64				Length
)
{
	LARGE_INTEGER	Offset;
	UINT8			Opcode;

	Offset.QuadPart = 0;
	*Length = 0;
	Opcode = SRB->Cdb[0];

	switch (Opcode)
	{
	case SCSIOP_READ6:
	case SCSIOP_WRITE6:
	{
		struct _CDB6READWRITE* CDB = (struct _CDB6READWRITE*)&SRB->Cdb;

		Offset.HighPart = 0;
		Offset.LowPart =
			CDB->LogicalBlockLsb |
			CDB->LogicalBlockMsb0 << 8 |
			CDB->LogicalBlockMsb1 << 16;
		*Length = CDB->TransferBlocks;
	}
	case SCSIOP_READ:
	case SCSIOP_WRITE:
	{
		struct _CDB10*	CDB = (struct _CDB10*)&SRB->Cdb;

		Offset.HighPart = 0;
		Offset.LowPart =
			CDB->LogicalBlockByte0 << 24 |
			CDB->LogicalBlockByte1 << 16 |
			CDB->LogicalBlockByte2 << 8 |
			CDB->LogicalBlockByte3;
		*Length =
			CDB->TransferBlocksLsb |
			CDB->TransferBlocksMsb << 8;
		break;
	}
	case SCSIOP_READ12:
	case SCSIOP_WRITE12:
	{
		struct _CDB12*	CDB = (struct _CDB12*)&SRB->Cdb;

		Offset.HighPart = 0;
		Offset.LowPart =
			CDB->LogicalBlock[3] |
			CDB->LogicalBlock[2] << 8 |
			CDB->LogicalBlock[1] << 16 |
			CDB->LogicalBlock[0] << 24;
		*Length =
			CDB->TransferLength[3] |
			CDB->TransferLength[2] << 8 |
			CDB->TransferLength[1] << 16 |
			CDB->TransferLength[0] << 24;
		break;
	}
	case SCSIOP_READ16:
	case SCSIOP_WRITE16:
	{
		struct _CDB16*	CDB = (struct _CDB16*)&SRB->Cdb;

		Offset.HighPart =
			CDB->LogicalBlock[3] |
			CDB->LogicalBlock[2] << 8 |
			CDB->LogicalBlock[1] << 16 |
			CDB->LogicalBlock[0] << 24;
		Offset.LowPart =
			CDB->LogicalBlock[7] |
			CDB->LogicalBlock[6] << 8 |
			CDB->LogicalBlock[5] << 16 |
			CDB->LogicalBlock[4] << 24;
		*Length =
			CDB->TransferLength[3] |
			CDB->TransferLength[2] << 8 |
			CDB->TransferLength[1] << 16 |
			CDB->TransferLength[0] << 24;
		break;
	}
	}
	*LBA = Offset.QuadPart;

	switch (Opcode)
	{
	case SCSIOP_READ6:
	case SCSIOP_READ:
	case SCSIOP_READ12:
	case SCSIOP_READ16:
		*IsRead = TRUE;
		break;
	case SCSIOP_WRITE6:
	case SCSIOP_WRITE:
	case SCSIOP_WRITE12:
	case SCSIOP_WRITE16:
		*IsRead = FALSE;
		break;
	}
}

VOID
MakeWorkItem(
	PDEVICE_OBJECT		DeviceObject,
	PSCSI_REQUEST_BLOCK	SRB,
	ULONGLONG			Time
)
{
	PWINIOWATCHER_WORK_ITEM		WinWorkItem;
	BOOLEAN						IsRead;
	UINT64						LBA;
	UINT64						Length;

	GetOpcodeAndLBA(SRB, &IsRead, &LBA, &Length);

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
	LARGE_INTEGER			Time;

	KeQuerySystemTime(&Time);
	SRB = StackLocation->Parameters.Scsi.Srb;

	MakeWorkItem(DeviceObject, SRB, Time.QuadPart);
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
	PDEVICE_EXTENSION	Ext;
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
		Ext = DeviceObject->DeviceExtension;

		IoSkipCurrentIrpStackLocation(Irp);
		Status = IoCallDriver(Ext->LowerDeviceObject, Irp);
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