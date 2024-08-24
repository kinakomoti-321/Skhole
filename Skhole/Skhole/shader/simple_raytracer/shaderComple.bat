@echo off

set GLSLANG_VALIDATOR= C:\VulkanSDK\1.3.283.0\Bin\glslangValidator.exe

for %%s in (raygen.rgen closesthit.rchit miss.rmiss) do (
    %GLSLANG_VALIDATOR% %%s -V -o %%s.spv --target-env vulkan1.2 
)

set "source_dir=%~dp0"

set "destination=C:\Users\youka\Desktop\Program\Skhole\Skhole\x64\Debug\shader\simple_raytracer"

set "file_extension=*.spv"

xcopy "%source_dir%\%file_extension%" "%destination%" /Y

echo ファイルをコピーしました。