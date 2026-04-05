@echo off
REM ============================================================
REM BEVoid — Android APK Build Script (Windows)
REM Требует: Android SDK + NDK, apksigner, zipalign
REM ============================================================

echo ============================================
echo  BEVoid — Android APK Build
echo ============================================

REM --- Пути (подставь свои!) ---
if "%ANDROID_SDK_ROOT%"=="" (
    set ANDROID_SDK_ROOT=C:\Users\%USERNAME%\AppData\Local\Android\Sdk
)
if "%ANDROID_NDK_HOME%"=="" (
    for /d %%D in ("%ANDROID_SDK_ROOT%\ndk\*") do set ANDROID_NDK_HOME=%%D
)

echo [1/7] SDK: %ANDROID_SDK_ROOT%
echo         NDK: %ANDROID_NDK_HOME%

if not exist "%ANDROID_SDK_ROOT%" (
    echo ERROR: ANDROID_SDK_ROOT не найден!
    echo Установи Android SDK через Android Studio.
    pause
    exit /b 1
)
if not exist "%ANDROID_NDK_HOME%" (
    echo ERROR: NDK не найден!
    echo Установи NDK через SDK Manager.
    pause
    exit /b 1
)

set CMAKE_EXE=%ANDROID_SDK_ROOT%\cmake\3.22.1\bin\cmake.exe
if not exist "%CMAKE_EXE%" (
    for /d %%D in ("%ANDROID_SDK_ROOT%\cmake\*") do set CMAKE_EXE=%%D\bin\cmake.exe
)

echo [2/7] Glad проверка...
if not exist "src\third_party\glad\glad.c" (
    echo ERROR: glad.c не найден!
    echo Сгенерируй с https://glad.dav1d.de/ (OpenGL ES 3.0, loader: YES)
    echo Положи glad.h и glad.c в src\third_party\glad\
    pause
    exit /b 1
)

REM --- Настройки ---
set ABI=arm64-v8a
set API_LEVEL=21
set BUILD_TYPE=Release

echo [3/7] ABI: %ABI%  API: %API_LEVEL%  Build: %BUILD_TYPE%

REM --- CMake configure ---
echo [4/7] CMake configure...
rmdir /s /q build-android 2>nul
mkdir build-android
cd build-android

"%CMAKE_EXE%" ^
  -DCMAKE_TOOLCHAIN_FILE="%ANDROID_NDK_HOME%\build\cmake\android.toolchain.cmake" ^
  -DANDROID_ABI=%ABI% ^
  -DANDROID_PLATFORM=%API_LEVEL% ^
  -DANDROID_STL=c++_shared ^
  -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
  -DANDROID=ON ^
  ..

if errorlevel 1 (
    echo CMake configure FAILED!
    cd ..
    pause
    exit /b 1
)

REM --- CMake build ---
echo [5/7] Building .so...
"%CMAKE_EXE%" --build . --config %BUILD_TYPE% -j8

if errorlevel 1 (
    echo Build FAILED!
    cd ..
    pause
    exit /b 1
)

cd ..

REM --- Копируем .so в jniLibs ---
echo [6/7] Packing APK...
mkdir android\app\src\main\jniLibs\%ABI% 2>nul
copy /Y build-android\libbevoid.so android\app\src\main\jniLibs\%ABI%\libbevoid.so >nul

REM --- Собираем APK через aapt ---
set AAPT=%ANDROID_SDK_ROOT%\build-tools\34.0.0\aapt.exe
if not exist "%AAPT%" (
    for /d %%D in ("%ANDROID_SDK_ROOT%\build-tools\*") do set AAPT=%%D\aapt.exe
)

if not exist "%AAPT%" (
    echo ERROR: aapt не найден! Установи Android SDK Build-Tools.
    pause
    exit /b 1
)

REM --- Подготовка APK ---
set APK_UNALIGNED=build\bevoid-unaligned.apk
set APK_FINAL=build\bevoid.apk

mkdir build 2>nul

REM Упаковываем ресурсы
"%AAPT%" package -f -M android\app\src\main\AndroidManifest.xml ^
  -S android\app\src\main\res ^
  -I "%ANDROID_SDK_ROOT%\platforms\android-34\android.jar" ^
  -F %APK_UNALIGNED% ^
  --auto-add-overlay

if errorlevel 1 (
    echo WARNING: aapt package failed, trying без ресурсов...
    "%AAPT%" package -f -M android\app\src\main\AndroidManifest.xml ^
      -I "%ANDROID_SDK_ROOT%\platforms\android-34\android.jar" ^
      -F %APK_UNALIGNED% ^
      --auto-add-overlay
)

REM --- Добавляем .so в APK ---
powershell -Command "Compress-Archive -Path 'android\app\src\main\jniLibs\*' -DestinationPath '%APK_UNALIGNED%' -Update"

REM --- zipalign ---
set ZIPALIGN=%ANDROID_SDK_ROOT%\build-tools\34.0.0\zipalign.exe
if not exist "%ZIPALIGN%" (
    for /d %%D in ("%ANDROID_SDK_ROOT%\build-tools\*") do set ZIPALIGN=%%D\zipalign.exe
)

if exist "%ZIPALIGN%" (
    "%ZIPALIGN%" -f -p 4 %APK_UNALIGNED% %APK_FINAL%
) else (
    copy /Y %APK_UNALIGNED% %APK_FINAL% >nul
)

REM --- Подпись debug ключом ---
set KEYSTORE=%USERPROFILE%\.android\debug.keystore
set APKSIGNER=%ANDROID_SDK_ROOT%\build-tools\34.0.0\apksigner.bat
if not exist "%APKSIGNER%" (
    for /d %%D in ("%ANDROID_SDK_ROOT%\build-tools\*") do set APKSIGNER=%%D\apksigner.bat
)

if exist "%APKSIGNER%" (
    call "%APKSIGNER%" sign --ks %KEYSTORE% ^
      --ks-pass pass:android ^
      --ks-key-alias androiddebugkey ^
      --key-pass pass:android ^
      %APK_FINAL%
)

echo [7/7] Done!
echo ============================================
echo  APK: %CD%\%APK_FINAL%
echo  ABI: %ABI%
echo  Установка: adb install -r %APK_FINAL%
echo ============================================

pause
