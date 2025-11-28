import http.server
import socketserver
import json
import os
from urllib.parse import urlparse, parse_qs

class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        
        # 处理WiFi扫描请求
        if path == '/scanwifi':
            self.handle_scan_wifi()
            return
            
        # 处理设备状态请求
        elif path == '/status':
            self.handle_device_status()
            return

        # 处理OTA更新页面请求
        elif path == '/ota_update':
            self.handle_ota_update_page()
            return

        # 处理文件列表请求
        elif path == '/file_list':
            self.handle_file_list()
            return

        # 其他请求使用默认处理
        else:
            print(f"[DEBUG] GET request to path: {path}")
            super().do_GET()
    
    def do_POST(self):
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        print(f"[DEBUG] POST request to path: {path}")

        # 澧峰勭悊WiFi淇濆瓨璇锋眰
        if path == '/savewifi':
            self.handle_save_wifi()
            return

        # 处理固件更新请求
        elif path == '/firmware_update':
            self.handle_firmware_update()
            return

        # 处理文件上传请求
        elif path in ['/filesystem_update', '/upload_file']:
            self.handle_file_upload(path)
            return

        # 鍏朵粬POST璇锋眰杩斿洖404
        else:
            print(f"[DEBUG] Unknown POST endpoint: {path}")
            self.send_error(404, "POST endpoint not found")
    
    def handle_scan_wifi(self):
        # 返回模拟的WiFi网络数据
        wifi_data = {
            "networks": [
                {"ssid": "Home_WiFi_5G", "signal": -45},
                {"ssid": "TP-LINK_2.4G", "signal": -55},
                {"ssid": "ChinaNet-XXXX", "signal": -65},
                {"ssid": "CMCC-XXXX", "signal": -70},
                {"ssid": "Guest_WiFi", "signal": -75}
            ]
        }
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(wifi_data).encode())
    
    def handle_device_status(self):
        print("Handling device status request")
        # 返回模拟的设备状态数据
        status_data = {
            "deviceName": "Smart Oven",  # This should be translated on the client side
            "firmwareVersion": "v0.8.9",
            "currentTemperature": 25,
            "targetTemperature": 180,
            "heatingTime": 0,
            "remainingTime": 0,
            "status": "standby",  # This should be translated on the client side
            "wifiConnected": True,
            "wifiSSID": "Home_WiFi_5G",
            "ipAddress": "192.168.1.100",
            "macAddress": "00:11:22:33:44:55"
        }
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(status_data).encode())
    
    def handle_save_wifi(self):
        # 读取POST数据
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        
        # 解析表单数据
        form_data = parse_qs(post_data.decode('utf-8'))
        ssid = form_data.get('ssid', [''])[0]
        password = form_data.get('password', [''])[0]
        
        # 返回成功响应
        response = {
            "status": "success",
            "message": f"WiFi configuration saved, SSID: {ssid}"  # This should be translated on the client side
        }
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(response).encode())

    def handle_firmware_update(self):
        print("[DEBUG] Handling firmware update request")

        try:
            # 获取Content-Type和Content-Length
            content_type = self.headers.get('Content-Type', '')
            content_length = int(self.headers.get('Content-Length', 0))

            print(f"[DEBUG] Content-Type: {content_type}, Content-Length: {content_length}")

            if not content_type.startswith('multipart/form-data'):
                self.send_error(400, "Invalid content type for firmware update")
                return

            # 解析multipart数据
            boundary = content_type.split('boundary=')[1] if 'boundary=' in content_type else None
            if not boundary:
                self.send_error(400, "No boundary found in multipart data")
                return

            print(f"[DEBUG] Boundary: {boundary}")

            # 读取请求体
            body = self.rfile.read(content_length)
            print(f"[DEBUG] Read {len(body)} bytes of request body")

            # 简单的multipart解析
            boundary_bytes = b'--' + boundary.encode()
            parts = body.split(boundary_bytes)

            uploaded_file = None
            filename = "firmware.bin"

            for part in parts:
                if b'Content-Disposition' in part:
                    # 查找filename
                    if b'filename=' in part:
                        filename_start = part.find(b'filename="') + len(b'filename="')
                        filename_end = part.find(b'"', filename_start)
                        if filename_end > filename_start:
                            filename = part[filename_start:filename_end].decode()
                            print(f"[DEBUG] Found filename: {filename}")

                            # 查找文件数据
                            data_start = part.find(b'\r\n\r\n') + 4
                            if data_start > 3:
                                uploaded_file = part[data_start:].rstrip(b'\r\n')
                                print(f"[DEBUG] Firmware file data size: {len(uploaded_file)} bytes")
                                break

            if uploaded_file:
                # 保存固件文件
                filepath = f"uploaded_{filename}"
                with open(filepath, 'wb') as f:
                    f.write(uploaded_file)

                print(f"[DEBUG] Firmware saved to: {filepath}")

                response = {
                    "status": "success",
                    "message": f"Firmware {filename} uploaded successfully",
                    "filename": filename,
                    "size": len(uploaded_file),
                    "action": "restart"
                }

                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps(response).encode())
            else:
                print("[DEBUG] No firmware file data found")
                self.send_error(400, "No firmware file uploaded")

        except Exception as e:
            print(f"[DEBUG] Error in firmware update: {str(e)}")
            self.send_error(500, f"Firmware update failed: {str(e)}")

    def handle_file_upload(self, path):
        print(f"[DEBUG] Handling file upload for path: {path}")

        try:
            # 获取Content-Type和Content-Length
            content_type = self.headers.get('Content-Type', '')
            content_length = int(self.headers.get('Content-Length', 0))

            print(f"[DEBUG] Content-Type: {content_type}, Content-Length: {content_length}")

            if not content_type.startswith('multipart/form-data'):
                self.send_error(400, "Invalid content type for file upload")
                return

            # 解析multipart数据
            boundary = content_type.split('boundary=')[1] if 'boundary=' in content_type else None
            if not boundary:
                self.send_error(400, "No boundary found in multipart data")
                return

            print(f"[DEBUG] Boundary: {boundary}")

            # 读取请求体
            body = self.rfile.read(content_length)
            print(f"[DEBUG] Read {len(body)} bytes of request body")

            # 简单的multipart解析（简化实现）
            boundary_bytes = b'--' + boundary.encode()
            parts = body.split(boundary_bytes)

            uploaded_file = None
            filename = None

            for part in parts:
                if b'Content-Disposition' in part:
                    # 查找filename
                    if b'filename=' in part:
                        filename_start = part.find(b'filename="') + len(b'filename="')
                        filename_end = part.find(b'"', filename_start)
                        if filename_end > filename_start:
                            filename = part[filename_start:filename_end].decode()
                            print(f"[DEBUG] Found filename: {filename}")

                            # 查找文件数据
                            data_start = part.find(b'\r\n\r\n') + 4
                            if data_start > 3:
                                uploaded_file = part[data_start:].rstrip(b'\r\n')
                                print(f"[DEBUG] File data size: {len(uploaded_file)} bytes")
                                break

            if uploaded_file and filename:
                # 保存文件
                filepath = f"uploaded_{filename}"
                with open(filepath, 'wb') as f:
                    f.write(uploaded_file)

                print(f"[DEBUG] File saved to: {filepath}")

                response = {
                    "status": "success",
                    "message": f"File {filename} uploaded successfully",
                    "filename": filename,
                    "size": len(uploaded_file)
                }

                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps(response).encode())
            else:
                print("[DEBUG] No file data found in upload")
                self.send_error(400, "No file uploaded")

        except Exception as e:
            print(f"[DEBUG] Error in file upload: {str(e)}")
            self.send_error(500, f"Upload failed: {str(e)}")

    def handle_file_list(self):
        print("[DEBUG] Handling file list request")
        try:
            import os
            files = []
            # 列出当前目录下的文件
            for filename in os.listdir('.'):
                if os.path.isfile(filename):
                    file_size = os.path.getsize(filename)
                    files.append({
                        "name": filename,
                        "size": file_size
                    })

            response = {"files": files}
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
            print(f"[DEBUG] File list sent: {len(files)} files")
        except Exception as e:
            print(f"[DEBUG] Error listing files: {str(e)}")
            self.send_error(500, f"Failed to list files: {str(e)}")

    def handle_ota_update_page(self):
        print("[DEBUG] Serving OTA update page")
        html_content = """<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>智能烤箱 - OTA升级</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        .container { max-width: 700px; margin: 0 auto; background: white; padding: 25px; border-radius: 10px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
        .status-info { background: #e8f4fd; border-left: 4px solid #007bff; padding: 15px; margin: 15px 0; border-radius: 4px; }
        .upgrade-option { display: flex; align-items: center; padding: 20px; margin: 15px 0; border: 2px solid #e0e0e0; border-radius: 8px; cursor: pointer; transition: all 0.3s; }
        .upgrade-option:hover { border-color: #007bff; background: #f8f9fa; }
        .option-icon { font-size: 24px; margin-right: 15px; width: 40px; text-align: center; }
        .option-content { flex: 1; }
        .option-title { font-size: 18px; font-weight: bold; margin-bottom: 5px; }
        .option-desc { color: #666; font-size: 14px; }
        .tab { display: none; }
        .active { display: block; }
        button { background: #007bff; color: white; border: none; padding: 12px 24px; border-radius: 6px; cursor: pointer; margin: 5px; font-size: 14px; font-weight: bold; }
        button:hover { background: #0056b3; }
        .progress { width: 100%; height: 20px; background: #f0f0f0; border-radius: 10px; margin: 15px 0; }
        .progress-bar { height: 100%; background: #007bff; border-radius: 10px; width: 0%; transition: width 0.3s; }
        .file-list { margin: 10px 0; }
        .file-item { background: #f8f9fa; padding: 8px 12px; margin: 5px 0; border-radius: 4px; border-left: 3px solid #007bff; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 智能烤箱控制器 OTA升级</h1>

        <div class="status-info">
            <strong>设备状态:</strong><br>
            • 固件版本: 0.8.9<br>
            • 运行时间: 正在运行<br>
            • 可用内存: 正常<br>
            • WiFi状态: 已连接
        </div>

        <div class="tab active" id="mainTab">
            <h3>📋 选择升级类型</h3>
            <p>请根据您的需求选择合适的升级方式：</p>

            <div class="upgrade-option" onclick="showTab('firmwareTab')">
                <div class="option-icon">🔧</div>
                <div class="option-content">
                    <div class="option-title">固件升级 (.bin 文件)</div>
                    <div class="option-desc">更新设备主程序，包含功能改进和错误修复。升级后设备将自动重启。</div>
                </div>
            </div>

            <div class="upgrade-option" onclick="showTab('fsTab')">
                <div class="option-icon">💾</div>
                <div class="option-content">
                    <div class="option-title">文件系统更新 (.bin 文件)</div>
                    <div class="option-desc">更新完整的文件系统镜像，包含所有HTML、JS、CSS等界面文件。</div>
                </div>
            </div>

            <div class="upgrade-option" onclick="showTab('fileTab')">
                <div class="option-icon">📄</div>
                <div class="option-content">
                    <div class="option-title">单个文件上传</div>
                    <div class="option-desc">逐个上传HTML、JS、CSS等文件，实时更新界面文件。</div>
                </div>
            </div>
        </div>

        <div class="tab" id="fileTab">
            <h3>📄 单个文件上传</h3>
            <p><strong>推荐方式：</strong>逐个上传文件，避免复杂的文件系统镜像更新。</p>
            <form action="/upload_file" method="post" enctype="multipart/form-data" onsubmit="return uploadSingleFile(this)">
                <p><strong>选择目标路径:</strong></p>
                <select name="target_path" style="margin:10px 0;padding:8px;border:1px solid #ddd;border-radius:4px;width:100%;">
                    <option value="/index.html">/index.html - 主控制页面</option>
                    <option value="/login.html">/login.html - 登录页面</option>
                    <option value="/wifi_config.html">/wifi_config.html - WiFi配置页面</option>
                    <option value="/device_status.html">/device_status.html - 设备状态页面</option>
                    <option value="/temperature_calibration.html">/temperature_calibration.html - 温度校准页面</option>
                    <option value="/settings_help.html">/settings_help.html - 设置帮助页面</option>
                    <option value="/mobile_utils.js">/mobile_utils.js - 移动工具脚本</option>
                    <option value="/lang.js">/lang.js - 多语言支持</option>
                </select>
                <p><strong>选择文件:</strong></p>
                <input type="file" name="file" accept=".html,.js,.css,.bin,.json,.txt" required style="margin:10px 0;padding:8px;border:1px solid #ddd;border-radius:4px;width:100%;">
                <br><button type="submit">📤 上传文件</button>
            </form>
            <div class="progress"><div class="progress-bar" id="fileProgress"></div></div>
            <p id="fileStatus"></p>
            <div class="file-list"><h4>当前文件列表:</h4><div id="fileList">加载中...</div></div>
            <button onclick="showTab('mainTab')">⬅️ 返回选择</button>
        </div>

        <div class="tab" id="firmwareTab">
            <h3>🔧 固件升级</h3>
            <p><strong>重要提示：</strong>固件升级将重启设备，请确保电源稳定。</p>
            <form action="/firmware_update" method="post" enctype="multipart/form-data" onsubmit="return uploadFirmware(this)">
                <p><strong>选择固件文件 (.bin):</strong></p>
                <input type="file" name="firmware" accept=".bin" required style="margin:10px 0;padding:8px;border:1px solid #ddd;border-radius:4px;width:100%;">
                <br><button type="submit">🚀 开始升级固件</button>
            </form>
            <div class="progress"><div class="progress-bar" id="firmwareProgress"></div></div>
            <p id="firmwareStatus"></p>
            <button onclick="showTab('mainTab')">⬅️ 返回选择</button>
        </div>

        <div class="tab" id="fsTab">
            <h3>💾 文件系统更新</h3>
            <p><strong>重要提示：</strong>文件系统更新将覆盖所有现有界面文件，请确保使用正确的.bin镜像文件。</p>
            <form action="/filesystem_update" method="post" enctype="multipart/form-data" onsubmit="return uploadFilesystem(this)">
                <p><strong>选择文件系统镜像 (.bin):</strong></p>
                <input type="file" name="filesystem" accept=".bin" required style="margin:10px 0;padding:8px;border:1px solid #ddd;border-radius:4px;width:100%;">
                <br><button type="submit">🚀 开始更新文件系统</button>
            </form>
            <div class="progress"><div class="progress-bar" id="fsProgress"></div></div>
            <p id="fsStatus"></p>
            <button onclick="showTab('mainTab')">⬅️ 返回选择</button>
        </div>
    </div>

    <script>
        function showTab(tabId) {
            console.log('Switching to tab:', tabId);
            var tabs = document.querySelectorAll('.tab');
            for(var i = 0; i < tabs.length; i++) {
                tabs[i].classList.remove('active');
            }
            var targetTab = document.getElementById(tabId);
            if(targetTab) {
                targetTab.classList.add('active');
                if(tabId === 'fileTab') {
                    loadFileList();
                }
            }
        }

        function uploadFirmware(form) {
            var xhr = new XMLHttpRequest();
            xhr.upload.onprogress = function(e) {
                if(e.lengthComputable) {
                    var percent = Math.round((e.loaded/e.total)*100);
                    document.getElementById('firmwareProgress').style.width = percent + '%';
                    document.getElementById('firmwareStatus').innerHTML = '上传进度: ' + percent + '%';
                }
            };
            xhr.onload = function() {
                if(xhr.status == 200) {
                    var response = JSON.parse(xhr.responseText);
                    if(response.status == 'success') {
                        document.getElementById('firmwareStatus').innerHTML = '✅ 固件升级成功！设备将在3秒后重启...';
                        setTimeout(function(){location.reload();}, 3000);
                    } else {
                        document.getElementById('firmwareStatus').innerHTML = '❌ 升级失败：' + response.message;
                    }
                } else {
                    document.getElementById('firmwareStatus').innerHTML = '❌ 升级失败：' + xhr.responseText;
                }
            };
            xhr.onerror = function() {
                document.getElementById('firmwareStatus').innerHTML = '❌ 网络错误，请重试';
            };
            xhr.open('POST', '/firmware_update');
            xhr.send(new FormData(form));
            return false;
        }

        function uploadFilesystem(form) {
            var xhr = new XMLHttpRequest();
            xhr.upload.onprogress = function(e) {
                if(e.lengthComputable) {
                    var percent = Math.round((e.loaded/e.total)*100);
                    document.getElementById('fsProgress').style.width = percent + '%';
                    document.getElementById('fsStatus').innerHTML = '上传进度: ' + percent + '%';
                }
            };
            xhr.onload = function() {
                if(xhr.status == 200) {
                    var response = JSON.parse(xhr.responseText);
                    if(response.status == 'success') {
                        document.getElementById('fsStatus').innerHTML = '✅ 文件系统更新成功！设备将在3秒后重启...';
                        // 如果响应中有action字段且为restart，则在设备重启后刷新页面
                        if(response.action == 'restart') {
                            setTimeout(function(){
                                document.getElementById('fsStatus').innerHTML = '🔄 设备正在重启，请稍候...';
                                // 等待设备重启完成后再刷新页面
                                setTimeout(function(){location.reload();}, 5000);
                            }, 3000);
                        } else {
                            setTimeout(function(){location.reload();}, 3000);
                        }
                    } else {
                        document.getElementById('fsStatus').innerHTML = '❌ 更新失败：' + response.message;
                    }
                } else {
                    document.getElementById('fsStatus').innerHTML = '❌ 更新失败：' + xhr.responseText;
                }
            };
            xhr.onerror = function() {
                document.getElementById('fsStatus').innerHTML = '❌ 网络错误，请重试';
            };
            xhr.open('POST', '/filesystem_update');
            xhr.send(new FormData(form));
            return false;
        }

        function uploadSingleFile(form) {
            var xhr = new XMLHttpRequest();
            xhr.upload.onprogress = function(e) {
                if(e.lengthComputable) {
                    var percent = Math.round((e.loaded/e.total)*100);
                    document.getElementById('fileProgress').style.width = percent + '%';
                    document.getElementById('fileStatus').innerHTML = '上传进度: ' + percent + '%';
                }
            };
            xhr.onload = function() {
                if(xhr.status == 200) {
                    var response = JSON.parse(xhr.responseText);
                    if(response.status == 'success') {
                        document.getElementById('fileStatus').innerHTML = '✅ 文件上传成功！';
                        loadFileList();
                    } else {
                        document.getElementById('fileStatus').innerHTML = '❌ 上传失败：' + response.message;
                    }
                } else {
                    document.getElementById('fileStatus').innerHTML = '❌ 上传失败：' + xhr.responseText;
                }
            };
            xhr.onerror = function() {
                document.getElementById('fileStatus').innerHTML = '❌ 网络错误，请重试';
            };
            xhr.open('POST', '/upload_file');
            xhr.send(new FormData(form));
            return false;
        }

        function loadFileList() {
            console.log('Loading file list...');
            fetch('/file_list').then(function(response) {
                return response.json();
            }).then(function(data) {
                console.log('File list loaded:', data);
                var listDiv = document.getElementById('fileList');
                listDiv.innerHTML = '';
                if(data.files && data.files.length > 0) {
                    for(var i = 0; i < data.files.length; i++) {
                        var item = document.createElement('div');
                        item.className = 'file-item';
                        item.innerHTML = data.files[i].name + ' (' + data.files[i].size + ' 字节)';
                        listDiv.appendChild(item);
                    }
                } else {
                    listDiv.innerHTML = '<div class="file-item">无文件</div>';
                }
            }).catch(function(error) {
                console.log('Error loading file list:', error);
                document.getElementById('fileList').innerHTML = '<div class="file-item">加载失败</div>';
            });
        }

        window.onload = function() {
            console.log('Page loaded, initializing...');
            loadFileList();
        };
    </script>
</body>
</html>"""

        self.send_response(200)
        self.send_header('Content-type', 'text/html; charset=utf-8')
        self.end_headers()
        self.wfile.write(html_content.encode('utf-8'))
        print("[DEBUG] OTA update page sent successfully")


# 鍒涘缓鏈嶅姟鍣?
PORT = 8083
Handler = CustomHTTPRequestHandler

with socketserver.TCPServer(("", PORT), Handler) as httpd:
    print(f"Server started on port {PORT}")
    print(f"Access URL: http://localhost:{PORT}")
    httpd.serve_forever()