#include "dbgmsg.h"
#include <wdm.h>
#include "ntddk.h"
#include "driver.h"

// Not good but whatever
//#pragma warning(disable:4101)
//#pragma warning(disable:4100)



PDRIVER_OBJECT DriverObjectRef;


PDEVICE_OBJECT MSNetDiagDeviceObject;

NTSTATUS RegisterDriverDeviceName(IN PDRIVER_OBJECT DriverObject)
{
	NTSTATUS ntStatus;
	UNICODE_STRING unicodeString;
	RtlInitUnicodeString(&unicodeString, DeviceNameBuffer);
	ntStatus = IoCreateDevice(
		DriverObject,
		0,
		&unicodeString,
		FILE_DEVICE_RK,
		0,
		TRUE,
		&MSNetDiagDeviceObject);
	return (ntStatus);
}

NTSTATUS RegisterDriverDeviceLink(IN PDRIVER_OBJECT DriverObject)
{
	NTSTATUS ntStatus;
	UNICODE_STRING unicodeString;
	UNICODE_STRING unicodeLinkString;

	RtlInitUnicodeString(&unicodeString, DeviceNameBuffer);
	RtlInitUnicodeString(&unicodeLinkString, DeviceLinkBuffer);

	ntStatus = IoCreateSymbolicLink(&unicodeLinkString, &unicodeString);
	return (ntStatus);
}

VOID Unload(IN PDRIVER_OBJECT DriverObject)
{
	DBG_TRACE("OnUnload", "Received signal to unload the driver");

	PDEVICE_OBJECT deviceObj = DriverObject->DeviceObject;
	UNICODE_STRING unicodeString;

	if(deviceObj!= NULL)
	{
		DBG_TRACE("OnUnload", "Unregistering driver's symbolic link");
		RtlInitUnicodeString(&unicodeString, DeviceLinkBuffer);
		IoDeleteSymbolicLink(&unicodeString);

		DBG_TRACE("OnUnload", "Unregistering driver's symbolic link");
		IoDeleteDevice(deviceObj);
	}


	return;
}


NTSTATUS defaultDispatch(IN PDEVICE_OBJECT, IN PIRP);
NTSTATUS dispatchIOControl(IN PDEVICE_OBJECT, IN PIRP);


NTSTATUS DriverEntry
(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING regPath
)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(regPath);

	DBG_TRACE("Driver Entry", "Driver is loading");

	NTSTATUS ntStatus;

	DBG_TRACE("Driver Entry", "Registering driver's device name");

	ntStatus = RegisterDriverDeviceName(DriverObject);

	if (!NT_SUCCESS(ntStatus))
	{
		DBG_TRACE("Driver Entry", "Failed to create device");
		return ntStatus;
	}
	DBG_TRACE("Driver Entry", "Registering driver's symbolic link");
	ntStatus = RegisterDriverDeviceLink(DriverObject);
	if (!NT_SUCCESS(ntStatus))
	{
		DBG_TRACE("Driver Entry", "Failed to create sym link");
		return ntStatus;
	}



	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i)
	{
		(*DriverObject).MajorFunction[i] = defaultDispatch;
	}

	(*DriverObject).MajorFunction[IRP_MJ_DEVICE_CONTROL] = dispatchIOControl;



	(*DriverObject).DriverUnload = Unload;

	DriverObjectRef = DriverObject;


	return(ntStatus);
}


NTSTATUS defaultDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP IRP)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	((*IRP).IoStatus).Status = STATUS_SUCCESS;
	((*IRP).IoStatus).Information = 0;
	IoCompleteRequest(IRP, IO_NO_INCREMENT);

	return(STATUS_SUCCESS);

}




NTSTATUS dispatchIOControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP IRP)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	PIO_STACK_LOCATION irpStack;
	PVOID inputBuffer;
	PVOID outputBuffer;
	ULONG inBufferLength;
	ULONG outBufferLength;
	ULONG ioctrlcode;
	NTSTATUS ntStatus;


	ntStatus = STATUS_SUCCESS;

	((*IRP).IoStatus).Status = STATUS_SUCCESS;
	((*IRP).IoStatus).Information = 0;

	inputBuffer = IRP->AssociatedIrp.SystemBuffer;
	outputBuffer = IRP->AssociatedIrp.SystemBuffer;

	irpStack = IoGetCurrentIrpStackLocation(IRP);
	inBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
	outBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
	ioctrlcode = irpStack->Parameters.DeviceIoControl.IoControlCode;

	DBG_TRACE("dispatchIOCONTROL", "Received a command");

	switch (ioctrlcode) {
	case IOCTL_TEST_CMD:
	{
		//TestCommand(inputBuffer, outputBuffer, inBufferLength, outBufferLength);
	}break;
	default: {
		DBG_TRACE("dispatchIOCONTROL", "control code not recognized");
	}break;
	}
	IoCompleteRequest(IRP, IO_NO_INCREMENT);
	return(STATUS_SUCCESS);

}