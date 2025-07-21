#pragma once

#define TARGET_DEVICE_NAME  L"\\Device\\HarddiskVolume3"

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT  LowerDeviceObject;
}   DEVICE_EXTENSION, *PDEVICE_EXTENSION;