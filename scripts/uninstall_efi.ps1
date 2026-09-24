$espDrive = "S:"

Write-Host ""
Write-Host "========================================"
Write-Host "  HWID Spoofer - EFI uninstall"
Write-Host "========================================"
Write-Host ""

$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "[-] Run as Administrator." -ForegroundColor Red
    exit 1
}

Write-Host "[1/3] Mount ESP..."
mountvol $espDrive /s 2>&1 | Out-Null

$bootMgrPath = "$espDrive\EFI\Microsoft\Boot\bootmgfw.efi"
$backupPath = "$espDrive\EFI\Microsoft\Boot\bootmgfw_orig.efi"

if (-not (Test-Path $backupPath)) {
    Write-Host "[-] bootmgfw_orig.efi missing." -ForegroundColor Red
    mountvol $espDrive /d 2>&1 | Out-Null
    exit 1
}

Write-Host "[2/3] Restore bootmgfw.efi..."
Copy-Item -Path $backupPath -Destination $bootMgrPath -Force
Write-Host "[+] Restored" -ForegroundColor Green

Write-Host "[3/3] Cleanup..."
Remove-Item $backupPath -Force -ErrorAction SilentlyContinue
$spooferDir = "$espDrive\EFI\HWIDSpoofer"
if (Test-Path $spooferDir) {
    Remove-Item $spooferDir -Recurse -Force -ErrorAction SilentlyContinue
}

mountvol $espDrive /d 2>&1 | Out-Null

Write-Host ""
Write-Host "[+] Done. Reboot." -ForegroundColor Green
Write-Host ""
