# Install verified build/gen from a private archive, then compile the native
# HD-text investigation branch. Keep proprietary images OUT of GitHub.
# Usage (PowerShell from a checked-out investigation branch):
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\hd_text_buildgen_windows.ps1 -BuildGenZip "C:\path\BuildGen.zip"
# To replace an existing build/gen, add -ReplaceGenerated (backs up the old one).
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$BuildGenZip,
    [switch]$ReplaceGenerated
)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$archive = (Resolve-Path -LiteralPath $BuildGenZip).Path
$stage = Join-Path $env:TEMP ('simgolf-buildgen-' + [guid]::NewGuid().ToString('N'))
$target = Join-Path $root 'build\gen'
$copied = $false
try {
    New-Item -ItemType Directory -Path $stage -Force | Out-Null
    Expand-Archive -LiteralPath $archive -DestinationPath $stage
    $candidate = @(Get-ChildItem -LiteralPath $stage -Recurse -File -Filter 'golf_protos.h' |
        Where-Object { $_.Directory.Name -eq 'golf' })
    if ($candidate.Count -ne 1) {
        throw "Expected exactly one build/gen/golf/golf_protos.h in the archive, found $($candidate.Count)"
    }
    $src = $candidate[0].Directory.Parent.FullName
    foreach ($needed in @('jgld\jgld_protos.h','images\golf.bin','images\jgld.bin')) {
        if (-not (Test-Path -LiteralPath (Join-Path $src $needed))) {
            throw "Generated source archive lacks $needed"
        }
    }
    if (Test-Path -LiteralPath (Join-Path $target 'golf\golf_protos.h')) {
        if ($ReplaceGenerated) {
            $backup = $target + '-backup-' + (Get-Date -Format 'yyyyMMdd-HHmmss')
            Move-Item -LiteralPath $target -Destination $backup
            Write-Host "Preserved previous translated source at $backup"
        } else {
            Write-Host 'Existing build/gen preserved; pass -ReplaceGenerated to install the supplied version.'
        }
    } elseif (Test-Path -LiteralPath $target) {
        throw 'build/gen exists but is incomplete; resolve it manually before proceeding'
    }
    if (-not (Test-Path -LiteralPath $target)) {
        New-Item -ItemType Directory -Path (Split-Path $target -Parent) -Force | Out-Null
        Copy-Item -LiteralPath $src -Destination $target -Recurse
        $copied = $true
    }
} finally {
    if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
}
foreach ($required in @('golf\golf_protos.h','jgld\jgld_protos.h','images\golf.bin','images\jgld.bin')) {
    if (-not (Test-Path -LiteralPath (Join-Path $target $required))) {
        throw "Missing $required from $target"
    }
}
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw 'CMake missing. Use Visual Studio 2022 Build Tools with CMake/Ninja and the Windows SDK.'
}
if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
    throw 'Ninja missing. Use Visual Studio 2022 Build Tools with CMake/Ninja.'
}
$toolRoot = if ($env:TOOLS_EXT) { $env:TOOLS_EXT } else { 'C:\simgolf-tools' }
$llvm = if ($env:SIMGOLF_LLVM) { $env:SIMGOLF_LLVM } else { Join-Path $toolRoot 'llvm' }
$sdl = Join-Path $toolRoot 'SDL2-2.32.10'
if (-not (Test-Path -LiteralPath (Join-Path $llvm 'bin\clang-cl.exe')) -or
    -not (Test-Path -LiteralPath (Join-Path $sdl 'include\SDL.h'))) {
    throw 'Native LLVM/SDL2 SDK missing. From Git Bash run: bash tools/install_native_tools.sh'
}
$env:SIMGOLF_LLVM = $llvm
Push-Location (Join-Path $root 'port')
try {
    & cmake --preset native "-DSIMGOLF_GEN=$target" "-DSIMGOLF_SDL2=$sdl"
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($LASTEXITCODE)" }
    & cmake --build --preset native
    if ($LASTEXITCODE -ne 0) { throw "Native compile/link failed ($LASTEXITCODE)" }
} finally { Pop-Location }
$exe = Join-Path $root 'build\native\golf.exe'
if (-not (Test-Path -LiteralPath $exe)) { throw 'Build command finished but golf.exe was not created' }
$hash = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash
Write-Host "Windows build ready: $exe"
Write-Host "SHA256: $hash"
Write-Host 'Use a separate test install. Do NOT overwrite the installed working game.'
Write-Host 'Overlay: SIMGOLF_HD_TEXT_DRAW=1   Replacement: SIMGOLF_HD_TEXT_REPLACE=1'
