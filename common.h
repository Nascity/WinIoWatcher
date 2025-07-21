#pragma once

#define DEVICE_NAME		L"\\Device\\WinIoWatcherCore"
#define SYMLINK_NAME	L"\\DosDevices\\WinIoWatcherCore"

#define SHARED_MEM_NAME		L"\\Sessions\\1\\BaseNamedObjects\\WinIoWatcherSharedMem"
#define EVENT_NAME			L"\\Sessions\\1\\BaseNamedObjects\\WinIoWatcherEvent"

#define USER_SHARED_MEM_NAME	L"WinIoWatcherSharedMem"
#define USER_EVENT_NAME			L"WinIoWatcherEvent"

typedef struct
{
	unsigned char		IsRead;
	unsigned long long	LBA;
	unsigned long		Length;
	unsigned long long	Time;
}	LOG, *PLOG;