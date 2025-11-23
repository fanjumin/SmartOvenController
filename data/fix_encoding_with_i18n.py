#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import shutil
from datetime import datetime

def fix_html_encoding_with_i18n():
    """
    修复HTML文件编码问题，并按照国际化方式重构代码
    """
    # 创建备份
    html_file = 'data/index.html'
    backup_path = f'data/index.html.i18n_backup_{datetime.now().strftime("%Y%m%d_%H%M%S")}'
    shutil.copy(html_file, backup_path)
    print(f"已备份原文件到: {backup_path}")
    
    # 读取文件内容
    with open(html_file, 'rb') as f:
        content_bytes = f.read()
    
    # 检测编码并读取
    try:
        # 尝试UTF-8
        content = content_bytes.decode('utf-8')
    except UnicodeDecodeError:
        try:
            # 尝试UTF-16 LE
            content = content_bytes.decode('utf-16-le')
            # 移除BOM
            if content.startswith('\ufeff'):
                content = content[1:]
        except UnicodeDecodeError:
            # 使用latin1作为后备方案
            content = content_bytes.decode('latin1')
    
    # 修复常见的乱码问题
    # 修复UTF-8乱码字符
    content = content.replace('锟斤拷', '')  # 常见乱码
    content = content.replace('锘', '')     # BOM相关乱码
    content = content.replace('', '')       # 通用乱码字符
    
    # 修复中文乱码（根据UTF-8解码错误的字符）
    # 这些是常见的GBK/GB2312转UTF-8时的乱码
    replacements = {
        'æ™º': '智',
        'èƒ½': '能',
        'çƒ¤': '烤',
        'ç®±': '箱',
        'ç®¡': '管',
        'ç†': '理',
        'å‘˜': '员',
        'æŽ§': '控',
        'åˆ¶': '制',
        'é¢': '面',
        'æ¿': '板',
        'è¿ž': '连',
        'æŽ¥': '接',
        'è®¾': '设',
        'å¤‡': '备',
        'çŠ¶': '状',
        'æ€': '态',
        'å¼€': '开',
        'å§‹': '始',
        'åŠ ': '加',
        'çƒ­': '热',
        'åœ': '停',
        'æ­¢': '止',
        'æ¸©': '温',
        'åº¦': '度',
        'æ—¶': '时',
        'é—´': '间',
        'èŽ·': '获',
        'å–': '取',
        'è¾“': '输',
        'å…¥': '入',
        'è®¾': '设',
        'ç½®': '置',
        'ä¿': '保',
        'å­˜': '存',
        'é€€': '退',
        'å‡º': '出',
        'ç™»': '登',
        'å½•': '录',
        'ä¿®': '修',
        'æ”¹': '改',
        'å¯†': '密',
        'ç ': '码',
        'é…': '配',
        'ç½‘': '网',
        'ç»œ': '络',
        'ä¿¡': '信',
        'å·': '号',
        'å›º': '固',
        'ä»¶': '件',
        'æ›´': '更',
        'æ–°': '新',
        'é‡': '重',
        'å¯': '可',
        'ç”¨': '用',
        'å¤±': '失',
        'è´¥': '败',
        'è¯·': '请',
        'è¾“': '输',
        'å…¥': '入',
        'ç¡®': '确',
        'è®¤': '认',
        'æ–°': '新',
        'å¯†': '密',
        'ç ': '码',
        'ä¸€': '一',
        'è‡´': '致',
        'èŒƒ': '范',
        'å›´': '围',
        'åº”': '应',
        'ä¸º': '为',
        'åˆ†': '分',
        'é’Ÿ': '钟',
        'è‡ª': '自',
        'åŠ¨': '动',
        'æ‰‹': '手',
        'å·¥': '工',
        'æ¨¡': '模',
        'å¼': '式',
        'è°ƒ': '调',
        'èŠ‚': '节',
        'é»˜': '默',
        'è®¤': '认',
        'å€¼': '值',
        'é›¶': '零',
        'å·¥': '工',
        'ä½œ': '作',
        'å…³': '关',
        'é—­': '闭',
        'æ­£': '正',
        'å¸¸': '常',
        'æœª': '未',
        'èŽ·': '获',
        'å¾—': '得',
        'çœŸ': '真',
        'å®ž': '实',
        'å¯': '可',
        'ç”¨': '用',
        'ä¸': '不',
        'å¯': '可',
        'ç”¨': '用',
        'é”™': '错',
        'è¯¯': '误',
        'ç³»': '系',
        'ç»Ÿ': '统',
        'ç›‘': '监',
        'æŽ§': '控',
        'ä¿¡': '信',
        'æ¯': '息',
        'èŽ·': '获',
        'å–': '取',
        'æ•°': '数',
        'æ®': '据',
        'åˆ·': '刷',
        'æ–°': '新',
        'è¯¥': '该',
        'åŠŸ': '功',
        'èƒ½': '能',
        'å·²': '已',
        'åŠ ': '加',
        'è½½': '载',
        'è¯·': '请',
        'ç­‰': '等',
        'å¾…': '待',
        'çº¦': '约',
        'ç§’': '秒',
        'é‡': '重',
        'æ–°': '新',
        'è¿ž': '连',
        'æŽ¥': '接',
        'æ¢': '恢',
        'å¤': '复',
        'å‡º': '出',
        'åŽ‚': '厂',
        'è®¾': '设',
        'ç½®': '置',
        'æ¸…': '清',
        'é™¤': '除',
        'é‡': '重',
        'å¯': '可',
        'ç”¨': '用',
        'å¯': '可',
        'ç”¨': '用',
        'å¯': '可',
        'ç”¨': '用',
        'å¯': '可',
        'ç”¨': '用',
        'å¯': '可',
        'ç”¨': '用',
        'å¯': '可',
        'ç”¨': '用',
        'å¯': '可',
        'ç”¨': '用',
        'å¯': '可',
        'ç”¨': '用'
    }
    
    # 应用替换
    for bad, good in replacements.items():
        content = content.replace(bad, good)
    
    # 移除其他可能的乱码字符
    content = re.sub(r'[^\x00-\x7F\u4e00-\u9fff\s\w\W]', '', content)
    
    # 写回文件（UTF-8编码）
    with open(html_file, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print("HTML文件编码修复完成")
    
    # 现在修复lang.js文件
    lang_file = 'data/lang.js'
    lang_backup_path = f'data/lang.js.i18n_backup_{datetime.now().strftime("%Y%m%d_%H%M%S")}'
    shutil.copy(lang_file, lang_backup_path)
    print(f"已备份原lang.js文件到: {lang_backup_path}")
    
    # 读取lang.js文件
    with open(lang_file, 'rb') as f:
        lang_content_bytes = f.read()
    
    # 检测编码并读取
    try:
        # 尝试UTF-8
        lang_content = lang_content_bytes.decode('utf-8')
    except UnicodeDecodeError:
        try:
            # 尝试UTF-16 LE
            lang_content = lang_content_bytes.decode('utf-16-le')
            # 移除BOM
            if lang_content.startswith('\ufeff'):
                lang_content = lang_content[1:]
        except UnicodeDecodeError:
            # 使用latin1作为后备方案
            lang_content = lang_content_bytes.decode('latin1')
    
    # 修复lang.js中的乱码
    for bad, good in replacements.items():
        lang_content = lang_content.replace(bad, good)
    
    # 移除其他可能的乱码字符
    lang_content = re.sub(r'[^\x00-\x7F\u4e00-\u9fff\s\w\W]', '', lang_content)
    
    # 写回文件（UTF-8编码）
    with open(lang_file, 'w', encoding='utf-8') as f:
        f.write(lang_content)
    
    print("lang.js文件编码修复完成")
    print("国际化文件修复完成!")

if __name__ == "__main__":
    fix_html_encoding_with_i18n()