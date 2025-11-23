import re
import shutil
from datetime import datetime

def precise_encoding_fix():
    """精确修复文件中的编码问题"""
    print("Starting precise encoding fix...")
    
    # 备份原文件
    backup_name = f"data/index.html.precise_fix_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    shutil.copy2('data/index.html', backup_name)
    print(f"Backup created: {backup_name}")
    
    # 以二进制模式读取文件
    with open('data/index.html', 'rb') as f:
        content_bytes = f.read()
    
    # 尝试以latin1解码后再重新编码为UTF-8
    try:
        # 先以latin1解码（不会出错）
        content_latin1 = content_bytes.decode('latin1')
        
        # 定义常见的乱码模式映射
        encoding_fixes = {
            # UTF-8中文字符被错误解码为latin1后的表现形式
            'è®¾å¤‡': '设备',
            'çŠ¶æ€': '状态',
            'è®¾ç½®': '设置',
            'æ¸©åº¦': '温度',
            'åŠ çƒ­': '加热',
            'æ¨¡å¼': '模式',
            'è‡ªåŠ¨': '自动',
            'æ‰‹åŠ¨': '手动',
            'è‡ªå®šä¹‰': '自定义',
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
            'å¯†ç ': '密码',
            'ç™»å½•': '登录',
            'ç³»ç»Ÿ': '系统',
            'ç›‘æŽ§': '监控',
            'æ­£å¸¸': '正常',
            'å¼€å§‹': '开始',
            'åœæ­¢': '停止',
            'å…³é—­': '关闭',
            'å¼€å¯': '开启',
            'æ›´æ–°': '更新',
            'å›ºä»¶': '固件',
            'é‡å¯': '重启',
            'æ¸…é™¤': '清除',
            'æ˜¾ç¤º': '显示',
            'åˆ†é’Ÿ': '分钟',
            'æ—¶é—´': '时间',
            'é«˜çº§': '高级',
            'å®‰å…¨': '安全',
            'è¯­è¨€': '语言',
            'å¸®åŠ©': '帮助',
            'æ”¯æŒ': '支持',
            'å…³äºŽ': '关于',
            'æ›´æ–°': '更新',
            'å‡ºåŽ‚': '出厂',
            'è®¾å¤‡çŠ¶æ€': '设备状态',
            'è®¾å¤‡è®¾ç½®': '设备设置',
            'ç³»ç»Ÿä¿¡æ¯': '系统信息',
            'å·¥ää½œæ¨¡å¼': '工作模式',
            'å¸®åŠ©ä¸Žæ”¯æŒ': '帮助与支持',
            'å…³äºŽæˆ‘ä»¬': '关于我们',
            'ç³»ç»Ÿæ›´æ–°': '系统更新',
            'æ¸…é™¤æ•°æ®': '清除数据',
            'å®žæ—¶ç›‘æŽ§': '实时监控',
            'åŽ†å²è®°å½•': '历史记录',
            'æ•°æ®åˆ†æž': '数据分析',
            'è¿œç¨‹æŽ§åˆ¶': '远程控制',
            'æ™ºèƒ½æŽ¨è': '智能推荐',
            'èŠ‚èƒ½ä¼˜åŒ–': '节能优化',
            'å®šæ—¶ä»»åŠ¡': '定时任务',
            'é¢„ç½®èœå•': '预制菜单',
            'è‡ªåŠ¨æ¸…æ´—': '自动清洗',
            'æ•…éšœè¯Šæ–­': '故障诊断',
            'ç”µæºç®¡ç†': '电源管理',
            'éŸ³é¢‘æé†’': '音频提醒',
            'å¯è§†åŒ–æ“ä½œ': '可视化操作',
            'å¤šè¯­è¨€æ”¯æŒ': '多语言支持',
            'ç”¨æˆ·æƒé™': '用户权限',
            'æ•°æ®å¤‡ä»½': '数据备份',
            'æ¢å¤å‡ºåŽ‚è®¾ç½®': '恢复出厂设置',
            'è¿žæŽ¥WiFiç½‘ç»œ': '连接WiFi网络',
            'æŸ¥çœ‹è®¾å¤‡ä¿¡æ¯': '查看设备信息',
            'è®¾ç½®æ—¶é—´å’Œæ—¥æœŸ': '设置时间和日期',
            'è°ƒæ•´åŠ çƒ­å‚æ•°': '调整加热参数',
            'é…ç½®è¿žæŽ¥å¯†ç ': '配置连接密码',
            'å®šä¹‰è‡ªå®šä¹‰èœå•': '定义自定义菜单',
            'è®¾ç½®è¯­è¨€å’Œåœ°åŒº': '设置语言和地区',
            'å¯ç”¨è¿œç¨‹è®¿é—®': '启用远程访问',
            'æŸ¥çœ‹æ“ä½œåŽ†å²': '查看操作历史',
            'å¯¼å‡ºæ•°æ®æ–‡ä»¶': '导出数据文件',
            'å¯¼å…¥é…ç½®æ–‡ä»¶': '导入配置文件'
        }
        
        # 应用修复
        fixed_count = 0
        for garbled, fixed in encoding_fixes.items():
            count = content_latin1.count(garbled)
            if count > 0:
                content_latin1 = content_latin1.replace(garbled, fixed)
                fixed_count += count
                print(f"Replaced '{garbled}' with '{fixed}' ({count} occurrences)")
        
        # 将修复后的内容以UTF-8编码写回文件
        with open('data/index.html', 'w', encoding='utf-8') as f:
            f.write(content_latin1)
        
        print(f"Precise encoding fix completed. Fixed {fixed_count} encoding issues.")
        return fixed_count
        
    except Exception as e:
        print(f"Error during encoding fix: {e}")
        return 0

if __name__ == "__main__":
    count = precise_encoding_fix()
    print(f"Process completed. Fixed {count} encoding issues in total.")