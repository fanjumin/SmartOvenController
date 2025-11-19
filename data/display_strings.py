import os

def display_chinese_strings():
    """显示提取的中文字符串"""
    file_path = os.path.join(os.path.dirname(__file__), 'chinese_strings_new.txt')
    
    if not os.path.exists(file_path):
        print("文件不存在:", file_path)
        return
    
    print("提取的中文字符串:")
    print("=" * 50)
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            
        # 显示包含实际中文内容的行（过滤掉只有符号的行）
        count = 0
        chinese_texts = []
        
        for line in lines:
            if line.strip() and not line.startswith('#'):
                # 提取引号中的内容
                if '"' in line:
                    parts = line.split('"')
                    if len(parts) >= 2:
                        chinese_text = parts[1]
                        # 检查是否包含中文字符
                        if any('\u4e00' <= char <= '\u9fff' for char in chinese_text):
                            chinese_texts.append(chinese_text)
        
        # 显示前100个中文字符串
        for i, text in enumerate(chinese_texts[:100]):
            print(f"{i+1:2d}. {text}")
        
        total_count = len(chinese_texts)
        if total_count > 100:
            print(f"\n还有 {total_count - 100} 个中文字符串未显示...")
        print(f"\n总共找到 {total_count} 个中文字符串")
        
    except Exception as e:
        print("读取文件时出错:", str(e))

if __name__ == "__main__":
    display_chinese_strings()