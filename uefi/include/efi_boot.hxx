#ifndef _EFI_BOOT_H_
#define _EFI_BOOT_H_

#include "efi_types.hxx"

typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;
typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;

struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    UINT64 Revision;
    EFI_STATUS (*OpenVolume)(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL*, EFI_FILE_PROTOCOL**);
};

struct _EFI_FILE_PROTOCOL {
    UINT64 Revision;
    EFI_STATUS (*Open)(EFI_FILE_PROTOCOL*, EFI_FILE_PROTOCOL**, CHAR16*, UINT64, UINT64);
    EFI_STATUS (*Close)(EFI_FILE_PROTOCOL*);
    EFI_STATUS (*Delete)(EFI_FILE_PROTOCOL*);
    EFI_STATUS (*Read)(EFI_FILE_PROTOCOL*, UINTN*, VOID*);
    EFI_STATUS (*Write)(EFI_FILE_PROTOCOL*, UINTN*, VOID*);
    EFI_STATUS (*GetPosition)(EFI_FILE_PROTOCOL*, UINT64*);
    EFI_STATUS (*SetPosition)(EFI_FILE_PROTOCOL*, UINT64);
    EFI_STATUS (*GetInfo)(EFI_FILE_PROTOCOL*, EFI_GUID*, UINTN*, VOID*);
    EFI_STATUS (*SetInfo)(EFI_FILE_PROTOCOL*, EFI_GUID*, UINTN, VOID*);
    EFI_STATUS (*Flush)(EFI_FILE_PROTOCOL*);
};

#define EFI_FILE_MODE_READ   0x0000000000000001ULL
#define EFI_FILE_MODE_WRITE  0x0000000000000002ULL
#define EFI_FILE_MODE_CREATE 0x8000000000000000ULL

typedef struct {
    UINT32 Revision;
    EFI_HANDLE ParentHandle;
    VOID *SystemTable;
    EFI_HANDLE DeviceHandle;
    EFI_DEVICE_PATH_PROTOCOL *FilePath;
    VOID *Reserved;
    UINT32 LoadOptionsSize;
    VOID *LoadOptions;
    VOID *ImageBase;
    UINT64 ImageSize;
    UINT32 ImageCodeType;
    UINT32 ImageDataType;
    EFI_STATUS (*Unload)(EFI_HANDLE);
} EFI_LOADED_IMAGE_PROTOCOL;

typedef struct {
    EFI_TABLE_HEADER Hdr;
    EFI_ALLOCATE_PAGES AllocatePages;
    EFI_FREE_PAGES FreePages;
    EFI_GET_MEMORY_MAP GetMemoryMap;
    EFI_ALLOCATE_POOL AllocatePool;
    EFI_FREE_POOL FreePool;
    EFI_STATUS (*CreateEvent)(UINT32, UINT32, VOID*, VOID*, VOID**);
    EFI_STATUS (*SetTimer)(VOID*, UINT32, UINT64);
    EFI_STATUS (*WaitForEvent)(UINTN, VOID**, UINTN*);
    EFI_STATUS (*SignalEvent)(VOID*);
    EFI_STATUS (*CloseEvent)(VOID*);
    EFI_STATUS (*CheckEvent)(VOID*);
    EFI_IMAGE_LOAD LoadImage;
    EFI_IMAGE_START StartImage;
    EFI_STATUS (*Exit)(EFI_HANDLE, EFI_STATUS, UINTN, CHAR16*);
    EFI_IMAGE_UNLOAD UnloadImage;
    EFI_EXIT_BOOT_SERVICES ExitBootServices;
    EFI_STATUS (*GetNextMonotonicCount)(UINT64*);
    EFI_STATUS (*Stall)(UINTN);
    EFI_STATUS (*SetWatchdogTimer)(UINTN, UINT64, UINTN, CHAR16*);
    EFI_HANDLE_PROTOCOL HandleProtocol;
    VOID *Reserved1;
    VOID *Reserved2;
    EFI_STATUS (*RegisterProtocolNotify)(EFI_GUID*, VOID*, VOID**);
    EFI_LOCATE_HANDLE LocateHandle;
    EFI_LOCATE_HANDLE_BUFFER LocateHandleBuffer;
    EFI_LOCATE_PROTOCOL LocateProtocol;
    EFI_STATUS (*InstallProtocolInterface)(EFI_HANDLE*, EFI_GUID*, UINT32, VOID*);
    EFI_STATUS (*ReinstallProtocolInterface)(EFI_HANDLE, EFI_GUID*, VOID*, VOID*);
    EFI_STATUS (*UninstallProtocolInterface)(EFI_HANDLE, EFI_GUID*, VOID*);
    EFI_STATUS (*CalculateCrc32)(VOID*, UINTN, UINT32*);
    EFI_COPY_MEM CopyMem;
    EFI_SET_MEM SetMem;
    EFI_STATUS (*CreateProtocolNotify)(EFI_GUID*, VOID*, VOID**);
} EFI_BOOT_SERVICES;

typedef struct {
    EFI_TABLE_HEADER Hdr;
    EFI_STATUS (*GetTime)(VOID*, VOID*);
    EFI_STATUS (*SetTime)(VOID*);
    EFI_STATUS (*GetWakeupTime)(BOOLEAN*, BOOLEAN*, VOID*);
    EFI_STATUS (*SetWakeupTime)(BOOLEAN, VOID*);
    EFI_STATUS (*SetVirtualAddressMap)(UINTN, UINTN, UINT32, EFI_MEMORY_DESCRIPTOR*);
    EFI_STATUS (*ConvertPointer)(UINTN, VOID**);
    EFI_STATUS (*GetVariable)(CHAR16*, EFI_GUID*, UINT32*, UINTN*, VOID*);
    EFI_STATUS (*GetNextVariableName)(UINTN*, CHAR16*, EFI_GUID*);
    EFI_STATUS (*SetVariable)(CHAR16*, EFI_GUID*, UINT32, UINTN, VOID*);
    EFI_STATUS (*GetNextHighMonotonicCount)(UINT32*);
    EFI_STATUS (*ResetSystem)(UINT32, EFI_STATUS, UINTN, VOID*);
} EFI_RUNTIME_SERVICES;

typedef struct {
    EFI_GUID VendorGuid;
    VOID *VendorTable;
} EFI_CONFIGURATION_TABLE;

typedef struct {
    EFI_STATUS (*Reset)(VOID*, BOOLEAN);
    EFI_STATUS (*OutputString)(VOID*, CHAR16*);
    EFI_STATUS (*TestString)(VOID*, CHAR16*);
    EFI_STATUS (*QueryMode)(VOID*, UINTN, UINTN*, UINTN*);
    EFI_STATUS (*SetMode)(VOID*, UINTN);
    EFI_STATUS (*SetAttribute)(VOID*, UINTN);
    EFI_STATUS (*ClearScreen)(VOID*);
    EFI_STATUS (*SetCursorPosition)(VOID*, UINTN, UINTN);
    EFI_STATUS (*EnableCursor)(VOID*, BOOLEAN);
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct {
    EFI_STATUS (*Reset)(VOID*, BOOLEAN);
    EFI_STATUS (*ReadKeyStroke)(VOID*, VOID*);
    VOID *WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

typedef struct {
    EFI_TABLE_HEADER Hdr;
    CHAR16 *FirmwareVendor;
    UINT32 FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    EFI_RUNTIME_SERVICES *RuntimeServices;
    EFI_BOOT_SERVICES *BootServices;
    UINTN NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE *ConfigurationTable;
} EFI_SYSTEM_TABLE;

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
    {0x0964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
    {0x5b1b31a1, 0x9562, 0x11d2, {0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#define EFI_DEVICE_PATH_PROTOCOL_GUID \
    {0x09576e91, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#define SMBIOS_TABLE_GUID \
    {0xeb9d2d31, 0x2d88, 0x11d3, {0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}

#define SMBIOS3_TABLE_GUID \
    {0xf2fd1544, 0x9794, 0x4a3c, {0x99, 0xd4, 0x1c, 0x42, 0xf7, 0xa0, 0x4b, 0x0c}}

#define EFI_FILE_INFO_GUID \
    {0x09576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#endif
