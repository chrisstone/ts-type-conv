@echo off
:: ============================================================================
:: @brief   Builds the ts-type-conv project on Windows with MSVC.
:: ============================================================================

SET "TARGET_DIR=build"

SETLOCAL EnableDelayedExpansion
SET "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

IF not exist "!VSWHERE!" (
    echo WARNING: vswhere.exe not found at default location.
    goto :end
)

FOR /F "usebackq tokens=*" %%i IN (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) DO (
    SET "InstallDir=%%i"
)

IF defined InstallDir (
    echo Visual Studio found in: !InstallDir!
    SET "VcvarsPath=!InstallDir!\VC\Auxiliary\Build\vcvars64.bat"

    IF exist "!VcvarsPath!" (
        echo Calling !VcvarsPath!
        call "!VcvarsPath!"
    ) ELSE (
        echo ERROR: vcvars64.bat not found at !VcvarsPath!. Make sure "Desktop development with C++" workload is installed.
    )
) ELSE (
    echo ERROR: Could not find a Visual Studio installation with the required C++ workload.
)

IF not exist "%TARGET_DIR%\" (
    mkdir "%TARGET_DIR%"
)

cd %TARGET_DIR%

cmake ..
cmake --build . --config Release
ctest -C Release --output-on-failure --output-junit test.xml -O test.log

:end
ENDLOCAL