#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil
import re
from datetime import datetime

def fix_chinese_encoding_advanced():
    """
    高级中文编码修复函数，专门处理UTF-8被错误解码的情况
    """
    # 创建备份
    html_file = 'data/index.html'
    backup_path = f'data/index.html.advanced_backup_{datetime.now().strftime("%Y%m%d_%H%M%S")}'
    shutil.copy(html_file, backup_path)
    print(f"已备份原文件到: {backup_path}")
    
    # 读取文件内容（以字节方式）
    with open(html_file, 'rb') as f:
        content_bytes = f.read()
    
    # 尝试多种解码方式
    content = None
    encodings = ['utf-8', 'gbk', 'gb2312', 'latin-1']
    
    for encoding in encodings:
        try:
            content = content_bytes.decode(encoding)
            if encoding != 'utf-8':
                # 如果不是UTF-8，需要重新编码为UTF-8
                content = content.encode(encoding).decode('utf-8')
            break
        except (UnicodeDecodeError, UnicodeEncodeError):
            continue
    
    if content is None:
        # 最后的备选方案
        content = content_bytes.decode('utf-8', errors='ignore')
    
    # 使用正则表达式查找和修复常见的乱码模式
    # 这些是UTF-8中文字符被错误解码为latin-1的结果
    patterns = {
        b'\xc3\xa6\xc5\x93\xc2\xba\xc3\xa8\xc2\x83\xc2\xbd\xc3\xa7\xc2\x83\xc2\xa4\xc3\xa7\xc2\xae\xc2\x91'.decode('latin-1'): '智能烤箱',
        b'\xc3\xa7\xc2\xae\xc2\xa1\xc3\xa7\xc2\x86'.decode('latin-1'): '管理',
        b'\xc3\xa6\xc5\x93\xc2\xbf\xc3\xa5\xc2\xba\xc2\xa6'.decode('latin-1'): '温度',
        b'\xc3\xa6\xc2\x97\xc3\xa6\xc2\x9d\xc2\xb4'.decode('latin-1'): '时间',
        b'\xc3\xa5\xc2\x8a\xc3\xa0\xc3\xa7\xc2\x83\xc2\xad'.decode('latin-1'): '加热',
        b'\xc3\xa7\xc2\x8a\xc2\xb6\xc3\xa6\xc2\x80'.decode('latin-1'): '状态',
        b'\xc3\xa8\xc2\xbf\xc2\x9e\xc3\xa6\xc2\x8e\xc2\xa5'.decode('latin-1'): '连接',
        b'\xc3\xa8\xc2\xae\xc2¾\xc3\xa5\xc2\xa4\xc2\x87'.decode('latin-1'): '设备',
        b'\xc3\xa5\xc2\xbc\xc2\x80\xc3\xa5\xc2\xa7\xc2\x8b'.decode('latin-1'): '开始',
        b'\xc3\xa5\xc2\x9c\xc2\xa6\xc3\xa6\xc2\xad\xc2\xa2'.decode('latin-1'): '停止',
        b'\xc3\xa8\xc2\xae\xc2¾\xc3\xa7\xc2\xbd\xc2\xae'.decode('latin-1'): '设置',
        b'\xc3\xa4\xc2\xbf\xc2\x9d\xc3\xa5\xc2\xad\xc2\x98'.decode('latin-1'): '保存',
        b'\xc3\xa9\xc2\x80\xc2\x80\xc3\xa5\xc2\x87\xc2xba'.decode('latin-1'): '退出',
        b'\xc3\xa7\xc2\x99\xc2\xbb\xc3\xa5\xc2\xbd\xc2\x95'.decode('latin-1'): '登录',
        b'\xc3\xa4\xc2\xbf\xc2\xae\xc3\xa6\xc2\x94\xc2\xb9\xc3\xa5\xc2\xaf\xc2\x86\xc3\xa7\xc2\xa0'.decode('latin-1'): '修改密码',
        b'\xc3\xa8\xc2\x87\xaa\xc3\xa5\xc2\x8a\xc2\xa8\xc3\xa6\xc2\xa8\xc2\xa1\xc3\xa5\xc2\xbc'.decode('latin-1'): '自动模式',
        b'\xc3\xa6\xc2\x89\xc2\x8b\xc3\xa5\xc2\x8a\xc2\xa8\xc3\xa6\xc2\xa8\xc2\xa1\xc3\xa5\xc2\xbc'.decode('latin-1'): '手动模式',
        b'\xc3\xa6\xc2\x98\xc2\xbe\xc3\xa7\xc2\xa4'.decode('latin-1'): '显示'
    }
    
    # 应用正则表达式替换
    for bad_pattern, good_text in patterns.items():
        # 使用正则表达式进行替换
        content = re.sub(re.escape(bad_pattern), good_text, content)
    
    # 手动修复一些特定的乱码
    manual_fixes = {
        'æ™ºèƒ½çƒ¤ç®±': '智能烤箱',
        'ç®¡ç†': '管理',
        'å‘˜æŽ§åˆ¶é¢': '员控制面',
        'æ¿åº¦': '温度',
        'åº¦': '度',
        'æ—¶é—´': '时间',
        'åŠ çƒ­': '加热',
        'åœ¨çº¿': '在线',
        'çŠ¶æ€': '状态',
        'è¿žæŽ¥': '连接',
        'è®¾å¤‡': '设备',
        'å¼€å§‹': '开始',
        'åœæ­¢': '停止',
        'èŽ·å–': '获取',
        'è¾“å…¥': '输入',
        'è®¾ç½®': '设置',
        'ä¿å­˜': '保存',
        'é€€å‡º': '退出',
        'ç™»å½•': '登录',
        'ä¿®æ”¹å¯†ç ': '修改密码',
        'é…è½½': '配置',
        'ç½‘ç»œ': '网络',
        'ä¿¡å·': '信号',
        'å›ºä»¶æ›´æ–°': '固件更新',
        'é‡è½½': '重载',
        'å¤±è´¥': '失败',
        'æˆæ•': '成功',
        'è¯·è¾“å…¥': '请输入',
        'ç¡®è®¤': '确认',
        'æ–°å¯†ç ': '新密码',
        'å½“å‰å¯†ç ': '当前密码',
        'ä¸€è‡´': '一致',
        'èŒƒå›´': '范围',
        'åº”ä¸º': '应为',
        'åˆ†é’Ÿ': '分钟',
        'è‡ªåŠ¨æ¨¡å¼': '自动模式',
        'æ‰‹åŠ¨æ¨¡å¼': '手动模式',
        'è°ƒèŠ‚': '调节',
        'é»˜è®¤å€¼': '默认值',
        'é›¶å€¼': '零值',
        'å·¥ä½œ': '工作',
        'å…³é—­': '关闭',
        'æ­£å¸¸': '正常',
        'æœªèŽ·å¾—': '未获得',
        'çœŸå®ž': '真实',
        'å¯ç”¨': '可用',
        'ä¸å¯ç”¨': '不可用',
        'é”™è¯¯': '错误',
        'ç³»ç»Ÿ': '系统',
        'ç›‘æŽ§': '监控',
        'ä¿¡æ¯': '信息',
        'æ•°æ®': '数据',
        'åˆ·æ–°': '刷新',
        'è¯¥åŠŸèƒ½': '该功能',
        'å·²åŠ è½½': '已加载',
        'è¯·ç­‰å¾…': '请等待',
        'çº¦ç§’': '约秒',
        'é‡è¿žæŽ¥': '重连接',
        'æ¢æ复å‡ºåŽ‚è®¾ç½®': '恢复出厂设置',
        'æ¸…é™¤': '清除',
        'é‡è½½': '重载',
        'æœ¬åœ°': '本地',
        'è¿œç¨‹': '远程',
        'æŽ§åˆ¶': '控制',
        'å‘½ä»¤': '命令',
        'å‘é€': '发送',
        'æŽ¥æ”¶': '接收',
        'å“åº”': '响应',
        'è¶…æ—¶': '超时',
        'æ— æ•ˆ': '无效',
        'æœ‰æ•ˆ': '有效',
        'å¯ç”¨': '可用',
        'ç¦ç”¨': '禁用',
        'å¯è§': '可见',
        'éšè—': '隐藏',
        'æ˜¾ç¤º': '显示',
        'éšè—': '隐藏',
        'å¯è§': '可见',
        'ç¦ç”¨': '禁用',
        'é‡è½½': '重载',
        'çº¦': '约',
        'é’Ÿ': '钟',
        'é‡è¿žæŽ¥': '重连接',
        'è®¾å¤‡': '设备',
        'æ­£åœ¨é‡è½½': '正在重载',
        'åŠ è½½ä¸­': '加载中',
        'è¿žæŽ¥ä¸­': '连接中',
        'ç­‰å¾…': '等待',
        'å®Œæˆ': '完成',
        'å¼€å§‹': '开始',
        'æš‚å': '暂',
        'å\ue21d\ue195': '置',
        'è\ue62c': '备',
        'å\ue62c': '备',
        'è\ue195': '置',
        'å\ue1bc': '置',
        'è\ue11d': '操',
        'è\ue1bc': '置',
        'è\ue632': '恢',
        'å\ue632': '恢',
        'è\ue044': '自',
        'ç\ue62c': '备',
        'é\ue21d\ue195': '置',
        'é\ue195': '置',
        'é\ue62c': '备',
        'é\ue1bc': '置',
        'é\ue11d': '操',
        'é\ue632': '恢',
        'é\ue044': '自',
        'ç\ue11d': '操',
        'ç\ue1bc': '置',
        'ç\ue195': '置',
        'ç\ue044': '自',
        'æ\ue21d\ue195': '置',
        'æ\ue195': '置',
        'æ\ue62c': '备',
        'æ\ue1bc': '置',
        'æ\ue11d': '操',
        'æ\ue632': '恢',
        'æ\ue044': '自'
    }
    
    # 应用手动替换
    for bad_text, good_text in manual_fixes.items():
        content = content.replace(bad_text, good_text)
    
    # 特殊处理标题
    content = content.replace('<title>鏅鸿兘鐑ょ\ue188 - 绠＄悊鍛樻帶鍒堕潰鏉?/title>', '<title>智能烤箱 - 管理员控制面板</title>')
    
    # 写回文件（UTF-8编码）
    with open(html_file, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print("HTML文件高级编码修复完成")
    
    # 修复lang.js文件
    lang_file = 'data/lang.js'
    lang_backup_path = f'data/lang.js.advanced_backup_{datetime.now().strftime("%Y%m%d_%H%M%S")}'
    shutil.copy(lang_file, lang_backup_path)
    print(f"已备份原lang.js文件到: {lang_backup_path}")
    
    # 读取lang.js文件（以字节方式）
    with open(lang_file, 'rb') as f:
        lang_content_bytes = f.read()
    
    # 尝试多种解码方式
    lang_content = None
    for encoding in encodings:
        try:
            lang_content = lang_content_bytes.decode(encoding)
            if encoding != 'utf-8':
                # 如果不是UTF-8，需要重新编码为UTF-8
                lang_content = lang_content.encode(encoding).decode('utf-8')
            break
        except (UnicodeDecodeError, UnicodeEncodeError):
            continue
    
    if lang_content is None:
        # 最后的备选方案
        lang_content = lang_content_bytes.decode('utf-8', errors='ignore')
    
    # 应用相同的修复
    for bad_pattern, good_text in patterns.items():
        lang_content = re.sub(re.escape(bad_pattern), good_text, lang_content)
    
    for bad_text, good_text in manual_fixes.items():
        lang_content = lang_content.replace(bad_text, good_text)
    
    # 写回文件（UTF-8编码）
    with open(lang_file, 'w', encoding='utf-8') as f:
        f.write(lang_content)
    
    print("lang.js文件高级编码修复完成")
    print("高级编码修复完成!")

if __name__ == "__main__":
    fix_chinese_encoding_advanced()