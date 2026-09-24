#include "credit.hxx"
#include "efi_types.hxx"
#include "efi_boot.hxx"
#include "efi_smbios.hxx"

extern EFI_SYSTEM_TABLE *g_ST;
extern EFI_BOOT_SERVICES *g_BS;
extern EFI_HANDLE g_ImageHandle;
extern void Print(const CHAR16 *s);
extern void PrintLine(const CHAR16 *s);
extern void MemCpy(VOID *dst, VOID *src, UINTN n);
extern void MemSet(VOID *dst, UINT8 val, UINTN n);
extern UINTN StrLen(CHAR16 *s);
extern BOOLEAN GuidEqual(EFI_GUID *a, EFI_GUID *b);
extern void SeedRng(UINT64 seed);
extern UINT64 XorShift64(void);
extern UINT64 GetTimerSeed(void);

extern void PatchSmbiosTables(void);

static EFI_STATUS OpenBootVolume(EFI_FILE_PROTOCOL **Root) {
    EFI_GUID loadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;
    EFI_STATUS status;

    status = g_BS->HandleProtocol(g_ImageHandle, &loadedImageGuid, (VOID**)&loadedImage);
    if (status) return status;

    status = g_BS->HandleProtocol(loadedImage->DeviceHandle, &fsGuid, (VOID**)&fs);
    if (status) return status;

    return fs->OpenVolume(fs, Root);
}

static UINT64 ReadSeedFile(void) {
    EFI_FILE_PROTOCOL *root;
    EFI_FILE_PROTOCOL *file;
    EFI_STATUS status;

    status = OpenBootVolume(&root);
    if (status) {
        PrintLine(L"[!] Cannot open boot volume for seed");
        return 0;
    }

    status = root->Open(root, &file, L"seed.txt", EFI_FILE_MODE_READ, 0);
    if (status) {
        PrintLine(L"[*] No seed file found, will create new");
        root->Close(root);
        return 0;
    }

    UINT8 buf[32];
    UINTN bufSize = 31;
    MemSet(buf, 0, 32);
    status = file->Read(file, &bufSize, buf);
    file->Close(file);
    root->Close(root);

    if (status || bufSize == 0) return 0;

    UINT64 seed = 0;
    for (UINTN i = 0; i < bufSize && i < 16; i++) {
        UINT8 c = buf[i];
        UINT64 v;
        if (c >= '0' && c <= '9') v = c - '0';
        else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
        else break;
        seed = (seed << 4) | v;
    }

    return seed;
}

static void WriteSeedFile(UINT64 seed) {
    EFI_FILE_PROTOCOL *root;
    EFI_FILE_PROTOCOL *file;
    EFI_STATUS status;

    status = OpenBootVolume(&root);
    if (status) {
        PrintLine(L"[!] Cannot open boot volume for seed write");
        return;
    }

    status = root->Open(root, &file, L"seed.txt",
                        EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0);
    if (status) {
        root->Close(root);
        PrintLine(L"[!] Cannot create seed file");
        return;
    }

    CHAR8 buf[20];
    static CHAR8 hex[] = "0123456789ABCDEF";
    for (int i = 15; i >= 0; i--) {
        buf[i] = hex[seed & 0xF];
        seed >>= 4;
    }
    buf[16] = '\n';
    buf[17] = 0;

    UINTN bufSize = 17;
    file->Write(file, &bufSize, buf);
    file->Close(file);
    root->Close(root);

    PrintLine(L"[+] Seed file written");
}

static EFI_STATUS LoadFileFromFs(EFI_HANDLE fsHandle, CHAR16 *path,
                                  VOID **buffer, UINTN *size) {
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;
    EFI_FILE_PROTOCOL *root;
    EFI_FILE_PROTOCOL *file;
    EFI_STATUS status;

    status = g_BS->HandleProtocol(fsHandle, &fsGuid, (VOID**)&fs);
    if (status) return status;

    status = fs->OpenVolume(fs, &root);
    if (status) return status;

    status = root->Open(root, &file, path, EFI_FILE_MODE_READ, 0);
    if (status) {
        root->Close(root);
        return status;
    }

    UINTN bufSize = 0x100000;
    UINT8 *buf;
    status = g_BS->AllocatePool(EfiLoaderData, bufSize, (VOID**)&buf);
    if (status) {
        file->Close(file);
        root->Close(root);
        return status;
    }

    UINTN totalRead = 0;
    while (1) {
        UINTN toRead = bufSize - totalRead;
        if (toRead == 0) {
            UINT8 *newBuf;
            UINTN newSize = bufSize * 2;
            status = g_BS->AllocatePool(EfiLoaderData, newSize, (VOID**)&newBuf);
            if (status) {
                g_BS->FreePool(buf);
                file->Close(file);
                root->Close(root);
                return status;
            }
            MemCpy(newBuf, buf, totalRead);
            g_BS->FreePool(buf);
            buf = newBuf;
            bufSize = newSize;
            toRead = bufSize - totalRead;
        }
        UINTN bytesRead = toRead;
        status = file->Read(file, &bytesRead, buf + totalRead);
        if (status) {
            g_BS->FreePool(buf);
            file->Close(file);
            root->Close(root);
            return status;
        }
        totalRead += bytesRead;
        if (bytesRead < toRead) break;
    }

    file->Close(file);
    root->Close(root);

    *buffer = buf;
    *size = totalRead;
    return 0;
}

static EFI_STATUS ChainLoadWindows(void) {
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_HANDLE *handles;
    UINTN numHandles;
    EFI_STATUS status;

    status = g_BS->LocateHandleBuffer(ByProtocol, &fsGuid, NULL, &numHandles, &handles);
    if (status) {
        PrintLine(L"[!] Cannot enumerate file systems");
        return status;
    }

    PrintLine(L"[*] Searching for Windows Boot Manager...");

    CHAR16 *paths[2] = {
        L"\\EFI\\Microsoft\\Boot\\bootmgfw_orig.efi",
        L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi"
    };

    for (UINTN i = 0; i < numHandles; i++) {
        for (int p = 0; p < 2; p++) {
            VOID *fileBuf = NULL;
            UINTN fileSize = 0;

            status = LoadFileFromFs(handles[i], paths[p], &fileBuf, &fileSize);
            if (status) continue;

            if (fileSize < 50000) {
                g_BS->FreePool(fileBuf);
                continue;
            }

            PrintLine(L"[+] Windows Boot Manager found!");

            EFI_HANDLE hImage;
            status = g_BS->LoadImage(FALSE, g_ImageHandle, NULL, fileBuf, fileSize, &hImage);
            if (status) {
                PrintLine(L"[!] LoadImage failed");
                g_BS->FreePool(fileBuf);
                continue;
            }

            g_BS->FreePool(fileBuf);

            PrintLine(L"[+] Starting Windows Boot Manager...");
            g_BS->Stall(1000000);

            status = g_BS->StartImage(hImage, NULL, NULL);
            if (status) {
                PrintLine(L"[!] StartImage failed");
            }
            return status;
        }
    }

    PrintLine(L"[!] Windows Boot Manager not found on any volume!");
    return EFI_NOT_FOUND;
}

extern "C" EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    g_ST = SystemTable;
    g_BS = SystemTable->BootServices;
    g_ImageHandle = ImageHandle;

    g_ST->ConOut->ClearScreen(g_ST->ConOut);

    PrintLine(L"");
    PrintLine(L"========================================");
    PrintLine(L"  HWID Spoofer");
    Print(L"  ");
    PrintLine(PROJECT_AUTHOR);
    PrintLine(L"========================================");
    PrintLine(L"");

    UINT64 seed = ReadSeedFile();
    if (seed == 0) {
        PrintLine(L"[*] Generating new random seed...");
        seed = GetTimerSeed();
        WriteSeedFile(seed);
    } else {
        PrintLine(L"[+] Using existing seed from file");
    }

    SeedRng(seed);

    PrintLine(L"[*] Patching SMBIOS tables...");
    PatchSmbiosTables();

    PrintLine(L"");
    PrintLine(L"[+] Spoofing complete. Loading Windows...");
    PrintLine(L"");

    g_BS->SetWatchdogTimer(0, 0, 0, NULL);

    EFI_STATUS status = ChainLoadWindows();
    if (status) {
        PrintLine(L"[!] Failed to chain-load Windows!");
        PrintLine(L"[*] System will pause. Press any key to continue boot...");
        UINTN index;
        g_BS->WaitForEvent(1, &g_ST->ConIn->WaitForKey, &index);
    }

    return status;
}
