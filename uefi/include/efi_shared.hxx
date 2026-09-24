#ifndef _EFI_SHARED_H_
#define _EFI_SHARED_H_

#include "efi_types.hxx"
#include "efi_boot.hxx"

extern EFI_SYSTEM_TABLE *g_ST;
extern EFI_BOOT_SERVICES *g_BS;
extern EFI_HANDLE g_ImageHandle;

UINTN StrLen(CHAR16 *s);
UINTN StrLen8(CHAR8 *s);
void MemCpy(VOID *dst, VOID *src, UINTN n);
void MemSet(VOID *dst, UINT8 val, UINTN n);
BOOLEAN GuidEqual(EFI_GUID *a, EFI_GUID *b);
void Print(const CHAR16 *s);
void PrintLine(const CHAR16 *s);

UINT64 XorShift64(void);
void SeedRng(UINT64 seed);
CHAR8 RandChar(void);
void RandHex(CHAR8 *buf, int len);
UINT8 RandByte(void);
UINT64 GetTimerSeed(void);

void PatchSmbiosTables(void);

#endif
