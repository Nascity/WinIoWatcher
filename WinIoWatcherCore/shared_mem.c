#include <ntifs.h>

#include "../common.h"
#include "nas_assert.h"
#include "shared_mem.h"

HANDLE	g_SharedMemHandle;
PVOID	g_SharedMemAddress;
HANDLE	g_SharedMemEventHandle;

NTSTATUS
WINIOWATCHER_InitSharedMemory(
	VOID
)
{
	UNICODE_STRING		SectionName;
	LARGE_INTEGER		MaxSize;
	OBJECT_ATTRIBUTES	ObjectAttribute;
	NTSTATUS			Status;
	SIZE_T				ViewSize;

	MaxSize.QuadPart = SHARED_MEM_SIZE;

	RtlInitUnicodeString(
		&SectionName,
		SHARED_MEM_NAME
	);

	InitializeObjectAttributes(
		&ObjectAttribute,
		&SectionName,
		OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);

	Status = ZwCreateSection(
		&g_SharedMemHandle,
		SECTION_ALL_ACCESS,
		&ObjectAttribute,
		&MaxSize,
		PAGE_READWRITE,
		SEC_COMMIT,
		NULL
	);
	if (Status == STATUS_OBJECT_NAME_COLLISION)
	{
		Status = ZwOpenSection(
			&g_SharedMemHandle,
			SECTION_ALL_ACCESS,
			&ObjectAttribute
		);
	}
	NAS_ASSERT(Status);

	ViewSize = SHARED_MEM_SIZE;
	Status = ZwMapViewOfSection(
		g_SharedMemHandle,
		ZwCurrentProcess(),
		&g_SharedMemAddress,
		0,
		SHARED_MEM_SIZE,
		NULL,
		&ViewSize,
		ViewShare,
		0,
		PAGE_READWRITE
	);
	NAS_ASSERT(Status);

	return STATUS_SUCCESS;
}

NTSTATUS
WINIOWATCHER_InitEvent(
	VOID
)
{
	UNICODE_STRING		EventName = RTL_CONSTANT_STRING(EVENT_NAME);
	OBJECT_ATTRIBUTES	ObjectAttribute;
	NTSTATUS			Status;

	InitializeObjectAttributes(
		&ObjectAttribute,
		&EventName,
		OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);

	Status = ZwCreateEvent(
		&g_SharedMemEventHandle,
		EVENT_ALL_ACCESS,
		&ObjectAttribute,
		NotificationEvent,
		FALSE
	);
	if (Status == STATUS_OBJECT_NAME_COLLISION)
	{
		Status = ZwOpenEvent(
			&g_SharedMemEventHandle,
			EVENT_ALL_ACCESS,
			&ObjectAttribute
		);
	}
	NAS_ASSERT(Status);

	return STATUS_SUCCESS;
}

VOID
WINIOWATCHER_Cleanup(
	VOID
)
{
	ZwClose(g_SharedMemEventHandle);
	ZwClose(g_SharedMemHandle);
}

VOID
WriteLog(
	PLOG	Log
)
{
	if (!g_SharedMemAddress || !g_SharedMemEventHandle)
		return;

	RtlZeroMemory(
		g_SharedMemAddress,
		sizeof(LOG)
	);

	RtlCopyMemory(
		g_SharedMemAddress,
		Log,
		sizeof(LOG)
	);

	ZwSetEvent(
		g_SharedMemEventHandle,
		NULL
	);
}

VOID
LoggerHandler(
	PDEVICE_OBJECT	DeviceObject,
	PVOID			Context
)
{
	PWINIOWATCHER_WORK_ITEM		WinWorkItem;

	UNREFERENCED_PARAMETER(DeviceObject);

	WinWorkItem = (PWINIOWATCHER_WORK_ITEM)Context;

	if (WinWorkItem->Magic != WORK_ITEM_MAGIC)
		return;

	WriteLog(&WinWorkItem->Log);

	IoFreeWorkItem(
		WinWorkItem->WorkItem
	);

	ExFreePoolWithTag(
		WinWorkItem,
		WORK_ITEM_TAG
	);
}