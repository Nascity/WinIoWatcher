#pragma once

#include <wdm.h>

#include "../common.h"

#define SHARED_MEM_SIZE		4096

#define WORK_ITEM_TAG		'wiw '

typedef struct
{
	PIO_WORKITEM  		WorkItem;
	LOG					Log;
}	WINIOWATCHER_WORK_ITEM, *PWINIOWATCHER_WORK_ITEM;

NTSTATUS
WINIOWATCHER_InitSharedMemory(
	VOID
);

NTSTATUS
WINIOWATCHER_InitEvent(
	VOID
);

VOID
WINIOWATCHER_Cleanup(
	VOID
);

VOID
LoggerHandler(
	PDEVICE_OBJECT	DeviceObject,
	PVOID			Context
);