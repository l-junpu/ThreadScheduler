@echo OFF

@REM [92m is Green, [93m is Yellow, [96m is Cyan, [97m is White (color for echo)
set RootPath=%cd%
set CMakeBuildFolder="%RootPath%/build/"


@REM Build Entire Project Using CMake
echo [96m==============================================================================
echo   Building CMake Project
echo ==============================================================================[97m
if exist CMakeBuildFolder (del "%RootPath%/build")
mkdir build
cd build
cmake .. -A x64 -T v142


echo [96m==============================================================================
echo   Done
echo ==============================================================================[97m
pause