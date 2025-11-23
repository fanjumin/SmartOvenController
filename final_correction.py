import shutil
import datetime

def final_correction():
    # Read the HTML file
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Create backup
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    shutil.copy2('data/index.html', f'data/index.html.final_correction_backup_{timestamp}')
    
    # Fix the specific乱码 character that's appearing many times
    # Based on our analysis, it seems to be a specific乱码 character
    fixed_content = content.replace('', '')
    
    # Write back to file
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(fixed_content)
    
    print("Final correction applied!")

if __name__ == "__main__":
    final_correction()