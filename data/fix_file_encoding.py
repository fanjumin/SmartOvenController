# -*- coding: utf-8 -*-
"""
修复文件编码问题的脚本
将index.html文件从错误的编码转换为UTF-8编码
"""

import os
import chardet

def fix_file_encoding(file_path):
    """修复文件编码"""
    # 检查文件是否存在
    if not os.path.exists(file_path):
        print(f"文件不存在: {file_path}")
        return False
    
    # 读取文件的原始字节
    with open(file_path, 'rb') as f:
        raw_data = f.read()
    
    # 检测文件编码
    detected = chardet.detect(raw_data)
    original_encoding = detected['encoding']
    confidence = detected['confidence']
    
    print(f"检测到的原始编码: {original_encoding} (置信度: {confidence:.2f})")
    
    # 如果已经是UTF-8且置信度高，不需要转换
    if original_encoding and original_encoding.lower() == 'utf-8' and confidence > 0.9:
        print("文件已经是UTF-8编码，无需转换")
        return True
    
    # 尝试用检测到的编码解码
    text = None
    if original_encoding:
        try:
            text = raw_data.decode(original_encoding)
            print(f"使用检测到的编码 {original_encoding} 成功解码")
        except Exception as e:
            print(f"使用检测到的编码 {original_encoding} 解码失败: {e}")
    
    # 如果上面的方法失败，尝试几种常见的编码
    if text is None:
        common_encodings = ['utf-8', 'gbk', 'gb2312', 'big5', 'latin1']
        for encoding in common_encodings:
            try:
                text = raw_data.decode(encoding)
                print(f"使用编码 {encoding} 成功解码")
                break
            except UnicodeDecodeError:
                continue
    
    # 如果所有编码都失败，使用latin1（它可以解码任何字节序列，但可能产生乱码）
    if text is None:
        text = raw_data.decode('latin1')
        print("使用latin1解码（可能包含乱码）")
    
    # 用UTF-8编码重新写入文件
    try:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(text)
        print(f"文件已成功转换为UTF-8编码: {file_path}")
        return True
    except Exception as e:
        print(f"写入文件失败: {e}")
        return False

if __name__ == "__main__":
    # 修复index.html文件
    index_file = os.path.join(os.path.dirname(__file__), 'index.html')
    print("修复index.html文件编码...")
    fix_file_encoding(index_file)
    
    # 修复lang.js文件
    lang_file = os.path.join(os.path.dirname(__file__), 'lang.js')
    print("修复lang.js文件编码...")
    fix_file_encoding(lang_file)
    
    print("编码修复完成!")