#pragma once

NTSTATUS
WINIOWATCHER_DispatchSCSI(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
);

NTSTATUS
WINIOWATCHER_DispatchDefault(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp
);