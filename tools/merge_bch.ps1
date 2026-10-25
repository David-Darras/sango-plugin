# Moves the textures of 3D models of X and Y to where ORAS looks for them.
#
# A map block of X and Y has no textures: they are in the area pack of the
# map. A block of ORAS is the same, and the game finds the textures in the area
# pack of ORAS. This script reads the blocks and the files with the textures,
# and writes one of these results in the format of ORAS:
#
#   -Names       prints the names of the textures that the blocks need.
#   -Container   writes the file Container with the textures (and the tables)
#                that the blocks need. The file is the part of an area pack
#                that holds the textures.
#   -Strip       writes the first block without textures and without tables,
#                as the blocks of ORAS are.
#   -Ground       lowers the block so that its ground is at the height 0 (or by
#                -ShiftY, when it is given).
#   (none)       writes the first block with its textures inside.
#
# -Model is a list of files, separated by commas. The script always prints a
# line "GROUND h": the mean height of the ground in the middle of the first
# block.
#
#   powershell -File tools/merge_bch.ps1 -SpicaDir C:\tools\spica -Model a.bch,b.bch -Names
#   powershell -File tools/merge_bch.ps1 -SpicaDir C:\tools\spica -Model a.bch -Textures t.bch -Container part.bch -Out part_new.bch
#
# export_xy_map.py runs this script for you.

param(
  [Parameter(Mandatory = $true)][string]$SpicaDir,
  [Parameter(Mandatory = $true)][string]$Model,
  [string[]]$Textures = @(),
  [string]$Container,
  [string]$Out,
  [switch]$Names,
  [switch]$Strip,
  [switch]$Ground,
  [double]$ShiftY = [double]::NaN,
  [int]$Version = 33
)

$ErrorActionPreference = "Stop"
[Reflection.Assembly]::LoadFrom((Join-Path $SpicaDir "SPICA.dll")) | Out-Null
$h3d = [SPICA.Formats.CtrH3D.H3D]
$blocks = @($Model -split "," | Where-Object { $_ } | ForEach-Object {
  , $h3d::Open([IO.File]::ReadAllBytes($_))
})
$block = $blocks[0]

$needed = New-Object System.Collections.Generic.HashSet[string]
foreach ($one in $blocks) {
  foreach ($mesh in $one.Models) {
    foreach ($material in $mesh.Materials) {
      foreach ($name in @($material.Texture0Name, $material.Texture1Name, $material.Texture2Name)) {
        if ($name -and $name -ne "projection_dummy") { [void]$needed.Add($name) }
      }
    }
  }
}

if ($Names) {
  $needed | Sort-Object | ForEach-Object { Write-Output $_ }
  return
}

# The mean height of the ground in the middle of the first block.
$sum = 0.0
$count = 0
foreach ($mesh in $block.Models[0].Meshes) {
  foreach ($vertex in $mesh.GetVertices()) {
    $p = $vertex.Position
    if ([math]::Abs($p.X) -lt 120 -and [math]::Abs($p.Z) -lt 120) {
      $sum += $p.Y
      $count++
    }
  }
}
$height = if ($count -gt 0) { $sum / $count } else { 0.0 }
Write-Host ("GROUND " + $height)

if ($Ground -or -not [double]::IsNaN($ShiftY)) {
  $shift = if ([double]::IsNaN($ShiftY)) { -$height } else { $ShiftY }
  foreach ($mesh in $block.Models[0].Meshes) {
    $o = $mesh.PositionOffset
    $mesh.PositionOffset = New-Object System.Numerics.Vector4 $o.X, ($o.Y + $shift), $o.Z, $o.W
    $c = $mesh.MeshCenter
    $mesh.MeshCenter = New-Object System.Numerics.Vector3 $c.X, ($c.Y + $shift), $c.Z
  }
  Write-Host ("SHIFT " + $shift)
}

if ($Container) {
  $scene = $h3d::Open([IO.File]::ReadAllBytes($Container))
} else {
  $scene = $block
}

$have = New-Object System.Collections.Generic.HashSet[string]
foreach ($texture in $scene.Textures) { [void]$have.Add($texture.Name) }
foreach ($path in ($Textures | ForEach-Object { $_ -split "," } | Where-Object { $_ })) {
  $source = $h3d::Open([IO.File]::ReadAllBytes($path))
  foreach ($texture in $source.Textures) {
    if ($needed.Contains($texture.Name) -and $have.Add($texture.Name)) {
      $scene.Textures.Add($texture)
    }
  }
}
$missing = @($needed | Where-Object { -not $have.Contains($_) })
if ($missing.Count -gt 0) { Write-Host ("MISSING " + ($missing -join ",")) }

if ($Container) {
  $tables = New-Object System.Collections.Generic.HashSet[string]
  foreach ($table in $scene.LUTs) { [void]$tables.Add($table.Name) }
  foreach ($one in $blocks) {
    foreach ($table in $one.LUTs) {
      if ($tables.Add($table.Name)) { $scene.LUTs.Add($table) }
    }
  }
}
if ($Strip) {
  $scene.Textures.Clear()
  $scene.LUTs.Clear()
  $scene.Flags = [SPICA.Formats.CtrH3D.H3DFlags]::IsFromNewConverter
}
$scene.BackwardCompatibility = [byte]$Version
$scene.ForwardCompatibility = [byte]$Version
$h3d::Save($Out, $scene)
