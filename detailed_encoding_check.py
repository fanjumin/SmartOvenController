import chardet
import os

filepath = 'data/index.html'
if os.path.exists(filepath):
    with open(filepath, 'rb') as f:
        rawdata = f.read()
        result = chardet.detect(rawdata)
        print('File encoding:', result)
        
        # Try different encodings
        encodings = ['utf-8', 'gbk', 'gb2312', 'latin1']
        for encoding in encodings:
            try:
                with open(filepath, 'r', encoding=encoding) as f:
                    content = f.read()
                    luanma_count = content.count('æ˜¾ç¤º') + content.count('æ¢å¤') + content.count('å‡ºå‚') + content.count('è®¾ç½®')
                    print(f"Encoding {encoding}: 乱码 count = {luanma_count}")
            except Exception as e:
                print(f"Error reading with {encoding}: {e}")