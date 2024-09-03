set GLSLANG_VALIDATOR= C:\VulkanSDK\1.3.283.0\Bin\glslangValidator.exe

%GLSLANG_VALIDATOR% example.comp -V -o example.comp.spv --target-env vulkan1.2 

set "source_dir=%~dp0"

set "destination=C:\Users\youka\Desktop\Program\Skhole\Skhole\x64\Debug\shader\simple_raytracer"

set "file_extension=*.spv"

xcopy "%source_dir%\%file_extension%" "%destination%" /Y

echo ファイルをコピーしました。