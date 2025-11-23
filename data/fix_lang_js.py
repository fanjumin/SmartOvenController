#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import sys

# 设置标准输出编码为UTF-8
sys.stdout.reconfigure(encoding='utf-8')

# 读取lang.js文件
with open('lang.js', 'r', encoding='utf-8') as f:
    content = f.read()

# 修复格式错误的行
# 修复类似 "cookies",: ""饼干"", 的行
content = re.sub(r'(\s*)"([^"]+)",:\s*"([^"]+)"', r'\1"\2": "\3"', content)

# 修复类似 " : "key",: ""value"", 的行
content = re.sub(r'(\s*)"\s*:\s*"([^"]+)",:\s*"([^"]+)"', r'\1"\2": "\3"', content)

# 修复类似 " : "key" 的行（没有逗号）
content = re.sub(r'(\s*)"\s*:\s*"([^"]+)"\s*$', r'\1"\2"', content)

# 再次修复，确保所有行都正确
lines = content.split('\n')
fixed_lines = []
for line in lines:
    # 修复类似 "cookies",: ""饼干"", 的行
    line = re.sub(r'"([^"]+)",:\s*"([^"]+)"', r'"\1": "\2"', line)
    # 修复类似 " : "key",: ""value"", 的行
    line = re.sub(r'"\s*:\s*"([^"]+)",:\s*"([^"]+)"', r'"\1": "\2"', line)
    fixed_lines.append(line)

content = '\n'.join(fixed_lines)

# 写回文件
with open('lang.js', 'w', encoding='utf-8') as f:
    f.write(content)

print("lang.js文件已修复！")