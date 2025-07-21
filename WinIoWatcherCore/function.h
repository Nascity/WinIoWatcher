#pragma once

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

NTSTATUS
WINIOWATCHER_DispatchDefault(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
);