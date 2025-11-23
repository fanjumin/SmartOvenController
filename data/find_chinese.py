import re
import sys

def find_chinese_strings(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Find all strings containing Chinese characters
    pattern = r'"([^"]*[\u4e00-\u9fff]+[^"]*)"'
    matches = re.findall(pattern, content)
    
    # Print unique matches with line numbers
    lines = content.split('\n')
    for i, line in enumerate(lines):
        for match in matches:
            if match in line:
                print(f"{i+1}: {match}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        find_chinese_strings(sys.argv[1])
    else:
        print("Usage: python find_chinese.py <file_path>")