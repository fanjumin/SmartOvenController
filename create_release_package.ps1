# Smart Oven Controller v0.8.1 Release Package Creation Script

Write-Host "================================" -ForegroundColor Cyan
Write-Host "Smart Oven Controller v0.8.1 Release Package Creator" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan

# Define version information
$version = "0.8.1"
$packageName = "SmartOvenController_v$version.zip"
$packagePath = "$PWD\$packageName"

Write-Host ""
Write-Host "Creating release package: $packageName" -ForegroundColor Yellow

# Check required files and directories
Write-Host "[1/5] Checking required files..." -ForegroundColor Yellow

# Check release directory v0.8.1 folder
if (-not (Test-Path "release\v0.8.1")) {
    Write-Host "❌ Error: release\v0.8.1 directory not found" -ForegroundColor Red
    exit 1
}

Write-Host "✅ Required files check completed" -ForegroundColor Green

# Create temporary directory for packaging
Write-Host ""
Write-Host "[2/5] Preparing packaging files..." -ForegroundColor Yellow
$tempDir = "$PWD\_temp_release"
if (Test-Path $tempDir) {
    Remove-Item $tempDir -Recurse -Force
}
New-Item -ItemType Directory -Path $tempDir | Out-Null

# Copy all files from release\v0.8.1 directory to temp directory
Write-Host "  Copying release\v0.8.1 directory contents..." -ForegroundColor Gray
Copy-Item -Path "release\v0.8.1\*" -Destination $tempDir -Recurse

# Copy firmware files (if they exist)
if (Test-Path ".pio\build\nodemcuv2\firmware.bin") {
    Write-Host "  Copying firmware file..." -ForegroundColor Gray
    Copy-Item ".pio\build\nodemcuv2\firmware.bin" "$tempDir\firmware.bin"
}

# Copy filesystem image (if it exists)
if (Test-Path ".pio\build\nodemcuv2\littlefs.bin") {
    Write-Host "  Copying filesystem image..." -ForegroundColor Gray
    Copy-Item ".pio\build\nodemcuv2\littlefs.bin" "$tempDir\littlefs.bin"
}

# Copy platformio.ini configuration file
Write-Host "  Copying configuration file..." -ForegroundColor Gray
Copy-Item "platformio.ini" "$tempDir\platformio.ini"

# Copy BUILD_INSTRUCTIONS.md file
Write-Host "  Copying build instructions..." -ForegroundColor Gray
Copy-Item "BUILD_INSTRUCTIONS.md" "$tempDir\BUILD_INSTRUCTIONS.md"

Write-Host "✅ Packaging files preparation completed" -ForegroundColor Green

# Create ZIP file
Write-Host ""
Write-Host "[3/5] Creating ZIP archive..." -ForegroundColor Yellow

# Delete existing ZIP file if it exists
if (Test-Path $packagePath) {
    Write-Host "  Deleting existing release package..." -ForegroundColor Gray
    Remove-Item $packagePath -Force
}

# Use Compress-Archive to create ZIP file
try {
    Compress-Archive -Path "$tempDir\*" -DestinationPath $packagePath -CompressionLevel Optimal
    Write-Host "✅ ZIP archive creation completed" -ForegroundColor Green
} catch {
    Write-Host "❌ Error creating ZIP archive: $_" -ForegroundColor Red
    # Clean up temporary directory
    if (Test-Path $tempDir) {
        Remove-Item $tempDir -Recurse -Force
    }
    exit 1
}

# Get ZIP file information
$packageInfo = Get-Item $packagePath
$packageSize = $packageInfo.Length

# Clean up temporary directory
Write-Host ""
Write-Host "[4/5] Cleaning up temporary files..." -ForegroundColor Yellow
Remove-Item $tempDir -Recurse -Force
Write-Host "✅ Temporary files cleanup completed" -ForegroundColor Green

# Verify created file
Write-Host ""
Write-Host "[5/5] Verifying release package..." -ForegroundColor Yellow
if (Test-Path $packagePath) {
    Write-Host "✅ Release package created successfully!" -ForegroundColor Green
    Write-Host ""
    Write-Host "================================" -ForegroundColor Cyan
    Write-Host "Release Package Information:" -ForegroundColor Cyan
    Write-Host "Filename: $packageName" -ForegroundColor Gray
    Write-Host "File size: $packageSize bytes" -ForegroundColor Gray
    Write-Host "File path: $packagePath" -ForegroundColor Gray
    Write-Host "================================" -ForegroundColor Cyan
} else {
    Write-Host "❌ Release package creation failed!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Press any key to exit..." -ForegroundColor Gray
$host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")