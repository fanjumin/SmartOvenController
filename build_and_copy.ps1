Write-Host "================================" -ForegroundColor Cyan
Write-Host "智能烤箱控制器构建脚本" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan

Write-Host ""
Write-Host "[1/4] 构建固件..." -ForegroundColor Yellow
pio run
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ 固件构建失败" -ForegroundColor Red
    pause
    exit 1
}
Write-Host "✅ 固件构建完成" -ForegroundColor Green

Write-Host ""
Write-Host "[2/4] 构建文件系统..." -ForegroundColor Yellow
pio run -t buildfs
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ 文件系统构建失败" -ForegroundColor Red
    pause
    exit 1
}
Write-Host "✅ 文件系统构建完成" -ForegroundColor Green

Write-Host ""
Write-Host "[3/4] 创建发布目录..." -ForegroundColor Yellow
if (!(Test-Path "release")) {
    New-Item -ItemType Directory -Name "release" | Out-Null
}

Write-Host ""
Write-Host "[4/4] 复制生成的文件..." -ForegroundColor Yellow
Copy-Item ".pio\build\nodemcuv2\firmware.bin" "release\"
Copy-Item ".pio\build\nodemcuv2\littlefs.bin" "release\"
Copy-Item "platformio.ini" "release\"
Copy-Item "BUILD_INSTRUCTIONS.md" "release\"

Write-Host ""
Write-Host "================================" -ForegroundColor Cyan
Write-Host "构建完成！生成的文件已复制到 release 目录：" -ForegroundColor Cyan
Write-Host "- firmware.bin (固件)" -ForegroundColor Gray
Write-Host "- littlefs.bin (文件系统)" -ForegroundColor Gray
Write-Host "- platformio.ini (配置文件)" -ForegroundColor Gray
Write-Host "- BUILD_INSTRUCTIONS.md (构建说明)" -ForegroundColor Gray
Write-Host "================================" -ForegroundColor Cyan

Write-Host ""
Write-Host "按任意键退出..." -ForegroundColor Gray
$host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")