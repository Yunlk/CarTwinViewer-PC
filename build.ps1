# ===================================================
# CarTwinViewer 一键构建脚本 (PowerShell)
# 用法: .\build.ps1
# ===================================================

$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SrcDir     = "$ProjectDir\src"
$SDL2Dir    = "$ProjectDir\sdl2"
$OutExe     = "$ProjectDir\build\CarTwinViewer.exe"

New-Item -ItemType Directory -Force -Path "$ProjectDir\build" | Out-Null

$SDL_Inc = "$SDL2Dir\include\SDL2"
$SDL_Lib = "$SDL2Dir\lib"

$Sources = @(
    "$SrcDir\main.cpp",
    "$SrcDir\Car.cpp",
    "$SrcDir\Road.cpp",
    "$SrcDir\Obstacle.cpp",
    "$SrcDir\Serial.cpp",
    "$SrcDir\Dashboard.cpp",
    "$SrcDir\UI.cpp"
)

$Gpp = "D:\mingw64\bin\g++.exe"
if (-not (Test-Path $Gpp)) { $Gpp = "g++.exe" }

Write-Host "=== 构建 CarTwinViewer ===" -ForegroundColor Cyan
Write-Host "编译器: $Gpp"
Write-Host "SDL2:    $SDL2Dir"

$Args = @(
    "-std=c++17", "-O2", "-g",
    "-I", $SDL_Inc,
    "-I", $SrcDir
) + $Sources + @(
    "-o", $OutExe,
    "-L", $SDL_Lib,
    # SDL2 链接顺序很重要: mingw32 → SDL2main → SDL2
    "-lmingw32", "-lSDL2main", "-lSDL2", "-lSDL2_image",
    "-mwindows",
    "-lgdi32", "-limm32", "-lole32", "-loleaut32", "-lversion", "-lwinmm", "-lsetupapi"
)

& $Gpp @Args 2>&1

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n✔ 编译成功: $OutExe" -ForegroundColor Green
    Copy-Item "$SDL2Dir\bin\SDL2.dll", "$SDL2Dir\bin\SDL2_image.dll" "$ProjectDir\build\" -Force
    New-Item -ItemType Directory -Force -Path "$ProjectDir\build\assets" | Out-Null
    Copy-Item "$ProjectDir\assets\*" "$ProjectDir\build\assets\" -Recurse -Force
    Write-Host "✔ DLL & assets 已复制到 build\" -ForegroundColor Green
} else {
    Write-Host "`n✘ 编译失败 (exit code: $LASTEXITCODE)" -ForegroundColor Red
}
