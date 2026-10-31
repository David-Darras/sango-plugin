@echo off
rem Builds Sango Plugin with Docker. You do not need devkitPro on your computer.
rem
rem   tools\docker_make.bat myhack       builds your ROM hack (make myhack)
rem   tools\docker_make.bat -j8          builds all the products
rem   tools\docker_make.bat --update     makes the image again (new tools)
rem
rem The first time, the script makes the Docker image "sango-build"
rem (tools\docker\Dockerfile). This takes some minutes.
setlocal
for %%I in ("%~dp0..") do set "ROOT=%%~fI"
set IMAGE=sango-build

if /I "%~1"=="--update" (
  call :build_image
  exit /b %errorlevel%
)

docker image inspect %IMAGE% >nul 2>&1
if errorlevel 1 (
  call :build_image
  if errorlevel 1 exit /b 1
)

docker run --rm -v "%ROOT%:/project" %IMAGE% make %*
exit /b %errorlevel%

:build_image
docker build --pull -t %IMAGE% "%ROOT%\tools\docker"
exit /b %errorlevel%
