#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <Ntstrsafe.h>


DECLSPEC_IMPORT NTSTATUS           PsLookupProcessByProcessId(_In_ HANDLE ProcessId, _Outptr_ PEPROCESS *Process);
DECLSPEC_IMPORT NTSTATUS NTAPI     MmCopyVirtualMemory(IN PEPROCESS FromProcess, IN PVOID FromAddress, IN PEPROCESS ToProcess, OUT PVOID ToAddress, IN SIZE_T BufferSize, IN KPROCESSOR_MODE PreviousMode, OUT PSIZE_T NumberOfBytesCopied);


#include "Define.h"


NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS Mj_Create(IN PDEVICE_OBJECT fdo, IN PIRP Irp);
NTSTATUS Mj_Close(IN PDEVICE_OBJECT fdo, IN PIRP Irp);
NTSTATUS Mj_ReadWrite(IN PDEVICE_OBJECT fdo, IN PIRP Irp);
NTSTATUS Dispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
VOID     UnloadRoutine(IN PDRIVER_OBJECT DriverObject);

NTSTATUS  WriteMemory(IN PMEMORY pCopy);
NTSTATUS  ReadMemory(IN PMEMORY pCopy);

#define DPRINT(msg, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, msg, __VA_ARGS__)
