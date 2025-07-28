#pragma once

#include <wdm.h>

#include "../common.h"

#define SHARED_MEM_SIZE		4096

#define WORK_ITEM_TAG		'wiwt'
#define WORK_ITEM_MAGIC		0xDEADCAFE

typedef struct
{
	INT				Magic;
	PIO_WORKITEM  	WorkItem;
	LOG				Log;
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

VOID
InitializingWorker(
	PDEVICE_OBJECT	DeviceObject,
	PVOID			Context
);