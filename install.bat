@echo off
echo Installing VST3 Plugin...

set "SOURCE=build\PresetEngine_artefacts\Release\VST3\PresetEngine.vst3"
set "DEST=C:\Program Files\Common Files\VST3\PresetEngine.vst3"

if not exist "%SOURCE%" (
    echo Error: Source VST3 not found at "%SOURCE%"
    echo Please build the project first.
    pause
    exit /b 1
)

echo Copying from "%SOURCE%" to "%DEST%"...
xcopy "%SOURCE%" "%DEST%" /E /I /Y

if %errorlevel% neq 0 (
    echo.
    echo Error: Failed to copy files.
    echo You may need to run this script as Administrator.
    pause
    exit /b 1
)

echo.
echo Installation Successful!
pause
