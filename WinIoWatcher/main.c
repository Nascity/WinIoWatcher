#include <stdio.h>
#include <Windows.h>

#include "device.h"
#include "logfile.h"
#include "shared_mem.h"

VOID
ExitHandler(
	VOID
)
{
	MemCleanup();
	FileCleanup();

	wprintf(L"Clean up.\n");
}

BOOL
Init(
	VOID
)
{
	if (!InitDeviceDriver())
	{
		wprintf(L"InitDeviceDriver Failed with %d\n", GetLastError());
		return FALSE;
	}
	wprintf(L"InitDeviceDriver.\n");

	if (!InitSharedMem())
	{
		wprintf(L"InitSharedMem Failed with %d\n", GetLastError());
		return FALSE;
	}
	wprintf(L"InitSharedMem.\n");

	if (!InitEvent())
	{
		wprintf(L"InitEvent Failed with %d\n", GetLastError());
		return FALSE;
	}
	wprintf(L"InitEvent.\n");

	if (!InitLogFile())
	{
		wprintf(L"InitLogFile Failed with %d\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

INT
wmain(
	INT		argc,
	LPWSTR	argv[]
)
{
	LOG		log;

	atexit(ExitHandler);

	if (!Init())
		return -1;

	while (TRUE)
	{
		ReadLog(&log);

		wprintf(
			L"%lld,%lld,%ld,%d",
			log.Time, log.LBA,
			log.Length, (INT)log.IsRead
		);

		WriteLog(&log);
	}
}