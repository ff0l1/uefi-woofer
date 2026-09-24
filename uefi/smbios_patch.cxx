#include "efi_types.hxx"
#include "efi_boot.hxx"
#include "efi_smbios.hxx"

extern EFI_SYSTEM_TABLE *g_ST;
extern void Print(const CHAR16 *s);
extern void PrintLine(const CHAR16 *s);
extern void MemCpy(VOID *dst, VOID *src, UINTN n);
extern BOOLEAN GuidEqual(EFI_GUID *a, EFI_GUID *b);
extern UINT64 XorShift64(void);
extern CHAR8 RandChar(void);
extern void RandHex(CHAR8 *buf, int len);
extern UINT8 RandByte(void);

static UINTN StrLen8(CHAR8 *s) {
    UINTN len = 0;
    while (s[len]) len++;
    return len;
}

static CHAR8 *GetSmbiosString(SMBIOS_HEADER *hdr, UINT8 index) {
    if (index == 0) return NULL;
    CHAR8 *start = (CHAR8*)hdr + hdr->Length;
    CHAR8 *p = start;
    UINT8 cur = 1;
    while (cur < index) {
        if (*p == 0) {
            cur++;
            p++;
            if (*p == 0) return NULL;
        } else {
            p++;
        }
    }
    return p;
}

static void PatchSmbiosString(SMBIOS_HEADER *hdr, UINT8 index, CHAR8 *newStr) {
    if (index == 0) return;
    CHAR8 *str = GetSmbiosString(hdr, index);
    if (!str) return;
    UINTN origLen = StrLen8(str);
    UINTN newLen = StrLen8(newStr);
    if (newLen > origLen) newLen = origLen;
    MemCpy(str, newStr, newLen);
    for (UINTN i = newLen; i < origLen; i++)
        str[i] = 0;
}

static UINTN GetSmbiosStructSize(SMBIOS_HEADER *hdr) {
    UINT8 *p = (UINT8*)hdr + hdr->Length;
    while (!(p[0] == 0 && p[1] == 0))
        p++;
    return (UINTN)(p - (UINT8*)hdr) + 2;
}

static VOID *FindSmbiosTable(UINTN *outSize) {
    EFI_GUID smbios3Guid = SMBIOS3_TABLE_GUID;
    EFI_GUID smbiosGuid = SMBIOS_TABLE_GUID;
    for (UINTN i = 0; i < g_ST->NumberOfTableEntries; i++) {
        EFI_CONFIGURATION_TABLE *entry = &g_ST->ConfigurationTable[i];
        if (GuidEqual(&entry->VendorGuid, &smbios3Guid)) {
            SMBIOS3_ENTRY_POINT *ep = (SMBIOS3_ENTRY_POINT*)entry->VendorTable;
            *outSize = ep->TableMaximumSize;
            return (VOID*)(UINTN)ep->TableAddress;
        }
        if (GuidEqual(&entry->VendorGuid, &smbiosGuid)) {
            SMBIOS_ENTRY_POINT *ep = (SMBIOS_ENTRY_POINT*)entry->VendorTable;
            *outSize = ep->TableLength;
            return (VOID*)(UINTN)ep->TableAddress;
        }
    }
    return NULL;
}

void PatchSmbiosTables(void) {
    UINTN tableSize;
    SMBIOS_HEADER *hdr = (SMBIOS_HEADER*)FindSmbiosTable(&tableSize);
    if (!hdr) {
        PrintLine(L"[!] SMBIOS table not found!");
        return;
    }
    PrintLine(L"[+] SMBIOS table found");

    UINTN offset = 0;
    int patched = 0;

    while (offset < tableSize) {
        SMBIOS_HEADER *cur = (SMBIOS_HEADER*)((UINT8*)hdr + offset);
        if (cur->Type == SMBIOS_TYPE_END) break;
        if (cur->Length < 4) break;

        CHAR8 serial[32];
        CHAR8 manufacturer[32];
        CHAR8 product[32];
        CHAR8 version[32];

        switch (cur->Type) {
        case SMBIOS_TYPE_SYSTEM: {
            SMBIOS_TYPE1 *t = (SMBIOS_TYPE1*)cur;
            RandHex(serial, 12);
            PatchSmbiosString(cur, t->SerialNumber, serial);
            RandHex(manufacturer, 10);
            PatchSmbiosString(cur, t->Manufacturer, manufacturer);
            RandHex(product, 10);
            PatchSmbiosString(cur, t->ProductName, product);
            RandHex(version, 8);
            PatchSmbiosString(cur, t->Version, version);
            for (int i = 0; i < 16; i++)
                t->Uuid[i] = RandByte();
            patched++;
            PrintLine(L"    [+] Type 1 (System) patched");
            break;
        }
        case SMBIOS_TYPE_BASEBOARD: {
            SMBIOS_TYPE2 *t = (SMBIOS_TYPE2*)cur;
            RandHex(serial, 12);
            PatchSmbiosString(cur, t->SerialNumber, serial);
            RandHex(manufacturer, 10);
            PatchSmbiosString(cur, t->Manufacturer, manufacturer);
            RandHex(product, 10);
            PatchSmbiosString(cur, t->Product, product);
            RandHex(version, 8);
            PatchSmbiosString(cur, t->Version, version);
            patched++;
            PrintLine(L"    [+] Type 2 (Baseboard) patched");
            break;
        }
        case SMBIOS_TYPE_CHASSIS: {
            SMBIOS_TYPE3 *t = (SMBIOS_TYPE3*)cur;
            RandHex(serial, 12);
            PatchSmbiosString(cur, t->SerialNumber, serial);
            RandHex(manufacturer, 10);
            PatchSmbiosString(cur, t->Manufacturer, manufacturer);
            RandHex(version, 8);
            PatchSmbiosString(cur, t->Version, version);
            patched++;
            PrintLine(L"    [+] Type 3 (Chassis) patched");
            break;
        }
        case SMBIOS_TYPE_PROCESSOR: {
            SMBIOS_TYPE4 *t = (SMBIOS_TYPE4*)cur;
            if (cur->Length >= 0x20) {
                RandHex(serial, 12);
                PatchSmbiosString(cur, t->SerialNumber, serial);
                RandHex(manufacturer, 10);
                PatchSmbiosString(cur, t->ProcessorManufacturer, manufacturer);
                patched++;
                PrintLine(L"    [+] Type 4 (Processor) patched");
            }
            break;
        }
        case SMBIOS_TYPE_MEMORY_DEVICE: {
            SMBIOS_TYPE17 *t = (SMBIOS_TYPE17*)cur;
            if (cur->Length >= 0x1B) {
                RandHex(serial, 12);
                PatchSmbiosString(cur, t->SerialNumber, serial);
                RandHex(manufacturer, 10);
                PatchSmbiosString(cur, t->Manufacturer, manufacturer);
                RandHex(version, 12);
                PatchSmbiosString(cur, t->PartNumber, version);
                patched++;
                PrintLine(L"    [+] Type 17 (Memory) patched");
            }
            break;
        }
        default:
            break;
        }
        offset += GetSmbiosStructSize(cur);
    }

    PrintLine(L"[+] SMBIOS patching complete");
}
