Write-Host "使用本地Gradle 9.2.0编译Android项目..." -ForegroundColor Green
Write-Host ""

# 设置Gradle路径
$GRADLE_HOME = "..\gradle-9.2.0"
$GRADLE_BAT = "$GRADLE_HOME\bin\gradle.bat"

# 检查Gradle是否可用
if (-not (Test-Path $GRADLE_BAT)) {
    Write-Host "错误：找不到Gradle 9.2.0" -ForegroundColor Red
    Write-Host "请确保gradle-9.2.0目录存在于项目根目录" -ForegroundColor Yellow
    exit 1
}

Write-Host "Gradle版本：" -ForegroundColor Cyan
& $GRADLE_BAT --version

Write-Host ""
Write-Host "开始编译Android项目..." -ForegroundColor Yellow

# 编译项目
& $GRADLE_BAT build

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "✅ 编译成功！" -ForegroundColor Green
    Write-Host "APK文件位置：app\build\outputs\apk\debug\app-debug.apk" -ForegroundColor Cyan
} else {
    Write-Host ""
    Write-Host "❌ 编译失败，错误代码：$LASTEXITCODE" -ForegroundColor Red
}

Write-Host ""
Write-Host "按任意键继续..." -ForegroundColor Gray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")