set os_version=windows\vs2017\x64\
set dep_base=..\..\..\..\deps\
set bin_base=..\..\..\..\bin\

rmdir /Q /S %bin_base%
rmdir /Q /S %dep_base%

mkdir %dep_base%include
mkdir %dep_base%lib

::Baselib
set src_base=..\..\..\..\..\..\..\..\Versions\Baselib\

::fmt
mkdir %dep_base%include\fmt
xcopy %src_base%fmt\v9.0.0\%os_version%include %dep_base%include\fmt /S /Y /C
xcopy %src_base%fmt\v9.0.0\%os_version%lib %dep_base%lib /S /Y /C
