$RepoRoot = Split-Path -Parent $PSScriptRoot
$efiFile = Join-Path $RepoRoot "dist\BOOTX64.EFI"
$espDrive = "S:"

Write-Host ""
Write-Host "========================================"
Write-Host "  HWID Spoofer - EFI Installer"
Write-Host "========================================"
Write-Host ""

$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "[-] Run as Administrator." -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $efiFile)) {
    Write-Host "[-] Missing dist\BOOTX64.EFI. Run build.bat first." -ForegroundColor Red
    exit 1
}

try {
    $secureBoot = Confirm-SecureBootUEFI
    if ($secureBoot) {
        Write-Host "[-] Secure Boot is on. Unsigned EFI won't load." -ForegroundColor Red
        Write-Host "    Disable it in firmware first." -ForegroundColor Yellow
        Write-Host ""
        $force = Read-Host "Install anyway? [y/N]"
        if ($force -ne 'y') { exit 0 }
    } else {
        Write-Host "[+] Secure Boot off" -ForegroundColor Green
    }
} catch {
    Write-Host "[?] Secure Boot check failed: $_" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "[1/5] Mounting ESP..."
mountvol $espDrive /s 2>&1 | Out-Null

if (-not (Test-Path "$espDrive\EFI\Microsoft\Boot")) {
    Write-Host "[-] ESP not at $espDrive, trying diskpart..." -ForegroundColor Red

    $dpScript = @"
list disk
select disk 0
list partition
select partition 1
assign letter=$($espDrive.Substring(0,1))
exit
"@
    $dpScript | Out-File "$env:TEMP\dp_mount.txt" -Encoding ASCII
    diskpart /s "$env:TEMP\dp_mount.txt" 2>&1 | Out-Null
    Remove-Item "$env:TEMP\dp_mount.txt"

    if (-not (Test-Path "$espDrive\EFI\Microsoft\Boot")) {
        Write-Host "[-] Could not mount ESP." -ForegroundColor Red
        exit 1
    }
}

$bootMgrPath = "$espDrive\EFI\Microsoft\Boot\bootmgfw.efi"
$backupPath = "$espDrive\EFI\Microsoft\Boot\bootmgfw_orig.efi"

if (Test-Path $backupPath) {
    Write-Host "[*] bootmgfw_orig.efi already exists" -ForegroundColor Yellow
    $reinstall = Read-Host "Reinstall using existing backup? [y/N]"
    if ($reinstall -ne 'y') {
        mountvol $espDrive /d 2>&1 | Out-Null
        exit 0
    }
} else {
    Write-Host "[2/5] Backing up bootmgfw.efi..."
    Copy-Item -Path $bootMgrPath -Destination $backupPath -Force
    Write-Host "[+] bootmgfw_orig.efi" -ForegroundColor Green
}

$backupSize = (Get-Item $backupPath).Length
Write-Host "[+] Backup: $backupSize bytes"
if ($backupSize -lt 100000) {
    Write-Host "[-] Backup too small, aborting." -ForegroundColor Red
    mountvol $espDrive /d 2>&1 | Out-Null
    exit 1
}

Write-Host "[3/5] Installing EFI..."
Copy-Item -Path $efiFile -Destination $bootMgrPath -Force
Write-Host "[+] bootmgfw.efi ($((Get-Item $efiFile).Length) bytes)" -ForegroundColor Green

Write-Host "[4/5] ESP marker..."
$spooferDir = "$espDrive\EFI\HWIDSpoofer"
if (-not (Test-Path $spooferDir)) {
    New-Item -ItemType Directory -Path $spooferDir -Force | Out-Null
}

@"
HWID Spoofer (ff0l) - $(Get-Date)
bootmgfw_orig.efi = original boot manager
"@ | Out-File "$spooferDir\installed.txt" -Encoding ASCII

Write-Host "[5/5] Unmounting ESP..."
mountvol $espDrive /d 2>&1 | Out-Null

Write-Host ""
Write-Host "[+] Done. Reboot to run the spoofer before Windows." -ForegroundColor Green
Write-Host "    Uninstall: scripts\uninstall_efi.bat" -ForegroundColor Green
Write-Host ""
Write-Host "    Recovery if boot breaks (WinPE cmd):"
Write-Host "    mountvol S: /s"
Write-Host "    copy S:\EFI\Microsoft\Boot\bootmgfw_orig.efi S:\EFI\Microsoft\Boot\bootmgfw.efi"
Write-Host ""
