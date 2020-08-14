#pragma once
/*идентификатор*/
#define FILE_DEVICE_MIRRORE   0x7209

#define UniqueProcessId 0x2e8
#define ActiveProcessLinks 0x2f0
#define ImageFileName 0x450

/*зоголовки*/
#define DVR_DEVICE_NAME           L"nskov64"
#define DVR_DEVICE_FILE           L"\\\\.\\" DVR_DEVICE_NAME
#define DEVICE_NAME               L"\\Device\\"     ##DVR_DEVICE_NAME
#define DOS_DEVICE_NAME           L"\\DosDevices\\" ##DVR_DEVICE_NAME

/*коды управления вводом-выводом*/
#define IOCTL_PRINT_DEBUG_MESS        (ULONG)CTL_CODE(FILE_DEVICE_MIRRORE, 0x953, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ_MEMORY             (ULONG)CTL_CODE(FILE_DEVICE_MIRRORE, 0x657, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IRP_HIDE_PROC                 (ULONG)CTL_CODE(FILE_DEVICE_MIRRORE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define ctl_write    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0366, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define ctl_read    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0367, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define ctl_base    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0368, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)


#define IOCTL_WRITE_MEMORY            (ULONG)CTL_CODE(FILE_DEVICE_MIRRORE, 0x411, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
typedef struct info_t {
	int pid;
	DWORD_PTR address;
	void* value;
	SIZE_T size;
	void* data;
}info, * p_info;


typedef struct _SET_ID_PROCESS{
	ULONG      pId;
} SET_ID_PROCESS, *PSET_ID_PROCESS;


typedef struct _MEMORY{
	ULONG     pId;
	ULONGLONG pBuffer;
	ULONGLONG pAdress;
	ULONGLONG iSize;
} MEMORY, *PMEMORY;

 
