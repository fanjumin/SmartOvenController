@echo off
chcp 65001 >nul

echo ================================================
echo ð¯ SmartOvenAppæºè½ç¼è¯è§£å³æ¹æ¡
echo ================================================
echo.

REM æ£æ¥Gradle 6.9.4
if not exist "..\gradle-6.9.4\bin\gradle.bat" (
    echo â éè¯¯ï¼æªæ¾å°Gradle 6.9.4
echo   è¯·ç¡®ä¿gradle-6.9.4æä»¶å¤¹ä½äºé¡¹ç®æ ¹ç®å½ä¸
echo.
    pause
    exit /b 1
)

echo â æ¾å°Gradle 6.9.4ï¼å®æ¹å¼å®¹çæ¬ï¼
echo.

REM æ¾ç¤ºçæ¬ä¿¡æ¯
echo ð Gradleçæ¬ä¿¡æ¯ï¼
"..\gradle-6.9.4\bin\gradle.bat" --version
echo.

REM å°è¯æ­£å¸¸ç¼è¯
echo ð å°è¯æ­£å¸¸ç¼è¯ï¼éè¦ç½ç»è¿æ¥ï¼...
echo   å¦æå¤±è´¥ï¼å¯è½æ¯ç½ç»æä»£çé®é¢
echo.

"..\gradle-6.9.4\bin\gradle.bat" clean build

if %ERRORLEVEL% EQU 0 (
    echo.
    echo â ç¼è¯æåï¼
echo.
echo ð± APKæä»¶ä½ç½®ï¼
echo    app\build\outputs\apk\debug\app-debug.apk
echo.
    goto SUCCESS
) else (
    echo.
    echo â ç¼è¯å¤±è´¥ï¼éè¯¯ä»£ç ï¼%ERRORLEVEL%
echo.
)

REM å¦æç¼è¯å¤±è´¥ï¼æä¾è§£å³æ¹æ¡
echo ð§ ç¼è¯å¤±è´¥è§£å³æ¹æ¡ï¼
echo.
echo æ¹æ¡1ï¼æ£æ¥ç½ç»è¿æ¥åä»£çè®¾ç½®
echo   - ç¡®ä¿æ¨çç½ç»å¯ä»¥è®¿é®GoogleåMavenä»åº
echo   - å¦æä½¿ç¨ä»£çï¼è¯·éç½®ä»£çè®¾ç½®
echo.
echo æ¹æ¡2ï¼ä½¿ç¨Android Studioï¼æ¨èï¼
echo   - æå¼Android Studio
echo   - File -> Open -> éæ©SmartOvenAppæä»¶å¤¹
echo   - Build -> Make Project
echo   - APKä½ç½®ï¼app\build\outputs\apk\debug\app-debug.apk
echo.
echo æ¹æ¡3ï¼æå¨éç½®ä»£ç
echo   - è¿è¡ setup_proxy.bat æ¥çä»£çè®¾ç½®æå
echo   - æè¿è¡ android_studio_proxy_guide.bat
echo.

:SUCCESS
echo.
echo ð¡ æç¤ºï¼
echo   - è¿æ¯æç¨³å®çå®æ¹å¼å®¹ç»åï¼Gradle 6.9.4 + AGP 4.2.2
echo   - å¦æå½ä»¤è¡ç¼è¯å¤±è´¥ï¼å¼ºçæ¨èä½¿ç¨Android Studio
echo.

pause