#!/usr/bin/env python3

"""
æºè½ç¤ç®±æ§å¶å¨ - æ¡é¢å®¢æ·ç«¯
æ¯æTCPè¿æ¥ï¼æ¿ä»£ä¸ç¨³å®çWebçé¢
"""

import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import socket
import threading
import time

class SmartOvenClient:
    def __init__(self, root):
        self.root = root
        self.root.title("æºè½ç¤ç®±æ§å¶å¨ - æ¡é¢å®¢æ·ç«¯")
        self.root.geometry("600x700")
        self.root.resizable(True, True)
        
        # è¿æ¥ç¶æ
        self.connected = False
        self.socket = None
        self.receive_thread = None
        self.heartbeat_thread = None
        self.last_heartbeat_time = 0
        
        self.create_widgets()
        
    def create_widgets(self):
        # ä¸»æ¡æ¶
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # æ é¢
        title_label = ttk.Label(main_frame, text="æºè½ç¤ç®±æ§å¶å¨", font=("Arial", 16, "bold"))
        title_label.grid(row=0, column=0, columnspan=2, pady=(0, 20))
        
        # è¿æ¥è®¾ç½®åºå
        conn_frame = ttk.LabelFrame(main_frame, text="è¿æ¥è®¾ç½®", padding="10")
        conn_frame.grid(row=1, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        # IPå°å
        ttk.Label(conn_frame, text="IPå°å:").grid(row=0, column=0, sticky=tk.W, padx=(0, 10))
        self.ip_entry = ttk.Entry(conn_frame, width=15)
        self.ip_entry.grid(row=0, column=1, sticky=tk.W)
        self.ip_entry.insert(0, "192.168.16.103")
        
        # ç«¯å£
        ttk.Label(conn_frame, text="ç«¯å£:").grid(row=0, column=2, sticky=tk.W, padx=(20, 10))
        self.port_entry = ttk.Entry(conn_frame, width=8)
        self.port_entry.grid(row=0, column=3, sticky=tk.W)
        self.port_entry.insert(0, "8080")
        
        # è¿æ¥æé®
        self.connect_btn = ttk.Button(conn_frame, text="è¿æ¥", command=self.toggle_connection)
        self.connect_btn.grid(row=0, column=4, padx=(20, 0))
        
        # è¿æ¥ç¶æ
        self.status_label = ttk.Label(conn_frame, text="ç¶æ: æªè¿æ¥")
        self.status_label.grid(row=1, column=0, columnspan=5, sticky=tk.W, pady=(10, 0))
        
        # æ§å¶åºå
        control_frame = ttk.LabelFrame(main_frame, text="ç¤ç®±æ§å¶", padding="10")
        control_frame.grid(row=2, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        # æ§å¶æé®
        btn_frame = ttk.Frame(control_frame)
        btn_frame.grid(row=0, column=0, columnspan=2)
        
        self.get_status_btn = ttk.Button(btn_frame, text="è·åç¶æ", 
                                       command=lambda: self.send_command("GET_STATUS"),
                                       state="disabled")
        self.get_status_btn.grid(row=0, column=0, padx=5, pady=5)
        
        self.set_target_btn = ttk.Button(btn_frame, text="è®¾ç½®æ¸©åº¦", 
                                        command=lambda: self.show_temp_dialog(),
                                        state="disabled")
        self.set_target_btn.grid(row=0, column=1, padx=5, pady=5)
        
        self.toggle_heat_btn = ttk.Button(btn_frame, text="åæ¢å ç­", 
                                         command=lambda: self.send_command("TOGGLE_HEAT"),
                                         state="disabled")
        self.toggle_heat_btn.grid(row=1, column=0, padx=5, pady=5)
        
        self.toggle_mode_btn = ttk.Button(btn_frame, text="åæ¢æ¨¡å¼", 
                                        command=lambda: self.send_command("TOGGLE_MODE"),
                                        state="disabled")
        self.toggle_mode_btn.grid(row=1, column=1, padx=5, pady=5)
        
        # æ¸©åº¦è®¾ç½®åºå
        temp_frame = ttk.Frame(control_frame)
        temp_frame.grid(row=1, column=0, columnspan=2, pady=(10, 0))
        
        ttk.Label(temp_frame, text="å¿«éæ¸©åº¦è®¾ç½®:").grid(row=0, column=0, sticky=tk.W)
        
        # å¸¸ç¨æ¸©åº¦æé®
        common_temps = ["100", "150", "200", "250"]
        for i, temp in enumerate(common_temps):
            btn = ttk.Button(temp_frame, text=f"{temp}Â°C", 
                           command=lambda t=temp: self.send_command(f"SET_TARGET:{t}"),
                           state="disabled")
            btn.grid(row=0, column=i+1, padx=5)
            setattr(self, f"temp_{temp}_btn", btn)
        
        # éä¿¡æ¥å¿åºå
        log_frame = ttk.LabelFrame(main_frame, text="éä¿¡æ¥å¿", padding="10")
        log_frame.grid(row=3, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 10))
        
        self.log_text = scrolledtext.ScrolledText(log_frame, width=70, height=15)
        self.log_text.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        self.log_text.insert(tk.END, "éä¿¡æ¥å¿å°æ¾ç¤ºå¨è¿é...\n\n")
        
        # éç½®ç½æ ¼æé
        main_frame.columnconfigure(0, weight=1)
        main_frame.rowconfigure(3, weight=1)
        log_frame.columnconfigure(0, weight=1)
        log_frame.rowconfigure(0, weight=1)
        
        # ç»å®çªå£å³é­äºä»¶
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
            self.connect_btn.config(text="æ­å¼è¿æ¥")
            self.status_label.config(text=f"ç¶æ: å·²è¿æ¥å° {ip}:{port}")
            self.enable_controls(True)
            
            # å¯å¨æ¥æ¶çº¿ç¨
            self.receive_thread = threading.Thread(target=self.receive_messages, daemon=True)
            self.receive_thread.start()
            
            # å¯å¨å¿è·³çº¿ç¨
            self.heartbeat_thread = threading.Thread(target=self.heartbeat_loop, daemon=True)
            self.heartbeat_thread.start()
            
            self.log_message(f"â æåè¿æ¥å° {ip}:{port}")
            
        except Exception as e:
            messagebox.showerror("è¿æ¥éè¯¯", f"è¿æ¥å¤±è´¥: {str(e)}")
            self.log_message(f"â è¿æ¥å¤±è´¥: {str(e)}")
    
    def disconnect(self):
        self.connected = False
        
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
        
        self.connect_btn.config(text="è¿æ¥")
        self.status_label.config(text="ç¶æ: æªè¿æ¥")
        self.enable_controls(False)
        self.log_message("ð å·²æ­å¼è¿æ¥")
    
    def send_command(self, command):
        if not self.connected:
            messagebox.showwarning("è­¦å", "è¯·åè¿æ¥å°è®¾å¤")
            return
        
        try:
            self.socket.sendall((command + "\n").encode('utf-8'))
            self.log_message(f"ð¤ åé: {command}")
        except Exception as e:
            self.log_message(f"â åéå¤±è´¥: {str(e)}")
            self.disconnect()
    
    def receive_messages(self):
        buffer = ""
        while self.connected:
            try:
                data = self.socket.recv(1024).decode('utf-8')
                if not data:
                    # ç©ºæ°æ®å¯è½æ¯è¿æ¥æ­£å¸¸å³é­ï¼ç­å¾ä¸æ®µæ¶é´åæ£æ¥
                    time.sleep(0.1)
                    continue
                    
                buffer += data
                lines = buffer.split('\n')
                buffer = lines[-1]  # ä¿çæªå®æçè¡
                
                for line in lines[:-1]:
                    if line.strip():
                        self.log_message(f"ð¥ æ¥æ¶: {line}")
                        
            except socket.timeout:
                # è¶æ¶æ¯æ­£å¸¸çï¼ç»§ç»­ç­å¾æ°æ®
                continue
            except ConnectionResetError:
                if self.connected:
                    self.log_message("â è¿æ¥è¢«è¿ç¨ä¸»æºéç½®")
                break
            except Exception as e:
                if self.connected:
                    self.log_message(f"â æ¥æ¶éè¯¯: {str(e)}")
                break
        
        if self.connected:
            self.root.after(0, self.disconnect)
    
    def show_temp_dialog(self):
        if not self.connected:
            messagebox.showwarning("è­¦å", "è¯·åè¿æ¥å°è®¾å¤")
            return
        
        dialog = tk.Toplevel(self.root)
        dialog.title("è®¾ç½®ç®æ æ¸©åº¦")
        dialog.geometry("300x150")
        dialog.transient(self.root)
        dialog.grab_set()
        
        ttk.Label(dialog, text="è¯·è¾å¥ç®æ æ¸©åº¦ (Â°C):").pack(pady=10)
        
        temp_entry = ttk.Entry(dialog, width=10)
        temp_entry.pack(pady=5)
        temp_entry.focus()
        
        def set_temp():
            temp = temp_entry.get().strip()
            if temp and temp.isdigit():
                self.send_command(f"SET_TARGET:{temp}")
                dialog.destroy()
            else:
                messagebox.showerror("éè¯¯", "è¯·è¾å¥ææçæ¸©åº¦å¼")
        
        btn_frame = ttk.Frame(dialog)
        btn_frame.pack(pady=10)
        
        ttk.Button(btn_frame, text="ç¡®å®", command=set_temp).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="åæ¶", command=dialog.destroy).pack(side=tk.LEFT, padx=5)
        
        dialog.bind('<Return>', lambda e: set_temp())
    
    def enable_controls(self, enabled):
        state = "normal" if enabled else "disabled"
        
        self.get_status_btn.config(state=state)
        self.set_target_btn.config(state=state)
        self.toggle_heat_btn.config(state=state)
        self.toggle_mode_btn.config(state=state)
        
        # æ¸©åº¦æé®
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
        """å¿è·³æºå¶ï¼å®æåéGET_STATUSå½ä»¤ä¿æè¿æ¥æ´»è·"""
        while self.connected:
            try:
                # æ¯30ç§åéä¸æ¬¡å¿è·³
                time.sleep(30)
                if self.connected:
                    self.send_command("GET_STATUS")
                    self.last_heartbeat_time = time.time()
            except Exception as e:
                if self.connected:
                    self.log_message(f"â å¿è·³åéå¤±è´¥: {str(e)}")
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