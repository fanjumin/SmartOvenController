import os

def fix_remaining_encoding():
    filepath = 'data/index.html'
    backup_path = 'data/index.html.backup2'
    
    if os.path.exists(filepath):
        # Create backup
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Save backup
        with open(backup_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        # Additional fixes for remaining乱码
        additional_fixes = {
            'è®¾å¤‡çŠ¶æ€': '设备状态',
            'è®¾å¤‡': '设备',
            'é‡å¯è®¾å¤‡': '重启设备',
            'è·å–è®¾å¤‡çŠ¶æ€ä¿¡æ¯': '获取设备状态信息',
            'æ­£åœ¨è¿æ¥è®¾å¤‡': '正在连接设备',
            'è®¾å¤‡çŠ¶æ€å¡ç‰‡': '设备状态卡片',
            'è®¾å¤‡çŠ¶æ€å˜é‡': '设备状态变量'
        }
        
        # Apply additional fixes
        for garbled, fixed in additional_fixes.items():
            content = content.replace(garbled, fixed)
        
        # Write fixed content back
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        
        print("Remaining encoding issues fixed successfully!")
        print(f"Backup saved as: {backup_path}")

if __name__ == "__main__":
    fix_remaining_encoding()