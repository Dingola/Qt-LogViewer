<#
.SYNOPSIS
Builds and runs the opt-in Qt-LogViewer large-file benchmark.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$DatasetPath,
    [string]$ResultsPath = "",
    [string]$BuildPath = ".\_build_performance",
    [switch]$IncludeMessageSort
)

$ErrorActionPreference = "Stop"
$repository = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..\..\..")).Path
$source = (Resolve-Path -LiteralPath $DatasetPath).Path
if ([string]::IsNullOrWhiteSpace($ResultsPath)) {
    $results = Join-Path $repository "Docs\Performance\Results\LargeFileBenchmarkResults.csv"
} else {
    $results = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ResultsPath)
}
$build = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($BuildPath)
$releaseCache = Join-Path $repository "_build_release\CMakeCache.txt"
$prefix = $null

if (Test-Path -LiteralPath $releaseCache) {
    $prefixLine = Get-Content -LiteralPath $releaseCache | Where-Object { $_ -like "CMAKE_PREFIX_PATH:PATH=*" } | Select-Object -First 1
    if ($prefixLine) { $prefix = $prefixLine.Substring($prefixLine.IndexOf("=") + 1) }
}
if (-not $prefix) { throw "CMAKE_PREFIX_PATH was not found in _build_release\CMakeCache.txt." }
$qtBin = Join-Path $prefix "bin"
$resultsDirectory = Split-Path -Parent $results
New-Item -ItemType Directory -Path $resultsDirectory -Force | Out-Null

& cmake -S $repository -B $build "-DCMAKE_BUILD_TYPE=Release" "-DCMAKE_PREFIX_PATH=$prefix" "-DQt-LogViewer_BUILD_TARGET_TYPE=static_library" "-DQt-LogViewer_BUILD_TEST_PROJECT=OFF" "-DQt-LogViewer_BUILD_PERFORMANCE_BENCHMARK=ON"
if ($LASTEXITCODE -ne 0) { throw "Performance build configuration failed." }

& cmake --build $build --config Release --target Qt-LogViewer_Performance -- /verbosity:minimal
if ($LASTEXITCODE -ne 0) { throw "Performance benchmark build failed." }

$executable = Join-Path $build "QT_Project_Benchmarks\Release\Qt-LogViewer_Performance.exe"
if (-not (Test-Path -LiteralPath $executable)) { throw "Benchmark executable was not produced at $executable." }

$temporaryDirectory = Join-Path ([System.IO.Path]::GetTempPath()) ("qt-logviewer-benchmark-" + [guid]::NewGuid().ToString("N"))
$temporaryDataset = Join-Path $temporaryDirectory ([System.IO.Path]::GetFileName($source))
try {
    New-Item -ItemType Directory -Path $temporaryDirectory | Out-Null
    Copy-Item -LiteralPath $source -Destination $temporaryDataset
    $commit = (& git -C $repository rev-parse --short=12 HEAD).Trim()
    & git -C $repository diff --quiet HEAD -- QT_Project QT_Project_Benchmarks CMakeLists.txt Scripts/Win/Performance ':(exclude)QT_Project/Resources/Translations/**'
    $sourceState = if ($LASTEXITCODE -eq 0) { $commit } else { "$commit+Arbeitskopie" }
    $storageRoot = Join-Path $temporaryDirectory "storage"
    New-Item -ItemType Directory -Path $storageRoot | Out-Null
    $previousPath = $env:PATH
    $previousPlatform = $env:QT_QPA_PLATFORM
    $env:PATH = "$qtBin;$previousPath"
    $env:QT_QPA_PLATFORM = "offscreen"
    $benchmarkArguments = @("--dataset", $temporaryDataset, "--results", $results, "--source-state", $sourceState, "--storage-root", $storageRoot)
    if ($IncludeMessageSort) { $benchmarkArguments += "--include-message-sort" }
    & $executable @benchmarkArguments
    if ($LASTEXITCODE -ne 0) { throw "Benchmark failed with exit code $LASTEXITCODE." }
    Get-Item -LiteralPath $results
    Get-Item -LiteralPath ([System.IO.Path]::ChangeExtension($results, ".md"))
}
finally {
    if ($null -ne $previousPath) { $env:PATH = $previousPath }
    if ($null -ne $previousPlatform) { $env:QT_QPA_PLATFORM = $previousPlatform } else { Remove-Item Env:QT_QPA_PLATFORM -ErrorAction SilentlyContinue }
    if (Test-Path -LiteralPath $temporaryDirectory) { Remove-Item -LiteralPath $temporaryDirectory -Recurse -Force }
}
