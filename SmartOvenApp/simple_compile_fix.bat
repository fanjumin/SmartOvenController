@echo off
chcp 65001 >nul

echo ================================================
echo Simple Compile Fix for Gradle Timeout
echo ================================================
echo.

echo Step 1: Check Gradle 6.9.4 availability...
if exist "..\gradle-6.9.4\bin\gradle.bat" (
    echo OK: Gradle 6.9.4 found
    set GRADLE_PATH=..\gradle-6.9.4\bin\gradle.bat
) else (
    echo ERROR: Gradle 6.9.4 not found
    echo Please ensure gradle-6.9.4 folder exists in project root
    pause
    exit /b 1
)

echo.
echo Step 2: Fix Gradle Wrapper configuration...

:: Create fixed gradle-wrapper.properties
echo distributionBase=GRADLE_USER_HOME > gradle\wrapper\gradle-wrapper.properties.fixed
echo distributionPath=wrapper/dists >> gradle\wrapper\gradle-wrapper.properties.fixed
echo # Use local Gradle 6.9.4 to avoid download timeout >> gradle\wrapper\gradle-wrapper.properties.fixed
echo distributionUrl=gradle-6.9.4 >> gradle\wrapper\gradle-wrapper.properties.fixed
echo zipStoreBase=GRADLE_USER_HOME >> gradle\wrapper\gradle-wrapper.properties.fixed
echo zipStorePath=wrapper/dists >> gradle\wrapper\gradle-wrapper.properties.fixed

copy gradle\wrapper\gradle-wrapper.properties.fixed gradle\wrapper\gradle-wrapper.properties >nul
del gradle\wrapper\gradle-wrapper.properties.fixed >nul

echo OK: Gradle configuration updated

echo.
echo Step 3: Fix repository configuration...

:: Create optimized settings.gradle
echo // Fixed settings.gradle - Multi-repository configuration > settings.gradle.fixed
echo pluginManagement { >> settings.gradle.fixed
echo     repositories { >> settings.gradle.fixed
echo         gradlePluginPortal() >> settings.gradle.fixed
echo         maven { url 'https://repo.huaweicloud.com/repository/maven/' } >> settings.gradle.fixed
echo         maven { url 'https://maven.aliyun.com/repository/public' } >> settings.gradle.fixed
echo         google() >> settings.gradle.fixed
echo         mavenCentral() >> settings.gradle.fixed
echo     } >> settings.gradle.fixed
echo } >> settings.gradle.fixed
echo. >> settings.gradle.fixed
echo dependencyResolutionManagement { >> settings.gradle.fixed
echo     repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS) >> settings.gradle.fixed
echo     repositories { >> settings.gradle.fixed
echo         maven { url 'https://repo.huaweicloud.com/repository/maven/' } >> settings.gradle.fixed
echo         maven { url 'https://maven.aliyun.com/repository/public' } >> settings.gradle.fixed
echo         maven { url 'https://maven.aliyun.com/repository/google' } >> settings.gradle.fixed
echo         google() >> settings.gradle.fixed
echo         mavenCentral() >> settings.gradle.fixed
echo     } >> settings.gradle.fixed
echo } >> settings.gradle.fixed
echo. >> settings.gradle.fixed
echo rootProject.name = "SmartOvenApp" >> settings.gradle.fixed
echo include ':app' >> settings.gradle.fixed

copy settings.gradle.fixed settings.gradle >nul
del settings.gradle.fixed >nul

echo OK: Repository configuration updated

echo.
echo Step 4: Start compilation process...
echo.

echo Method 1: Direct compilation with local Gradle...
%GRADLE_PATH% clean build
if %errorlevel% equ 0 goto :success

echo.
echo Method 1 failed, trying Method 2...
echo.

echo Method 2: Using gradlew...
gradlew.bat clean build
if %errorlevel% equ 0 goto :success

echo.
echo Both methods failed, using Android Studio recommended...
goto :android_studio

:success
echo.
echo COMPILATION SUCCESSFUL!
echo APK location: app\build\outputs\apk\
echo.
goto :end

:android_studio
echo.
echo RECOMMENDED SOLUTION: Use Android Studio
echo.
echo Steps:
echo 1. Open Android Studio
echo 2. File -> Open -> Select SmartOvenApp folder
echo 3. Build -> Make Project
echo 4. Android Studio will handle all issues automatically
echo.
echo This is the most reliable solution!

:end
echo.
echo ================================================
echo Script execution completed
echo ================================================
pause