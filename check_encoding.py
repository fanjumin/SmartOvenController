import chardet
import os

filepath = 'data/index.html'
if os.path.exists(filepath):
    with open(filepath, 'rb') as f:
        rawdata = f.read()
        result = chardet.detect(rawdata)
        print('File encoding:', result)
        
        # Try to read with detected encoding
        try:
            with open(filepath, 'r', encoding=result['encoding']) as f:
                content = f.read()
                print("File read successfully with detected encoding")
                # Look for common乱码 patterns
                if 'æ˜¾ç¤º' in content or 'æ¢å¤' in content:
                    print("Found乱码 characters in file")
                else:
                    print("No obvious乱码 found")
        except Exception as e:
            print(f"Error reading file: {e}")