#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import sys

# 设置标准输出编码为UTF-8
sys.stdout.reconfigure(encoding='utf-8')

# 读取index.html文件
with open('index.html', 'r', encoding='utf-8') as f:
    content = f.read()

# 查找alert、prompt、confirm中的中文文本
alerts = re.findall(r'alert\s*\(\s*[\'\"](.*?)[\'\"]\s*\)', content)
prompts = re.findall(r'prompt\s*\(\s*[\'\"](.*?)[\'\"]\s*\)', content)
confirms = re.findall(r'confirm\s*\(\s*[\'\"](.*?)[\'\"]\s*\)', content)

# 合并所有硬编码文本
all_hardcoded = alerts + prompts + confirms

# 筛选出包含中文的文本
chinese_texts = [text for text in all_hardcoded if re.search(r'[\u4e00-\u9fff]', text)]

print(f'找到 {len(chinese_texts)} 个硬编码的中文文本:')
for i, text in enumerate(chinese_texts):
    print(f'  {i+1}. {text}')