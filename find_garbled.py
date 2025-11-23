def find_garbled_characters():
    """查找文件中的乱码字符"""
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 查找所有包含Ã的字符串
    pos = 0
    count = 0
    positions = []
    
    while pos < len(content) and count < 50:
        idx = content.find('Ã', pos)
        if idx == -1:
            break
        # 获取周围的字符以便查看上下文
        start = max(0, idx - 10)
        end = min(len(content), idx + 20)
        print(f"Position {idx}: '{content[start:end]}'")
        positions.append(idx)
        pos = idx + 1
        count += 1
    
    print(f"\nFound {len(positions)} occurrences of 'Ã' character")
    
    # 分析乱码模式
    patterns = {}
    for pos in positions[:20]:  # 分析前20个
        # 获取包含Ã的完整字符串
        start = pos
        while start > 0 and content[start-1] != ' ' and content[start-1] != '\n':
            start -= 1
        end = pos
        while end < len(content) and content[end] != ' ' and content[end] != '\n' and content[end] != '<' and content[end] != '>':
            end += 1
        pattern = content[start:end]
        if pattern in patterns:
            patterns[pattern] += 1
        else:
            patterns[pattern] = 1
    
    print("\nCommon patterns found:")
    for pattern, freq in sorted(patterns.items(), key=lambda x: x[1], reverse=True)[:10]:
        print(f"  {pattern}: {freq} times")

if __name__ == "__main__":
    find_garbled_characters()