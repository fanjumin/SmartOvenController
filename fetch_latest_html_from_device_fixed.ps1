#!/usr/bin/env pwsh
<#
从设备中获取最新的HTML文件
该脚本通过HTTP请求从智能烤箱控制器设备获取最新的HTML文件内容
并保存到本地data目录，以确保使用最新版本
#>

# 确保目录存在
function Ensure-DirectoryExists {
    param(
        [string]$directory
    )
    if (-not (Test-Path -Path $directory)) {
        New-Item -Path $directory -ItemType Directory -Force | Out-Null
        Write-Host "Directory created: $directory"
    }
}

# 从设备获取文件
function Fetch-FileFromDevice {
    param(
        [string]$deviceIp,
        [string]$filePath,
        [string]$outputDir
    )
    
    $url = "http://${deviceIp}/${filePath}"
    $outputPath = Join-Path -Path $outputDir -ChildPath (Split-Path -Path $filePath -Leaf)
    
    try {
        Write-Host "Fetching: $filePath"
        $response = Invoke-WebRequest -Uri $url -TimeoutSec 10 -UseBasicParsing
        
        if (-not $response.Content) {
            Write-Host "Warning: Empty content for $filePath"
            return $false
        }
        
        $response.Content | Out-File -FilePath $outputPath -Encoding UTF8 -Force
        $fileSize = (Get-Item -Path $outputPath).Length
        Write-Host "✓ Saved to: $outputPath ($fileSize bytes)"
        return $true
    }
    catch {
        Write-Host "✗ Failed to fetch $filePath: $($_.Exception.Message)"
        return $false
    }
}

# Main function
function Main {
    Write-Host "========== Fetch Latest HTML Files =========="
    
    # Default IP
    $defaultIp = "192.168.4.1"
    
    # Get device IP
    $deviceIp = Read-Host -Prompt "Enter device IP [default: $defaultIp]"
    if (-not $deviceIp) {
        $deviceIp = $defaultIp
    }
    
    # HTML files to fetch
    $htmlFiles = @(
        "index.html",
        "device_status.html",
        "login.html",
        "settings_help.html",
        "wifi_config.html",
        "smart_oven_controller.html",
        "temperature_control.html"
    )
    
    # Create output directory
    $outputDir = "data_latest"
    Ensure-DirectoryExists -directory $outputDir
    
    # Backup current files
    $backupDir = "data_backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
    if (Test-Path -Path "data") {
        $dataFiles = Get-ChildItem -Path "data" -Filter "*.html" -ErrorAction SilentlyContinue
        if ($dataFiles) {
            Ensure-DirectoryExists -directory $backupDir
            foreach ($file in $dataFiles) {
                Copy-Item -Path $file.FullName -Destination (Join-Path -Path $backupDir -ChildPath $file.Name) -Force
            }
        }
    }
    
    # Fetch files
    $successCount = 0
    foreach ($htmlFile in $htmlFiles) {
        if (Fetch-FileFromDevice -deviceIp $deviceIp -filePath $htmlFile -outputDir $outputDir) {
            $successCount++
        } else {
            if (Fetch-FileFromDevice -deviceIp $deviceIp -filePath "data/$htmlFile" -outputDir $outputDir) {
                $successCount++
            }
        }
        Start-Sleep -Seconds 1
    }
    
    # Summary
    Write-Host "\nCompleted! Success: $successCount/$($htmlFiles.Count) files"
    Write-Host "Files saved to: $outputDir"
    
    # Ask to update main data directory
    if ($successCount -gt 0) {
        $replace = Read-Host -Prompt "\nUpdate main data directory? (y/n)"
        if ($replace.ToLower() -eq 'y') {
            Ensure-DirectoryExists -directory "data"
            $latestFiles = Get-ChildItem -Path $outputDir -Filter "*.html" -ErrorAction SilentlyContinue
            foreach ($file in $latestFiles) {
                Copy-Item -Path $file.FullName -Destination (Join-Path -Path "data" -ChildPath $file.Name) -Force
                Write-Host "Copied: $($file.Name)"
            }
        }
    }
}

Main