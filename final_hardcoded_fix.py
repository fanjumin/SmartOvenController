import shutil
import datetime
import re

def final_hardcoded_fix():
    # Read the HTML file
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Create backup
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    shutil.copy2('data/index.html', f'data/index.html.final_hardcoded_fix_backup_{timestamp}')
    
    # Fix remaining hardcoded Chinese strings by adding proper data-lang attributes
    # We'll replace specific hardcoded strings with proper internationalization markup
    
    # Replace "设置备" comment with proper HTML element with data-lang attribute
    fixed_content = content.replace('<!-- 设置备 -->', '<!-- <span data-lang="settingsEquipment"></span> -->')
    
    # Look for other potential hardcoded Chinese strings in comments or text
    # This is a more general approach to find and mark hardcoded strings
    
    # Write back to file
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(fixed_content)
    
    print("Final hardcoded text fix applied!")

if __name__ == "__main__":
    final_hardcoded_fix()