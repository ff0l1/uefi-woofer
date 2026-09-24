# UEFI HWID Spoofer

**ff0l**

Two-stage hardware identifier spoofing for Windows: a **pre-OS UEFI application** patches in-memory SMBIOS tables before the kernel reads them, and an optional **userspace tool** adjusts OS-visible identifiers (MAC, volume serial, registry GUIDs).

Built with freestanding GCC (no EDK2). Install scripts target the EFI System Partition (ESP) or a bootable USB.

---

## How it works

### Boot flow (UEFI component)

```
Firmware loads bootmgfw.efi (this project when installed)
        │
        ├─ Read seed.txt on boot volume (or create new seed)
        ├─ Patch SMBIOS structures in RAM
        └─ Chain-load original Windows Boot Manager (bootmgfw_orig.efi / bootmgfw.efi)
                │
                ▼
           Windows boots with spoofed firmware tables
```

The UEFI app does **not** modify flash NVRAM or on-disk SMBIOS blobs permanently. Changes exist only for that boot session in the copy of the table the firmware/OS consumes.

### SMBIOS (UEFI)

The spoofer locates the SMBIOS entry point via the EFI system configuration table (SMBIOS 3.0 or legacy), walks structures, and patches string fields in place. Replacement strings must not exceed the original length (padding with nulls).

| Type | Structure        | Fields touched                                      |
|------|------------------|-----------------------------------------------------|
| 1    | System           | Manufacturer, product, version, serial, UUID        |
| 2    | Baseboard        | Manufacturer, product, version, serial              |
| 3    | Chassis          | Manufacturer, version, serial                       |
| 4    | Processor        | Manufacturer, serial (when structure is large enough)|
| 17   | Memory device    | Manufacturer, part number, serial                   |

Random values come from an xorshift64 PRNG seeded from `seed.txt` or, on first run, from the UEFI runtime clock.

### Userspace component

Runs on Windows as Administrator. It does **not** replace the UEFI layer; it complements it for identifiers Windows reads from elsewhere:

| Target            | Mechanism                                      |
|-------------------|------------------------------------------------|
| Network MAC       | `NetworkAddress` under the class net adapter key |
| Volume serial     | Boot sector patch (NTFS / FAT32 / FAT)         |
| Machine GUID      | `HKLM\SOFTWARE\Microsoft\Cryptography`         |
| Product ID        | `HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion` (+ InstallDate) |

**Not covered without a kernel driver:** physical disk serials (SMART / storage stack), GPU PCI IDs, and other kernel-enumerated hardware.

### Persistence of identity

- **`seed.txt`** on the boot volume (ESP or USB). Same seed gives the same SMBIOS spoof until you delete the file.
- **Userspace** runs on demand; `install_all` also drops a logon scheduled task.

---

## Repository layout

```
├── build.bat                 Build UEFI + userspace into dist/
├── uefi/                     Freestanding EFI application
│   ├── main                  Entry, seed I/O, chain-load Windows
│   ├── efi_lib               Console, memory, RNG
│   ├── smbios_patch          SMBIOS walk + patch
│   └── include/              EFI headers, credit
├── userspace/
│   └── spoofer               Windows registry / volume / MAC logic
├── scripts/                  Install, uninstall, USB creator (Admin)
└── tools/                    build_uefi.bat, build_userspace.bat
```

Build artifacts (gitignored):

```
dist/
├── BOOTX64.EFI
├── spoofer.exe
└── obj/                      Object files
```

---

## Requirements

- **Build:** Windows, x86_64 MinGW-w64 `gcc` (MSYS2 `mingw-w64-ucrt-x86_64-gcc` is fine)
- **Target machine:** UEFI firmware, Secure Boot off for the EFI binary
- **Install scripts:** elevated cmd/PowerShell

`build.bat` looks for `gcc` on `PATH`, then common MSYS2/MinGW install paths. Override with:

```bat
set MINGW_BIN=C:\msys64\ucrt64\bin
build.bat
```

---

## Build

From the repo root (cmd or `cmd /c build.bat` from PowerShell):

```bat
build.bat
```

Output: `dist\BOOTX64.EFI` and `dist\spoofer.exe`.

Single targets:

```bat
tools\build_uefi.bat
tools\build_userspace.bat
```

---

## Quick start

1. Build from the repo root: `build.bat`
2. Right-click a script under `scripts\` → **Run as administrator** (or elevated cmd)
3. Reboot after ESP/USB install so the UEFI piece runs before Windows

Each `.bat` in `scripts\` is a wrapper around the matching `.ps1` (same name). Use the `.bat` unless you prefer PowerShell directly.

---

## Scripts (`scripts\`)

Run these from the repo root, or double-click the `.bat` in Explorer (still needs Admin where noted).

| Script | Purpose |
|--------|---------|
| `install_all.bat` | ESP hook + copy `dist\spoofer.exe` to `%ProgramData%\HWIDSpoofer`, run once, register logon task `HWIDSpoofer` |
| `install_efi.bat` | ESP hook only: backup `bootmgfw.efi` → `bootmgfw_orig.efi`, install built EFI as `bootmgfw.efi` |
| `make_usb.bat` | Wipe selected USB disk, FAT32 GPT, copy `dist\BOOTX64.EFI` → `\EFI\BOOT\BOOTX64.EFI` |
| `uninstall_all.bat` | Remove logon task, restore `bootmgfw.efi` from backup, delete `%ProgramData%\HWIDSpoofer` |
| `uninstall_efi.bat` | Restore boot manager from `bootmgfw_orig.efi` only, remove `EFI\HWIDSpoofer` marker |

| Requirement | Install scripts | USB script |
|-------------|-----------------|------------|
| Administrator | Yes | Yes (diskpart) |
| `build.bat` finished | Yes (`dist\` must exist) | Yes (`dist\BOOTX64.EFI`) |
| Secure Boot | Off for UEFI hook / USB boot | Off |

PowerShell (same logic, no `.bat` pause):

```powershell
cd path\to\repo
powershell -ExecutionPolicy Bypass -NoProfile -File .\scripts\install_all.ps1
```

---

## How to use (pick one path)

### Full stack (UEFI + userspace)

Best if Secure Boot is off and you want SMBIOS + registry/MAC/volume spoofing.

```bat
build.bat
scripts\install_all.bat
```

Reboot. On boot: UEFI console patches SMBIOS, then Windows loads. At logon: `HWIDSpoofer` task runs `spoofer.exe --silent`.

### UEFI only (ESP, no userspace install)

Does not touch `%ProgramData%` or scheduled tasks. Only replaces the boot manager on the ESP.

```bat
build.bat
scripts\install_efi.bat
```

Reboot. Same pre-Windows SMBIOS patch; run `dist\spoofer.exe` yourself if you want userspace changes.

### USB boot (no ESP changes)

No `bootmgfw.efi` swap on the internal drive. Boot from USB each time (or when you need a spoof session).

```bat
build.bat
scripts\make_usb.bat
```

Pick the disk when prompted (types `YES` to confirm). In firmware, boot USB first. First run creates `seed.txt` on the USB; delete it to get new SMBIOS values.

### Userspace only (no UEFI install)

No ESP hook. Does not patch SMBIOS at boot.

```bat
build.bat
dist\spoofer.exe
```

Silent (no pause at exit):

```bat
dist\spoofer.exe --silent
```

If `install_all` skipped UEFI because Secure Boot was on, it can still install userspace when you confirm the prompt.

---

## Uninstall

```bat
scripts\uninstall_all.bat
scripts\uninstall_efi.bat
```

Use `uninstall_all` if you ran `install_all`. Use `uninstall_efi` if you only ran `install_efi`. USB installs do not need uninstall (remove the stick or delete `\EFI\BOOT\BOOTX64.EFI`).

ESP recovery if Windows will not boot (WinPE or install media cmd):

```bat
mountvol S: /s
copy S:\EFI\Microsoft\Boot\bootmgfw_orig.efi S:\EFI\Microsoft\Boot\bootmgfw.efi
```

---

## Secure Boot

If `Confirm-SecureBootUEFI` reports enabled, the UEFI binary will not execute. Disable Secure Boot in firmware, or use userspace-only installation when the all-in-one installer offers it.

---

## Disclaimer

Your machine. Back up `bootmgfw.efi` before touching the ESP. Wrong backup = no boot until you restore from WinPE.
