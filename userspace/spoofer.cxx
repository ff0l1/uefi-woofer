#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NETCLASS_KEY "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}"
#define CRYPTO_KEY   "SOFTWARE\\Microsoft\\Cryptography"

static UINT64 g_seed;

static UINT64 XorShift64(void) {
    UINT64 x = g_seed;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    g_seed = x; return x;
}

static void SeedRng(void) {
    g_seed = (UINT64)time(NULL) ^ (UINT64)GetTickCount64() ^ 0xDEADBEEFCAFEBABEULL;
    if (!g_seed) g_seed = 0x1234567890ABCDEFULL;
}

static void RandMac(char *buf) {
    sprintf(buf, "02%02X%02X%02X%02X%02X",
        (unsigned)(XorShift64() % 256), (unsigned)(XorShift64() % 256),
        (unsigned)(XorShift64() % 256), (unsigned)(XorShift64() % 256),
        (unsigned)(XorShift64() % 256));
}

static int SpoofMacAddresses(void) {
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, NETCLASS_KEY, 0, KEY_READ|KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS) { printf("[-] Cannot open network registry key\n"); return 0; }

    int index = 0, spoofed = 0;
    char subKey[256]; DWORD subKeyLen;

    while (1) {
        subKeyLen = sizeof(subKey);
        if (RegEnumKeyExA(hKey, index++, subKey, &subKeyLen, 0,0,0,0) != ERROR_SUCCESS) break;

        HKEY hSub;
        if (RegOpenKeyExA(hKey, subKey, 0, KEY_READ|KEY_WRITE, &hSub) != ERROR_SUCCESS) continue;

        char desc[256]; DWORD descLen = sizeof(desc);
        if (RegQueryValueExA(hSub, "DriverDesc", 0,0, (LPBYTE)desc, &descLen) != ERROR_SUCCESS) {
            RegCloseKey(hSub); continue;
        }

        char mac[16]; RandMac(mac);
        if (RegSetValueExA(hSub, "NetworkAddress", 0, REG_SZ, (LPBYTE)mac, (DWORD)(strlen(mac)+1)) == ERROR_SUCCESS) {
            printf("[+] MAC: %s -> %s\n", desc, mac);
            spoofed++;
        }
        RegCloseKey(hSub);
    }
    RegCloseKey(hKey);

    if (spoofed) {
        printf("[*] Restarting network adapters...\n");
        system("netsh interface set interface \"Ethernet\" admin=disable 2>nul");
        system("netsh interface set interface \"Ethernet\" admin=enable 2>nul");
        system("netsh interface set interface \"Wi-Fi\" admin=disable 2>nul");
        system("netsh interface set interface \"Wi-Fi\" admin=enable 2>nul");
        printf("[+] Network adapters restarted\n");
    }
    return spoofed;
}

static int SpoofVolumeSerial(char drive) {
    char path[8]; sprintf(path, "\\\\.\\%c:", drive);
    HANDLE h = CreateFileA(path, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (h == INVALID_HANDLE_VALUE) { printf("[-] Cannot open %c:\n", drive); return 0; }

    DWORD br; BOOL locked = DeviceIoControl(h, FSCTL_LOCK_VOLUME, 0,0, 0,0, &br, 0);

    BYTE sec[512]; DWORD got;
    if (!ReadFile(h, sec, 512, &got, 0) || got != 512) {
        if (locked) DeviceIoControl(h, FSCTL_UNLOCK_VOLUME, 0,0, 0,0, &br, 0);
        CloseHandle(h); return 0;
    }

    DWORD oldS = 0, newS = (DWORD)XorShift64();
    int off = -1;

    if (!memcmp(sec+3, "NTFS", 4)) off = 0x48;
    else if (!memcmp(sec+0x52, "FAT32", 5)) off = 0x67;
    else if (!memcmp(sec+0x36, "FAT", 3)) off = 0x27;

    if (off < 0) {
        printf("[-] Unknown FS on %c:\n", drive);
        if (locked) DeviceIoControl(h, FSCTL_UNLOCK_VOLUME, 0,0, 0,0, &br, 0);
        CloseHandle(h); return 0;
    }

    memcpy(&oldS, sec+off, 4);
    memcpy(sec+off, &newS, 4);

    SetFilePointer(h, 0, 0, FILE_BEGIN);
    DWORD wrote;
    if (!WriteFile(h, sec, 512, &wrote, 0) || wrote != 512) {
        printf("[-] Cannot write %c: boot sector\n", drive);
        if (locked) DeviceIoControl(h, FSCTL_UNLOCK_VOLUME, 0,0, 0,0, &br, 0);
        CloseHandle(h); return 0;
    }
    FlushFileBuffers(h);
    if (locked) DeviceIoControl(h, FSCTL_UNLOCK_VOLUME, 0,0, 0,0, &br, 0);
    CloseHandle(h);

    printf("[+] %c: serial: %08X -> %08X\n", drive, oldS, newS);
    return 1;
}

static int SpoofAllVolumeSerials(void) {
    printf("[*] Spoofing volume serials...\n");
    int n = 0;
    for (char d = 'C'; d <= 'Z'; d++) {
        char rp[4]; sprintf(rp, "%c:\\", d);
        if (GetDriveTypeA(rp) == DRIVE_FIXED) n += SpoofVolumeSerial(d);
    }
    return n;
}

static int SpoofMachineGuid(void) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, CRYPTO_KEY, 0, KEY_READ|KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        printf("[-] Cannot open Cryptography key\n"); return 0;
    }
    GUID g; CoCreateGuid(&g);
    char buf[40];
    sprintf(buf, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1],
        g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
    LONG r = RegSetValueExA(hKey, "MachineGuid", 0, REG_SZ, (LPBYTE)buf, (DWORD)(strlen(buf)+1));
    RegCloseKey(hKey);
    if (r == ERROR_SUCCESS) { printf("[+] MachineGuid: %s\n", buf); return 1; }
    printf("[-] Failed to set MachineGuid\n"); return 0;
}

static int SpoofProductId(void) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ|KEY_WRITE, &hKey) != ERROR_SUCCESS) return 0;
    char pid[24];
    sprintf(pid, "%05d-%03d-%07d-%05d", (int)(XorShift64()%99999), (int)(XorShift64()%999), (int)(XorShift64()%9999999), (int)(XorShift64()%99999));
    LONG r = RegSetValueExA(hKey, "ProductId", 0, REG_SZ, (LPBYTE)pid, (DWORD)(strlen(pid)+1));
    DWORD dt = (DWORD)(XorShift64() & 0x7FFFFFFF);
    RegSetValueExA(hKey, "InstallDate", 0, REG_DWORD, (LPBYTE)&dt, sizeof(dt));
    RegCloseKey(hKey);
    if (r == ERROR_SUCCESS) { printf("[+] ProductId: %s\n", pid); return 1; }
    return 0;
}

int main(int argc, char *argv[]) {
    printf("\n========================================\n");
    printf("  HWID Spoofer - userspace\n");
    printf("  ff0l\n");
    printf("========================================\n\n");

    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY auth = SECURITY_NT_AUTHORITY;
    PSID adminSid = NULL;
    if (AllocateAndInitializeSid(&auth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &adminSid)) {
        CheckTokenMembership(NULL, adminSid, &isAdmin);
        FreeSid(adminSid);
    }
    if (!isAdmin) {
        printf("[-] Must run as Administrator!\n");
        system("pause");
        return 1;
    }

    SeedRng();

    printf("[*] Spoofing MAC addresses...\n");
    int mac = SpoofMacAddresses();
    int vol = SpoofAllVolumeSerials();
    int guid = SpoofMachineGuid();
    int pid = SpoofProductId();

    printf("\n========================================\n");
    printf("  Results:\n");
    printf("    MAC addresses:  %d\n", mac);
    printf("    Volume serials: %d\n", vol);
    printf("    Machine GUID:   %s\n", guid ? "OK" : "FAIL");
    printf("    Product ID:     %s\n", pid ? "OK" : "FAIL");
    printf("========================================\n");
    printf("\n[*] Done. Reboot for changes to take effect.\n");
    printf("[*] Disk drive serials CANNOT be spoofed without a kernel driver.\n");

    if (argc > 1 && !strcmp(argv[1], "--silent")) return 0;
    system("pause");
    return 0;
}
