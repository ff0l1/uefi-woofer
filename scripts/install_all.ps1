$RepoRoot = Split-Path -Parent $PSScriptRoot
$efiFile = Join-Path $RepoRoot "dist\BOOTX64.EFI"
$userspaceExe = Join-Path $RepoRoot "dist\spoofer.exe"
$installDir = "$env:ProgramData\HWIDSpoofer"
$espDrive = "S:"

Write-Host ""
Write-Host "========================================"
Write-Host "  HWID Spoofer - Full install"
Write-Host "========================================"
Write-Host ""

$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "[-] Run as Administrator." -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $efiFile)) { Write-Host "[-] Missing dist\BOOTX64.EFI. Run build.bat." -ForegroundColor Red; exit 1 }
if (-not (Test-Path $userspaceExe)) { Write-Host "[-] Missing dist\spoofer.exe. Run build.bat." -ForegroundColor Red; exit 1 }

try {
    $sb = Confirm-SecureBootUEFI
    if ($sb) {
        Write-Host "[-] Secure Boot is on. UEFI part won't run until it's off." -ForegroundColor Red
        $installUserspace = Read-Host "Userspace only? [y/N]"
        if ($installUserspace -ne 'y') { exit 0 }
        $skipUefi = $true
    } else {
        Write-Host "[+] Secure Boot off" -ForegroundColor Green
        $skipUefi = $false
    }
} catch {
    Write-Host "[?] Secure Boot check failed: $_" -ForegroundColor Yellow
    $skipUefi = $false
}

if (-not $skipUefi) {
    Write-Host ""
    Write-Host "=== UEFI ===" -ForegroundColor Cyan

    Write-Host "[1/4] Mounting ESP..."
    mountvol $espDrive /s 2>&1 | Out-Null

    if (-not (Test-Path "$espDrive\EFI\Microsoft\Boot")) {
        Write-Host "[-] Cannot mount ESP" -ForegroundColor Red
        $skipUefi = $true
    } else {
        $bootMgr = "$espDrive\EFI\Microsoft\Boot\bootmgfw.efi"
        $backup = "$espDrive\EFI\Microsoft\Boot\bootmgfw_orig.efi"

        if (Test-Path $backup) {
            Write-Host "[*] Using existing bootmgfw_orig.efi" -ForegroundColor Yellow
        } else {
            Write-Host "[2/4] Backing up bootmgfw.efi..."
            Copy-Item $bootMgr $backup -Force
            $bSize = (Get-Item $backup).Length
            Write-Host "[+] Backup: $bSize bytes" -ForegroundColor Green
            if ($bSize -lt 100000) {
                Write-Host "[-] Backup too small, skipping UEFI." -ForegroundColor Red
                $skipUefi = $true
            }
        }

        if (-not $skipUefi) {
            Write-Host "[3/4] Installing EFI..."
            Copy-Item $efiFile $bootMgr -Force
            Write-Host "[+] UEFI hooked" -ForegroundColor Green
            Write-Host "[4/4] Unmounting ESP..."
            mountvol $espDrive /d 2>&1 | Out-Null
        }
    }
}

Write-Host ""
Write-Host "=== Userspace ===" -ForegroundColor Cyan

Write-Host "[1/3] Copy to ProgramData..."
if (-not (Test-Path $installDir)) { New-Item -ItemType Directory -Path $installDir -Force | Out-Null }
Copy-Item $userspaceExe "$installDir\spoofer.exe" -Force
Write-Host "[+] $installDir\spoofer.exe" -ForegroundColor Green

Write-Host "[2/3] Run once..."
& "$installDir\spoofer.exe" --silent
Write-Host "[+] Done" -ForegroundColor Green

Write-Host "[3/3] Logon task..."
$action = New-ScheduledTaskAction -Execute "$installDir\spoofer.exe" -Argument "--silent"
$trigger = New-ScheduledTaskTrigger -AtLogOn -User $env:USERNAME
$settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -StartWhenAvailable
$principal = New-ScheduledTaskPrincipal -UserId "SYSTEM" -RunLevel Highest -LogonType ServiceAccount
Register-ScheduledTask -TaskName "HWIDSpoofer" -Action $action -Trigger $trigger -Settings $settings -Principal $principal -Force 2>&1 | Out-Null
Write-Host "[+] Task HWIDSpoofer" -ForegroundColor Green

Write-Host ""
Write-Host "[+] Install finished. Reboot." -ForegroundColor Green
if (-not $skipUefi) {
    Write-Host "    UEFI runs pre-Windows; userspace at logon." -ForegroundColor Green
}
Write-Host "    Uninstall: scripts\uninstall_all.bat"
Write-Host ""
