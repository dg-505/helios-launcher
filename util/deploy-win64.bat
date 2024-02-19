@echo off

rem clean up previously created setup files
del "..\..\deploy\*win64*"
rmdir /s /q "..\..\deploy\WindowsInstaller"

rem get path to the executable
set "defaultExePath=..\..\build\win11\qtcreator\release\bin\helios-launcher.exe"
set /p exePath=Please specify the helios-launcher.exe to deploy [default: %defaultExePath%]:
if "%exePath%"=="" (
    echo Using default '%defaultExePath%'
    set "exePath=%defaultExePath%"
)
if not exist "%exePath%" (
    echo The file specified does not exist.
    pause
    exit /b
)

rem copy required files to installer directory
mkdir "..\..\deploy\WindowsInstaller"
mkdir "..\..\deploy\WindowsInstaller\bin"
mkdir "..\..\deploy\WindowsInstaller\ico"
mkdir "..\..\deploy\WindowsInstaller\plugins"
copy "%exePath%" "..\..\deploy\WindowsInstaller\bin"
copy "..\res\heliospp.ico" "..\..\deploy\WindowsInstaller\ico"
copy "..\LICENSE" "..\..\deploy\WindowsInstaller"
copy "..\README.md" "..\..\deploy\WindowsInstaller"

rem Get required libraries that the executable needs
"%USERPROFILE%\Qt\5.15.16\mingw81_64\bin\windeployqt.exe" "..\..\deploy\WindowsInstaller\bin\helios-launcher.exe" --plugindir "..\..\deploy\WindowsInstaller\plugins"  --no-translations

rem Get other libraries windeployqt doesn't handle
copy "%USERPROFILE%\Qt\5.15.16\mingw81_64\bin\libgcc_s_seh-1.dll" "..\..\deploy\WindowsInstaller\bin"
copy "%USERPROFILE%\Qt\5.15.16\mingw81_64\bin\libstdc++-6.dll" "..\..\deploy\WindowsInstaller\bin"
copy "%USERPROFILE%\Qt\5.15.16\mingw81_64\bin\libwinpthread-1.dll" "..\..\deploy\WindowsInstaller\bin"

rem remove unnecessary stuff
rmdir /s /q "..\..\deploy\WindowsInstaller\plugins\iconengines"
rmdir /s /q "..\..\deploy\WindowsInstaller\plugins\imageformats"
del "..\..\deploy\WindowsInstaller\bin\D3Dcompiler_47.dll"
del "..\..\deploy\WindowsInstaller\bin\libEGL.dll"
del "..\..\deploy\WindowsInstaller\bin\libGLESv2.dll"
del "..\..\deploy\WindowsInstaller\bin\opengl32sw.dll"
del "..\..\deploy\WindowsInstaller\bin\Qt5Svg.dll"

rem pack zip for portable installation
setlocal enabledelayedexpansion
for /f "tokens=3 delims= " %%a in ('findstr /r /c:"^#define Version " ".\win\packInstaller.iss"') do (
    set "version=%%a"
    set "version=!version:"=!" rem Remove surrounding quotes
)
powershell -nologo -noprofile -command "Compress-Archive -Path '..\..\deploy\WindowsInstaller\*' -DestinationPath '..\..\deploy\helios-launcher-v%version%!%-win64-portable.zip'"
endlocal

rem create InnoSetup installer
"%PROGRAMFILES(X86)%\Inno Setup 6\ISCC.exe" ".\win\packInstaller.iss"

pause
