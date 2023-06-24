@echo OFF
cls
SET mypath=%~dp0
pushd %mypath%\TEMP\
start grey_merger.exe
popd