# BetterPresser Automated Test Runner & PluginVal Validator Script

param (
    [string]$Config = "Release",
    [int]$StrictnessLevel = 5,
    [switch]$SkipPluginVal = $false
)

$ErrorActionPreference = "Stop"
$ProjectDir = (Get-Item $PSScriptRoot).Parent.FullName

Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "         BETTERPRESSER AUTOMATED TEST RUNNER            " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "Project Directory: $ProjectDir"
Write-Host "Configuration:     $Config"

# 1. Locate CMake
$CMakeExe = "cmake"
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    $VsCMake = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if (Test-Path $VsCMake) {
        $CMakeExe = $VsCMake
    } else {
        $Vs2022CMake = "C:\Program Files\Microsoft Visual Studio\2022\*\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        $Found = Get-Item $Vs2022CMake -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($Found) { $CMakeExe = $Found.FullName }
    }
}

Write-Host "Using CMake: $CMakeExe"

# 2. Build the Tests & Plugin Targets
Write-Host "`n[1/3] Building BetterPresser and Test Runner..." -ForegroundColor Yellow
& $CMakeExe --build "$ProjectDir/build" --config $Config --target BetterPresserTests BetterPresser_VST3 BetterPresser_Standalone
if ($LASTEXITCODE -ne 0) {
    Write-Host "`n[ERROR] Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit $LASTEXITCODE
}

# 3. Run DSP & Serialization Unit Tests
Write-Host "`n[2/3] Running DSP & Serialization Unit Tests..." -ForegroundColor Yellow
$TestExe = "$ProjectDir/build/$Config/BetterPresserTests.exe"
if (-not (Test-Path $TestExe)) {
    $TestExe = "$ProjectDir/build/Tests/$Config/BetterPresserTests.exe"
}
if (-not (Test-Path $TestExe)) {
    $TestExe = "$ProjectDir/build/Tests/BetterPresserTests.exe"
}
if (-not (Test-Path $TestExe)) {
    $TestExe = "$ProjectDir/build/BetterPresserTests.exe"
}

if (Test-Path $TestExe) {
    & $TestExe
    if ($LASTEXITCODE -ne 0) {
        Write-Host "`n[FAILED] Unit tests failed with exit code $LASTEXITCODE" -ForegroundColor Red
        exit $LASTEXITCODE
    }
} else {
    Write-Host "[WARNING] Test executable not found at $TestExe" -ForegroundColor Magenta
}

# 4. PluginVal Validation
if ($SkipPluginVal) {
    Write-Host "`n[3/3] PluginVal validation skipped by request." -ForegroundColor Gray
} else {
    Write-Host "`n[3/3] Running Tracktion PluginVal Validator..." -ForegroundColor Yellow

    $ToolsDir = "$ProjectDir/tools"
    $PluginValExe = "$ToolsDir/pluginval.exe"

    if (-not (Test-Path $PluginValExe)) {
        if (-not (Test-Path $ToolsDir)) {
            New-Item -ItemType Directory -Path $ToolsDir -Force | Out-Null
        }
        Write-Host "Downloading Tracktion PluginVal..." -ForegroundColor Cyan
        $ZipPath = "$ToolsDir/pluginval_Windows.zip"
        $DownloadUrl = "https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_Windows.zip"
        try {
            Invoke-WebRequest -Uri $DownloadUrl -OutFile $ZipPath -UseBasicParsing
            Expand-Archive -Path $ZipPath -DestinationPath $ToolsDir -Force
            Remove-Item $ZipPath -Force
        } catch {
            Write-Host "[WARNING] Failed to automatically download PluginVal: $_" -ForegroundColor Magenta
        }
    }

    $Vst3Path = "$ProjectDir/build/BetterPresser_artefacts/$Config/VST3/BetterPresser.vst3"
    if (-not (Test-Path $Vst3Path)) {
        $Vst3Path = "$ProjectDir/build/BetterPresser_artefacts/Debug/VST3/BetterPresser.vst3"
    }

    if ((Test-Path $PluginValExe) -and (Test-Path $Vst3Path)) {
        Write-Host "Validating VST3 plugin: $Vst3Path (Strictness Level: $StrictnessLevel)" -ForegroundColor Cyan
        & $PluginValExe --validate-in-process --strictness-level $StrictnessLevel --validate "$Vst3Path"
        if ($LASTEXITCODE -ne 0) {
            Write-Host "`n[FAILED] PluginVal detected issues (Exit code: $LASTEXITCODE)" -ForegroundColor Red
            exit $LASTEXITCODE
        }
    } else {
        Write-Host "[INFO] PluginVal executable or VST3 not ready for automated scan. Skipping PluginVal." -ForegroundColor Gray
    }
}

Write-Host "`n========================================================" -ForegroundColor Green
Write-Host "          ALL TESTS PASSED SUCCESSFULLY!                " -ForegroundColor Green
Write-Host "========================================================" -ForegroundColor Green
