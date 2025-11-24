// 多语言支持
const translations = {
    'zh': {
        // 标题和标签页
        'settingsHelpTitle': '设置与帮助',
        'settingsTab': '设置',
        'pidSettingsTab': 'PID',
        'helpTab': '帮助',
        'aboutTab': '关于',

        // 设置标签页 - 温度设置
        'tempSettings': '温度设置',
        'tempUnit': '温度单位',
        'celsius': '摄氏度 (°C)',
        'fahrenheit': '华氏度 (°F)',
        'tempPrecision': '温度精度',
        'precision01': '0.1°C',
        'precision1': '1°C',

        // 设置标签页 - 界面设置
        'uiSettings': '界面设置',
        'themeMode': '主题模式',
        'lightMode': '浅色模式',
        'darkMode': '深色模式',
        'autoMode': '自动模式',
        'language': '语言',
        'chinese': '中文',
        'english': 'English',

        // 设置标签页 - 系统设置
        'systemSettings': '系统设置',
        'saveSettings': '保存设置',
        'restartDevice': '重启设备',
        'factoryReset': '恢复出厂设置',
        'confirmFactoryReset': '确认恢复出厂设置',

        // PID设置标签页
        'pidControllerSettings': 'PID参数设置',
        'pidConfigDescription': '配置PID控制器参数以优化温度控制精度',
        'enablePID': '启用PID控制器',
        'pidControlMode': '控制模式',
        'autoModePID': '自动模式',
        'manualModePID': '手动模式',
        'pidParameters': 'PID参数',
        'proportional': '比例系数 (Kp)',
        'integral': '积分系数 (Ki)',
        'derivative': '微分系数 (Kd)',
        'outputLimit': '输出限制 (%)',
        'sampleTime': '采样时间 (ms)',
        'deadband': '死区范围 (°C)',
        'savePIDSettings': '保存PID设置',
        'resetPIDDefaults': '恢复默认值',

        // 帮助标签页
        'faq': '常见问题',
        'connectDeviceQuestion': '如何连接设备？',
        'connectDeviceAnswer': '确保设备处于配网模式（指示灯闪烁），然后在手机Wi-Fi设置中选择设备热点进行连接。',
        'tempAccuracyQuestion': '如何提高温度控制精度？',
        'tempAccuracyAnswer': '可以调整PID参数来优化控制效果。如果不确定如何设置，建议使用默认值。',
        'usageGuide': '使用指南',
        'basicOperation': '基础操作',
        'basicOperationContent': '1. 连接电源并等待设备启动\n2. 通过Wi-Fi连接设备\n3. 在移动应用中设置目标温度\n4. 开始烘烤\n5. 完成后设备会自动停止',
        'recipeSharing': '食谱分享',
        'recipeSharingContent': '您可以在社区中分享您的烘焙食谱，也可以从其他用户那里获取灵感。',
        'technicalSupport': '技术支持',
        'supportPhone': '客服电话',
        'supportEmail': '客服邮箱',
        'contactSupport': '联系技术支持',

        // 关于标签页
        'deviceInfo': '设备信息',
        'deviceModel': '设备型号',
        'serialNumber': '序列号',
        'firmwareVersion': '固件版本',
        'hardwareVersion': '硬件版本',
        'systemStatus': '系统状态',
        'uptime': '运行时间',
        'memoryUsage': '内存使用',
        'storageSpace': '存储空间',
        'networkStatus': '网络状态',
        'versionInfo': '版本信息',
        'appVersion': '应用版本',
        'buildDate': '构建日期',
        'developer': '开发者',
        'checkForUpdates': '检查更新',
        'exportLogs': '导出日志',

        // 恢复出厂设置模态框
        'resetWarning': '此操作将清除所有设置并恢复到出厂默认状态，包括：',
        'wifiConfig': 'Wi-Fi连接配置',
        'tempSettingsHistory': '温度设置历史',
        'otherConfig': '其他个性化配置',
        'irreversibleAction': '此操作不可逆，请谨慎操作。',
        'enterPassword': '请输入确认密码',
        'cancel': '取消',
        'confirmReset': '确认重置',

        // JavaScript提示信息
        'settingsSaved': '设置已保存',

        // WiFi配置页面
        'wifiConfigTitle': '智能烤箱设备WiFi配置页面',
        'deviceNetworkConfig': '智能烤箱控制器 - WiFi配置页面',
        'wifiNetworkConfig': 'WiFi网络连接设置',
        'deviceId': '设备ID:',
        'currentTempDisplay': '当前温度: %CURRENT_TEMP%°C',
        'wifiNetworkName': 'WiFi网络名称',
        'wifiPassword': 'WiFi密码',
        'enterWifiPassword': '请输入WiFi密码',
        'connectWifi': '连接WiFi',
        'scanWifiNetworks': '点击搜索WiFi网络',
        'scanWifiNetworksText': '点击搜索WiFi网络',
        'scanningWifi': '扫描WiFi网络中...',
        'scanningWifiOption': '扫描WiFi网络中...',
        'selectWifiNetwork': '请选择WiFi网络',
        'noWifiNetworksFound': '未找到可用的WiFi网络',
        'wifiScanFailed': '扫描WiFi网络失败',
        'scanFailedRetry': '扫描失败，请重试',
        'scanResult': '扫描完成',
        'networkCount': '个网络',
        'warningResetConfirmation': '警告：此操作将重置所有设置并清除WiFi配置！确定要继续吗？',
        'resetting': '正在重置...',
        'resetSuccess': '重置成功...设备正在重启',
        'confirmRestartMessage': '确定要重启设备吗？重启过程需要约30秒。',
        'deviceRebooting': '设备正在重启...',
        'deviceRebootComplete': '设备重启完成，请重新连接',
        'restartFailedLog': '重启设备失败:',
        'restartFailed': '重启失败，请检查连接',
        'factoryResetComplete': '设备已恢复出厂设置',
        'resetFailedLog': '重置设备失败:',
        'resetFailed': '重置失败，请检查连接',
        'passwordError': '密码错误，请重试',
        'supportRequestSubject': '智能电烤箱技术支持请求',
        'deviceInfoLabel': '设备信息：',
        'deviceModelLabel': '设备型号',
        'firmwareVersionLabel': '固件版本',
        'issueDescriptionLabel': '问题描述：',
        'checkingForUpdates': '正在检查更新...',
        'alreadyLatestVersion': '当前已是最新版本',
        'logsExported': '日志已导出',
        'pidSettingsSaved': 'PID设置已保存',
        'confirmResetPIDMessage': '确定要恢复PID设置的默认值吗？',
        'pidSettingsReset': 'PID设置已恢复默认值',

        // 登录页面
        'loginTitle': '智能电烤箱 - 管理员登录',
        'ovenTitle': '智能电烤箱',
        'adminLogin': '管理员登录',
        'adminPassword': '管理员密码',
        'enterAdminPassword': '请输入管理员密码',
        'login': '登录',
        'versionInfo': '智能电烤箱控制器 v0.8.7',

        // 烘焙模式
        'bread': '面包',
        'cake': '蛋糕',
        'cookies': '饼干',
        'thaw': '解冻',
        'pizza': '披萨',
        'roast': '烤肉',
        'custom': '自定义',

        // 状态信息
        'connected': '已连接',
        'disconnected': '未连接',
        'heating': '加热中',
        'stopped': '已停止',
        'switchedToAutoMode': '已切换到自动模式',
        'switchedToManualMode': '已切换到手动模式，请设置温度和时间',
        'switchedToCustomMode': '已切换到自定义模式，您可以手动调整温度和时间',
        'switchedToPresetMode': '已切换到{name}模式：{temperature}°C，{time}分钟',

        // 错误信息
        'networkDisconnected': '网络已断开',
        'networkRequestFailed': '网络请求失败',
        'readDataFailed': '读取数据失败',
        'connectionLost': '连接丢失',
        'connectionRestored': '连接已恢复',
        'deviceOffline': '设备离线',
        'deviceOnline': '设备在线',
        'invalidInput': '输入无效',
        'inputOutOfRange': '输入超出范围',
        'requiredField': '必填字段',
        'operationSuccessful': '操作成功',
        'operationFailed': '操作失败',
        'settingsSaved': '设置已保存',
        'settingsSaveFailed': '设置保存失败',
        'dataRefreshed': '数据已刷新',
        'confirmLogout': '确定要退出登录吗？',
        'logoutFailed': '退出登录失败',
        'newPasswordMismatch': '新密码输入不一致',
        'passwordChangedSuccessfully': '密码修改成功',
        'passwordChangeFailed': '密码修改失败，请检查当前密码是否正确',
        'pleaseEnterCurrentPassword': '请输入当前密码：',
        'pleaseEnterNewPassword': '请输入新密码：',
        'pleaseConfirmNewPasswordAgain': '请确认新密码：',
        'pleaseEnterAdminPassword': '请输入管理员密码确认操作：',

        // 温度相关
        'temperatureRangeShouldBe50To250': '温度范围应为 50-250°C',
        'timeRangeShouldBe1To120': '时间范围应为 1-120 分钟',
        'temperatureRangeShouldBe': '温度范围应为',
        'temperatureSetFailed': '温度设置失败',
        'currentTemperature': '当前温度',
        'targetTemperature': '目标温度',
        'temperatureCalibration': '温度校准',

        // 系统监控
        'systemMonitoring': '系统监控',
        'deviceStatus': '设备状态',
        'deviceStatusCard': '设备状态卡片',
        'memoryUsage': '内存使用',
        'runtime': '运行时间',
        'wifiSignal': 'WiFi信号',
        'wifiStatus': 'WiFi状态',
        'systemStatus': '系统状态',

        // 控制命令
        'startHeating': '开启加热',
        'stopHeating': '停止加热',
        'startHeatingTo': '开始加热到',
        'mode': '模式',
        'minutes': '分钟',

        // 其他
        'update': '更新',
        'address': '地址',
        'smartOvenController': '智能电烤箱控制器',
        'advancedBakingController': '高级烘焙控制器',
        'chineseEnglish': '中/En',
        'demoMode': '演示模式',
        'selectWiFi': '选择WiFi',
        'searchWiFiNetworks': '搜索WiFi网络...',
        'pleaseSelectNetwork': '请选择网络',
        'wifiPassword': 'WiFi密码',
        'enterWiFiPassword': '请输入WiFi密码',
        'connectWiFi': '连接WiFi',
        'pleaseSelectWiFiNetwork': '请选择WiFi网络',
        'pleaseEnterWiFiPassword': '请输入WiFi密码',
        'savingWiFiConfig': '正在保存WiFi配置...',
        'wifiConfigSavedSuccessfully': 'WiFi配置保存成功！设备将在3秒后重启...',
        'deviceRestartingPleaseCheckConnection': '设备重启中，请稍后检查连接状态...',
        'configSaveFailed': '配置保存失败',
        'unknownError': '未知错误',
        'networkErrorPleaseCheckDeviceConnection': '网络错误，请检查设备连接状态',
        'scanningWiFiNetworks': '正在扫描WiFi网络...',
        'foundNetworks': '发现',
        'wifiNetworks': '个WiFi网络',
        'scanFailedNoNetworkData': '扫描失败: 未找到网络数据',
        'scanFailedUsingMockData': '扫描失败，使用模拟数据',
        'configInterfaceReadyScanningWiFi': '配网界面已就绪，正在扫描WiFi网络...',
        'selectedNetwork': '已选择网络',
        'wifiConfigSaveFailed': 'WiFi配置保存失败'
    },

    'en': {
        // Title and Tabs
        'settingsHelpTitle': 'Settings & Help',
        'settingsTab': 'Settings',
        'pidSettingsTab': 'PID',
        'helpTab': 'Help',
        'aboutTab': 'About',

        // Settings Tab - Temperature Settings
        'tempSettings': 'Temperature Settings',
        'tempUnit': 'Temperature Unit',
        'celsius': 'Celsius (°C)',
        'fahrenheit': 'Fahrenheit (°F)',
        'tempPrecision': 'Temperature Precision',
        'precision01': '0.1°C',
        'precision1': '1°C',

        // Settings Tab - UI Settings
        'uiSettings': 'UI Settings',
        'themeMode': 'Theme Mode',
        'lightMode': 'Light Mode',
        'darkMode': 'Dark Mode',
        'autoMode': 'Auto Mode',
        'language': 'Language',
        'chinese': '中文',
        'english': 'English',

        // Settings Tab - System Settings
        'systemSettings': 'System Settings',
        'saveSettings': 'Save Settings',
        'restartDevice': 'Restart Device',
        'factoryReset': 'Factory Reset',
        'confirmFactoryReset': 'Confirm Factory Reset',

        // PID Settings Tab
        'pidControllerSettings': 'PID Controller Settings',
        'pidConfigDescription': 'Configure PID controller parameters to optimize temperature control accuracy',
        'enablePID': 'Enable PID Controller',
        'pidControlMode': 'Control Mode',
        'autoModePID': 'Auto Mode',
        'manualModePID': 'Manual Mode',
        'pidParameters': 'PID Parameters',
        'proportional': 'Proportional Gain (Kp)',
        'integral': 'Integral Gain (Ki)',
        'derivative': 'Derivative Gain (Kd)',
        'outputLimit': 'Output Limit (%)',
        'sampleTime': 'Sample Time (ms)',
        'deadband': 'Deadband Range (°C)',
        'savePIDSettings': 'Save PID Settings',
        'resetPIDDefaults': 'Reset to Defaults',

        // Help Tab
        'faq': 'FAQ',
        'connectDeviceQuestion': 'How to connect the device?',
        'connectDeviceAnswer': 'Ensure the device is in pairing mode (LED blinking), then select the device hotspot in your phone\'s Wi-Fi settings to connect.',
        'tempAccuracyQuestion': 'How to improve temperature control accuracy?',
        'tempAccuracyAnswer': 'You can adjust the PID parameters to optimize control performance. If you\'re unsure how to set them, it\'s recommended to use the default values.',
        'usageGuide': 'User Guide',
        'basicOperation': 'Basic Operation',
        'basicOperationContent': '1. Connect power and wait for the device to start\n2. Connect to the device via Wi-Fi\n3. Set target temperature in the mobile app\n4. Start baking\n5. The device will automatically stop when finished',
        'recipeSharing': 'Recipe Sharing',
        'recipeSharingContent': 'You can share your baking recipes in the community or get inspiration from other users.',
        'technicalSupport': 'Technical Support',
        'supportPhone': 'Support Phone',
        'supportEmail': 'Support Email',
        'contactSupport': 'Contact Support',

        // About Tab
        'deviceInfo': 'Device Information',
        'deviceModel': 'Device Model',
        'serialNumber': 'Serial Number',
        'firmwareVersion': 'Firmware Version',
        'hardwareVersion': 'Hardware Version',
        'systemStatus': 'System Status',
        'uptime': 'Uptime',
        'memoryUsage': 'Memory Usage',
        'storageSpace': 'Storage Space',
        'networkStatus': 'Network Status',
        'versionInfo': 'Version Info',
        'appVersion': 'App Version',
        'buildDate': 'Build Date',
        'developer': 'Developer',
        'checkForUpdates': 'Check for Updates',
        'exportLogs': 'Export Logs',

        // Factory Reset Modal
        'resetWarning': 'This operation will clear all settings and restore to factory defaults, including:',
        'wifiConfig': 'Wi-Fi connection configuration',
        'tempSettingsHistory': 'Temperature settings history',
        'otherConfig': 'Other personalized configurations',
        'irreversibleAction': 'This action is irreversible, please proceed with caution.',
        'enterPassword': 'Please enter confirmation password',
        'cancel': 'Cancel',
        'confirmReset': 'Confirm Reset',

        // JavaScript Messages
        'settingsSaved': 'Settings saved',

        // WiFi Configuration Page
        'wifiConfigTitle': 'Smart Oven Device WiFi Configuration Page',
        'deviceNetworkConfig': 'Smart Oven Controller - WiFi Configuration',
        'wifiNetworkConfig': 'WiFi Network Connection Setup',
        'deviceId': 'Device ID:',
        'currentTempDisplay': 'Current Temperature: %CURRENT_TEMP%°C',
        'wifiNetworkName': 'WiFi Network Name',
        'wifiPassword': 'WiFi Password',
        'enterWifiPassword': 'Please enter WiFi password',
        'connectWifi': 'Connect WiFi',
        'scanWifiNetworks': 'Click to Search WiFi Networks',
        'scanWifiNetworksText': 'Click to Search WiFi Networks',
        'scanningWifi': 'Scanning WiFi networks...',
        'scanningWifiOption': 'Scanning WiFi networks...',
        'selectWifiNetwork': 'Please select WiFi network',
        'noWifiNetworksFound': 'No available WiFi networks found',
        'wifiScanFailed': 'Failed to scan WiFi networks',
        'scanFailedRetry': 'Scan failed, please retry',
        'scanResult': 'Scan complete',
        'networkCount': ' networks',
        'warningResetConfirmation': 'Warning: This operation will reset all settings and clear WiFi configuration! Are you sure you want to continue?',
        'resetting': 'Resetting...',
        'resetSuccess': 'Reset successful... Device is restarting',
        'confirmRestartMessage': 'Are you sure you want to restart the device? The restart process takes about 30 seconds.',
        'deviceRebooting': 'Device is restarting...',
        'deviceRebootComplete': 'Device restart complete, please reconnect',
        'restartFailedLog': 'Failed to restart device:',
        'restartFailed': 'Restart failed, please check connection',
        'factoryResetComplete': 'Device has been restored to factory settings',
        'resetFailedLog': 'Failed to reset device:',
        'resetFailed': 'Reset failed, please check connection',
        'passwordError': 'Incorrect password, please try again',
        'supportRequestSubject': 'Smart Oven Technical Support Request',
        'deviceInfoLabel': 'Device Information:',
        'deviceModelLabel': 'Device Model',
        'firmwareVersionLabel': 'Firmware Version',
        'issueDescriptionLabel': 'Issue Description:',
        'checkingForUpdates': 'Checking for updates...',
        'alreadyLatestVersion': 'Already on the latest version',
        'logsExported': 'Logs exported',
        'pidSettingsSaved': 'PID settings saved',
        'confirmResetPIDMessage': 'Are you sure you want to reset PID settings to default values?',
        'pidSettingsReset': 'PID settings have been reset to default values',

        // Login Page
        'loginTitle': 'Smart Oven - Admin Login',
        'ovenTitle': 'Smart Oven',
        'adminLogin': 'Admin Login',
        'adminPassword': 'Admin Password',
        'enterAdminPassword': 'Please enter admin password',
        'login': 'Login',
        'versionInfo': 'Smart Oven Controller v0.8.7',

        // Baking Modes
        'bread': 'Bread',
        'cake': 'Cake',
        'cookies': 'Cookies',
        'thaw': 'Thaw',
        'pizza': 'Pizza',
        'roast': 'Roast',
        'custom': 'Custom',

        // Status Information
        'connected': 'Connected',
        'disconnected': 'Disconnected',
        'heating': 'Heating',
        'stopped': 'Stopped',
        'switchedToAutoMode': 'Switched to auto mode',
        'switchedToManualMode': 'Switched to manual mode, please set temperature and time',
        'switchedToCustomMode': 'Switched to custom mode, you can manually adjust temperature and time',
        'switchedToPresetMode': 'Switched to {name} mode: {temperature}°C, {time} minutes',

        // Error Messages
        'networkDisconnected': 'Network disconnected',
        'networkRequestFailed': 'Network request failed',
        'readDataFailed': 'Read data failed',
        'connectionLost': 'Connection lost',
        'connectionRestored': 'Connection restored',
        'deviceOffline': 'Device offline',
        'deviceOnline': 'Device online',
        'invalidInput': 'Invalid input',
        'inputOutOfRange': 'Input out of range',
        'requiredField': 'Required field',
        'operationSuccessful': 'Operation successful',
        'operationFailed': 'Operation failed',
        'settingsSaved': 'Settings saved',
        'settingsSaveFailed': 'Settings save failed',
        'dataRefreshed': 'Data refreshed',
        'confirmLogout': 'Are you sure you want to logout?',
        'logoutFailed': 'Logout failed',
        'newPasswordMismatch': 'New password mismatch',
        'passwordChangedSuccessfully': 'Password changed successfully',
        'passwordChangeFailed': 'Password change failed, please check current password is correct',
        'pleaseEnterCurrentPassword': 'Please enter current password:',
        'pleaseEnterNewPassword': 'Please enter new password:',
        'pleaseConfirmNewPasswordAgain': 'Please confirm new password:',
        'pleaseEnterAdminPassword': 'Please enter admin password to confirm operation:',

        // Temperature Related
        'temperatureRangeShouldBe50To250': 'Temperature range should be 50-250°C',
        'timeRangeShouldBe1To120': 'Time range should be 1-120 minutes',
        'temperatureRangeShouldBe': 'Temperature range should be',
        'temperatureSetFailed': 'Temperature setting failed',
        'currentTemperature': 'Current Temperature',
        'targetTemperature': 'Target Temperature',
        'temperatureCalibration': 'Temperature Calibration',

        // System Monitoring
        'systemMonitoring': 'System Monitoring',
        'deviceStatus': 'Device Status',
        'deviceStatusCard': 'Device Status Card',
        'memoryUsage': 'Memory Usage',
        'runtime': 'Runtime',
        'wifiSignal': 'WiFi Signal',
        'wifiStatus': 'WiFi Status',
        'systemStatus': 'System Status',

        // Control Commands
        'startHeating': 'Start Heating',
        'stopHeating': 'Stop Heating',
        'startHeatingTo': 'Start heating to',
        'mode': 'Mode',
        'minutes': 'minutes',

        // Other
        'update': 'Update',
        'address': 'Address',
        'smartOvenController': 'Smart Oven Controller',
        'advancedBakingController': 'Advanced Baking Controller',
        'chineseEnglish': 'Zh/En',
        'demoMode': 'Demo Mode',
        'selectWiFi': 'Select WiFi',
        'searchWiFiNetworks': 'Search WiFi networks...',
        'pleaseSelectNetwork': 'Please select network',
        'wifiPassword': 'WiFi Password',
        'enterWiFiPassword': 'Please enter WiFi password',
        'connectWiFi': 'Connect WiFi',
        'pleaseSelectWiFiNetwork': 'Please select WiFi network',
        'pleaseEnterWiFiPassword': 'Please enter WiFi password',
        'savingWiFiConfig': 'Saving WiFi configuration...',
        'wifiConfigSavedSuccessfully': 'WiFi configuration saved successfully! Device will restart in 3 seconds...',
        'deviceRestartingPleaseCheckConnection': 'Device restarting, please check connection status later...',
        'configSaveFailed': 'Configuration save failed',
        'unknownError': 'Unknown error',
        'networkErrorPleaseCheckDeviceConnection': 'Network error, please check device connection status',
        'scanningWiFiNetworks': 'Scanning WiFi networks...',
        'foundNetworks': 'Found',
        'wifiNetworks': 'WiFi networks',
        'scanFailedNoNetworkData': 'Scan failed: No network data found',
        'scanFailedUsingMockData': 'Scan failed, using mock data',
        'configInterfaceReadyScanningWiFi': 'Configuration interface ready, scanning WiFi networks...',
        'selectedNetwork': 'Selected network',
        'wifiConfigSaveFailed': 'WiFi configuration save failed'
    }
};

// 获取当前语言设置，默认为中文
function getCurrentLanguage() {
    return localStorage.getItem('language') || 'zh';
}

// 设置语言
function setLanguage(lang) {
    localStorage.setItem('language', lang);
    applyTranslations();
}

// 获取翻译文本
function getTranslation(key, params = {}) {
    const lang = getCurrentLanguage();
    let translation = translations[lang][key] || key;

    // 替换参数
    for (const [paramKey, paramValue] of Object.entries(params)) {
        translation = translation.replace(new RegExp(`{${paramKey}}`, 'g'), paramValue);
    }

    return translation;
}

// 应用翻译
function applyTranslations() {
    // 翻译所有带有data-lang属性的元素
    document.querySelectorAll('[data-lang]').forEach(element => {
        const key = element.getAttribute('data-lang');
        const translation = getTranslation(key);

        // 如果是placeholder属性，则设置placeholder
        if (element.hasAttribute('placeholder')) {
            element.setAttribute('placeholder', translation);
        } else {
            // 否则设置文本内容
            element.textContent = translation;
        }
    });
}

// 切换语言
function switchLanguage() {
    const currentLang = getCurrentLanguage();
    const newLang = currentLang === 'zh' ? 'en' : 'zh';
    setLanguage(newLang);

    // 更新语言切换按钮文本 - 找到内部的span元素
    const langToggleBtn = document.getElementById('lang-toggle');
    if (langToggleBtn) {
        const spanElement = langToggleBtn.querySelector('span[data-lang="chineseEnglish"]');
        if (spanElement) {
            spanElement.textContent = newLang === 'zh' ? '中/En' : 'Zh/En';
        } else {
            // 如果没有span元素，则直接设置按钮文本
            langToggleBtn.textContent = newLang === 'zh' ? '中/En' : 'Zh/En';
        }
    }
}

// 页面加载完成后应用翻译
document.addEventListener('DOMContentLoaded', function() {
    applyTranslations();
});