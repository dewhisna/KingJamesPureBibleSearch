:: Drag a sample file onto this batch file to compile it with DEV-CPP
:: Edit this batch file as needed for your environment

:: Set the path to your installation of Dev-Cpp.
SET PATH=%PATH%;C:\Dev-Cpp\bin;C:\Program Files\Dev-Cpp\bin;

%0\
cd %0\..
cd /d %0\..

cls

@if not exist "C:\Dev-Cpp\bin\gcc.exe" goto lblprogra

:: For those installed under "C:\Dev-CPP\"
gcc.exe -c disphelper.c -o disphelper.o -I"C:/Dev-Cpp/include"
gcc.exe -c %1 -o %1.o -I"C:/Dev-Cpp/include"
g++.exe %1.o disphelper.o -o %1.exe -L"C:/Dev-Cpp/lib" -lole32 -loleaut32 -luuid
@goto lblend

:lblprogra

:: For those installed under "C:\Program Files\Dev-Cpp\"
gcc.exe -c disphelper.c -o disphelper.o -I"C:/Program Files/Dev-Cpp/include"
gcc.exe -c %1 -o %1.o -I"C:/Program Files/Dev-Cpp/include"
g++.exe %1.o disphelper.o -o %1.exe -L"C:/Program Files/Dev-Cpp/lib" -lole32 -loleaut32 -luuid
@goto lblend

:lblend

@del *.o
@echo.
@pause
