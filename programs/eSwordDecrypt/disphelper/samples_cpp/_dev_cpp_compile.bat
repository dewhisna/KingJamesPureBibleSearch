:: Drag a sample file onto this batch file to compile it with DEV-CPP
:: Edit this batch file as needed for your environment

:: Set the path to your installation of Dev-Cpp.
SET PATH=%PATH%;C:\Dev-Cpp\bin;C:\Program Files\Dev-Cpp\bin;

%0\
cd %0\..
cd /d %0\..
cls

SET DEV_PATH=C:\Dev-CPP\
@if not exist "C:\Dev-Cpp\bin\gcc.exe" SET DEV_PATH=C:\Program Files\Dev-Cpp

gcc.exe -c disphelper.c -o disphelper.o -I"%DEV_PATH%\include"
g++.exe %1 -o %1.exe disphelper.o -I"%DEV_PATH%\include\c++" -I"%DEV_PATH%\include\c++\mingw32" -I"%DEV_PATH%\include" -L"%DEV_PATH%\lib" -lole32 -loleaut32 -luuid

@del *.o
@echo.
@pause
