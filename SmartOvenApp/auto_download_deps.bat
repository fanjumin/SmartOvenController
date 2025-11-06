@echo off
chcp 65001 >nul

echo ================================================
echo ð¤ æºè½ä¾èµä¸è½½åç¼è¯èæ¬
echo ================================================
echo.

echo ð æ£æµç½ç»è¿æ¥ç¶æ...
ping -n 3 www.google.com >nul 2>&1
if %errorlevel% equ 0 (
    echo â å¯ä»¥è®¿é®Googleï¼ç½ç»è¿æ¥æ­£å¸¸
    goto :direct_download
) else (
    echo â æ æ³è®¿é®Googleï¼å¯è½éè¦ä»£ç
    goto :proxy_check
)

:direct_download
echo.
echo ð å°è¯ç´æ¥ä¸è½½ä¾èµå...
echo ä½¿ç¨å®æ¹ä»åºéç½®...

:: æ¢å¤ä½¿ç¨å®æ¹ä»åºéç½®
echo // ä½¿ç¨å®æ¹ä»åºéç½® > settings.gradle.temp
echo pluginManagement { >> settings.gradle.temp
echo     repositories { >> settings.gradle.temp
echo         gradlePluginPortal() >> settings.gradle.temp
echo         google() >> settings.gradle.temp
echo         mavenCentral() >> settings.gradle.temp
echo     } >> settings.gradle.temp
echo } >> settings.gradle.temp
echo. >> settings.gradle.temp
echo dependencyResolutionManagement { >> settings.gradle.temp
echo     repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS) >> settings.gradle.temp
echo     repositories { >> settings.gradle.temp
echo         google() >> settings.gradle.temp
echo         mavenCentral() >> settings.gradle.temp
echo     } >> settings.gradle.temp
echo } >> settings.gradle.temp
echo. >> settings.gradle.temp
echo rootProject.name = "SmartOvenApp" >> settings.gradle.temp
echo include ':app' >> settings.gradle.temp

copy settings.gradle.temp settings.gradle >nul
del settings.gradle.temp >nul

echo.
echo ð å¼å§ç¼è¯é¡¹ç®...
..\gradle-6.9.4\bin\gradle.bat clean build
if %errorlevel% equ 0 (
    echo â ç¼è¯æåï¼
    goto :success
) else (
    echo â ç¼è¯å¤±è´¥ï¼å°è¯å¶ä»æ¹æ¡...
    goto :proxy_check
)

:proxy_check
echo.
echo ð æ£æ¥å½åä»£çè®¾ç½®...
netsh winhttp show proxy
echo.
echo ð ä»£çéç½®éé¡¹ï¼
echo 1. ä½¿ç¨Android Studioï¼æ¨èï¼
echo 2. éç½®ç³»ç»ä»£ç
echo 3. ä½¿ç¨ç¦»çº¿å
echo.
set /p choice="è¯·éæ©æ¹æ¡ (1/2/3): "

if "%choice%"=="1" goto :android_studio
if "%choice%"=="2" goto :setup_proxy  
if "%choice%"=="3" goto :offline_mode

echo â æ æéæ©ï¼é»è®¤ä½¿ç¨Android Studioæ¹æ¡
goto :android_studio

:android_studio
echo.
echo ð¡ Android Studioæ¹æ¡ï¼
echo 1. æå¼Android Studio
echo 2. File -> Open -> éæ©SmartOvenAppæä»¶å¤¹
echo 3. Build -> Make Project
echo 4. Android Studioä¼èªå¨å¤çç½ç»åä¾èµé®é¢
echo.
echo â è¿æ¯ææ¨èçè§£å³æ¹æ¡ï¼
goto :end

:setup_proxy
echo.
echo ð§ ä»£çéç½®æ¹æ¡ï¼
echo è¯·è¿è¡ setup_proxy.bat æ¥çè¯¦ç»éç½®æå
echo æèç³»ç½ç»ç®¡çåè·åä»£çæå¡å¨ä¿¡æ¯
start setup_proxy.bat
goto :end

:offline_mode
echo.
echo ð¦ ç¦»çº¿æ¨¡å¼ï¼
echo éè¦æå¨ä¸è½½ä¾èµåå°æ¬å°ç¼å­
start google_maven_download_guide.bat
goto :end

:success
echo.
echo ð æ­åï¼é¡¹ç®ç¼è¯æåï¼
echo ð± APKæä»¶ä½ç½®ï¼app\build\outputs\apk\
echo.

:end
echo.
echo ================================================
echo èæ¬æ§è¡å®æ
echo ================================================
pause