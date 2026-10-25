# Converts the 3D models (.bch files) of a folder to the format of ORAS.
#
# It uses the library SPICA.dll of the program SPICA (Windows only). The
# format of the models of X and Y is older than the format of ORAS. SPICA
# reads the old format and writes the new one.
#
#   powershell -File tools/convert_bch.ps1 -SpicaDir C:\tools\spica -InDir in -OutDir out
#
# export_xy_npc_models.py runs this script for you.

param(
  [Parameter(Mandatory = $true)][string]$SpicaDir,
  [Parameter(Mandatory = $true)][string]$InDir,
  [Parameter(Mandatory = $true)][string]$OutDir,
  [int]$Version = 33
)

$ErrorActionPreference = "Stop"
[Reflection.Assembly]::LoadFrom((Join-Path $SpicaDir "SPICA.dll")) | Out-Null
$h3d = [SPICA.Formats.CtrH3D.H3D]
New-Item -ItemType Directory -Force $OutDir | Out-Null

foreach ($file in Get-ChildItem $InDir -Filter *.bch) {
  try {
    $scene = $h3d::Open([IO.File]::ReadAllBytes($file.FullName))
    $scene.BackwardCompatibility = [byte]$Version
    $scene.ForwardCompatibility = [byte]$Version
    $h3d::Save((Join-Path $OutDir $file.Name), $scene)
  } catch {
    Write-Host ("FAILED " + $file.Name + ": " + $_.Exception.Message)
  }
}
