#ifndef _EFI_SMBIOS_H_
#define _EFI_SMBIOS_H_

#include "efi_types.hxx"

#pragma pack(push, 1)

typedef struct {
    UINT8  AnchorString[4];
    UINT8  EntryPointStructureChecksum;
    UINT8  EntryPointLength;
    UINT8  MajorVersion;
    UINT8  MinorVersion;
    UINT16 MaxStructureSize;
    UINT8  EntryPointRevision;
    UINT8  FormattedArea[5];
    UINT8  IntermediateAnchorString[5];
    UINT8  IntermediateChecksum;
    UINT16 TableLength;
    UINT32 TableAddress;
    UINT16 NumberOfStructures;
    UINT8  BcdRevision;
} SMBIOS_ENTRY_POINT;

typedef struct {
    UINT8  AnchorString[5];
    UINT8  EntryPointStructureChecksum;
    UINT8  EntryPointLength;
    UINT8  MajorVersion;
    UINT8  MinorVersion;
    UINT8  DocRev;
    UINT8  EntryPointRevision;
    UINT8  Reserved;
    UINT32 TableMaximumSize;
    UINT64 TableAddress;
} SMBIOS3_ENTRY_POINT;

typedef struct {
    UINT8  Type;
    UINT8  Length;
    UINT16 Handle;
} SMBIOS_HEADER;

typedef struct {
    SMBIOS_HEADER Hdr;
    UINT8 Vendor;
    UINT8 Version;
    UINT16 StartingAddressSegment;
    UINT8 ReleaseDate;
    UINT8 RomSize;
    UINT8 Characteristics[8];
    UINT8 ExtCharacteristics[2];
    UINT8 SystemBiosMajorRelease;
    UINT8 SystemBiosMinorRelease;
    UINT8 EcFirmwareMajorRelease;
    UINT8 EcFirmwareMinorRelease;
} SMBIOS_TYPE0;

typedef struct {
    SMBIOS_HEADER Hdr;
    UINT8 Manufacturer;
    UINT8 ProductName;
    UINT8 Version;
    UINT8 SerialNumber;
    UINT8 Uuid[16];
    UINT8 WakeUpType;
    UINT8 SkuNumber;
    UINT8 Family;
} SMBIOS_TYPE1;

typedef struct {
    SMBIOS_HEADER Hdr;
    UINT8 Manufacturer;
    UINT8 Product;
    UINT8 Version;
    UINT8 SerialNumber;
    UINT8 AssetTag;
    UINT8 FeatureFlags;
    UINT8 LocationInChassis;
    UINT16 ChassisHandle;
    UINT8 BoardType;
    UINT8 NumberOfContainedObjectHandles;
    UINT16 ContainedObjectHandles[1];
} SMBIOS_TYPE2;

typedef struct {
    SMBIOS_HEADER Hdr;
    UINT8 Manufacturer;
    UINT8 Type;
    UINT8 Version;
    UINT8 SerialNumber;
    UINT8 AssetTag;
    UINT8 BootupState;
    UINT8 PowerSupplyState;
    UINT8 ThermalState;
    UINT8 SecurityStatus;
    UINT32 OemDefined;
    UINT8 Height;
    UINT8 NumberOfPowerCords;
    UINT8 ContainedElementCount;
    UINT8 ContainedElementRecordLength;
    UINT8 ContainedElements[1];
} SMBIOS_TYPE3;

typedef struct {
    SMBIOS_HEADER Hdr;
    UINT8 SocketDesignation;
    UINT8 ProcessorType;
    UINT8 ProcessorFamily;
    UINT8 ProcessorManufacturer;
    UINT64 ProcessorId;
    UINT8 ProcessorVersion;
    UINT8 Voltage;
    UINT16 ExternalClock;
    UINT16 MaxSpeed;
    UINT16 CurrentSpeed;
    UINT8 Status;
    UINT8 ProcessorUpgrade;
    UINT16 L1CacheHandle;
    UINT16 L2CacheHandle;
    UINT16 L3CacheHandle;
    UINT8 SerialNumber;
    UINT8 AssetTag;
    UINT8 PartNumber;
    UINT8 CoreCount;
    UINT8 CoreEnabled;
    UINT8 ThreadCount;
    UINT16 ProcessorCharacteristics;
    UINT16 ProcessorFamily2;
    UINT16 CoreCount2;
    UINT16 CoreEnabled2;
    UINT16 ThreadCount2;
} SMBIOS_TYPE4;

typedef struct {
    SMBIOS_HEADER Hdr;
    UINT16 MemoryArrayHandle;
    UINT16 MemoryErrorInformationHandle;
    UINT16 TotalWidth;
    UINT16 DataWidth;
    UINT16 Size;
    UINT8 FormFactor;
    UINT8 DeviceSet;
    UINT8 DeviceLocator;
    UINT8 BankLocator;
    UINT8 MemoryType;
    UINT16 TypeDetail;
    UINT16 Speed;
    UINT8 Manufacturer;
    UINT8 SerialNumber;
    UINT8 AssetTag;
    UINT8 PartNumber;
    UINT8 Attributes;
    UINT32 ExtendedSize;
    UINT16 ConfiguredMemorySpeed;
    UINT16 MinimumVoltage;
    UINT16 MaximumVoltage;
    UINT16 ConfiguredVoltage;
} SMBIOS_TYPE17;

#pragma pack(pop)

#define SMBIOS_TYPE_BIOS              0
#define SMBIOS_TYPE_SYSTEM            1
#define SMBIOS_TYPE_BASEBOARD         2
#define SMBIOS_TYPE_CHASSIS           3
#define SMBIOS_TYPE_PROCESSOR         4
#define SMBIOS_TYPE_MEMORY_DEVICE     17
#define SMBIOS_TYPE_END               127

#endif
