// 移动端H5页面通用工具函数

class MobileOvenController {
    constructor() {
        this.baseURL = window.location.origin;
        this.settings = this.loadSettings();
        this.init();
    }

    // 初始化
    init() {
        this.setupTheme();
        this.setupEventListeners();
        this.checkConnection();
    }

    // 加载设置
    loadSettings() {
        const defaultSettings = {
            tempUnit: 'celsius',
            tempPrecision: 1,
            autoSaveTemp: true,
            tempAlert: true,
            themeMode: 'light',
            language: 'zh-CN',
            vibration: true,
            sound: true,
            refreshInterval: 5,
            dataRetention: 30
        };

        try {
            const saved = localStorage.getItem('ovenSettings');
            return saved ? { ...defaultSettings, ...JSON.parse(saved) } : defaultSettings;
        } catch (error) {
            console.error('加载设置失败:', error);
            return defaultSettings;
        }
    }

    // 保存设置
    saveSettings(newSettings = {}) {
        this.settings = { ...this.settings, ...newSettings };
        try {
            localStorage.setItem('ovenSettings', JSON.stringify(this.settings));
            this.applySettings();
            return true;
        } catch (error) {
            console.error('保存设置失败:', error);
            return false;
        }
    }

    // 应用设置
    applySettings() {
        this.applyTheme();
        this.applyLanguage();
        this.setupRefreshInterval();
    }

    // 设置主题
    setupTheme() {
        if (this.settings.themeMode === 'auto') {
            const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
            document.documentElement.setAttribute('data-theme', prefersDark ? 'dark' : 'light');
        } else {
            document.documentElement.setAttribute('data-theme', this.settings.themeMode);
        }
    }

    // 应用主题
    applyTheme() {
        this.setupTheme();
    }

    // 应用语言
    applyLanguage() {
        // 这里可以添加多语言支持
        document.documentElement.lang = this.settings.language;
    }

    // 设置刷新间隔
    setupRefreshInterval() {
        if (this.refreshTimer) {
            clearInterval(this.refreshTimer);
        }
        
        this.refreshTimer = setInterval(() => {
            this.refreshData();
        }, this.settings.refreshInterval * 1000);
    }

    // 设置事件监听器
    setupEventListeners() {
        // 处理页面可见性变化
        document.addEventListener('visibilitychange', () => {
            if (document.hidden) {
                this.onPageHidden();
            } else {
                this.onPageVisible();
            }
        });

        // 处理网络状态变化
        window.addEventListener('online', () => this.onNetworkOnline());
        window.addEventListener('offline', () => this.onNetworkOffline());

        // 处理页面卸载
        window.addEventListener('beforeunload', () => this.onPageUnload());
    }

    // API调用方法
    async apiCall(endpoint, options = {}) {
        const url = `${this.baseURL}${endpoint}`;
        const defaultOptions = {
            method: 'GET',
            headers: {
                'Content-Type': 'application/json',
            },
            timeout: 10000 // 10秒超时
        };

        const config = { ...defaultOptions, ...options };

        try {
            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), config.timeout);
            
            const response = await fetch(url, {
                ...config,
                signal: controller.signal
            });
            
            clearTimeout(timeoutId);

            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            const contentType = response.headers.get('content-type');
            if (contentType && contentType.includes('application/json')) {
                return await response.json();
            } else {
                return await response.text();
            }
        } catch (error) {
            if (error.name === 'AbortError') {
                throw new Error('请求超时');
            }
            throw error;
        }
    }

    // 获取设备状态
    async getDeviceStatus() {
        try {
            const data = await this.apiCall('/status');
            return this.formatDeviceStatus(data);
        } catch (error) {
            console.error('获取设备状态失败:', error);
            throw error;
        }
    }

    // 格式化设备状态
    formatDeviceStatus(data) {
        return {
            currentTemp: this.formatTemperature(data.currentTemp || 0),
            targetTemp: this.formatTemperature(data.targetTemp || 0),
            tempDiff: this.formatTemperature((data.targetTemp || 0) - (data.currentTemp || 0)),
            heatingEnabled: data.heatingEnabled || false,
            wifiConnected: data.wifiConnected || false,
            wifiSSID: data.wifiSSID || '--',
            ipAddress: data.ipAddress || '--',
            rssi: data.rssi || '--',
            deviceId: data.deviceId || '--',
            firmwareVersion: data.firmwareVersion || '--',
            uptime: this.formatUptime(data.uptime || 0),
            freeHeap: data.freeHeap || 0,
            lastUpdate: new Date().toLocaleString()
        };
    }

    // 格式化温度
    formatTemperature(temp) {
        if (this.settings.tempUnit === 'fahrenheit') {
            temp = (temp * 9/5) + 32;
        }
        return Math.round(temp / this.settings.tempPrecision) * this.settings.tempPrecision;
    }

    // 格式化运行时间
    formatUptime(seconds) {
        const days = Math.floor(seconds / 86400);
        const hours = Math.floor((seconds % 86400) / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        
        if (days > 0) {
            return `${days}天 ${hours}小时`;
        } else if (hours > 0) {
            return `${hours}小时 ${minutes}分钟`;
        } else {
            return `${minutes}分钟`;
        }
    }

    // 设置目标温度
    async setTargetTemperature(temp) {
        try {
            const data = await this.apiCall('/set-temp', {
                method: 'POST',
                body: JSON.stringify({ temperature: temp })
            });
            
            this.showToast(`温度已设置为 ${temp}°C`);
            return data;
        } catch (error) {
            console.error('设置温度失败:', error);
            this.showToast('设置温度失败');
            throw error;
        }
    }

    // 控制加热
    async setHeating(enabled) {
        try {
            const data = await this.apiCall('/set-heating', {
                method: 'POST',
                body: JSON.stringify({ enabled: enabled })
            });
            
            this.showToast(enabled ? '加热已启动' : '加热已停止');
            return data;
        } catch (error) {
            console.error('控制加热失败:', error);
            this.showToast('控制加热失败');
            throw error;
        }
    }

    // 配置WiFi
    async configureWiFi(ssid, password) {
        try {
            const data = await this.apiCall('/wifi-config', {
                method: 'POST',
                body: JSON.stringify({ ssid, password })
            });
            
            this.showToast('WiFi配置已提交');
            return data;
        } catch (error) {
            console.error('配置WiFi失败:', error);
            this.showToast('配置WiFi失败');
            throw error;
        }
    }

    // 重启设备
    async restartDevice() {
        try {
            const data = await this.apiCall('/restart', {
                method: 'POST'
            });
            
            this.showToast('设备正在重启...');
            return data;
        } catch (error) {
            console.error('重启设备失败:', error);
            this.showToast('重启设备失败');
            throw error;
        }
    }

    // 检查连接状态
    async checkConnection() {
        try {
            await this.apiCall('/ping', { timeout: 3000 });
            this.connectionStatus = 'connected';
            return true;
        } catch (error) {
            this.connectionStatus = 'disconnected';
            return false;
        }
    }

    // 刷新数据
    async refreshData() {
        if (this.isRefreshing) return;
        
        this.isRefreshing = true;
        try {
            const status = await this.getDeviceStatus();
            this.dispatchEvent('dataUpdated', status);
        } catch (error) {
            this.dispatchEvent('dataError', error);
        } finally {
            this.isRefreshing = false;
        }
    }

    // 事件系统
    eventListeners = {};

    on(event, callback) {
        if (!this.eventListeners[event]) {
            this.eventListeners[event] = [];
        }
        this.eventListeners[event].push(callback);
    }

    off(event, callback) {
        if (this.eventListeners[event]) {
            this.eventListeners[event] = this.eventListeners[event].filter(cb => cb !== callback);
        }
    }

    dispatchEvent(event, data) {
        if (this.eventListeners[event]) {
            this.eventListeners[event].forEach(callback => {
                try {
                    callback(data);
                } catch (error) {
                    console.error(`事件处理错误 ${event}:`, error);
                }
            });
        }
    }

    // 工具方法
    showToast(message, duration = 2000) {
        const toast = document.createElement('div');
        toast.className = 'mobile-toast';
        toast.textContent = message;
        
        document.body.appendChild(toast);
        
        setTimeout(() => toast.classList.add('show'), 10);
        
        setTimeout(() => {
            toast.classList.remove('show');
            setTimeout(() => {
                if (toast.parentNode) {
                    toast.parentNode.removeChild(toast);
                }
            }, 300);
        }, duration);
    }

    vibrate(pattern = [100]) {
        if (this.settings.vibration && navigator.vibrate) {
            navigator.vibrate(pattern);
        }
    }

    playSound(type = 'click') {
        if (this.settings.sound) {
            // 这里可以添加声音播放逻辑
            console.log(`播放声音: ${type}`);
        }
    }

    // 页面生命周期方法
    onPageHidden() {
        if (this.refreshTimer) {
            clearInterval(this.refreshTimer);
        }
    }

    onPageVisible() {
        this.setupRefreshInterval();
        this.refreshData();
    }

    onNetworkOnline() {
        this.showToast('网络已连接');
        this.refreshData();
    }

    onNetworkOffline() {
        this.showToast('网络连接已断开');
    }

    onPageUnload() {
        if (this.refreshTimer) {
            clearInterval(this.refreshTimer);
        }
    }

    // 数据导出
    exportData(data, filename = 'oven_data.json') {
        const blob = new Blob([JSON.stringify(data, null, 2)], { 
            type: 'application/json' 
        });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = filename;
        a.click();
        URL.revokeObjectURL(url);
        
        this.showToast('数据已导出');
    }

    // 数据导入
    importData(file) {
        return new Promise((resolve, reject) => {
            const reader = new FileReader();
            reader.onload = (e) => {
                try {
                    const data = JSON.parse(e.target.result);
                    resolve(data);
                } catch (error) {
                    reject(new Error('文件格式错误'));
                }
            };
            reader.onerror = () => reject(new Error('读取文件失败'));
            reader.readAsText(file);
        });
    }
}

// 创建全局实例
window.mobileOvenController = new MobileOvenController();

// 工具函数
function formatTime(timestamp) {
    return new Date(timestamp).toLocaleString();
}

function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}

function throttle(func, limit) {
    let inThrottle;
    return function() {
        const args = arguments;
        const context = this;
        if (!inThrottle) {
            func.apply(context, args);
            inThrottle = true;
            setTimeout(() => inThrottle = false, limit);
        }
    };
}

function isValidTemperature(temp) {
    return typeof temp === 'number' && temp >= 0 && temp <= 300;
}

function isValidSSID(ssid) {
    return typeof ssid === 'string' && ssid.length >= 1 && ssid.length <= 32;
}

function isValidPassword(password) {
    return typeof password === 'string' && password.length >= 8 && password.length <= 64;
}

// 本地存储工具
const storage = {
    set(key, value) {
        try {
            localStorage.setItem(key, JSON.stringify(value));
            return true;
        } catch (error) {
            console.error('存储数据失败:', error);
            return false;
        }
    },

    get(key, defaultValue = null) {
        try {
            const item = localStorage.getItem(key);
            return item ? JSON.parse(item) : defaultValue;
        } catch (error) {
            console.error('读取数据失败:', error);
            return defaultValue;
        }
    },

    remove(key) {
        try {
            localStorage.removeItem(key);
            return true;
        } catch (error) {
            console.error('删除数据失败:', error);
            return false;
        }
    },

    clear() {
        try {
            localStorage.clear();
            return true;
        } catch (error) {
            console.error('清空存储失败:', error);
            return false;
        }
    }
};

// 全局导出
window.formatTime = formatTime;
window.debounce = debounce;
window.throttle = throttle;
window.isValidTemperature = isValidTemperature;
window.isValidSSID = isValidSSID;
window.isValidPassword = isValidPassword;
window.storage = storage;