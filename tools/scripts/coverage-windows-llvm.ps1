param(
  [string]$BuildDir = "build-llvm"
)
$ErrorActionPreference = "Stop"

$llvm = "C:\Program Files\LLVM\bin"
$env:Path = "$llvm;$env:Path"

if (!(Test-Path $BuildDir)) { throw "Build dir '$BuildDir' not found. Run CMake first." }

cmake --build $BuildDir --target coverage-llvm
$index = Join-Path $BuildDir "coverage-llvm\index.html"
if (Test-Path $index) {
  Start-Process $index
  Write-Host "Opened $index" -ForegroundColor Green
} else {
  Write-Warning "coverage-llvm/index.html not found."
}