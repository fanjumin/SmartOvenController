#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
固件反编译工具 - 从固件备份文件中提取完整的SmartOvenController.ino文件
"""

import os
import re
import struct
from datetime import datetime

def extract_ino_from_firmware(firmware_path, output_path=None):
    """从固件备份文件中提取.ino文件"""
    print("=== 开始提取SmartOvenController.ino文件 ===")
    
    if not os.path.exists(firmware_path):
        print(f"错误: 固件文件 {firmware_path} 不存在")
        return False
    
    if output_path is None:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        output_path = f"SmartOvenController_extracted_{timestamp}.ino"
    
    # 读取固件文件
    with open(firmware_path, 'rb') as f:
        firmware_data = f.read()
    
    print(f"固件文件大小: {len(firmware_data)} 字节")
    
    # 搜索.ino文件的关键特征
    # 1. 搜索Arduino代码的常见模式
    patterns = [
        rb'#include\s*<[^>]+>',  # 包含头文件
        rb'void\s+setup\s*\(\s*\)',  # setup函数
        rb'void\s+loop\s*\(\s*\)',  # loop函数
        rb'//\s*[=]+',  # 注释分隔符
        rb'#define\s+\w+',  # 宏定义
        rb'int\s+\w+\s*=',  # 变量定义
    ]
    
    # 搜索代码段
    code_segments = []
    
    # 搜索setup和loop函数之间的代码
    setup_pattern = rb'void\s+setup\s*\(\s*\)\s*\{'
    loop_pattern = rb'void\s+loop\s*\(\s*\)\s*\{'
    
    setup_matches = list(re.finditer(setup_pattern, firmware_data, re.IGNORECASE))
    loop_matches = list(re.finditer(loop_pattern, firmware_data, re.IGNORECASE))
    
    print(f"找到setup函数: {len(setup_matches)} 个")
    print(f"找到loop函数: {len(loop_matches)} 个")
    
    # 尝试提取完整的代码
    # 搜索常见的Arduino代码结构
    arduino_patterns = [
        rb'#include\s*<[^>]+>.*?void\s+setup\s*\(\s*\).*?void\s+loop\s*\(\s*\).*?\}',
        rb'//\s*SmartOvenController.*?void\s+setup',
        rb'THERMO_CLK\s*=',
        rb'MAX6675\s+thermo',
        rb'WebServer\s+webServer',
    ]
    
    # 尝试不同的提取策略
    extracted_content = None
    
    # 策略1: 搜索文件头
    header_patterns = [
        rb'//\s*=+.*?SmartOvenController.*?v[0-9\.]+',
        rb'//\s*SmartOvenController',
    ]
    
    for pattern in header_patterns:
        matches = list(re.finditer(pattern, firmware_data, re.IGNORECASE | re.DOTALL))
        if matches:
            print(f"找到文件头: {len(matches)} 个")
            # 从文件头开始提取到文件末尾
            start_pos = matches[0].start()
            # 尝试找到合理的结束位置
            end_patterns = [
                rb'}\s*$',  # 文件结束的大括号
                rb'//\s*END\s*OF\s*FILE',
            ]
            
            for end_pattern in end_patterns:
                end_matches = list(re.finditer(end_pattern, firmware_data[start_pos:], re.IGNORECASE))
                if end_matches:
                    end_pos = start_pos + end_matches[-1].end()
                    extracted_content = firmware_data[start_pos:end_pos]
                    print(f"使用策略1提取成功，长度: {len(extracted_content)} 字节")
                    break
    
    # 策略2: 如果策略1失败，尝试基于setup和loop函数提取
    if extracted_content is None and setup_matches and loop_matches:
        # 找到第一个setup函数的位置
        setup_start = setup_matches[0].start()
        # 向前搜索文件头
        header_search_range = firmware_data[max(0, setup_start-2000):setup_start]
        header_matches = list(re.finditer(rb'//\s*[=]+', header_search_range))
        
        if header_matches:
            header_start = setup_start - 2000 + header_matches[0].start()
        else:
            header_start = max(0, setup_start - 1000)
        
        # 找到最后一个loop函数的结束位置
        loop_end = loop_matches[-1].end()
        # 向后搜索文件结束
        footer_search_range = firmware_data[loop_end:min(len(firmware_data), loop_end+5000)]
        footer_matches = list(re.finditer(rb'}\s*$', footer_search_range))
        
        if footer_matches:
            file_end = loop_end + footer_matches[-1].end()
        else:
            file_end = min(len(firmware_data), loop_end + 3000)
        
        extracted_content = firmware_data[header_start:file_end]
        print(f"使用策略2提取成功，长度: {len(extracted_content)} 字节")
    
    # 策略3: 如果以上都失败，尝试基于已知的代码特征提取
    if extracted_content is None:
        # 搜索已知的代码片段
        known_patterns = [
            rb'THERMO_CLK\s*=\s*\d+',
            rb'MAX6675\s+thermo',
            rb'WebServer\s+webServer',
            rb'EEPROM\s*\.',
        ]
        
        for pattern in known_patterns:
            matches = list(re.finditer(pattern, firmware_data, re.IGNORECASE))
            if matches:
                # 以第一个匹配项为中心，提取前后内容
                center_pos = matches[0].start()
                start_pos = max(0, center_pos - 50000)  # 向前50KB
                end_pos = min(len(firmware_data), center_pos + 200000)  # 向后200KB
                
                extracted_content = firmware_data[start_pos:end_pos]
                print(f"使用策略3提取成功，长度: {len(extracted_content)} 字节")
                break
    
    # 如果提取成功，保存文件
    if extracted_content:
        try:
            # 尝试解码为UTF-8
            decoded_content = extracted_content.decode('utf-8', errors='ignore')
            
            # 修复常见的编码问题
            decoded_content = fix_encoding_issues(decoded_content)
            
            # 保存文件
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(decoded_content)
            
            print(f"✓ SmartOvenController.ino文件提取成功!")
            print(f"文件保存位置: {output_path}")
            print(f"文件大小: {len(decoded_content)} 字符")
            
            # 分析提取的内容
            analyze_extracted_content(decoded_content)
            
            return True
            
        except Exception as e:
            print(f"✗ 保存文件时出错: {e}")
            return False
    else:
        print("✗ 无法从固件中提取有效的.ino文件内容")
        return False

def fix_encoding_issues(content):
    """修复编码问题"""
    # 修复常见的乱码字符
    encoding_fixes = {
        '�': '',  # 移除无法识别的字符
        '\ufffd': '',  # 移除替换字符
    }
    
    for bad_char, replacement in encoding_fixes.items():
        content = content.replace(bad_char, replacement)
    
    # 修复常见的注释乱码
    comment_fixes = {
        '读�?': '读取',
        '有效�?': '有效性',
        '准确�?': '准确性',
        '温�?': '温度',
        '偏移�?': '偏移量',
        '空�?': '空间',
        '结�?': '结果',
        '状�?': '状态',
        '完�?': '完成',
        '连�?': '连接',
        '信�?': '信息',
        '正�?': '正在',
        '环�?': '环境',
        '新版�?': '新版本',
        '功�?': '功能',
        '控制�?': '控制器',
        '设�?': '设置',
    }
    
    for bad_text, fixed_text in comment_fixes.items():
        content = content.replace(bad_text, fixed_text)
    
    return content

def analyze_extracted_content(content):
    """分析提取的内容"""
    print("\n=== 内容分析 ===")
    
    # 统计基本信息
    lines = content.split('\n')
    print(f"总行数: {len(lines)}")
    
    # 检查关键函数
    functions_found = {
        'setup': 'setup' in content,
        'loop': 'loop' in content,
        'WiFi': 'WiFi' in content,
        'MAX6675': 'MAX6675' in content,
        'WebServer': 'WebServer' in content,
        'EEPROM': 'EEPROM' in content,
    }
    
    print("关键函数/类检查:")
    for func, found in functions_found.items():
        status = "✓" if found else "✗"
        print(f"  {status} {func}")
    
    # 检查版本信息
    version_patterns = [
        r'v[0-9]+\.[0-9]+\.[0-9]+',
        r'版本:\s*[0-9\.]+',
    ]
    
    for pattern in version_patterns:
        matches = re.findall(pattern, content)
        if matches:
            print(f"版本信息: {matches[0]}")
            break
    
    # 检查编码问题
    encoding_issues = content.count('�')
    if encoding_issues > 0:
        print(f"警告: 发现 {encoding_issues} 个编码问题字符")
    else:
        print("✓ 编码检查正常")

def main():
    firmware_path = "firmware_backup_20251107_212839.bin"
    
    if not os.path.exists(firmware_path):
        print(f"错误: 固件文件 {firmware_path} 不存在")
        return
    
    print("固件反编译工具 - SmartOvenController.ino提取")
    print("=" * 60)
    
    # 提取.ino文件
    success = extract_ino_from_firmware(firmware_path)
    
    if success:
        print("\n=== 提取完成 ===")
        print("下一步操作:")
        print("1. 检查提取的.ino文件内容")
        print("2. 修复可能存在的编码问题")
        print("3. 验证代码逻辑完整性")
        print("4. 与现有版本进行比较")
    else:
        print("\n=== 提取失败 ===")
        print("建议:")
        print("1. 检查固件文件是否完整")
        print("2. 尝试使用其他反编译工具")
        print("3. 从Git历史中恢复原始代码")

if __name__ == "__main__":
    main()