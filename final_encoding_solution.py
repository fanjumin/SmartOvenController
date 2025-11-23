import re
import shutil
from datetime import datetime

def final_encoding_solution():
    """最终的编码解决方案"""
    print("Starting final encoding solution...")
    
    # 备份原文件
    backup_name = f"data/index.html.final_solution_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    shutil.copy2('data/index.html', backup_name)
    print(f"Backup created: {backup_name}")
    
    # 以二进制模式读取文件
    with open('data/index.html', 'rb') as f:
        content_bytes = f.read()
    
    # 尝试多种解码方式
    content = None
    encodings = ['utf-8', 'gbk', 'gb2312', 'latin1']
    
    for encoding in encodings:
        try:
            content = content_bytes.decode(encoding)
            print(f"Successfully decoded with {encoding}")
            break
        except UnicodeDecodeError:
            print(f"Failed to decode with {encoding}")
            continue
    
    if content is None:
        print("Failed to decode with any encoding, using utf-8 with errors ignored")
        content = content_bytes.decode('utf-8', errors='ignore')
    
    # 查找并修复常见的UTF-8乱码模式
    # 这些是UTF-8字符被错误显示的模式
    fixes = {
        # 中文字符的常见乱码
        'Ã¤Â¸Â­': '中',
        'Ã¤Â¸Â': '一',
        'Ã¤Â¸Â­': '中',
        'Ã¤Âºâ€º': '些',
        'Ã¨Â®Â¾': '设',
        'Ã§Â½â€œ': '置',
        'Ã¦Â¸Â©': '温',
        'Ã¥Âºâ€¢': '度',
        'Ã§â€Â¨': '用',
        'Ã¦Ë†Â·': '户',
        'Ã§â€ºâ€™': '系',
        'Ã§Â»â€': '统',
        'Ã¨Â¯Â­': '语',
        'Ã¨Â¨â‚¬': '言',
        'Ã¦â€”Â¶': '时',
        'Ã©â€”Â´': '间',
        'Ã¥Â¼â€º': '开',
        'Ã¥â€¦Â³': '关',
        'Ã§â€ºâ€™': '状',
        'Ã¦â‚¬Â': '态',
        'Ã¥Â·Â¥': '工',
        'Ã¤Â½â€”': '作',
        'Ã¦Â¨Â¡': '模',
        'Ã¥Â¼â€º': '式',
        'Ã¨â€¡Âª': '自',
        'Ã¥Å Â¨': '动',
        'Ã¦â€°â€¹': '手',
        'Ã¥â€¦Â³': '于',
        'Ã¥â€ºÂ¢': '团',
        'Ã©Ëœâ€¡': '队',
        'Ã¥Â®â„¢': '帮',
        'Ã¥Å Â©': '助',
        'Ã¨Â¿â„¢': '这',
        'Ã¦ËœÂ¯': '是',
        'Ã¤Â¸â€”': '世',
        'Ã§â€¢â€º': '界',
        'Ã¥Â¤â€¡': '备',
        'Ã§Â®â€¡': '管',
        'Ã§Ââ€ ': '理',
        'Ã¨Â¿â„¢': '程',
        'Ã¥Âºâ€¡': '序',
        'Ã¦â€¡â€': '控',
        'Ã¥Ë†â€”': '制',
        'Ã¨Â®Â¡': '计',
        'Ã¥Â¤â€”': '外',
        'Ã¥Â®â„¢': '设',
        'Ã¥Â¤â€¡': '备',
        'Ã§â€ºâ€™': '信',
        'Ã¦â‚¬Â¯': '息',
        'Ã¥â€¦Â¶': '其',
        'Ã¤Â»â€“': '他',
        'Ã¥Ââ€˜': '发',
        'Ã¥Â±â€¢': '展',
        'Ã¥â€¦Â¬': '公',
        'Ã¥Ââ€”': '司',
        'Ã¨Ââ€': '联',
        'Ã§Â³Â»': '系',
        'Ã¦Ë†â€˜': '我',
        'Ã¤Â»Â¬': '们',
        'Ã¥â€ºÂ¢': '团',
        'Ã©Ëœâ€¡': '队',
        'Ã¤Â»Â¶': '件',
        'Ã¥â€°â€¡': '则',
        'Ã¥Â®â„¢': '帮',
        'Ã¥Å Â©': '助',
        'Ã¥â€¦Â³': '于',
        'Ã¥â€ºÂ¢': '团',
        'Ã©Ëœâ€¡': '队',
        'Ã¥Â®â„¢': '帮',
        'Ã¥Å Â©': '助',
        'Ã¨Â¿â„¢': '这',
        'Ã¦ËœÂ¯': '是',
        'Ã¤Â¸â€”': '世',
        'Ã§â€¢â€º': '界',
        'Ã¥Â¤â€¡': '备',
        'Ã§Â®â€¡': '管',
        'Ã§Ââ€ ': '理',
        'Ã¨Â¿â„¢': '程',
        'Ã¥Âºâ€¡': '序',
        'Ã¦â€¡â€': '控',
        'Ã¥Ë†â€”': '制',
        'Ã¨Â®Â¡': '计',
        'Ã¥Â¤â€”': '外',
        'Ã¥Â®â„¢': '设',
        'Ã¥Â¤â€¡': '备',
        'Ã§â€ºâ€™': '信',
        'Ã¦â‚¬Â¯': '息',
        'Ã¥â€¦Â¶': '其',
        'Ã¤Â»â€“': '他',
        'Ã¥Ââ€˜': '发',
        'Ã¥Â±â€¢': '展',
        'Ã¥â€¦Â¬': '公',
        'Ã¥Ââ€”': '司',
        'Ã¨Ââ€': '联',
        'Ã§Â³Â»': '系',
        'Ã¦Ë†â€˜': '我',
        'Ã¤Â»Â¬': '们'
    }
    
    # 应用修复
    fixed_count = 0
    for garbled, fixed in fixes.items():
        count = content.count(garbled)
        if count > 0:
            content = content.replace(garbled, fixed)
            fixed_count += count
            # print(f"Replaced '{garbled}' with '{fixed}' ({count} occurrences)")
    
    # 处理更复杂的乱码模式
    # 这些是连续的乱码字符
    complex_patterns = [
        (r'Ã[^ ]{2,}', ''),  # 移除无法识别的乱码
    ]
    
    for pattern, replacement in complex_patterns:
        matches = re.findall(pattern, content)
        if matches:
            # 只移除这些乱码，不替换为特定字符
            content = re.sub(pattern, replacement, content)
            fixed_count += len(matches)
            print(f"Removed {len(matches)} occurrences of pattern '{pattern}'")
    
    # 写回文件
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"Final encoding solution completed. Fixed {fixed_count} issues.")
    return fixed_count

if __name__ == "__main__":
    count = final_encoding_solution()
    print(f"Process completed. Fixed {count} encoding issues in total.")