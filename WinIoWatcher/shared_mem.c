#include <Windows.h>

#include "../common.h"
#include "shared_mem.h"

HANDLE	g_SectionMapping;
LPVOID	g_SharedMemAddr;
HANDLE	g_Event;

BOOL
InitSharedMem(
	VOID
)
{
	g_SectionMapping = OpenFileMappingW(
		FILE_MAP_READ,
		FALSE,
		USER_SHARED_MEM_NAME
	);
	if (g_SectionMapping == NULL)
		return FALSE;

	g_SharedMemAddr = MapViewOfFile(
		g_SectionMapping,
		FILE_MAP_READ,
		0,
		0,
		0
	);
	if (g_SharedMemAddr == NULL)
	{
		CloseHandle(g_SectionMapping);
		return FALSE;
	}

	return TRUE;
}

BOOL
InitEvent(
	VOID
)
{
	g_Event = OpenEventW(
		SYNCHRONIZE | EVENT_MODIFY_STATE,
		FALSE,
		USER_EVENT_NAME
	);
	if (g_Event == NULL)
		return FALSE;

	return TRUE;
}

VOID
MemCleanup(
	VOID
)
{
	if (g_SharedMemAddr)
		UnmapViewOfFile(g_SharedMemAddr);
	if (g_SectionMapping)
		CloseHandle(g_SectionMapping);
	if (g_Event)
		CloseHandle(g_Event);
}

VOID
ReadLog(
	PLOG	pLog
)
{
	DWORD	dwResult;

	while (TRUE)
	{
		dwResult = WaitForSingleObject(
			g_Event,
			INFINITE
		);

		if (dwResult == WAIT_OBJECT_0)
			break;
	}

	memcpy(pLog, g_SharedMemAddr, sizeof(LOG));

	ResetEvent(g_Event);
}