@echo off

set GLSLANG_VALIDATOR= C:\VulkanSDK\1.3.283.0\Bin\glslangValidator.exe

setlocal enabledelayedexpansion

set fileList=
for /r shader %%f in (*.comp *.rgen *.rmiss *.rchit) do (
	%GLSLANG_VALIDATOR% %%f -V -o %%f.spv --target-env vulkan1.2   
)

set "dest=..\x64\Debug\shader"

xcopy /E /I /Y "shader" "%dest%"