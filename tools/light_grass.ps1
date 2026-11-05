# Makes the unlit grass models of X and Y lit, like the other grass models.
#
# In the files of X and Y, two of the five grass models (the yellow flower and
# the tall grass) ignore the light: their material has no fragment lighting.
# This script copies the lighting setup (the flags, the constants and the
# texture combiners) of the model 0 to the models that are given.
#
#   powershell -File tools/light_grass.ps1 -SpicaDir C:\tools\spica -Dir folder -Models "1,4"
#
# The folder has the files 0.bch, 1.bch... that convert_bch.ps1 made.
# export_xy_grass.py runs this script for you.

param(
  [Parameter(Mandatory = $true)][string]$SpicaDir,
  [Parameter(Mandatory = $true)][string]$Dir,
  [Parameter(Mandatory = $true)][string]$Models,
  [int]$Version = 33
)

$ErrorActionPreference = "Stop"
[Reflection.Assembly]::LoadFrom((Join-Path $SpicaDir "SPICA.dll")) | Out-Null
$h3d = [SPICA.Formats.CtrH3D.H3D]

$reference = $h3d::Open([IO.File]::ReadAllBytes((Join-Path $Dir "0.bch")))
$source = $reference.Models[0].Materials[0].MaterialParams
$names = "Flags", "Constant0Color", "Constant1Color", "Constant2Color",
         "Constant3Color", "Constant4Color", "Constant5Color",
         "Constant0Assignment", "Constant1Assignment", "Constant2Assignment",
         "Constant3Assignment", "Constant4Assignment", "Constant5Assignment",
         "TexEnvStages", "TexEnvBufferColor"

foreach ($index in ($Models -split ",")) {
  $path = Join-Path $Dir ("{0}.bch" -f $index)
  $scene = $h3d::Open([IO.File]::ReadAllBytes($path))
  foreach ($model in $scene.Models) {
    foreach ($material in $model.Materials) {
      $target = $material.MaterialParams
      foreach ($name in $names) {
        $member = $target.GetType().GetField($name)
        if ($null -ne $member) {
          $member.SetValue($target, $member.GetValue($source))
        } else {
          $property = $target.GetType().GetProperty($name)
          if ($null -ne $property -and $property.CanWrite) {
            $property.SetValue($target, $property.GetValue($source))
          }
        }
      }
    }
  }
  $scene.BackwardCompatibility = [byte]$Version
  $scene.ForwardCompatibility = [byte]$Version
  $h3d::Save($path, $scene)
  Write-Host ("lit " + $path)
}
