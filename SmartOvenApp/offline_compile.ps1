Write-Host "========================================" -ForegroundColor Green
Write-Host "智能烤箱APP离线编译脚本" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""

# 检查Java版本
Write-Host "当前Java版本:" -ForegroundColor Cyan
java -version

Write-Host ""
Write-Host "解决方案: 使用本地Gradle 9.2.0 + 兼容Java 8的Android Gradle Plugin" -ForegroundColor Yellow
Write-Host ""

# 方案1: 使用本地Gradle 9.2.0 + 兼容配置
Write-Host "方案1: 使用本地Gradle 9.2.0编译..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# 设置本地Gradle路径
$GRADLE_HOME = "..\gradle-9.2.0"
$GRADLE_BAT = "$GRADLE_HOME\bin\gradle.bat"

if (Test-Path $GRADLE_BAT) {
    Write-Host "✓ 找到本地Gradle 9.2.0" -ForegroundColor Green
    Write-Host "Gradle版本信息:" -ForegroundColor Cyan
    & $GRADLE_BAT --version
    
    Write-Host ""
    Write-Host "开始编译Android项目..." -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    
    # 执行编译
    & $GRADLE_BAT clean build
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "========================================" -ForegroundColor Green
        Write-Host "✓ 编译成功！" -ForegroundColor Green
        Write-Host "APK文件位置: app\build\outputs\apk\" -ForegroundColor Green
        Write-Host "========================================" -ForegroundColor Green
    } else {
        Write-Host ""
        Write-Host "========================================" -ForegroundColor Red
        Write-Host "✗ 编译失败" -ForegroundColor Red
        Write-Host "========================================" -ForegroundColor Red
    }
} else {
    Write-Host "✗ 错误: 找不到本地Gradle 9.2.0" -ForegroundColor Red
}

Write-Host ""
Write-Host "按任意键退出..." -ForegroundColor Yellow
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")