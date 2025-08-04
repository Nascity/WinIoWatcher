#pragma once

#define SYMBOLIC_LINK_NAME  L"\\??\\WinIoWatcherCore"

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT  LowerDeviceObject;
}   DEVICE_EXTENSION, *PDEVICE_EXTENSION;