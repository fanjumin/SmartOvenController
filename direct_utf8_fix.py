import re
import shutil
from datetime import datetime

def direct_utf8_fix():
    """直接修复UTF-8解码错误"""
    # 备份原文件
    backup_name = f"data/index.html.utf8_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    shutil.copy2('data/index.html', backup_name)
    print(f"Backup created: {backup_name}")
    
    # 读取文件内容为二进制
    with open('data/index.html', 'rb') as f:
        content_bytes = f.read()
    
    # 尝试将内容作为latin1解码，然后重新编码为UTF-8
    try:
        # 将bytes按latin1解码（每个字节都有效）
        latin1_decoded = content_bytes.decode('latin1')
        
        # 查找常见的UTF-8乱码模式
        # 这些是UTF-8中文字符被错误解码为latin1的结果
        fixes = {
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
            'é‡å¯': '重启',
            'æ¢å¤': '恢复',
            'å‡ºåŽ‚': '出厂',
            'æ¸…é™¤': '清除',
            'å°†è¢«': '将被'
        }
        
        # 应用修复
        fixed_count = 0
        for garbled, fixed in fixes.items():
            count = latin1_decoded.count(garbled)
            if count > 0:
                latin1_decoded = latin1_decoded.replace(garbled, fixed)
                fixed_count += count
                print(f"Replaced '{garbled}' with '{fixed}' ({count} occurrences)")
        
        # 将修复后的内容重新编码为UTF-8
        fixed_bytes = latin1_decoded.encode('utf-8')
        
        # 写回文件
        with open('data/index.html', 'wb') as f:
            f.write(fixed_bytes)
        
        print(f"Fixed {fixed_count} garbled patterns")
        return fixed_count
        
    except Exception as e:
        print(f"Error during UTF-8 fix: {e}")
        return 0

if __name__ == "__main__":
    count = direct_utf8_fix()
    print(f"Process completed. Fixed {count} garbled patterns.")