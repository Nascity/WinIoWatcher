#include <Windows.h>

#define USERLAND
#include "../common.h"

BOOL
InitDeviceDriver(
	VOID
)
{
	HANDLE	hDriver;
	BOOL	bSuccess;

	hDriver = CreateFileW(
		L"\\\\.\\WinIoWatcherCore",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);
	if (hDriver == INVALID_HANDLE_VALUE)
		return FALSE;

	bSuccess = DeviceIoControl(
		hDriver,
		IOCTL_WINIOWATCHER_INIT,
		NULL,
		0,
		NULL,
		0,
		NULL,
		NULL
	);

	CloseHandle(hDriver);
	return bSuccess;
}