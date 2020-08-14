#include "Driver.h"




#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, UnloadRoutine)  
#pragma alloc_text(PAGE, Mj_Create)
#pragma alloc_text(PAGE, Mj_Close)
#pragma alloc_text(PAGE, Mj_ReadWrite)
#pragma alloc_text(PAGE, Dispatch)

#pragma alloc_text(PAGE, WriteMemory)
#pragma alloc_text(PAGE, ReadMemory)


VOID
hide_proc(char *pc)
{
	PEPROCESS currentProc = (PEPROCESS)PsGetCurrentProcess();
	PEPROCESS startProc = (PEPROCESS)PsGetCurrentProcess();

	PLIST_ENTRY activeProcLinks;
	PUCHAR pImageFileName;
	PUINT32 pPidProc;

	for (; ((DWORD64)startProc != (DWORD64)currentProc);)
	{
		pImageFileName = (PUCHAR)((DWORD64)currentProc + ImageFileName);
		pPidProc = (PUINT32)((DWORD64)currentProc + UniqueProcessId);
		activeProcLinks = (PLIST_ENTRY)((DWORD64)currentProc + ActiveProcessLinks);
		startProc = (PEPROCESS)((DWORD64)activeProcLinks->Flink - ActiveProcessLinks);

		if (!strcmp((const char*)pImageFileName, TEXT(pc))) {
			*((PDWORD64)activeProcLinks->Blink) = (DWORD64)activeProcLinks->Flink;
			*((PDWORD64)(activeProcLinks->Flink) + 1) = (DWORD64)activeProcLinks->Blink;

			activeProcLinks->Blink = (PLIST_ENTRY)&activeProcLinks->Flink;
			activeProcLinks->Flink = (PLIST_ENTRY)&activeProcLinks->Flink;
		}
	}
}

NTSTATUS 
Dispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	PIO_STACK_LOCATION StackLocation = IoGetCurrentIrpStackLocation(Irp);
	PVOID              SystemBuffer = Irp->AssociatedIrp.SystemBuffer;
	ULONG              InputBufferLength = StackLocation->Parameters.DeviceIoControl.InputBufferLength;

	switch (StackLocation->MajorFunction)
	{
	case IRP_MJ_DEVICE_CONTROL:
	{
		// Диспетчеризация по IOCTL кодам
		switch (StackLocation->Parameters.DeviceIoControl.IoControlCode)
		{
		case IOCTL_PRINT_DEBUG_MESS:
		{
			DPRINT("[xBox]:IOCTL_PRINT_DEBUG_MESS\n");
			break;
		}
		case IOCTL_READ_MEMORY:
		{
			if (InputBufferLength >= sizeof(MEMORY) && SystemBuffer)
				Irp->IoStatus.Status = ReadMemory((PMEMORY)SystemBuffer);
			else
				Irp->IoStatus.Status = STATUS_INFO_LENGTH_MISMATCH;
			break;
		}
		case IOCTL_WRITE_MEMORY:
		{
			if (InputBufferLength >= sizeof(MEMORY) && SystemBuffer)
				Irp->IoStatus.Status = WriteMemory((PMEMORY)SystemBuffer);
			else
				Irp->IoStatus.Status = STATUS_INFO_LENGTH_MISMATCH;
			break;
		}
		case IRP_HIDE_PROC:
		{
			SystemBuffer = Irp->AssociatedIrp.SystemBuffer;
			Irp->IoStatus.Information = strlen(SystemBuffer);
			hide_proc(SystemBuffer);
		}
		default:
			DPRINT("[xBox]:Unknown IRP\n");
			Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
			break;
		}
	}
	break;
	}
	status = Irp->IoStatus.Status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	
	DPRINT("[xBox]:Hello!\n");
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT deviceObject = NULL;
	UNICODE_STRING dName;
	UNICODE_STRING dLink;
	RtlUnicodeStringInit(&dName, DEVICE_NAME);

	status = IoCreateDevice(DriverObject, 0, &dName, FILE_DEVICE_MIRRORE, 0, FALSE, &deviceObject);
	if (!NT_SUCCESS(status))
	{
		DPRINT("[FiveSeven]:IoCreateDevice failed:  0x%X\n", status);
		return status;
	}

	DriverObject->DriverUnload = UnloadRoutine;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = Mj_Create;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = Mj_Close;
	DriverObject->MajorFunction[IRP_MJ_READ] = Mj_ReadWrite;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = Mj_ReadWrite;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Dispatch;

	RtlUnicodeStringInit(&dLink, DOS_DEVICE_NAME);
	status = IoCreateSymbolicLink(&dLink, &dName);
	if (!NT_SUCCESS(status))
	{
		DPRINT("[xBox]:IoCreateSymbolicLink failed:  0x%X\n", status);

		IoDeleteDevice(deviceObject);
	}
	return STATUS_SUCCESS;
}

NTSTATUS 
ReadMemory(IN PMEMORY pCopy) 
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pProcess = NULL;
	__try
	{
		status = PsLookupProcessByProcessId((HANDLE)pCopy->pId, &pProcess);

		if (NT_SUCCESS(status))
		{
	
			SIZE_T bytes = 0;

			status = MmCopyVirtualMemory(pProcess, (PVOID)pCopy->pAdress, PsGetCurrentProcess(), (PVOID)pCopy->pBuffer, pCopy->iSize, KernelMode, &bytes);
		}
		else
			DPRINT("[FiveSeven]:%s: PsLookupProcessByProcessId failed: 0x%X\n", __FUNCTION__, status);

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DPRINT("[FiveSeven]:%s: Exception\n", __FUNCTION__);
		status = STATUS_UNHANDLED_EXCEPTION;
	}

	if (pProcess)
		ObDereferenceObject(pProcess);
	return status;
}

NTSTATUS  WriteMemory(IN PMEMORY pCopy) {

	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pProcess = NULL;

	__try
	{
		status = PsLookupProcessByProcessId((HANDLE)pCopy->pId, &pProcess);

		if (NT_SUCCESS(status))
		{
			SIZE_T bytes = 0;
			status = MmCopyVirtualMemory(PsGetCurrentProcess(), (PVOID)pCopy->pBuffer, pProcess, (PVOID)pCopy->pAdress, pCopy->iSize, KernelMode, &bytes);
		}
		else
			DPRINT("[DISKDUUMP]:%s: PsLookupProcessByProcessId failed with status 0x%X\n", __FUNCTION__, status);

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DPRINT("[DISKDUUMP]:%s: Exception\n", __FUNCTION__);
		status = STATUS_UNHANDLED_EXCEPTION;
	}
	if (pProcess)
		ObDereferenceObject(pProcess);
	return status;
}

NTSTATUS 
Mj_Create(IN PDEVICE_OBJECT fdo, IN PIRP Irp) {
	
	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Irp->IoStatus.Status;
}

NTSTATUS 
Mj_Close(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS 
Mj_ReadWrite(IN PDEVICE_OBJECT fdo, IN PIRP Irp) 
{
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

VOID 
UnloadRoutine(IN PDRIVER_OBJECT DriverObject) {

	UNICODE_STRING dLink;
	RtlUnicodeStringInit(&dLink, DOS_DEVICE_NAME);
	IoDeleteSymbolicLink(&dLink);
	IoDeleteDevice(DriverObject->DeviceObject);
	DPRINT("[FiveSeven]:Goodbye!\n");
}
