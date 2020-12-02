@echo off

set BUILDER=msvc16
set SOURCE_PLATFORM=windows
set TARGET_PLATFORM=windows
set GENERATOR="Visual Studio 16 2019"
set Boost_ROOT="C:/boost_1_74_0"
set CONFIGURATION="debug"

::----------------------------
:: DO NOT EDIT THE TEXT BELOW!
::----------------------------

set TARGET=%SOURCE_PLATFORM%-%TARGET_PLATFORM%-%BUILDER%-%CONFIGURATION%

mkdir BUILD
cd BUILD
mkdir %TARGET%
cd %TARGET%

cmake -DTARGET_DIR=%TARGET% -DTARGET_PLATFORM=%TARGET_PLATFORM% -DBoost_ROOT=%Boost_ROOT% ^
-DCONFIGURATION=%CONFIGURATION% -G %GENERATOR% -A x64 ../..

cmake --build . --config %CONFIGURATION%
