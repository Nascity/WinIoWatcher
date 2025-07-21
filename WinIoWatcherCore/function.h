#pragma once

NTSTATUS
WINIOWATCHER_DispatchCreate(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
);

NTSTATUS
WINIOWATCHER_DispatchRead(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
);

NTSTATUS
WINIOWATCHER_DispatchWrite(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
);