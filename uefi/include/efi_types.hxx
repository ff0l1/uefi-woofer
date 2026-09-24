#ifndef _EFI_H_
#define _EFI_H_

typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned int        UINT32;
typedef unsigned long long  UINT64;
typedef unsigned long long  UINTN;
typedef long long           INTN;
typedef char                CHAR8;
#ifdef __cplusplus
typedef wchar_t             CHAR16;
#else
typedef unsigned short      CHAR16;
#endif
typedef void                VOID;
typedef UINTN               EFI_STATUS;
typedef VOID               *EFI_HANDLE;
typedef UINT64              EFI_PHYSICAL_ADDRESS;
typedef unsigned char       BOOLEAN;

#ifndef NULL
#ifdef __cplusplus
#define NULL nullptr
#else
#define NULL ((VOID*)0)
#endif
#endif
#define TRUE  1
#define FALSE 0

#define EFI_SUCCESS             0
#define EFI_ERROR(x)            (0x8000000000000000ULL | (UINT64)(x))
#define EFI_LOAD_ERROR          EFI_ERROR(1)
#define EFI_INVALID_PARAMETER   EFI_ERROR(2)
#define EFI_UNSUPPORTED         EFI_ERROR(3)
#define EFI_BAD_BUFFER_SIZE     EFI_ERROR(4)
#define EFI_BUFFER_TOO_SMALL    EFI_ERROR(5)
#define EFI_NOT_READY           EFI_ERROR(6)
#define EFI_DEVICE_ERROR        EFI_ERROR(7)
#define EFI_WRITE_PROTECTED     EFI_ERROR(8)
#define EFI_OUT_OF_RESOURCES    EFI_ERROR(9)
#define EFI_VOLUME_CORRUPTED    EFI_ERROR(10)
#define EFI_VOLUME_FULL         EFI_ERROR(11)
#define EFI_NO_MEDIA            EFI_ERROR(12)
#define EFI_MEDIA_CHANGED       EFI_ERROR(13)
#define EFI_NOT_FOUND           EFI_ERROR(14)
#define EFI_ACCESS_DENIED       EFI_ERROR(15)
#define EFI_NO_MAPPING          EFI_ERROR(18)
#define EFI_ALREADY_STARTED     EFI_ERROR(20)
#define EFI_ABORTED             EFI_ERROR(21)

#define BIT(x) (1ULL << (x))

typedef struct {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8  Data4[8];
} EFI_GUID;

typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

typedef struct _EFI_DEVICE_PATH_PROTOCOL {
    UINT8 Type;
    UINT8 SubType;
    UINT8 Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

typedef struct {
    UINT32 Type;
    UINT64 PhysicalStart;
    UINT64 VirtualStart;
    UINT64 NumberOfPages;
    UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef enum {
    AllHandles,
    ByRegisterNotify,
    ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020

typedef EFI_STATUS (*EFI_ALLOCATE_POOL)(EFI_MEMORY_TYPE, UINTN, VOID**);
typedef EFI_STATUS (*EFI_FREE_POOL)(VOID*);
typedef EFI_STATUS (*EFI_ALLOCATE_PAGES)(EFI_ALLOCATE_TYPE, EFI_MEMORY_TYPE, UINTN, EFI_PHYSICAL_ADDRESS*);
typedef EFI_STATUS (*EFI_FREE_PAGES)(EFI_PHYSICAL_ADDRESS, UINTN);
typedef EFI_STATUS (*EFI_GET_MEMORY_MAP)(UINTN*, EFI_MEMORY_DESCRIPTOR*, UINTN*, UINTN*, UINT32*);
typedef VOID (*EFI_COPY_MEM)(VOID*, VOID*, UINTN);
typedef VOID (*EFI_SET_MEM)(VOID*, UINTN, UINT8);
typedef EFI_STATUS (*EFI_IMAGE_LOAD)(BOOLEAN, EFI_HANDLE, EFI_DEVICE_PATH_PROTOCOL*, VOID*, UINTN, EFI_HANDLE*);
typedef EFI_STATUS (*EFI_IMAGE_START)(EFI_HANDLE, UINTN*, CHAR16**);
typedef EFI_STATUS (*EFI_HANDLE_PROTOCOL)(EFI_HANDLE, EFI_GUID*, VOID**);
typedef EFI_STATUS (*EFI_LOCATE_PROTOCOL)(EFI_GUID*, VOID*, VOID**);
typedef EFI_STATUS (*EFI_LOCATE_HANDLE)(EFI_LOCATE_SEARCH_TYPE, EFI_GUID*, VOID*, UINTN*, EFI_HANDLE*);
typedef EFI_STATUS (*EFI_LOCATE_HANDLE_BUFFER)(EFI_LOCATE_SEARCH_TYPE, EFI_GUID*, VOID*, UINTN*, EFI_HANDLE**);
typedef EFI_STATUS (*EFI_IMAGE_UNLOAD)(EFI_HANDLE);
typedef EFI_STATUS (*EFI_OPEN_PROTOCOL)(EFI_HANDLE, EFI_GUID*, VOID**, EFI_HANDLE, EFI_HANDLE, UINT32);
typedef EFI_STATUS (*EFI_CLOSE_PROTOCOL)(EFI_HANDLE, EFI_GUID*, EFI_HANDLE, EFI_HANDLE);
typedef EFI_STATUS (*EFI_STALL)(UINTN);
typedef EFI_STATUS (*EFI_SET_WATCHDOG_TIMER)(UINTN, UINT64, UINTN, CHAR16*);
typedef EFI_STATUS (*EFI_EXIT_BOOT_SERVICES)(EFI_HANDLE, UINTN);

#endif
