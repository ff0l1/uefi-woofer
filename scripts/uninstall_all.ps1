$installDir = "$env:ProgramData\HWIDSpoofer"
$espDrive = "S:"

Write-Host ""
Write-Host "========================================"
Write-Host "  HWID Spoofer - uninstall all"
Write-Host "========================================"
Write-Host ""

$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) { Write-Host "[-] Run as Administrator." -ForegroundColor Red; exit 1 }

Write-Host "[1/4] Task..."
Unregister-ScheduledTask -TaskName "HWIDSpoofer" -Confirm:$false -ErrorAction SilentlyContinue
Write-Host "[+] OK" -ForegroundColor Green

Write-Host "[2/4] ESP bootmgr..."
mountvol $espDrive /s 2>&1 | Out-Null
$bootMgr = "$espDrive\EFI\Microsoft\Boot\bootmgfw.efi"
$backup = "$espDrive\EFI\Microsoft\Boot\bootmgfw_orig.efi"

if (Test-Path $backup) {
    Copy-Item $backup $bootMgr -Force
    Remove-Item $backup -Force
    Write-Host "[+] bootmgfw restored" -ForegroundColor Green
} else {
    Write-Host "[*] No UEFI backup" -ForegroundColor Yellow
}
mountvol $espDrive /d 2>&1 | Out-Null

Write-Host "[3/4] ProgramData..."
if (Test-Path $installDir) {
    Remove-Item $installDir -Recurse -Force
    Write-Host "[+] Removed $installDir" -ForegroundColor Green
}

Write-Host "[4/4] Done."
Write-Host ""
Write-Host "[+] Reboot." -ForegroundColor Green
Write-Host ""
