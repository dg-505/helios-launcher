@echo off

rem clean up previously created setup files
del "..\..\release\*win64*"
rmdir /s /q "..\..\release\WindowsInstaller"

rem get path to the executable
for /f %%A in ('powershell -ExecutionPolicy Bypass -File ".\win\getExePath.ps1"') do set "exePath=%%A"

if not exist "%exePath%" (
    echo Cancelled.
    cmd /k
)

rem copy required files to installer directory
mkdir "..\..\release\WindowsInstaller"
mkdir "..\..\release\WindowsInstaller\ico"
copy "%exePath%" "..\..\release\WindowsInstaller"
copy "..\res\heliospp.ico" "..\..\release\WindowsInstaller\ico"
copy "..\LICENSE" "..\..\release\WindowsInstaller"
copy "..\README.md" "..\..\release\WindowsInstaller"

rem Get required libraries that the executable needs
"%USERPROFILE%\Qt\5.15.16\mingw81_64\bin\windeployqt.exe" "..\..\release\WindowsInstaller\helios-launcher.exe"

rem Get other libraries windeployqt doesn't handle
copy "%USERPROFILE%\Qt\5.15.16\mingw81_64\bin\libgcc_s_seh-1.dll" "..\..\release\WindowsInstaller"
copy "%USERPROFILE%\Qt\5.15.16\mingw81_64\bin\libstdc++-6.dll" "..\..\release\WindowsInstaller"
copy "%USERPROFILE%\Qt\5.15.16\mingw81_64\bin\libwinpthread-1.dll" "..\..\release\WindowsInstaller"

rem remove unnecessary stuff
rmdir /s /q "..\..\release\WindowsInstaller\iconengines"
rmdir /s /q "..\..\release\WindowsInstaller\imageformats"
rmdir /s /q "..\..\release\WindowsInstaller\translations"
del "..\..\release\WindowsInstaller\D3Dcompiler_47.dll"
del "..\..\release\WindowsInstaller\libEGL.dll"
del "..\..\release\WindowsInstaller\libGLESv2.dll"
del "..\..\release\WindowsInstaller\opengl32sw.dll"
del "..\..\release\WindowsInstaller\Qt5Svg.dll"

rem pack zip for portable installation
setlocal enabledelayedexpansion
for /f "tokens=3 delims= " %%a in ('findstr /r /c:"^#define Version " ".\win\packInstaller.iss"') do (
    set "version=%%a"
    set "version=!version:"=!" rem Remove surrounding quotes
)
powershell -nologo -noprofile -command "Compress-Archive -Path '..\..\release\WindowsInstaller\*' -DestinationPath '..\..\release\helios-launcher-v%version%!%-win64-portable.zip'"
endlocal

rem create InnoSetup installer
"%PROGRAMFILES(X86)%\Inno Setup 6\ISCC.exe" ".\win\packInstaller.iss"

cmd /k
