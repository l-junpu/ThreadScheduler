@echo OFF

@REM [92m is Green, [93m is Yellow, [96m is Cyan, [97m is White (color for echo)

set RootPath=%cd%

@REM Open Solution File
echo [96m==============================================================================
echo   Opening Visual Studio Solution
echo ==============================================================================[97m

@REM Querying Location Of 
for /f "usebackq tokens=1* delims=: " %%i in (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop`) do (
  if /i "%%i"=="productPath" set devenv=%%j
)


if "%devenv%" == "" (
  echo [93m Sorry, we could not find vswhere.exe![97m
) else (
  echo [96m Located devenv at: [97m"%devenv%"
  echo [96m Opening Solution...
  "%devenv%" "%RootPath%\build\ThreadScheduler.sln"
  EXIT /B
)