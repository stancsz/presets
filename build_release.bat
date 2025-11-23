@echo off
echo Building YamlPresetPlugin...

if not exist build mkdir build
cd build

echo Configuring CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release -A x64

echo Building Release...
cmake --build . --config Release --parallel 4

cd ..

if not exist release mkdir release

echo Zipping VST3...
powershell Compress-Archive -Path "build\YamlPresetPlugin_artefacts\Release\VST3\YamlPresetPlugin.vst3" -DestinationPath "release\presets_windows.zip" -Force

echo Done. Artifact at release\presets_windows.zip
