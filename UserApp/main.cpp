
#include <iostream>
#include <Windows.h>
#include "../KMDriver/shared.h"
#include "usermode.h"


int setDeviceHandle(HANDLE*);
int TestOperation(HANDLE);

int main()
{
    int retCode = STATUS_SUCCESS;
    HANDLE hDeviceFile = INVALID_HANDLE_VALUE;

	retCode = setDeviceHandle(&hDeviceFile);
	if (retCode != STATUS_SUCCESS) { return(retCode); }
	retCode = TestOperation(hDeviceFile);
	if (retCode != STATUS_SUCCESS) { return(retCode); }

	CloseHandle(hDeviceFile);
}

int setDeviceHandle(HANDLE *pHandle)
{
	printf("[setDeviceHandle] Opening handle to %s\n", UserlandPath);

	*pHandle = CreateFileA(
		UserlandPath,
		GENERIC_READ | GENERIC_WRITE,
		0,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	if (*pHandle==INVALID_HANDLE_VALUE)
	{
		printf("[setDeviceHandle] Handle to %s invalid\n", UserlandPath);
		return STATUS_FAILURE;
	}

	printf("[setDeviceHandle] device file handle acquired\n");
	return STATUS_SUCCESS;

	
}


int TestOperation(HANDLE hDeviceFile)
{
	BOOL opStatus = TRUE;
	DWORD nBufferSize = 32;
	DWORD bytesRead = 0;
	
	char* inBuffer = static_cast<char*>(malloc(nBufferSize));
	char* outBuffer = static_cast<char*>(malloc(nBufferSize));

	if ((inBuffer == nullptr) || (outBuffer == nullptr))
	{
		printf("[TestOperation] malloc failed");
		return(STATUS_FAILURE);
	}
	ZeroMemory(inBuffer, nBufferSize);
	ZeroMemory(outBuffer, nBufferSize);
	const char inStr[] = "This is the input buffer";
	const char outStr[] = "This is the output buffer";
	
	sprintf_s(inBuffer, sizeof(inStr), inStr);
	sprintf_s(outBuffer, sizeof(outStr), outStr);

	opStatus = DeviceIoControl(
		hDeviceFile,
		(DWORD)IOCTL_TEST_CMD,
		(LPVOID)inBuffer,
		nBufferSize,
		(LPVOID)outBuffer,
		nBufferSize,
		&bytesRead,
		nullptr);
	if (opStatus==FALSE)
	{
		printf("[TestOperation] DeviceIoControl() Failed\n");
	}
	printf("[TestOperation] bytesRead=%d\n",bytesRead);
	printf("[TestOperation] outBuffer=%s\n",outBuffer);
	free(inBuffer);
	free(outBuffer);
	return(STATUS_SUCCESS);
}