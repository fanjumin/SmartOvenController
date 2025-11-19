import re

# Read the file and check for invalid characters
with open('index.html', 'rb') as f:
    content = f.read()

# Look for any invalid UTF-8 sequences
try:
    content.decode('utf-8')
    print("File is valid UTF-8")
except UnicodeDecodeError as e:
    print(f"UTF-8 decode error: {e}")

# Look for any unusual characters in the first 1000 bytes
text = content[:1000].decode('utf-8', errors='replace')
print("First 200 characters:")
print(repr(text[:200]))