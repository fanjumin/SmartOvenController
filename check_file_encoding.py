import chardet

# 检查文件编码
with open('data/index.html', 'rb') as f:
    raw_data = f.read()
    result = chardet.detect(raw_data)
    print(f'文件编码: {result["encoding"]} (置信度: {result["confidence"]})')