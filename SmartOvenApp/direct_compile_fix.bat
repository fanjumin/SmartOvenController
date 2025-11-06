@echo off
chcp 65001 >nul

echo ================================================
echo Direct Compile Fix - Alternative Repository Setup
echo ================================================
echo.

echo Step 1: Create alternative repository configuration...

echo // Alternative repository configuration with multiple mirrors > settings.gradle.alternative
echo pluginManagement { >> settings.gradle.alternative
echo     repositories { >> settings.gradle.alternative
echo         gradlePluginPortal() >> settings.gradle.alternative
echo         maven { url 'https://maven.aliyun.com/repository/public' } >> settings.gradle.alternative
echo         maven { url 'https://maven.aliyun.com/repository/google' } >> settings.gradle.alternative
echo         maven { url 'https://repo.huaweicloud.com/repository/maven/' } >> settings.gradle.alternative
echo         maven { url 'https://plugins.gradle.org/m2/' } >> settings.gradle.alternative
echo         google() >> settings.gradle.alternative
echo         mavenCentral() >> settings.gradle.alternative
echo     } >> settings.gradle.alternative
echo } >> settings.gradle.alternative
echo. >> settings.gradle.alternative
echo dependencyResolutionManagement { >> settings.gradle.alternative
echo     repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS) >> settings.gradle.alternative
echo     repositories { >> settings.gradle.alternative
echo         maven { url 'https://maven.aliyun.com/repository/public' } >> settings.gradle.alternative
echo         maven { url 'https://maven.aliyun.com/repository/google' } >> settings.gradle.alternative
echo         maven { url 'https://repo.huaweicloud.com/repository/maven/' } >> settings.gradle.alternative
echo         maven { url 'https://plugins.gradle.org/m2/' } >> settings.gradle.alternative
echo         google() >> settings.gradle.alternative
echo         mavenCentral() >> settings.gradle.alternative
echo     } >> settings.gradle.alternative
echo } >> settings.gradle.alternative
echo. >> settings.gradle.alternative
echo rootProject.name = "SmartOvenApp" >> settings.gradle.alternative
echo include ':app' >> settings.gradle.alternative

copy settings.gradle.alternative settings.gradle >nul
del settings.gradle.alternative >nul

echo OK: Alternative repository configuration applied

echo.
echo Step 2: Create gradle.properties with offline mode and proxy settings...

echo # Gradle properties for offline and proxy settings > gradle.properties.fixed
echo org.gradle.jvmargs=-Xmx2048m -Dfile.encoding=UTF-8 >> gradle.properties.fixed
echo android.useAndroidX=true >> gradle.properties.fixed
echo android.enableJetifier=true >> gradle.properties.fixed
echo # Try to use offline mode if dependencies are cached >> gradle.properties.fixed
echo org.gradle.offline=true >> gradle.properties.fixed

copy gradle.properties.fixed gradle.properties >nul
del gradle.properties.fixed >nul

echo OK: Gradle properties configured

echo.
echo Step 3: Try compilation with local Gradle 6.9.4...
echo.

echo Attempt 1: Using local Gradle with detailed output...
..\gradle-6.9.4\bin\gradle.bat clean build --info --stacktrace
if %errorlevel% equ 0 goto :success

echo.
echo Attempt 1 failed, trying Attempt 2 with gradlew...
echo.

echo Attempt 2: Using gradlew with retry logic...
gradlew.bat clean build --refresh-dependencies
if %errorlevel% equ 0 goto :success

echo.
echo Attempt 2 failed, trying Attempt 3 with offline mode...
echo.

echo Attempt 3: Using offline mode...
gradlew.bat clean build --offline
if %errorlevel% equ 0 goto :success

echo.
echo All attempts failed, recommending Android Studio...
goto :android_studio

:success
echo.
echo ================================================
echo COMPILATION SUCCESSFUL! ðŸŽ‰
echo ================================================
echo.
echo APK files should be available at:
echo app\build\outputs\apk\
echo.
echo Please check the directory for generated APK files.
echo.
goto :end

:android_studio
echo.
echo ================================================
echo RECOMMENDED SOLUTION: Use Android Studio
echo ================================================
echo.
echo Android Studio will handle all dependency issues automatically.
echo.
echo Steps to use Android Studio:
echo 1. Open Android Studio
echo 2. Select 'Open an existing Android Studio project'
echo 3. Navigate to and select the SmartOvenApp folder
echo 4. Wait for Android Studio to sync and download dependencies
echo 5. Go to Build -> Make Project (or Ctrl+F9)
echo 6. Android Studio will automatically resolve all issues
echo.
echo This is the most reliable method for Android projects!
echo.

:end
echo ================================================
echo Script execution completed
echo ================================================
pause