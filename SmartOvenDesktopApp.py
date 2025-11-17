#!/usr/bin/env python3

"""
智能烤箱控制器 - 桌面客户端
支持TCP连接，替代不稳定的Web界面
"""

import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import socket
import threading
import time

class SmartOvenClient:
    def __init__(self, root):
        self.root = root
        self.root.title("智能烤箱控制器 - 桌面客户端")
        self.root.geometry("600x700")
        self.root.resizable(True, True)
        
        # 连接状态
        self.connected = False
        self.socket = None
        self.receive_thread = None
        self.heartbeat_thread = None
        self.last_heartbeat_time = 0
        
        self.create_widgets()
        
    def create_widgets(self):
        # 主框架
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # 标题
        title_label = ttk.Label(main_frame, text="智能烤箱控制器", font=("Arial", 16, "bold"))
        title_label.grid(row=0, column=0, columnspan=2, pady=(0, 20))
        
        # 连接配置区域
        conn_frame = ttk.LabelFrame(main_frame, text="连接配置", padding="10")
        conn_frame.grid(row=1, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        # IP地址
        ttk.Label(conn_frame, text="IP地址:").grid(row=0, column=0, sticky=tk.W, padx=(0, 10))
        self.ip_entry = ttk.Entry(conn_frame, width=15)
        self.ip_entry.grid(row=0, column=1, sticky=tk.W)
        self.ip_entry.insert(0, "192.168.16.103")
        
        # 端口
        ttk.Label(conn_frame, text="端口:").grid(row=0, column=2, sticky=tk.W, padx=(20, 10))
        self.port_entry = ttk.Entry(conn_frame, width=8)
        self.port_entry.grid(row=0, column=3, sticky=tk.W)
        self.port_entry.insert(0, "8080")
        
        # 连接按钮
        self.connect_btn = ttk.Button(conn_frame, text="连接", command=self.toggle_connection)
        self.connect_btn.grid(row=0, column=4, padx=(20, 0))
        
        # 连接状态
        self.status_label = ttk.Label(conn_frame, text="状态: 未连接")
        self.status_label.grid(row=1, column=0, columnspan=5, sticky=tk.W, pady=(10, 0))
        
        # 控制区域
        control_frame = ttk.LabelFrame(main_frame, text="烤箱控制", padding="10")
        control_frame.grid(row=2, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        # 控制按钮
        btn_frame = ttk.Frame(control_frame)
        btn_frame.grid(row=0, column=0, columnspan=2)
        
        self.get_status_btn = ttk.Button(btn_frame, text="获取状态", 
                                       command=lambda: self.send_command("GET_STATUS"),
                                       state="disabled")
        self.get_status_btn.grid(row=0, column=0, padx=5, pady=5)
        
        self.set_target_btn = ttk.Button(btn_frame, text="设置温度", 
                                        command=lambda: self.show_temp_dialog(),
                                        state="disabled")
        self.set_target_btn.grid(row=0, column=1, padx=5, pady=5)
        
        self.toggle_heat_btn = ttk.Button(btn_frame, text="切换加热", 
                                         command=lambda: self.send_command("TOGGLE_HEAT"),
                                         state="disabled")
        self.toggle_heat_btn.grid(row=1, column=0, padx=5, pady=5)
        
        self.toggle_mode_btn = ttk.Button(btn_frame, text="切换模式", 
                                        command=lambda: self.send_command("TOGGLE_MODE"),
                                        state="disabled")
        self.toggle_mode_btn.grid(row=1, column=1, padx=5, pady=5)
        
        # 温度设置区域
        temp_frame = ttk.Frame(control_frame)
        temp_frame.grid(row=1, column=0, columnspan=2, pady=(10, 0))
        
        ttk.Label(temp_frame, text="快速温度设置:").grid(row=0, column=0, sticky=tk.W)
        
        # 常用温度按钮
        common_temps = ["100", "150", "200", "250"]
        for i, temp in enumerate(common_temps):
            btn = ttk.Button(temp_frame, text=f"{temp}°C", 
                           command=lambda t=temp: self.send_command(f"SET_TARGET:{t}"),
                           state="disabled")
            btn.grid(row=0, column=i+1, padx=5)
            setattr(self, f"temp_{temp}_btn", btn)
        
        # 通信日志区域
        log_frame = ttk.LabelFrame(main_frame, text="通信日志", padding="10")
        log_frame.grid(row=3, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 10))
        
        self.log_text = scrolledtext.ScrolledText(log_frame, width=70, height=15)
        self.log_text.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        self.log_text.insert(tk.END, "通信日志将显示在这里...\n\n")
        
        # 配置网格权重
        main_frame.columnconfigure(0, weight=1)
        main_frame.rowconfigure(3, weight=1)
        log_frame.columnconfigure(0, weight=1)
        log_frame.rowconfigure(0, weight=1)
        
        # 绑定窗口关闭事件
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
    def toggle_connection(self):
        if not self.connected:
            self.connect()
        else:
            self.disconnect()
    
    def connect(self):
        try:
            ip = self.ip_entry.get()
            port = int(self.port_entry.get())
            
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(5)
            self.socket.connect((ip, port))
            
            self.connected = True
            self.connect_btn.config(text="断开连接")
            self.status_label.config(text=f"状态: 已连接到 {ip}:{port}")
            self.enable_controls(True)
            
            # 启动接收线程
            self.receive_thread = threading.Thread(target=self.receive_messages, daemon=True)
            self.receive_thread.start()
            
            # 启动心跳线程
            self.heartbeat_thread = threading.Thread(target=self.heartbeat_loop, daemon=True)
            self.heartbeat_thread.start()
            
            self.log_message(f"✅ 成功连接到 {ip}:{port}")
            
        except Exception as e:
            messagebox.showerror("连接错误", f"连接失败: {str(e)}")
            self.log_message(f"❌ 连接失败: {str(e)}")
    
    def disconnect(self):
        self.connected = False
        
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
        
        self.connect_btn.config(text="连接")
        self.status_label.config(text="状态: 未连接")
        self.enable_controls(False)
        self.log_message("🔌 已断开连接")
    
    def send_command(self, command):
        if not self.connected:
            messagebox.showwarning("警告", "请先连接到设备")
            return
        
        try:
            self.socket.sendall((command + "\n").encode('utf-8'))
            self.log_message(f"📤 发送: {command}")
        except Exception as e:
            self.log_message(f"❌ 发送失败: {str(e)}")
            self.disconnect()
    
    def receive_messages(self):
        buffer = ""
        while self.connected:
            try:
                data = self.socket.recv(1024).decode('utf-8')
                if not data:
                    # 空数据可能是连接正常关闭，等待一段时间再检查
                    time.sleep(0.1)
                    continue
                    
                buffer += data
                lines = buffer.split('\n')
                buffer = lines[-1]  # 保留未完成的行
                
                for line in lines[:-1]:
                    if line.strip():
                        self.log_message(f"📥 接收: {line}")
                        
            except socket.timeout:
                # 超时是正常的，继续等待数据
                continue
            except ConnectionResetError:
                if self.connected:
                    self.log_message("❌ 连接被远程主机重置")
                break
            except Exception as e:
                if self.connected:
                    self.log_message(f"❌ 接收错误: {str(e)}")
                break
        
        if self.connected:
            self.root.after(0, self.disconnect)
    
    def show_temp_dialog(self):
        if not self.connected:
            messagebox.showwarning("警告", "请先连接到设备")
            return
        
        dialog = tk.Toplevel(self.root)
        dialog.title("设置目标温度")
        dialog.geometry("300x150")
        dialog.transient(self.root)
        dialog.grab_set()
        
        ttk.Label(dialog, text="请输入目标温度 (°C):").pack(pady=10)
        
        temp_entry = ttk.Entry(dialog, width=10)
        temp_entry.pack(pady=5)
        temp_entry.focus()
        
        def set_temp():
            temp = temp_entry.get().strip()
            if temp and temp.isdigit():
                self.send_command(f"SET_TARGET:{temp}")
                dialog.destroy()
            else:
                messagebox.showerror("错误", "请输入有效的温度值")
        
        btn_frame = ttk.Frame(dialog)
        btn_frame.pack(pady=10)
        
        ttk.Button(btn_frame, text="确认", command=set_temp).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="取消", command=dialog.destroy).pack(side=tk.LEFT, padx=5)
        
        dialog.bind('<Return>', lambda e: set_temp())
    
    def enable_controls(self, enabled):
        state = "normal" if enabled else "disabled"
        
        self.get_status_btn.config(state=state)
        self.set_target_btn.config(state=state)
        self.toggle_heat_btn.config(state=state)
        self.toggle_mode_btn.config(state=state)
        
        # 温度按钮
        for temp in ["100", "150", "200", "250"]:
            btn = getattr(self, f"temp_{temp}_btn")
            btn.config(state=state)
    
    def log_message(self, message):
        timestamp = time.strftime("%H:%M:%S")
        log_entry = f"[{timestamp}] {message}\n"
        
        self.root.after(0, lambda: self._update_log(log_entry))
    
    def _update_log(self, log_entry):
        self.log_text.insert(tk.END, log_entry)
        self.log_text.see(tk.END)
    
    def heartbeat_loop(self):
        """心跳机制，定期发送GET_STATUS命令保持连接活跃"""
        while self.connected:
            try:
                # 每30秒发送一次心跳
                time.sleep(30)
                if self.connected:
                    self.send_command("GET_STATUS")
                    self.last_heartbeat_time = time.time()
            except Exception as e:
                if self.connected:
                    self.log_message(f"❌ 心跳发送失败: {str(e)}")
                break
    
    def on_closing(self):
        self.disconnect()
        self.root.destroy()

def main():
    root = tk.Tk()
    app = SmartOvenClient(root)
    root.mainloop()

if __name__ == "__main__":
    main()