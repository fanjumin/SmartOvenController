import re

def analyze_hardcoded_chinese():
    """精确分析HTML文件中的硬编码中文文本"""
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
    print("\\nSample Chinese texts in HTML tags (not in data-lang):")
    count = 0
    for text in chinese_tag_texts[:30]:
        # 检查是否在data-lang属性中
        if text not in data_lang_texts:
            print(f"  {count+1:2d}. {text}")
            count += 1
            if count >= 15:
                break
    
    # 查找JavaScript中的中文文本
    js_content = re.findall(r'<script[^>]*>(.*?)</script>', content, re.DOTALL)
    js_chinese_texts = []
    
    for script in js_content:
        # 查找JavaScript字符串中的中文文本
        js_strings = re.findall(r'[\'\"]([^\'\"]*[\\u4e00-\\u9fff][^\'\"]*)[\'\"]', script)
        for match in js_strings:
            if re.search(r'[\\u4e00-\\u9fff]', match):
                js_chinese_texts.append(match)
    
    print(f"\\nChinese texts in JavaScript: {len(js_chinese_texts)}")
    print("\\nSample Chinese texts in JavaScript:")
    for i, text in enumerate(js_chinese_texts[:15]):
        print(f"  {i+1:2d}. {text}")
    
    # 统计需要国际化的文本
    texts_to_internationalize = []
    for text in chinese_tag_texts:
        if text not in data_lang_texts and len(text.strip()) > 1:
            texts_to_internationalize.append(text)
    
    print(f"\\nTexts that need to be internationalized: {len(texts_to_internationalize)}")
    
    # 显示需要国际化的文本示例
    print("\\nSample texts that need to be internationalized:")
    for i, text in enumerate(texts_to_internationalize[:20]):
        print(f"  {i+1:2d}. {text}")

if __name__ == "__main__":
    analyze_hardcoded_chinese()