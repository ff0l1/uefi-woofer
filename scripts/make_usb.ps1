$RepoRoot = Split-Path -Parent $PSScriptRoot
$efiFile = Join-Path $RepoRoot "dist\BOOTX64.EFI"

if (-not (Test-Path $efiFile)) {
    Write-Host "[-] Missing dist\BOOTX64.EFI. Run build.bat first." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================"
Write-Host "  HWID Spoofer - USB"
Write-Host "========================================"
Write-Host ""

$drives = Get-CimInstance -ClassName Win32_DiskDrive | Where-Object { $_.MediaType -match "Removable|External" -or $_.InterfaceType -eq "USB" }

if (-not $drives) {
    $drives = Get-CimInstance -ClassName Win32_DiskDrive | Where-Object { $_.Size -lt 64GB }
}

if (-not $drives) {
    Write-Host "[-] No drives found." -ForegroundColor Red
    exit 1
}

Write-Host "Drives:"
$i = 0
foreach ($d in $drives) {
    $sizeGB = [math]::Round($d.Size / 1GB, 1)
    Write-Host "  [$i] $($d.Model) - $sizeGB GB (disk $($d.Index))"
    $i++
}

Write-Host ""
$selection = Read-Host "Pick number (q quit)"

if ($selection -eq 'q') { exit 0 }

$disk = $drives[[int]$selection]
if (-not $disk) {
    Write-Host "[-] Bad selection." -ForegroundColor Red
    exit 1
}

$diskNum = $disk.Index
$sizeGB = [math]::Round($disk.Size / 1GB, 1)
Write-Host ""
Write-Host "Erases disk $diskNum - $($disk.Model) ($sizeGB GB)"
$confirm = Read-Host "Type YES to continue"

if ($confirm -ne 'YES') {
    Write-Host "[*] Cancelled."
    exit 0
}

Write-Host ""
Write-Host "[1/4] diskpart clean/format..."

$diskpartScript = @"
select disk $diskNum
clean
convert gpt
create partition primary
format fs=fat32 quick
assign
exit
"@

$diskpartScript | Out-File -FilePath "$env:TEMP\dp_script.txt" -Encoding ASCII
diskpart /s "$env:TEMP\dp_script.txt" 2>&1 | Out-Null
Remove-Item "$env:TEMP\dp_script.txt"

Start-Sleep -Seconds 2
$vol = Get-Volume | Where-Object { $_.DriveType -eq 'Removable' -and $_.DriveLetter } | Sort-Object DriveLetter | Select-Object -Last 1

if (-not $vol -or -not $vol.DriveLetter) {
    $vol = Get-Volume | Where-Object { $_.FileSystem -eq 'FAT32' -and $_.DriveLetter } | Sort-Object DriveLetter | Select-Object -Last 1
}

if (-not $vol -or -not $vol.DriveLetter) {
    Write-Host "[-] No drive letter after format." -ForegroundColor Red
    exit 1
}

$driveLetter = $vol.DriveLetter
Write-Host "[+] $driveLetter`: FAT32"

Write-Host "[2/4] \\EFI\\BOOT..."
$efiBootDir = "${driveLetter}:\EFI\BOOT"
New-Item -ItemType Directory -Path $efiBootDir -Force | Out-Null

Write-Host "[3/4] Copy BOOTX64.EFI..."
Copy-Item -Path $efiFile -Destination "$efiBootDir\BOOTX64.EFI" -Force

Write-Host "[4/4] Check..."
if (Test-Path "$efiBootDir\BOOTX64.EFI") {
    $size = (Get-Item "$efiBootDir\BOOTX64.EFI").Length
    Write-Host ""
    Write-Host "[+] USB ready: $driveLetter`:\EFI\BOOT\BOOTX64.EFI ($size bytes)" -ForegroundColor Green
    Write-Host "    Boot USB first in firmware. seed.txt on first run; delete to re-roll." -ForegroundColor Green
    Write-Host ""
} else {
    Write-Host "[-] Copy failed." -ForegroundColor Red
}
