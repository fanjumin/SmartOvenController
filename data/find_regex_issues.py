import re
import os

def find_regex_issues(file_path):
    """查找HTML/JS文件中可能存在问题的正则表达式"""
    if not os.path.exists(file_path):
        print(f"文件不存在: {file_path}")
        return []
        
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    lines = content.split('\n')
    issues = []
    
    # 查找可能的正则表达式模式
    for i, line in enumerate(lines):
        # 匹配类似 /pattern/flags 的模式
        # 更精确地匹配JavaScript中的正则表达式
        patterns = re.findall(r'/[^/\n\r]*?[gimuy]*', line)
        for pattern in patterns:
            if len(pattern) > 1 and '/' in pattern:
                # 检查标志是否有效
                if pattern.endswith('/'):
                    # 没有标志，这是有效的
                    continue
                else:
                    # 有标志，检查是否有效
                    parts = pattern.split('/')
                    if len(parts) > 1:
                        flags = parts[-1]
                        # 检查是否有无效的标志
                        invalid_flags = set(flags) - set('gimuy')
                        if invalid_flags and len(flags) > 0:
                            issues.append({
                                'line': i + 1,
                                'text': pattern,
                                'invalid_flags': invalid_flags,
                                'full_line': line.strip()
                            })
    
    return issues

def find_new_regexp_issues(file_path):
    """查找new RegExp()中的问题"""
    if not os.path.exists(file_path):
        print(f"文件不存在: {file_path}")
        return []
        
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    lines = content.split('\n')
    issues = []
    
    # 查找new RegExp()模式
    for i, line in enumerate(lines):
        # 匹配new RegExp("pattern", "flags") 或 new RegExp('pattern', 'flags')
        matches = re.findall(r'new\s+RegExp\s*\(\s*["\'][^"\']*["\']\s*,\s*["\']([^"\']*)["\']\s*\)', line, re.IGNORECASE)
        for flags in matches:
            # 检查标志是否有效
            invalid_flags = set(flags) - set('gimuy')
            if invalid_flags and len(flags) > 0:
                issues.append({
                    'line': i + 1,
                    'text': f'new RegExp(..., "{flags}")',
                    'invalid_flags': invalid_flags,
                    'full_line': line.strip()
                })
    
    return issues

def check_all_files():
    """检查所有相关文件"""
    files_to_check = [
        "index.html",
        "mobile_utils.js",
        "lang.js",
        "captive_portal.html",
        "device_status.html",
        "login.html"
    ]
    
    script_dir = os.path.dirname(__file__)
    
    for file_name in files_to_check:
        file_path = os.path.join(script_dir, file_name)
        if os.path.exists(file_path):
            print(f"\n检查文件: {file_name}")
            print("=" * 40)
            
            regex_issues = find_regex_issues(file_path)
            regexp_issues = find_new_regexp_issues(file_path)
            
            all_issues = regex_issues + regexp_issues
            
            if all_issues:
                print(f"在 {file_name} 中找到 {len(all_issues)} 个可能的正则表达式问题:")
                for issue in all_issues:
                    print(f"  行 {issue['line']}: {issue['text']} (无效标志: {issue['invalid_flags']})")
                    print(f"    完整行: {issue['full_line'][:100]}...")
            else:
                print(f"在 {file_name} 中未找到明显的正则表达式问题")

if __name__ == "__main__":
    check_all_files()