import shutil
import datetime
import re

def ultimate_fix():
    # Read the HTML file
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Create backup
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    shutil.copy2('data/index.html', f'data/index.html.ultimate_fix_backup_{timestamp}')
    
    # Fix remaining garbled characters
    # These are hex-encoded characters that appear as乱码
    fixes = {
        '\x80': '',
        '\x81': '',
        '\x82': '',
        '\x83': '',
        '\x84': '',
        '\x85': '',
        '\x86': '',
        '\x87': '',
        '\x88': '',
        '\x89': '',
        '\x8a': '',
        '\x8b': '',
        '\x8c': '',
        '\x8d': '',
        '\x8e': '',
        '\x8f': '',
        '\x90': '',
        '\x91': '',
        '\x92': '',
        '\x93': '',
        '\x94': '',
        '\x95': '',
        '\x96': '',
        '\x97': '',
        '\x98': '',
        '\x99': '',
        '\x9a': '',
        '\x9b': '',
        '\x9c': '',
        '\x9d': '',
        '\x9e': '',
        '\x9f': '',
    }
    
    # Apply fixes
    fixed_content = content
    total_fixes = 0
    
    for pattern, replacement in fixes.items():
        count = fixed_content.count(pattern)
        total_fixes += count
        fixed_content = fixed_content.replace(pattern, replacement)
    
    # Additional regex patterns for more complex乱码
    # Remove sequences of non-printable characters
    fixed_content = re.sub(r'[\x00-\x1f\x7f-\x9f]+', '', fixed_content)
    
    # Write back to file
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(fixed_content)
    
    print(f"Ultimate fix applied! Removed {total_fixes}乱码 characters.")

if __name__ == "__main__":
    ultimate_fix()