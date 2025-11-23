#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil
from datetime import datetime

def fix_encoding_precisely():
    """
    精确修复编码问题，处理UTF-8被错误解码的情况
    """
    # 创建备份
    html_file = 'data/index.html'
    backup_path = f'data/index.html.precise_backup_{datetime.now().strftime("%Y%m%d_%H%M%S")}'
    shutil.copy(html_file, backup_path)
    print(f"已备份原文件到: {backup_path}")
    
    # 读取文件内容（以字节方式）
    with open(html_file, 'rb') as f:
        content_bytes = f.read()
    
    # 尝试不同的解码方式来修复乱码
    # 常见情况：UTF-8编码的文本被错误地以latin-1解码
    try:
        # 先以latin-1解码（不会出错），然后再编码为latin-1字节，最后以UTF-8解码
        content = content_bytes.decode('latin-1')
        # 将错误解码的字符串重新编码为latin-1字节，再以UTF-8解码
        content = content.encode('latin-1').decode('utf-8')
    except (UnicodeDecodeError, UnicodeEncodeError):
        # 如果上述方法失败，尝试其他方法
        try:
            content = content_bytes.decode('utf-8')
        except UnicodeDecodeError:
            try:
                content = content_bytes.decode('gbk')
            except UnicodeDecodeError:
                content = content_bytes.decode('utf-8', errors='ignore')
    
    # 修复特定的乱码模式
    # 这些是UTF-8中文字符被错误解码为latin-1的结果
    replacements = {
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
        'ç¦ç”¨': '禁用'
    }
    
    # 应用替换
    for bad, good in replacements.items():
        content = content.replace(bad, good)
    
    # 写回文件（UTF-8编码）
    with open(html_file, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print("HTML文件精确编码修复完成")
    
    # 修复lang.js文件
    lang_file = 'data/lang.js'
    lang_backup_path = f'data/lang.js.precise_backup_{datetime.now().strftime("%Y%m%d_%H%M%S")}'
    shutil.copy(lang_file, lang_backup_path)
    print(f"已备份原lang.js文件到: {lang_backup_path}")
    
    # 读取lang.js文件（以字节方式）
    with open(lang_file, 'rb') as f:
        lang_content_bytes = f.read()
    
    # 尝试不同的解码方式来修复乱码
    try:
        # 先以latin-1解码（不会出错），然后再编码为latin-1字节，最后以UTF-8解码
        lang_content = lang_content_bytes.decode('latin-1')
        lang_content = lang_content.encode('latin-1').decode('utf-8')
    except (UnicodeDecodeError, UnicodeEncodeError):
        # 如果上述方法失败，尝试其他方法
        try:
            lang_content = lang_content_bytes.decode('utf-8')
        except UnicodeDecodeError:
            try:
                lang_content = lang_content_bytes.decode('gbk')
            except UnicodeDecodeError:
                lang_content = lang_content_bytes.decode('utf-8', errors='ignore')
    
    # 应用相同的替换
    for bad, good in replacements.items():
        lang_content = lang_content.replace(bad, good)
    
    # 写回文件（UTF-8编码）
    with open(lang_file, 'w', encoding='utf-8') as f:
        f.write(lang_content)
    
    print("lang.js文件精确编码修复完成")
    print("精确编码修复完成!")

if __name__ == "__main__":
    fix_encoding_precisely()