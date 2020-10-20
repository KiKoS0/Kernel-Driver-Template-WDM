
#include <iostream>
#include <Windows.h>
#include "../KMDriver/shared.h"
#include "usermode.h"


int setDeviceHandle(HANDLE*);
int TestOperation(HANDLE);
SC_HANDLE installDriver(LPCTSTR, LPCTSTR);
int loadDriver(SC_HANDLE);

int main()
{
    int retCode = STATUS_SUCCESS;
    HANDLE hDeviceFile = INVALID_HANDLE_VALUE;

	WCHAR driverPath[512];
	ZeroMemory(driverPath, sizeof(driverPath));
	GetCurrentDirectory(sizeof driverPath, (LPWSTR) driverPath);
	wcsncat_s(driverPath, L"\\", 1);
	wcsncat_s(driverPath, DriverExecutableName, wcslen(DriverExecutableName));
	wprintf(L"Kernel driver path: %s\n", driverPath);

	SC_HANDLE hService;

	hService = installDriver(DeviceNameSC, driverPath);
	if(hService== nullptr)
	{
		return STATUS_FAILURE;
	}
	
	retCode = loadDriver(hService);
	if (retCode != STATUS_SUCCESS) { return(retCode); }

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

SC_HANDLE installDriver(LPCTSTR driverName,LPCTSTR binaryPath)
{
	SC_HANDLE scmDBHandle = OpenSCManager(
		nullptr, nullptr, SC_MANAGER_ALL_ACCESS
	);

	if(scmDBHandle== nullptr)
	{
		printf("[installDriver] could not open handle to SCM db\n");
		return nullptr;
	}
	SC_HANDLE svcHandle = CreateService
	(
		scmDBHandle,
		driverName,
		driverName,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		binaryPath,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr);

	if (svcHandle == nullptr)
	{
		if (GetLastError() == ERROR_SERVICE_EXISTS)
		{
			printf("[installDriver] driver already installed\n");
			svcHandle = OpenService(scmDBHandle, driverName, SERVICE_ALL_ACCESS);
			if (svcHandle==nullptr)
			{
				printf("[installDriver] could not open handle to driver (driver exists)\n");
				CloseServiceHandle(scmDBHandle);
				return  nullptr;
			}
			CloseServiceHandle(scmDBHandle);
			return(svcHandle);
		}
		printf("[installDriver] could not open handle to driver\n");
		CloseServiceHandle(scmDBHandle);
		return nullptr;
	}

	printf("[installDriver] function succeeded \n");
	CloseServiceHandle(scmDBHandle);
	return(svcHandle);
	
}

int loadDriver(SC_HANDLE svcHandle)
{
	if(StartService(svcHandle,0,nullptr) ==0)
	{
		DWORD lastError = GetLastError();
		printf("ERROR : %d\n", lastError);
		if(lastError == ERROR_SERVICE_ALREADY_RUNNING)
		{
			printf("[loadDriver] driver already running \n");
			return STATUS_SUCCESS;
		}
		printf("[loadDriver] failed to load driver \n");
		return STATUS_FAILURE;
	}
	printf("[loadDriver] driver loaded successfully \n");
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