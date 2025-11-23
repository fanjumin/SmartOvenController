import re

def find_real_hardcoded_chinese():
    """查找真正的硬编码中文文本（不在data-lang属性中的中文文本）"""
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 查找所有中文文本片段
    chinese_matches = re.findall(r'[\\u4e00-\\u9fff]+', content)
    print(f"Total Chinese characters found: {len(chinese_matches)}")
    
    # 查找data-lang属性中的中文文本
    data_lang_matches = re.findall(r'data-lang=[\'"](.*?)[\'"]', content)
    data_lang_chinese = []
    for match in data_lang_matches:
        if re.search(r'[\\u4e00-\\u9fff]', match):
            data_lang_chinese.append(match)
    
    print(f"Chinese texts in data-lang attributes: {len(data_lang_chinese)}")
    
    # 查找JavaScript中的中文文本
    js_chinese_matches = re.findall(r'[\'\"]([^\'\"]*[\\u4e00-\\u9fff][^\'\"]*)[\'\"]', content)
    js_chinese = []
    for match in js_chinese_matches:
        if re.search(r'[\\u4e00-\\u9fff]', match):
            js_chinese.append(match)
    
    print(f"Chinese texts in JavaScript strings: {len(js_chinese)}")
    
    # 查找HTML标签内的中文文本（排除script和style标签）
    # 先移除script和style标签内容
    content_without_scripts = re.sub(r'<(script|style)[^>]*>.*?</\\1>', '', content, flags=re.DOTALL)
    # 查找标签之间的中文文本
    tag_content_matches = re.findall(r'>[^<]*[\\u4e00-\\u9fff][^<]*<', content_without_scripts)
    tag_chinese_texts = []
    for match in tag_content_matches:
        # 清理匹配结果
        cleaned = match.strip('><').strip()
        # 只保留包含中文且不是纯空白的文本
        if cleaned and re.search(r'[\\u4e00-\\u9fff]', cleaned):
            tag_chinese_texts.append(cleaned)
    
    print(f"Chinese texts in HTML tag content: {len(tag_chinese_texts)}")
    if tag_chinese_texts:
        print("Sample HTML tag Chinese texts:")
        for i, text in enumerate(tag_chinese_texts[:10]):
            print(f"  {i+1}. {text}")
    
    return tag_chinese_texts

if __name__ == "__main__":
    find_real_hardcoded_chinese()