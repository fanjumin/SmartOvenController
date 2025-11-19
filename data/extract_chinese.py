import re
import os
import codecs

def extract_chinese_strings(file_path):
    """从HTML和JS文件中提取中文字符串"""
    chinese_strings = set()
    
    # 读取文件内容
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 匹配引号中的中文字符串
    # 匹配单引号中的中文
    single_quote_pattern = r"'([^']*[\u4e00-\u9fff][^']*)'"
    # 匹配双引号中的中文
    double_quote_pattern = r'"([^"]*[\u4e00-\u9fff][^"]*)"'
    # 匹配反引号中的中文
    backtick_quote_pattern = r'`([^`]*[\u4e00-\u9fff][^`]*)`'
    # 匹配HTML标签中的中文
    html_tag_pattern = r'>[^<]*([\u4e00-\u9fff]+)[^<]*<'
    
    # 查找所有匹配项
    patterns = [single_quote_pattern, double_quote_pattern, backtick_quote_pattern, html_tag_pattern]
    
    for pattern in patterns:
        matches = re.findall(pattern, content)
        for match in matches:
            # 过滤掉纯标点符号和非常短的字符串
            if len(match.strip()) > 1:
                # 移除HTML标签
                clean_text = re.sub(r'<[^>]+>', '', match)
                if len(clean_text.strip()) > 1 and re.search(r'[\u4e00-\u9fff]', clean_text):
                    # 过滤掉包含代码的行
                    if not re.search(r'[{}();]', clean_text) and not re.search(r'\$\{.*\}', clean_text):
                        chinese_strings.add(clean_text.strip())
    
    return chinese_strings

def save_chinese_strings(strings, output_file):
    """将中文字符串保存到文件"""
    # 使用UTF-8编码写入文件，不带BOM
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write("# 提取出的中文字符串\n")
        f.write("# 格式: \"中文字符串\" : \"translation_key\"\n\n")
        for string in sorted(strings):
            # 生成翻译键（简单的空格替换为下划线）
            key = re.sub(r'[^\w\u4e00-\u9fff]', '_', string.lower())
            key = re.sub(r'_+', '_', key)  # 合并多个下划线
            key = key.strip('_')  # 移除首尾下划线
            f.write(f'"{string}" : "{key}",\n')

def main():
    # 获取脚本所在目录
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # 要处理的文件
    files_to_process = [
        os.path.join(script_dir, 'index.html'),
        os.path.join(script_dir, 'mobile_utils.js')
    ]
    
    # 输出文件
    output_file = os.path.join(script_dir, 'chinese_strings_new.txt')
    
    # 收集所有中文字符串
    all_chinese_strings = set()
    
    for file_path in files_to_process:
        if os.path.exists(file_path):
            print(f"Processing {os.path.basename(file_path)}...")
            strings = extract_chinese_strings(file_path)
            print(f"Found {len(strings)} Chinese strings in {os.path.basename(file_path)}")
            all_chinese_strings.update(strings)
        else:
            print(f"File not found: {file_path}")
    
    # 保存结果
    save_chinese_strings(all_chinese_strings, output_file)
    print(f"\nTotal unique Chinese strings found: {len(all_chinese_strings)}")
    print(f"Results saved to {output_file}")

if __name__ == "__main__":
    main()