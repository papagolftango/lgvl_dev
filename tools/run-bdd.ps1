param(
  [string]$Tags = "",
  [switch]$DryRun,
  [string]$BridgeMode = "mock",
  [string]$BridgeUrl = "http://127.0.0.1:8000",
  [string]$SerialPort = "",
  [int]$SerialBaud = 115200
)

$root = Split-Path -Parent $MyInvocation.MyCommand.Path | Split-Path -Parent
Set-Location $root

# Create venv under tests/.venv
$venv = Join-Path $root "tests/.venv"
if (!(Test-Path $venv)) {
  python -m venv $venv
}

# Activate venv
$activate = Join-Path $venv "Scripts/Activate.ps1"
. $activate

pip install --upgrade pip
pip install -r tests/bdd/requirements.txt

$env:BRIDGE_MODE = $BridgeMode
if ($BridgeMode -eq 'http') { $env:BRIDGE_URL = $BridgeUrl }
if ($BridgeMode -eq 'serial') { $env:SERIAL_PORT = $SerialPort; $env:SERIAL_BAUD = "$SerialBaud" }

$behaveArgs = @()
if ($Tags) { $behaveArgs += ("--tags=$Tags") }
if ($DryRun) { $behaveArgs += ("--dry-run") }

Set-Location tests/bdd
behave @behaveArgs
