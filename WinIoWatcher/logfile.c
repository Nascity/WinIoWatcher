#include <Windows.h>
#include <stdio.h>

#include "logfile.h"

HANDLE	g_File;

BOOL
InitLogFile(
	VOID
)
{
	g_File = CreateFileW(
		LOG_FILE_NAME,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (g_File == INVALID_HANDLE_VALUE)
		return FALSE;

	return TRUE;
}

VOID
FileCleanup(
	VOID
)
{
	CloseHandle(g_File);
}

VOID
WriteLog(
	PLOG	pLog
)
{
	WCHAR	szBuffer[1024];

	swprintf(
		szBuffer,
		1024,
		L"%lld,%lld,%lld,%d\n",
		pLog->Time, pLog->LBA,
		pLog->Length, (INT)pLog->IsRead & 0x1
	);

	WriteFile(
		g_File,
		szBuffer,
		wcslen(szBuffer) * sizeof(WCHAR),
		NULL,
		NULL
	);
}