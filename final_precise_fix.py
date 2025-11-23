import shutil
import datetime
import re

def final_precise_fix():
    # Read the HTML file
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Create backup
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    shutil.copy2('data/index.html', f'data/index.html.final_precise_fix_backup_{timestamp}')
    
    # Fix remaining garbled characters
    # Based on our analysis, these are common patterns we found
    fixes = {
        '误': '',  # Remove stray "误" characters
        'Â': '',   # Remove stray "Â" characters
        'Ã': '',   # Remove stray "Ã" characters
        'Â¤': '',  # Remove stray "Â¤" characters
        'Â¶': '',  # Remove stray "Â¶" characters
        'Â§': '',  # Remove stray "Â§" characters
        'Â°': '',  # Remove stray "Â°" characters
        'Â¯': '',  # Remove stray "Â¯" characters
        'Â½': '',  # Remove stray "Â½" characters
        'Â¼': '',  # Remove stray "Â¼" characters
        'Â¾': '',  # Remove stray "Â¾" characters
        'Â£': '',  # Remove stray "Â£" characters
        'Â¢': '',  # Remove stray "Â¢" characters
        'Â¥': '',  # Remove stray "Â¥" characters
        'Â©': '',  # Remove stray "Â©" characters
        'Â®': '',  # Remove stray "Â®" characters
        'Â§': '',  # Remove stray "Â§" characters
        'Âµ': '',  # Remove stray "Âµ" characters
        'Â²': '',  # Remove stray "Â²" characters
        'Â³': '',  # Remove stray "Â³" characters
        'Â¹': '',  # Remove stray "Â¹" characters
    }
    
    # Apply fixes
    fixed_content = content
    total_fixes = 0
    
    for pattern, replacement in fixes.items():
        count = fixed_content.count(pattern)
        total_fixes += count
        fixed_content = fixed_content.replace(pattern, replacement)
    
    # Write back to file
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(fixed_content)
    
    print(f"Final precise fix applied! Removed {total_fixes} characters.")

if __name__ == "__main__":
    final_precise_fix()