@echo off
echo ========================================
echo   Java兼容性修复脚本
echo ========================================
echo.

echo [1/5] 检查当前Java版本...
java -version 2>&1 | find "version"
echo.

echo [2/5] 设置兼容的Java版本...
echo # Java版本兼容性配置> gradle.properties
echo org.gradle.java.home=%JAVA_HOME%>> gradle.properties
echo org.gradle.jvmargs=-Xmx2048m -Dfile.encoding=UTF-8>> gradle.properties
echo android.useAndroidX=true>> gradle.properties
echo android.enableJetifier=true>> gradle.properties
echo.

echo [3/5] 强制使用Java 8兼容模式...
echo # 强制使用Java 8兼容模式>> gradle.properties
echo org.gradle.jvmargs=-Xmx2048m -Dfile.encoding=UTF-8 -Dorg.gradle.java.home=%JAVA_HOME%>> gradle.properties
echo.

echo [4/5] 清理Gradle缓存...
if exist ".gradle" (
    echo 删除.gradle目录...
    rmdir /s /q .gradle 2>nul
)

echo [5/5] 创建兼容性编译脚本...
echo @echo off> compatible_build.bat
echo echo 使用兼容模式编译...>> compatible_build.bat
echo set JAVA_HOME=%JAVA_HOME%>> compatible_build.bat
echo set GRADLE_OPTS=-Dorg.gradle.java.home=%%JAVA_HOME%%>> compatible_build.bat
echo .\gradlew.bat assembleDebug -Dorg.gradle.java.home=%%JAVA_HOME%%>> compatible_build.bat
echo pause>> compatible_build.bat

echo.
echo ========================================
echo   修复完成！现在可以尝试编译
echo ========================================
echo.
echo 解决方案：
echo 1. 使用兼容模式：运行 compatible_build.bat
echo 2. 或者手动设置环境变量：
echo    set JAVA_HOME=您的Java8安装路径
echo    .\gradlew.bat assembleDebug
echo.
echo 如果仍有问题，建议安装Java 8或Java 11
echo.
pause