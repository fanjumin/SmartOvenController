# -*- coding: utf-8 -*-
# 使用二进制模式读取文件
with open('index.html', 'rb') as f:
    content_bytes = f.read()

# 使用latin1编码解码（可以处理所有字节）
content = content_bytes.decode('latin1')

# 替换乱码字符串
# 注意：这里使用的是乱码字符串的实际字符表示
replacements = [
    ('/* 绉诲姩绔寜閽偣鍑绘晥鏋?*/', '/* 移动端按钮点击效果 */'),
    ('/* 绉诲姩绔紭鍖栨粴鍔?*/', '/* 移动端优化滚动 */'),
    ('// 娣诲姞CSS鍔ㄧ敾', '// 添加CSS动画')
]

for old, new in replacements:
    content = content.replace(old, new)

# 转换回字节并写入文件
with open('index.html', 'wb') as f:
    f.write(content.encode('utf-8'))

print('Successfully fixed all encoding issues')