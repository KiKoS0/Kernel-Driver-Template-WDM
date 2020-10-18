#pragma once



const char UserlandPath[] = "\\\\.\\msnetdiag";
#define FILE_DEVICE_RK 0x00008001

#define IOCTL_TEST_CMD \
	CTL_CODE(FILE_DEVICE_RK,0x801,METHOD_BUFFERED,FILE_READ_DATA | FILE_WRITE_DATA)