#pragma once



const char UserlandPath[] = "\\\\.\\msnetdiag";

const WCHAR DeviceNameBuffer[] = L"\\Device\\msnetdiag";

const WCHAR DeviceLinkBuffer[] = L"\\DosDevices\\msnetdiag";
const WCHAR DriverExecutableName[] = L"KMDriver.sys";
const WCHAR DeviceNameSC[] = L"srv3";


#define FILE_DEVICE_RK 0x00008001

#define IOCTL_TEST_CMD \
	CTL_CODE(FILE_DEVICE_RK,0x801,METHOD_BUFFERED,FILE_READ_DATA | FILE_WRITE_DATA)