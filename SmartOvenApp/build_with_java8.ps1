Write-Host "========================================" -ForegroundColor Green
Write-Host "智能烤箱APP编译脚本 (兼容Java 8)" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""

# 设置Gradle路径（使用兼容Java 8的版本）
$GRADLE_HOME = "..\gradle-7.0.2"
$GRADLE_BAT = "$GRADLE_HOME\bin\gradle.bat"

# 检查Gradle是否可用
if (!(Test-Path $GRADLE_BAT)) {
    Write-Host "错误: 找不到Gradle可执行文件" -ForegroundColor Red
    Write-Host "请确保gradle-7.0.2目录存在于项目根目录" -ForegroundColor Yellow
    Write-Host "如果需要，请运行: install_gradle.bat" -ForegroundColor Yellow
    exit 1
}

Write-Host "使用Gradle版本:" -ForegroundColor Cyan
& $GRADLE_BAT --version

if ($LASTEXITCODE -ne 0) {
    Write-Host "错误: Gradle版本检查失败" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "开始编译Android项目..." -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green

# 执行编译
& $GRADLE_BAT clean build

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "编译成功！" -ForegroundColor Green
    Write-Host "APK文件位置: app\build\outputs\apk\" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "编译失败，请检查错误信息" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
}

Write-Host "按任意键继续..." -ForegroundColor Yellow
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")