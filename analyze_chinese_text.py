import re

def analyze_chinese_text():
    """分析HTML文件中的中文文本"""
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 移除script和style标签内容
    content_cleaned = re.sub(r'<(script|style)[^>]*>.*?</\\1>', '', content, flags=re.DOTALL)
    
    # 查找data-lang属性中的文本
    data_lang_texts = re.findall(r'data-lang=[\'"]([^\'"]*)[\'"]', content_cleaned)
    print(f"Texts in data-lang attributes: {len(data_lang_texts)}")
    
    # 查找标签之间的文本内容（包含中文的）
    tag_contents = re.findall(r'>[^<]*[\\u4e00-\\u9fff][^<]*<', content_cleaned)
    chinese_tag_texts = []
    for match in tag_contents:
        # 清理匹配结果
        cleaned = match.strip('><').strip()
        # 只保留包含中文且不是纯空白的文本
        if cleaned and re.search(r'[\\u4e00-\\u9fff]', cleaned):
            chinese_tag_texts.append(cleaned)
    
    print(f"Chinese texts in HTML tags: {len(chinese_tag_texts)}")
    
    # 显示一些示例
    print("\\nSample Chinese texts in HTML tags:")
    for i, text in enumerate(chinese_tag_texts[:15]):
        print(f"  {i+1:2d}. {text}")
    
    # 查找JavaScript中的中文文本
    js_strings = re.findall(r'[\'\"]([^\'\"]*[\\u4e00-\\u9fff][^\'\"]*)[\'\"]', content)
    chinese_js_texts = []
    for match in js_strings:
        if re.search(r'[\\u4e00-\\u9fff]', match):
            chinese_js_texts.append(match)
    
    print(f"\\nChinese texts in JavaScript: {len(chinese_js_texts)}")
    print("\\nSample Chinese texts in JavaScript:")
    for i, text in enumerate(chinese_js_texts[:10]):
        print(f"  {i+1:2d}. {text}")

if __name__ == "__main__":
    analyze_chinese_text()