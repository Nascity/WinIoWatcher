#pragma once

typedef struct
{
	PDEVICE_OBJECT	NextLayerDeviceObject;
	PDEVICE_OBJECT 	PhysicalDeviceObject;
	PDEVICE_OBJECT 	DeviceObject;
	UNICODE_STRING	UnicodeString;
}	DEVICE_EXTENSION, *PDEVICE_EXTENSION;