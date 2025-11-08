#!/usr/bin/env python3

"""
æºè½çµç¤ç®±é¡µé¢èªå¨å·æ°å·¥å·
è§£å³Webé¡µé¢ç¼å­å¯¼è´çé¡µé¢ä¸æ´æ°é®é¢
"""

import os
import time
import subprocess
import webbrowser

def add_timestamp_to_html():
    """å¨HTMLæä»¶ä¸­æ·»å æ¶é´æ³ï¼å¼ºå¶æµè§å¨éæ°å è½½"""
    
    html_files = ['index.html', 'mobile_home.html', 'temperature_control.html']
    
    for html_file in html_files:
        if os.path.exists(html_file):
            print(f"ð å¤çæä»¶: {html_file}")
            
            # è¯»åæä»¶åå®¹
            with open(html_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # æ£æ¥æ¯å¦å·²ç»ææ¶é´æ³
            if '?v=' in content:
                # æ´æ°æ¶é´æ³
                import re
                timestamp = str(int(time.time()))
                content = re.sub(r'\?v=\d+', f'?v={timestamp}', content)
                print(f"   ð æ´æ°æ¶é´æ³: {timestamp}")
            else:
                # æ·»å æ¶é´æ³å°CSSåJSå¼ç¨
                timestamp = str(int(time.time()))
                
                # å¨headæ ç­¾åæ·»å æ¶é´æ³
                if '<head>' in content:
                    head_pos = content.find('<head>') + len('<head>')
                    timestamp_comment = f'\n    <!-- èªå¨å·æ°æ¶é´æ³: {timestamp} -->'
                    content = content[:head_pos] + timestamp_comment + content[head_pos:]
                
                # ä¸ºCSSåJSæä»¶æ·»å æ¶é´æ³åæ°
                content = content.replace('.css"', f'.css?v={timestamp}"')
                content = content.replace('.js"', f'.js?v={timestamp}"')
                print(f"   â æ·»å æ¶é´æ³: {timestamp}")
            
            # ååæä»¶
            with open(html_file, 'w', encoding='utf-8') as f:
                f.write(content)
            
            print(f"   â æä»¶æ´æ°å®æ")

def restart_web_server():
    """éå¯Webæå¡å¨"""
    print("\nð éå¯Webæå¡å¨...")
    
    # æ¥æ¾å¹¶ç»æ­¢ç°æçWebæå¡å¨è¿ç¨
    try:
        # å¨Windowsä¸æ¥æ¾Python HTTPæå¡å¨è¿ç¨
        result = subprocess.run(['tasklist', '/fi', 'imagename eq python.exe'], 
                              capture_output=True, text=True)
        if 'python.exe' in result.stdout:
            print("   â ï¸ æ£æµå°è¿è¡çPythonè¿ç¨ï¼å»ºè®®æå¨éå¯æå¡å¨")
    except:
        pass
    
    print("   ð¡ å»ºè®®æå¨éå¯: python -m http.server 8000")

def open_browser_with_refresh():
    """ç¨å¼ºå¶å·æ°æ¹å¼æå¼æµè§å¨"""
    print("\nð æå¼æµè§å¨å¹¶å¼ºå¶å·æ°...")
    
    # ä½¿ç¨webbrowseræå¼é¡µé¢
    url = "http://localhost:8000"
    webbrowser.open(url)
    
    print(f"   â å·²æå¼: {url}")
    print("   ð è¯·å¨æµè§å¨ä¸­æ§è¡:")
    print("   - Windows: Ctrl+Shift+R")
    print("   - Mac: Cmd+Shift+R")
    print("   - ææF12 â å³é®å·æ° â 'æ¸ç©ºç¼å­å¹¶ç¡¬æ§éæ°å è½½'")

def main():
    """ä¸»å½æ°"""
    print("ð§ æºè½çµç¤ç®±é¡µé¢èªå¨å·æ°å·¥å·")
    print("=" * 50)
    
    # 1. æ·»å æ¶é´æ³å°HTMLæä»¶
    add_timestamp_to_html()
    
    # 2. éå¯Webæå¡å¨å»ºè®®
    restart_web_server()
    
    # 3. æå¼æµè§å¨
    open_browser_with_refresh()
    
    print("\n" + "=" * 50)
    print("ð¯ é¡µé¢å·æ°æä½å®æï¼")
    print("ð åç»­æä½:")
    print("1. å¦æé¡µé¢ä»æªæ´æ°ï¼è¯·æå¨éå¯Webæå¡å¨")
    print("2. å¨æµè§å¨ä¸­å¼ºå¶å·æ°é¡µé¢")
    print("3. æ¸é¤æµè§å¨ç¼å­")

if __name__ == "__main__":
    main()