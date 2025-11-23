import os

def verify_fix():
    filepath = 'data/index.html'
    
    if os.path.exists(filepath):
        # Read the fixed file
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Check for remaining乱码
        luanma_patterns = ['æ˜¾ç¤º', 'è®¾ç½®', 'æ‰€æœ‰è®¾ç½®', 'è®¾å¤‡', 'å°†è¢«', 'æ¸…é™¤']
        found_luanma = []
        
        for pattern in luanma_patterns:
            if pattern in content:
                # Find all occurrences
                start = 0
                while True:
                    pos = content.find(pattern, start)
                    if pos == -1:
                        break
                    # Get context around the乱码
                    context_start = max(0, pos - 20)
                    context_end = min(len(content), pos + len(pattern) + 20)
                    context = content[context_start:context_end]
                    found_luanma.append((pattern, pos, context))
                    start = pos + 1
        
        if found_luanma:
            print(f"Found {len(found_luanma)} remaining乱码 patterns:")
            for pattern, pos, context in found_luanma[:10]:  # Show first 10
                print(f"  Position {pos}: '{pattern}' in context: ...{context}...")
        else:
            print("No obvious乱码 patterns found!")
        
        # Check for hard-coded Chinese text
        hard_coded_chinese = []
        lines = content.split('\n')
        for i, line in enumerate(lines):
            # Look for Chinese characters that are not in data-lang attributes
            if ('="' in line and '"' in line and 
                not ('data-lang="' in line) and 
                not ('lang.js' in line) and
                not ('getTranslation' in line)):
                # Check for Chinese characters
                chinese_chars = []
                for char in line:
                    if '\u4e00' <= char <= '\u9fff':  # Chinese character range
                        chinese_chars.append(char)
                if len(chinese_chars) > 3:  # More than 3 Chinese characters
                    hard_coded_chinese.append((i+1, line.strip(), ''.join(chinese_chars)))
        
        if hard_coded_chinese:
            print(f"\nFound {len(hard_coded_chinese)} lines with potential hard-coded Chinese text:")
            for line_num, line, chars in hard_coded_chinese[:10]:  # Show first 10
                print(f"  Line {line_num}: {chars}")
                # Show a snippet of the line
                if len(line) > 50:
                    print(f"    Snippet: ...{line[:50]}...")
                else:
                    print(f"    Snippet: {line}")
        else:
            print("\nNo obvious hard-coded Chinese text found outside of data-lang attributes!")

if __name__ == "__main__":
    verify_fix()