# SmartOvenApp 构建指南

## 问题分析
当前项目无法构建的主要原因是Java兼容性问题：
- 系统缺少JDK 8（只有JRE）
- 模块兼容性错误阻止Gradle正常运行

## 解决方案（按推荐顺序）

### 方案1：使用Android Studio构建（推荐）
这是最简单可靠的方法：

1. 打开Android Studio
2. 选择 "Open an existing Android Studio project"
3. 导航到 `f:\Github\SmartOvenController\SmartOvenApp` 并打开
4. Android Studio会自动下载所需的依赖和工具
5. 点击 "Build" → "Make Project" 或 "Build" → "Build Bundle(s) / APK(s)"

### 方案2：手动安装JDK 8
如果必须使用命令行构建：

1. **下载JDK 8**：
   - 访问：https://www.oracle.com/java/technologies/javase/javase8-archive-downloads.html
   - 下载Windows x64版本的JDK 8（如 jdk-8u381-windows-x64.exe）

2. **安装JDK 8**：
   - 运行下载的安装程序
   - 接受许可协议
   - 使用默认安装路径（C:\\Program Files\\Java\\jdk1.8.0_XXX）

3. **设置环境变量**：
   - 设置 `JAVA_HOME=C:\Program Files\Java\jdk1.8.0_XXX`
   - 将 `%JAVA_HOME%\bin` 添加到PATH环境变量

4. **验证安装**：
   ```cmd
   java -version
   javac -version
   ```

5. **构建项目**：
   ```cmd
   cd f:\Github\SmartOvenController\SmartOvenApp
   gradlew.bat clean build
   ```

### 方案3：使用项目提供的脚本
项目目录中已包含以下构建脚本：

- `build_with_java8.bat` - 使用Java 8环境构建
- `install_jdk8_and_build.bat` - 指导安装JDK 8并构建
- `download_and_install_jdk8.bat` - 下载和安装JDK 8的指导脚本

## 故障排除

### 常见错误及解决方案

1. **"Unable to make field accessible" 错误**
   - 原因：模块兼容性问题
   - 解决方案：使用方案1（Android Studio）或方案2（安装JDK 8）

2. **"Could not find tools.jar" 错误**
   - 原因：只有JRE没有JDK
   - 解决方案：安装完整的JDK 8

3. **Gradle版本兼容性问题**
   - 项目支持Gradle 6.9.4、7.6.4、9.2.0
   - 使用项目根目录中对应的Gradle版本

## 项目结构
```
SmartOvenController/
├── gradle-6.9.4/     # Gradle 6.9.4
├── gradle-7.6.4/     # Gradle 7.6.4  
├── gradle-9.2.0/     # Gradle 9.2.0
└── SmartOvenApp/     # Android应用项目
    ├── build_with_java8.bat
    ├── install_jdk8_and_build.bat
    ├── download_and_install_jdk8.bat
    └── BUILD_GUIDE.md (本文件)
```

## 推荐工作流程
1. **首选**：使用Android Studio打开项目构建
2. **备选**：手动安装JDK 8后使用命令行构建
3. **最后选择**：使用项目提供的辅助脚本

构建成功后，APK文件将生成在：`app\build\outputs\apk\` 目录中。