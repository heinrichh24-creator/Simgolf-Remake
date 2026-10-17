# Windows build for the native HD-text investigation branch.
# Keeps private generated translated source OUT of GitHub.
# Run in an x86 Visual Studio Developer PowerShell with LLVM, CMake/Ninja,
# and SDL2 developer files installed (see tools/install_native_tools.sh).
[CmdletBinding()]
param(
  [string]$GeneratedZip = "",
  [string]$ToolsRoot = "C:\simgolf-tools",
  [switch]$CheckOnly
)
$ErrorActionPreference = 'Stop'
$repo = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$port = Join-Path $repo 'port'
if (-not (Test-Path (Join-Path $port 'CMakePresets.json'))) {
  throw 'Run this script from a complete source repository checkout.'
}
if ($GeneratedZip) {
  $archive = (Resolve-Path -LiteralPath $GeneratedZip).Path
  $unpack = Join-Path ([System.IO.Path]::GetTempPath()) ('simgolf-gen-' + [guid]::NewGuid().ToString('N'))
  New-Item -ItemType Directory -Path $unpack -Force | Out-Null
  try {
    Expand-Archive -LiteralPath $archive -DestinationPath $unpack
    $candidates = @(Get-ChildItem -LiteralPath $unpack -Recurse -File -Filter 'golf_protos.h' |
       Where-Object { $_.FullName -match '[\\/]build[\\/]gen[\\/]golf[\\/]golf_protos\.h$' })
    if ($candidates.Count -ne 1) {
      throw "Expected exactly one translated build/gen in archive, found $($candidates.Count)."
    }
    $gen = $candidates[0].Directory.Parent.FullName
    $dest = Join-Path $repo 'build/gen'
    New-Item -ItemType Directory -Path $dest -Force | Out-Null
    Copy-Item -Path (Join-Path $gen '*') -Destination $dest -Recurse -Force
    Write-Host 'Imported PRIVATE generated source locally (not added to Git).'
  }
  finally {
    Remove-Item -LiteralPath $unpack -Force -Recurse -ErrorAction SilentlyContinue
  }
}
foreach ($rel in @('build/gen/golf/golf_protos.h',
                   'build/gen/jgld/jgld_protos.h',
                   'build/gen/images/golf.bin',
                   'build/gen/images/jgld.bin')) {
  if (-not (Test-Path -LiteralPath (Join-Path $repo $rel))) {
    throw "Missing generated prerequisite: $rel. Supply -GeneratedZip Build:gen.zip."
  }
}
$clang = Join-Path $ToolsRoot 'llvm/bin/clang-cl.exe'
$sdl = Join-Path $ToolsRoot 'SDL2-2.32.10/include/SDL.h'
if (-not $CheckOnly) {
  foreach ($tool in @($clang, $sdl)) {
    if (-not (Test-Path -LiteralPath $tool)) { throw "Missing tool: $tool" }
  }
  if (-not $env:LIB) {
    throw 'Open an x86 Visual Studio Developer PowerShell to supply MSVC/Windows SDK libs.'
  }
  if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw 'CMake is required.'
  }
  if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
    throw 'Ninja is required.'
  }
}
if ($CheckOnly) {
  Write-Host 'Generated source/header/image prerequisites present; Windows compilation not requested.'
  return
}
$env:TOOLS_EXT = $ToolsRoot
$env:SIMGOLF_LLVM = Join-Path $ToolsRoot 'llvm'
Push-Location $port
try {
  & cmake --preset native
  if ($LASTEXITCODE -ne 0) { throw 'Native CMake configure failed.' }
  & cmake --build --preset native
  if ($LASTEXITCODE -ne 0) { throw 'Native build failed.' }
}
finally { Pop-Location }
$exe = Join-Path $repo 'build/native/golf.exe'
if (-not (Test-Path -LiteralPath $exe)) { throw 'The build returned success without golf.exe.' }
Get-FileHash -LiteralPath $exe -Algorithm SHA256 | Format-List
Write-Host "Built: $exe"
Write-Host 'No files from build/gen have been uploaded to GitHub.'
