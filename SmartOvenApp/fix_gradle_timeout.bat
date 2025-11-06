@echo off
chcp 65001 >nul

echo ================================================
echo ð§ ä¿®å¤Gradleä¸è½½è¶æ¶é®é¢
echo ================================================
echo.

echo ð å½åé®é¢ï¼Gradle 7.6.4ä¸è½½è¶æ¶
echo â éè¯¯ä¿¡æ¯ï¼java.net.SocketTimeoutException: Connect timed out
echo.

echo ð¡ è§£å³æ¹æ¡ï¼ä½¿ç¨æ¬å°å·²ä¸è½½çGradle 6.9.4
echo.

echo ð å¼å§ä¿®å¤...
echo.

:: æ¹æ³1ï¼ç´æ¥ä½¿ç¨æ¬å°Gradle 6.9.4ç¼è¯
echo æ¹æ³1ï¼ç´æ¥ä½¿ç¨æ¬å°Gradle 6.9.4ç¼è¯
echo æ§è¡å½ä»¤ï¼..\gradle-6.9.4\bin\gradle.bat clean build
echo.

..\gradle-6.9.4\bin\gradle.bat clean build
if %errorlevel% equ 0 goto :success

echo.
echo â æ¹æ³1å¤±è´¥ï¼å°è¯æ¹æ³2...
echo.

:: æ¹æ³2ï¼ä¿®æ¹Gradle Wrapperéç½®
echo æ¹æ³2ï¼ä¿®æ¹Gradle Wrapperéç½®ä½¿ç¨æ¬å°Gradle
echo.

:: åå»ºä¿®å¤åçgradle-wrapper.properties
echo distributionBase=GRADLE_USER_HOME > gradle\wrapper\gradle-wrapper.properties.fixed
echo distributionPath=wrapper/dists >> gradle\wrapper\gradle-wrapper.properties.fixed
echo # ä½¿ç¨æ¬å°Gradle 6.9.4ï¼é¿åä¸è½½è¶æ¶ >> gradle\wrapper\gradle-wrapper.properties.fixed
echo distributionUrl=gradle-6.9.4 >> gradle\wrapper\gradle-wrapper.properties.fixed
echo zipStoreBase=GRADLE_USER_HOME >> gradle\wrapper\gradle-wrapper.properties.fixed
echo zipStorePath=wrapper/dists >> gradle\wrapper\gradle-wrapper.properties.fixed

copy gradle\wrapper\gradle-wrapper.properties.fixed gradle\wrapper\gradle-wrapper.properties >nul
del gradle\wrapper\gradle-wrapper.properties.fixed >nul

echo â Gradle Wrapperéç½®å·²æ´æ°
echo.

echo ð ä½¿ç¨ä¿®å¤åçéç½®éæ°ç¼è¯...
echo.

gradlew.bat clean build
if %errorlevel% equ 0 goto :success

echo.
echo â æ¹æ³2å¤±è´¥ï¼ä½¿ç¨æç»æ¹æ¡...
echo.

:: æç»æ¹æ¡ï¼ä½¿ç¨Android Studio
echo æç»æ¹æ¡ï¼ä½¿ç¨Android Studio
echo.
echo ð¡ Android Studioä¼èªå¨å¤çGradleä¸è½½é®é¢
echo.
echo ð æä½æ­¥éª¤ï¼
echo 1. æå¼Android Studio
echo 2. File -> Open -> éæ©SmartOvenAppæä»¶å¤¹
echo 3. Build -> Make Project
echo 4. Android Studioä¼èªå¨ä¸è½½æ­£ç¡®çGradleçæ¬
echo.

echo â è¿æ¯æå¯é çè§£å³æ¹æ¡ï¼
goto :end

:success
echo.
echo ð æ­åï¼ç¼è¯æåï¼
echo ð± APKæä»¶ä½ç½®ï¼app\build\outputs\apk\
echo.

:end
echo.
echo ================================================
echo ä¿®å¤å®æ
echo ================================================
pause