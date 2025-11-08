#!/usr/bin/env pwsh

<#
从设备中获取最新的HTML文件
该脚本通过HTTP请求从智能烤箱控制器设备获取最新的HTML文件内容
并保存到本地data目录，以确保使用最新版本
#>

# 确保目录存在，如果不存在则创建
function Ensure-DirectoryExists {
    param(
        [string]$directory
    )
    if (-not (Test-Path -Path $directory)) {
        New-Item -Path $directory -ItemType Directory -Force | Out-Null
        Write-Host "已创建目录: $directory"
    }
}

# 从设备获取指定文件并保存到本地
function Fetch-FileFromDevice {
    param(
        [string]$deviceIp,
        [string]$filePath,
        [string]$outputDir
    )
    
    $url = "http://${deviceIp}/${filePath}"
    $outputPath = Join-Path -Path $outputDir -ChildPath (Split-Path -Path $filePath -Leaf)
    
    try {
        Write-Host "正在从设备获取: $filePath"
        Write-Host "请求URL: $url"
        
        # 发送请求
        $response = Invoke-WebRequest -Uri $url -TimeoutSec 10 -UseBasicParsing
        
        # 检查响应内容是否为空
        if (-not $response.Content) {
            Write-Host "警告: 从设备获取的 $filePath 内容为空"
            return $false
        }
        
        # 保存文件
        $response.Content | Out-File -FilePath $outputPath -Encoding UTF8 -Force
        
        # 获取文件大小
        $fileSize = (Get-Item -Path $outputPath).Length
        Write-Host "✓ 成功保存到: $outputPath"
        Write-Host "文件大小: $fileSize 字节"
        return $true
    }
    catch {
        if ($_.Exception.GetType().Name -eq "WebException") {
            if ($_.Exception.Response) {
                $statusCode = [int]$_.Exception.Response.StatusCode
                $statusDescription = $_.Exception.Response.StatusDescription
                Write-Host "✗ HTTP错误: $statusCode - $statusDescription"
            } else {
                Write-Host "✗ 连接错误: 无法连接到设备 $deviceIp"
                Write-Host "  请检查设备是否在线，IP地址是否正确"
            }
        } else {
            Write-Host "✗ 获取文件时出错: $($_.Exception.Message)"
        }
        return $false
    }
}

# 主函数
function Main {
    Write-Host "========== 从智能烤箱控制器设备获取最新HTML文件 =========="
    Write-Host "该工具将从设备中获取最新的HTML文件并保存到本地data目录"
    Write-Host ""
    
    # 默认设备IP地址
    $defaultIp = "192.168.4.1"  # 设备在AP模式下的默认IP
    
    # 获取用户输入的设备IP
    $deviceIp = Read-Host -Prompt "请输入设备IP地址 [默认: $defaultIp]"
    if (-not $deviceIp) {
        $deviceIp = $defaultIp
    }
    
    Write-Host "使用设备IP: $deviceIp"
    Write-Host ""
    
    # 要获取的HTML文件列表
    $htmlFiles = @(
        "index.html",
        "device_status.html",
        "login.html",
        "settings_help.html",
        "wifi_config.html",
        "smart_oven_controller.html",
        "temperature_control.html"
    )
    
    # 创建输出目录
    $outputDir = "data_latest"
    Ensure-DirectoryExists -directory $outputDir
    
    # 创建备份目录
    $backupDir = "data_backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
    if (Test-Path -Path "data") {
        $dataFiles = Get-ChildItem -Path "data" -Filter "*.html" -ErrorAction SilentlyContinue
        if ($dataFiles) {
            Ensure-DirectoryExists -directory $backupDir
            
            # 备份当前data目录中的文件
            foreach ($file in $dataFiles) {
                $src = $file.FullName
                $dst = Join-Path -Path $backupDir -ChildPath $file.Name
                Copy-Item -Path $src -Destination $dst -Force
                Write-Host "已备份: $($file.Name) -> $backupDir/$($file.Name)"
            }
        }
    }
    
    Write-Host "\n开始从设备获取HTML文件..."
    Write-Host "=" -Repeat 60
    
    # 获取文件
    $successCount = 0
    foreach ($htmlFile in $htmlFiles) {
        # 有些设备可能将文件放在根目录，有些可能在data目录下
        # 先尝试直接路径
        if (Fetch-FileFromDevice -deviceIp $deviceIp -filePath $htmlFile -outputDir $outputDir) {
            $successCount++
        } else {
            # 再尝试data路径
            Write-Host "尝试从data路径获取: $htmlFile"
            if (Fetch-FileFromDevice -deviceIp $deviceIp -filePath "data/$htmlFile" -outputDir $outputDir) {
                $successCount++
            }
        }
        Write-Host ""
        Start-Sleep -Seconds 1  # 避免请求过快
    }
    
    # 总结
    Write-Host "=" -Repeat 60
    Write-Host "获取完成！成功: $successCount/$($htmlFiles.Count) 个文件"
    Write-Host "\n最新文件已保存到: $outputDir"
    
    if ($backupDir) {
        Write-Host "原有文件已备份到: $backupDir"
    }
    
    # 询问是否要替换现有的data目录
    if ($successCount -gt 0) {
        $replace = Read-Host -Prompt "\n是否要将获取的文件复制到主data目录？(y/n)"
        if ($replace.ToLower() -eq 'y') {
            Ensure-DirectoryExists -directory "data"
            $latestFiles = Get-ChildItem -Path $outputDir -Filter "*.html" -ErrorAction SilentlyContinue
            foreach ($file in $latestFiles) {
                $src = $file.FullName
                $dst = Join-Path -Path "data" -ChildPath $file.Name
                Copy-Item -Path $src -Destination $dst -Force
                Write-Host "已复制: $($file.Name) -> data/$($file.Name)" $($file.Name) -> data/$($file.Name)"
            }
            Write-Host "\n✓ 已成功更新data目录中的文件"
        }
    }
    
    Write-Host "\n提示:"
    Write-Host "1. 如果无法连接到设备，请确保设备已开机并处于正常工作状态"
    Write-Host "2. 检查设备IP地址是否正确"
    Write-Host "3. 确认设备已连接到WiFi网络(如果不在AP模式)"
}

# 执行主函数
Main