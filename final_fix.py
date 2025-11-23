import os

def final_fix():
    filepath = 'data/index.html'
    backup_path = 'data/index.html.final_backup'
    
    if os.path.exists(filepath):
        # Create backup
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Save backup
        with open(backup_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        # Comprehensive fixes for remaining乱码
        comprehensive_fixes = {
            # Device related
            'è®¾å¤‡': '设备',
            'è®¾å¤‡çŠ¶æ€': '设备状态',
            'é‡å¯è®¾å¤‡': '重启设备',
            'è·å–è®¾å¤‡çŠ¶æ€ä¿¡æ¯': '获取设备状态信息',
            'æ­£åœ¨è¿æ¥è®¾å¤‡': '正在连接设备',
            'è®¾å¤‡çŠ¶æ€å¡ç‰‡': '设备状态卡片',
            'è®¾å¤‡çŠ¶æ€å˜é‡': '设备状态变量',
            
            # Settings related
            'è®¾ç½®': '设置',
            'æ‰€æœ‰è®¾ç½®': '所有设置',
            'é…ç½®': '配置',
            'æ¸…é™¤': '清除',
            'å°†è¢«': '将被',
            
            # Other common patterns
            'å¯ã€‚': '。',  # Part of "设备将重启"
            'å¹¶æ¢': '并恢',
            'å¤å‡º': '复出',
            'çŠ¶æ€': '状态',
            'ä¿¡æ¯': '信息'
        }
        
        # Apply comprehensive fixes
        for garbled, fixed in comprehensive_fixes.items():
            content = content.replace(garbled, fixed)
        
        # Write fixed content back
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        
        print("Final encoding issues fixed successfully!")
        print(f"Backup saved as: {backup_path}")

if __name__ == "__main__":
    final_fix()