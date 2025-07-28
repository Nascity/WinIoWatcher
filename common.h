#pragma once

#include <winioctl.h>

#define DEVICE_NAME		L"\\Device\\WinIoWatcherCore"

#define SHARED_MEM_NAME		L"\\Sessions\\1\\BaseNamedObjects\\WinIoWatcherSharedMem"
#define EVENT_NAME			L"\\Sessions\\1\\BaseNamedObjects\\WinIoWatcherEvent"

#define USER_SHARED_MEM_NAME	L"WinIoWatcherSharedMem"
#define USER_EVENT_NAME			L"WinIoWatcherEvent"

#define IOCTL_WINIOWATCHER_INIT	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct
{
	unsigned char		IsRead;
	unsigned long long	LBA;
	unsigned long		Length;
	unsigned long long	Time;
}	LOG, *PLOG;