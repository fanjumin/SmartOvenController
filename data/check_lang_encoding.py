import chardet

# 检查lang.js文件编码
with open('lang.js', 'rb') as f:
    raw_data = f.read()
    encoding_info = chardet.detect(raw_data)
    print(f'Encoding: {encoding_info["encoding"]}, Confidence: {encoding_info["confidence"]}')