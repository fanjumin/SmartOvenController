#!/usr/bin/env python

"""
修复SmartOvenController.ino文件的版本信息和编码问题
"""

import os
import shutil

# 源文件路径
ino_file = "f:\\Github\\SmartOvenController\\src\\SmartOvenController.ino"
# 备份文件路径
backup_file = "f:\\Github\\SmartOvenController\\src\\SmartOvenController.ino.backup"

def main():
    print("开始修复SmartOvenController.ino文件...")
    
    # 创建备份
    if os.path.exists(ino_file):
        shutil.copy2(ino_file, backup_file)
        print(f"已创建备份文件: {backup_file}")
    else:
        print(f"错误: 找不到源文件 {ino_file}")
        return
    
    try:
        # 尝试用不同的编码读取文件
        encodings = ['utf-8', 'gbk', 'cp936', 'iso-8859-1']
        content = None
        
        for encoding in encodings:
            try:
                with open(ino_file, 'r', encoding=encoding) as f:
                    content = f.read()
                print(f"成功使用 {encoding} 编码读取文件")
                break
            except UnicodeDecodeError:
                continue
        
        if content is None:
            print("错误: 无法解码文件内容")
            return
        
        # 更新版本信息
        content = content.replace('v0.6.1', 'v0.7.7')
content = content.replace('版本: 0.6.1', '版本: 0.7.7')
content = content.replace('v0.6.5', 'v0.7.7')
content = content.replace('版本: 0.6.5', '版本: 0.7.7')
content = content.replace('v0.6.6', 'v0.7.7')
content = content.replace('版本: 0.6.6', '版本: 0.7.7')
content = content.replace('v0.7.0', 'v0.7.7')
content = content.replace('版本: 0.7.0', '版本: 0.7.7')
content = content.replace('v0.7.5', 'v0.7.7')
content = content.replace('版本: 0.7.5', '版本: 0.7.7')
        
        # 用UTF-8编码重新写入文件
        with open(ino_file, 'w', encoding='utf-8') as f:
            f.write(content)
        
        print("文件修复完成！")
        print("- 版本号已更新为 v0.7.7")
        print("- 文件已保存为 UTF-8 编码")
        print("- 备份文件已创建")
        
    except Exception as e:
        print(f"修复过程中出错: {str(e)}")
        # 如果出错，尝试恢复备份
        if os.path.exists(backup_file):
            shutil.copy2(backup_file, ino_file)
            print("已从备份恢复原始文件")

if __name__ == "__main__":
    main()