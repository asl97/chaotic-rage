rmdir /S /Q files
mkdir files

set CL=/DRELEASE
"c:\WINDOWS\microsoft.net\Framework\v4.0.30319\MSBuild.exe" /t:Clean,Build /p:Platform="Win32" "..\msvc2010\Chaotic Rage.vcxproj" 
if %errorlevel% neq 0 pause && exit /b %errorlevel% 
copy /-Y ..\msvc2010\Debug\chaoticrage.exe files

:: set CL=/DRELEASE
:: "c:\WINDOWS\microsoft.net\Framework\v4.0.30319\MSBuild.exe" /t:Clean,Build /p:Platform="Win32" "..\msvc2010\DedicatedServer.vcxproj" 
:: if %errorlevel% neq 0 pause && exit /b %errorlevel% 
:: copy /-Y ..\msvc2010\Debug\dedicatedserver.exe files

xcopy /E /I ..\..\data files\data
xcopy /E /I ..\..\maps files\maps

copy /-Y ..\..\*.bmp files
copy /-Y ..\windows\lib\*.dll files
copy /-Y ..\redist-msvc\*.dll files
copy /-Y ..\redist-msvc\*.manifest files

copy /-Y ..\..\maptool\bin\Debug\maptool.exe files


"C:\Program Files (x86)\NSIS\makensis.exe" chaoticrage.nsi
if %errorlevel% neq 0 pause && exit /b %errorlevel% 

echo All done
pause
exit /b
