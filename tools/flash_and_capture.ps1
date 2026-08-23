param(
  [Parameter(Mandatory = $true)][int]$Demo,
  [string]$Port = "COM3",
  [int]$Baud = 115200,
  [int]$Seconds = 6,
  [string]$Preset = "Performance",
  [switch]$EnableFaultTrigger
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot ".."))
$cli = "C:\ST\STM32CubeCLT_1.21.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
$secureElf = Join-Path $root "Secure\build\U585_S.elf"
$nsElf = Join-Path $root "NonSecure\build\U585_NS.elf"
$outDir = Join-Path $root "docs\superpowers\measured"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null
$outSuffix = if ($EnableFaultTrigger) { "-fault" } else { "" }
$outFile = Join-Path $outDir ("demo{0:D2}-com3{1}.txt" -f $Demo, $outSuffix)

$buildType = if ($Preset -eq "Performance") { "Performance" } else { "Debug" }
$faultFlag = if ($EnableFaultTrigger) { "ON" } else { "OFF" }

Write-Host "=== Build Secure + NonSecure demo $Demo (fault=$faultFlag) ==="
Push-Location $root
cmake --preset $Preset "-DU585_ACTIVE_DEMO=$Demo" "-DU585_EXP04_ENABLE_FAULT_TRIGGER=$faultFlag" | Out-Host
cmake --build --preset $Preset --target U585_S | Out-Host
cmake -S NonSecure -B NonSecure/build -G Ninja "-DCMAKE_TOOLCHAIN_FILE=$root/gcc-arm-none-eabi.cmake" "-DCMAKE_BUILD_TYPE=$buildType" "-DU585_ACTIVE_DEMO=$Demo" "-DU585_EXP04_ENABLE_FAULT_TRIGGER=$faultFlag" | Out-Host
cmake --build NonSecure/build | Out-Host
if ($LASTEXITCODE -ne 0) {
  throw "Build failed with exit $LASTEXITCODE"
}
Pop-Location

Write-Host "=== Flash ==="
& $cli -c port=SWD freq=4000 mode=UR -d $secureElf -d $nsElf
if ($LASTEXITCODE -ne 0) {
  throw "Flash failed with exit $LASTEXITCODE"
}

Write-Host "=== Capture $Port ${Seconds}s (reset after open) ==="
$lines = New-Object System.Collections.Generic.List[string]
try {
  $sp = [System.IO.Ports.SerialPort]::new($Port, $Baud, "None", 8, "One")
  $sp.NewLine = "`r`n"
  $sp.ReadTimeout = 200
  $sp.DtrEnable = $true
  $sp.RtsEnable = $true
  $sp.Open()
  Start-Sleep -Milliseconds 100
  try { [void]$sp.ReadExisting() } catch {}
  & $cli -c port=SWD freq=4000 mode=UR -hardRst | Out-Null
  Start-Sleep -Milliseconds 300
  $deadline = [DateTime]::UtcNow.AddSeconds($Seconds)
  while ([DateTime]::UtcNow -lt $deadline) {
    try {
      $chunk = $sp.ReadExisting()
      if (-not [string]::IsNullOrEmpty($chunk)) {
        foreach ($line in ($chunk -split "`r?`n")) {
          $line = $line.TrimEnd()
          if ($line.Length -gt 0) { $lines.Add($line) }
        }
      }
    } catch {
    }
    Start-Sleep -Milliseconds 50
  }
  $sp.Close()
} catch {
  $lines.Add("SERIAL_ERROR: $($_.Exception.Message)")
}

$header = @(
  "# U585 demo $Demo VCP capture",
  "# port=$Port baud=$Baud seconds=$Seconds fault=$faultFlag",
  "# utc=$([DateTime]::UtcNow.ToString('o'))",
  ""
)
($header + $lines) | Set-Content -Encoding utf8 $outFile
Write-Host "Wrote $($lines.Count) lines -> $outFile"
$lines | Select-Object -First 40
