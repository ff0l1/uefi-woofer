#include "efi_types.hxx"
#include "efi_boot.hxx"

EFI_SYSTEM_TABLE *g_ST;
EFI_BOOT_SERVICES *g_BS;
EFI_HANDLE g_ImageHandle;

UINTN StrLen(CHAR16 *s) {
    UINTN len = 0;
    while (s[len]) len++;
    return len;
}

UINTN StrLen8(CHAR8 *s) {
    UINTN len = 0;
    while (s[len]) len++;
    return len;
}

void MemCpy(VOID *dst, VOID *src, UINTN n) {
    UINT8 *d = (UINT8*)dst;
    UINT8 *s = (UINT8*)src;
    while (n--) *d++ = *s++;
}

void MemSet(VOID *dst, UINT8 val, UINTN n) {
    UINT8 *d = (UINT8*)dst;
    while (n--) *d++ = val;
}

BOOLEAN GuidEqual(EFI_GUID *a, EFI_GUID *b) {
    if (a->Data1 != b->Data1) return FALSE;
    if (a->Data2 != b->Data2) return FALSE;
    if (a->Data3 != b->Data3) return FALSE;
    for (int i = 0; i < 8; i++)
        if (a->Data4[i] != b->Data4[i]) return FALSE;
    return TRUE;
}

void Print(const CHAR16 *s) {
    g_ST->ConOut->OutputString(g_ST->ConOut, const_cast<CHAR16*>(s));
}

void PrintLine(const CHAR16 *s) {
    Print(s);
    static const CHAR16 crlf[] = { '\r','\n',0 };
    Print(crlf);
}

void PrintHex(UINT64 val) {
    CHAR16 buf[20];
    static const CHAR16 hex[] = {
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',0
    };
    for (int i = 15; i >= 0; i--) {
        buf[i] = hex[val & 0xF];
        val >>= 4;
    }
    buf[16] = 0;
    PrintLine(buf);
}

static UINT64 g_Seed;

UINT64 XorShift64(void) {
    UINT64 x = g_Seed;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    g_Seed = x;
    return x;
}

void SeedRng(UINT64 seed) {
    g_Seed = seed;
    if (g_Seed == 0) g_Seed = 0xDEADBEEFCAFEBABEULL;
}

CHAR8 RandChar(void) {
    static CHAR8 charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    return charset[XorShift64() % 36];
}

void RandHex(CHAR8 *buf, int len) {
    for (int i = 0; i < len; i++)
        buf[i] = RandChar();
    buf[len] = 0;
}

UINT8 RandByte(void) {
    return (UINT8)(XorShift64() & 0xFF);
}

UINT64 GetTimerSeed(void) {
    UINT8 timeBuf[16];
    MemSet(timeBuf, 0, 16);
    if (g_ST->RuntimeServices) {
        g_ST->RuntimeServices->GetTime(timeBuf, NULL);
    }
    UINT64 seed = 0;
    MemCpy(&seed, timeBuf, 8);
    if (seed == 0) seed = 0x1234567890ABCDEFULL;
    return seed;
}
