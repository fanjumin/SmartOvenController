// 移动端H5页面通用工具函数

class MobileOvenController {
    constructor() {
        // 使用实际的设备IP地址，而不是当前网页的origin
        this.baseURL = 'http://192.168.16.104';
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
        // 解析设备返回的实际数据格式
        // 设备返回格式: {"device_id":"oven-8591756","firmware_version":"0.8.6","temperature":14.22,"target_temperature":180.00,"heating_enabled":false,"wifi_connected":true,"ip_address":"192.168.16.104"}
        
        const currentTemp = data.temperature || data.currentTemp || 0;
        const targetTemp = data.target_temperature || data.targetTemp || 0;
        
        return {
            currentTemp: this.formatTemperature(currentTemp),
            targetTemp: this.formatTemperature(targetTemp),
            tempDiff: this.formatTemperature(targetTemp - currentTemp),
            heatingEnabled: data.heating_enabled || data.heatingEnabled || false,
            wifiConnected: data.wifi_connected || data.wifiConnected || false,
            wifiSSID: data.wifi_ssid || '--',
            ipAddress: data.ip_address || data.ipAddress || '--',
            rssi: data.rssi || '--',
            deviceId: data.device_id || data.deviceId || '--',
            firmwareVersion: data.firmware_version || data.firmwareVersion || '--',
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
            return `${days}天${hours}小时`;
        } else if (hours > 0) {
            return `${hours}小时${minutes}分钟`;
        } else {
            return `${minutes}分钟`;
        }
    }

    // 检查连接状态
    async checkConnection() {
        try {
            await this.apiCall('/status');
            return true;
        } catch (error) {
            console.error('连接检查失败:', error);
            return false;
        }
    }

    // 刷新数据
    async refreshData() {
        try {
            const status = await this.getDeviceStatus();
            this.onDataUpdated(status);
            return status;
        } catch (error) {
            this.onDataError(error);
            throw error;
        }
    }

    // 设置加热状态
    async setHeating(enabled) {
        try {
            const response = await this.apiCall('/heating', {
                method: 'POST',
                body: JSON.stringify({ enabled: enabled })
            });
            
            if (response.success) {
                this.onHeatingStateChanged(enabled);
                return true;
            } else {
                throw new Error('设置加热状态失败');
            }
        } catch (error) {
            console.error('设置加热状态失败:', error);
            throw error;
        }
    }

    // 设置目标温度
    async setTargetTemperature(temperature) {
        try {
            const response = await this.apiCall('/temperature', {
                method: 'POST',
                body: JSON.stringify({ temperature: temperature })
            });
            
            if (response.success) {
                this.onTemperatureChanged(temperature);
                return true;
            } else {
                throw new Error('设置温度失败');
            }
        } catch (error) {
            console.error('设置温度失败:', error);
            throw error;
        }
    }



    // 连接WiFi
    async connectWiFi(ssid, password) {
        try {
            const formData = new URLSearchParams();
            formData.append('ssid', ssid);
            formData.append('password', password);
            
            const response = await this.apiCall('/savewifi', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: formData
            });
            
            if (response.status === 'success') {
                this.onWiFiConnected(ssid);
                return true;
            } else {
                throw new Error('WiFi连接失败: ' + (response.message || '未知错误'));
            }
        } catch (error) {
            console.error('WiFi连接失败:', error);
            throw error;
        }
    }

    // 断开WiFi连接
    async disconnectWiFi() {
        try {
            const response = await this.apiCall('/wifidisconnect', {
                method: 'POST'
            });
            
            if (response.success) {
                this.onWiFiDisconnected();
                return true;
            } else {
                throw new Error('断开WiFi失败');
            }
        } catch (error) {
            console.error('断开WiFi失败:', error);
            throw error;
        }
    }

    // 重启设备
    async rebootDevice() {
        try {
            const response = await this.apiCall('/reboot', {
                method: 'POST'
            });
            
            if (response.success) {
                this.onDeviceRebooting();
                return true;
            } else {
                throw new Error('重启设备失败');
            }
        } catch (error) {
            console.error('重启设备失败:', error);
            throw error;
        }
    }

    // 恢复出厂设置
    async factoryReset() {
        try {
            const response = await this.apiCall('/factoryreset', {
                method: 'POST'
            });
            
            if (response.success) {
                this.onFactoryReset();
                return true;
            } else {
                throw new Error('恢复出厂设置失败');
            }
        } catch (error) {
            console.error('恢复出厂设置失败:', error);
            throw error;
        }
    }

    // 事件处理函数
    onDataUpdated(data) {
        // 触发自定义事件
        this.emit('dataUpdated', data);
    }

    onDataError(error) {
        this.emit('dataError', error);
    }

    onHeatingStateChanged(enabled) {
        this.emit('heatingStateChanged', enabled);
    }

    onTemperatureChanged(temperature) {
        this.emit('temperatureChanged', temperature);
    }

    onWiFiConnected(ssid) {
        this.emit('wifiConnected', ssid);
    }

    onWiFiDisconnected() {
        this.emit('wifiDisconnected');
    }

    onDeviceRebooting() {
        this.emit('deviceRebooting');
    }

    onFactoryReset() {
        this.emit('factoryReset');
    }

    onPageHidden() {
        // 页面隐藏时暂停刷新
        if (this.refreshTimer) {
            clearInterval(this.refreshTimer);
        }
    }

    onPageVisible() {
        // 页面重新可见时恢复刷新
        this.setupRefreshInterval();
        this.refreshData();
    }

    onNetworkOnline() {
        this.emit('networkOnline');
        this.refreshData();
    }

    onNetworkOffline() {
        this.emit('networkOffline');
    }

    onPageUnload() {
        // 清理资源
        if (this.refreshTimer) {
            clearInterval(this.refreshTimer);
        }
    }

    // 事件系统
    events = {};

    on(event, callback) {
        if (!this.events[event]) {
            this.events[event] = [];
        }
        this.events[event].push(callback);
    }

    off(event, callback) {
        if (this.events[event]) {
            this.events[event] = this.events[event].filter(cb => cb !== callback);
        }
    }

    emit(event, data) {
        if (this.events[event]) {
            this.events[event].forEach(callback => {
                try {
                    callback(data);
                } catch (error) {
                    console.error(`事件处理错误 (${event}):`, error);
                }
            });
        }
    }

    // 移动端特定功能
    vibrate(pattern = [100]) {
        if (this.settings.vibration && 'vibrate' in navigator) {
            try {
                navigator.vibrate(pattern);
            } catch (error) {
                console.warn('振动功能不可用:', error);
            }
        }
    }

    playSound(soundType = 'click') {
        if (this.settings.sound) {
            // 这里可以添加声音播放逻辑
            console.log('播放声音:', soundType);
        }
    }

    // 获取设备信息
    getDeviceInfo() {
        return {
            userAgent: navigator.userAgent,
            platform: navigator.platform,
            language: navigator.language,
            screenWidth: screen.width,
            screenHeight: screen.height,
            isMobile: this.isMobileDevice(),
            isTouchDevice: 'ontouchstart' in window
        };
    }

    // 检测是否为移动设备
    isMobileDevice() {
        return /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    }

    // 获取网络状态
    getNetworkStatus() {
        return {
            online: navigator.onLine,
            connection: navigator.connection ? {
                effectiveType: navigator.connection.effectiveType,
                downlink: navigator.connection.downlink,
                rtt: navigator.connection.rtt
            } : null
        };
    }

    // 显示通知
    showNotification(title, message, type = 'info') {
        if ('Notification' in window && Notification.permission === 'granted') {
            new Notification(title, {
                body: message,
                icon: '/favicon.ico'
            });
        } else {
            // 回退到浏览器提示
            alert(`${title}: ${message}`);
        }
    }

    // 请求通知权限
    async requestNotificationPermission() {
        if ('Notification' in window) {
            const permission = await Notification.requestPermission();
            return permission === 'granted';
        }
        return false;
    }

    // 本地存储工具
    setStorage(key, value) {
        try {
            localStorage.setItem(key, JSON.stringify(value));
            return true;
        } catch (error) {
            console.error('存储数据失败:', error);
            return false;
        }
    }

    getStorage(key, defaultValue = null) {
        try {
            const value = localStorage.getItem(key);
            return value ? JSON.parse(value) : defaultValue;
        } catch (error) {
            console.error('读取数据失败:', error);
            return defaultValue;
        }
    }

    removeStorage(key) {
        try {
            localStorage.removeItem(key);
            return true;
        } catch (error) {
            console.error('删除数据失败:', error);
            return false;
        }
    }

    // 工具函数
    debounce(func, wait) {
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

    throttle(func, limit) {
        let inThrottle;
        return function(...args) {
            if (!inThrottle) {
                func.apply(this, args);
                inThrottle = true;
                setTimeout(() => inThrottle = false, limit);
            }
        };
    }

    // 格式化时间
    formatTime(date = new Date()) {
        return date.toLocaleTimeString('zh-CN', {
            hour12: false,
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        });
    }

    formatDate(date = new Date()) {
        return date.toLocaleDateString('zh-CN');
    }

    // 生成随机ID
    generateId(length = 8) {
        return Math.random().toString(36).substr(2, length);
    }

    // 验证输入
    validateInput(input, type = 'text') {
        const validators = {
            email: (value) => /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(value),
            password: (value) => value.length >= 8,
            number: (value) => !isNaN(value) && isFinite(value),
            temperature: (value) => {
                const num = parseFloat(value);
                return !isNaN(num) && num >= 0 && num <= 300;
            }
        };

        const validator = validators[type] || (() => true);
        return validator(input);
    }
}

// 创建全局实例
window.mobileOvenController = new MobileOvenController();

// 导出模块（如果支持）
if (typeof module !== 'undefined' && module.exports) {
    module.exports = MobileOvenController;
}