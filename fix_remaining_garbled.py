import re
import shutil
from datetime import datetime

# 常见的乱码字符组合
garbled_patterns = {
    'æ˜¾ç¤º': '显示',
    'è®¾å¤‡': '设备',
    'çŠ¶æ€': '状态',
    'è®¾ç½®': '设置',
    'åˆ†é’Ÿ': '分钟',
    'åŠ çƒ­': '加热',
    'æ¸©åº¦': '温度',
    'æ¨¡å¼': '模式',
    'è‡ªåŠ¨': '自动',
    'æ‰‹åŠ¨': '手动',
    'è‡ªå®šä¹‰': '自定义',
    'åˆ‡æ¢': '切换',
    'æç¤º': '提示',
    'ç¡®è®¤': '确认',
    'å–æ¶ˆ': '取消',
    'ä¿å­˜': '保存',
    'å¤±è´¥': '失败',
    'æˆåŠŸ': '成功',
    'é”™è¯¯': '错误',
    'è­¦å‘Š': '警告',
    'ä¿¡æ¯': '信息',
    'ç½‘ç»œ': '网络',
    'è¿žæŽ¥': '连接',
    'æ–­å¼€': '断开',
    'å¯†ç ': '密码',
    'ç™»å½•': '登录',
    'ç™»å‡º': '登出',
    'ç³»ç»Ÿ': '系统',
    'ç›‘æŽ§': '监控',
    'æ­£å¸¸': '正常',
    'å¼€å§‹': '开始',
    'åœæ­¢': '停止',
    'å…³é—­': '关闭',
    'å¼€å¯': '开启',
    'å·¥ä½œ': '工作',
    'è¯·è¾“å…¥': '请输入',
    'é€‰æ‹©': '选择',
    'æ›´æ–°': '更新',
    'å›ºä»¶': '固件',
    'WiFi': 'WiFi',
    'é‡å¯': '重启',
    'æ¢å¤': '恢复',
    'å‡ºåŽ‚': '出厂',
    'æ¸…é™¤': '清除',
    'å°†è¢«': '将被'
}

def fix_garbled_characters():
    """修复文件中的乱码字符"""
    # 备份原文件
    backup_name = f"data/index.html.backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    shutil.copy2('data/index.html', backup_name)
    print(f"Backup created: {backup_name}")
    
    # 读取文件
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 计算修复前的乱码数量
    initial_garbled_count = 0
    for pattern in garbled_patterns:
        initial_garbled_count += content.count(pattern)
    
    print(f"Initial garbled characters found: {initial_garbled_count}")
    
    # 修复乱码
    fixed_count = 0
    for garbled, fixed in garbled_patterns.items():
        count = content.count(garbled)
        if count > 0:
            content = content.replace(garbled, fixed)
            fixed_count += count
            print(f"Replaced '{garbled}' with '{fixed}' ({count} occurrences)")
    
    # 写回文件
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"Fixed {fixed_count} garbled characters")
    return fixed_count

if __name__ == "__main__":
    count = fix_garbled_characters()
    print(f"Process completed. Fixed {count} garbled characters.")