#include <wdm.h>
#include <minwindef.h>
#include "dbgmsg.h"
#include "synchack.h"


KIRQL RaiseIRQL();
void LowerIRQL(IRQL);
PKDPC AcquireLock();
NTSTATUS ReleaseLock(PVOID);

void SyncHack()
{
	KIRQL irql;
	PKDPC dpcPtr;

	irql = RaiseIRQL();
	dpcPtr = AcquireLock();
	// 
	// Use shared data
	// 
	ReleaseLock(dpcPtr);
	LowerIRQL(irql);
}


KIRQL RaiseIRQL()
{
	KIRQL previous;

	KIRQL current = KeGetCurrentIrql();
	previous = current;
	if(current < DISPATCH_LEVEL)
	{
		KeRaiseIrql(DISPATCH_LEVEL, &previous);
	}
	return previous;
}

void LowerIRQL(KIRQL previous)
{
	KeLowerIrql(previous);
}

LONG nCPUsLocked;
LONG CanReleaseLock;



void lockRoutine(
	IN PKDPC dpc,
	IN PVOID context,
	IN PVOID arg1,
	IN PVOID arg2)
{
	DBG_PRINT2("[lockRoutine]: beginCPU[%u]", KeGetCurrentProcessorNumberEx(NULL));
	InterlockedIncrement(&nCPUsLocked);

	while (InterlockedCompareExchange(&CanReleaseLock, 1, 1) == 0)
	{
		__asm {
			nop;
		}
	}

	InterlockedDecrement(&nCPUsLocked);
	DBG_PRINT2("[lockRoutine]: endCPU[%u]", KeGetCurrentProcessorNumberEx(NULL));

}


PKDPC AcquireLock()
{
	PKDPC dpcArray;
	DWORD nOtherCPUs;

	if(KeGetCurrentIrql()!=DISPATCH_LEVEL)
	{
		DBG_TRACE("AcquireLock", "Current IRQL is not DISPATCH_LEVEL(0x2)");
		return NULL;
	}

	InterlockedAnd(&CanReleaseLock, 0);
	InterlockedAnd(&nCPUsLocked, 0);

	dpcArray = (PKDPC)ExAllocatePoolWithTag
		(
		NonPagedPool,
		KeNumberProcessors * sizeof(KDPC),
		'1gaT');
	if(dpcArray==NULL)
	{
		DBG_TRACE("AcquireLock", "Could not allocate dpcArray");
		return NULL;
	}

	DWORD currentCpuID = KeGetCurrentProcessorNumberEx(NULL);

	for(DWORD i = 0;i < KeNumberProcessors;++i)
	{
		PKDPC dpcPtr = &(dpcArray[i]);
		if(i == currentCpuID) continue;
		KeInitializeDpc(dpcPtr, lockRoutine, NULL);
		KeSetTargetProcessorDpc(dpcPtr, i);
		KeInsertQueueDpc(dpcPtr, NULL, NULL);
	}

	nOtherCPUs = KeNumberProcessors-1;
	InterlockedCompareExchange(&nCPUsLocked, nOtherCPUs, nOtherCPUs);
	while(nCPUsLocked != nOtherCPUs)
	{
		// busy wait for other cpus getting locked
		__asm{
			nop;
		}
		InterlockedCompareExchange(&nCPUsLocked, nOtherCPUs, nOtherCPUs);
	}

	return dpcArray;
}



NTSTATUS ReleaseLock(PVOID dpcPtr)
{
	InterlockedIncrement(&CanReleaseLock);

	InterlockedCompareExchange(&nCPUsLocked, 0, 0);
	while(nCPUsLocked!=0)
	{
		__asm {
			nop;
		}
		InterlockedCompareExchange(&nCPUsLocked, 0, 0);
	}
	if(dpcPtr!=NULL)
	{
		ExFreePool(dpcPtr);
	}
	return STATUS_SUCCESS;
}