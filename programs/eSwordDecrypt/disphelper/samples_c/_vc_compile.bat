:: Drag a sample file onto this batch file to compile it with Visual C++
:: Edit this batch file as needed for your environment

:: Call vcvars32.bat for your installation of VC++.

:: This is for VC 6.
call "C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32"
:: This is for VC.NET.
call "C:\Program Files\Microsoft Visual Studio .NET\Vc7\bin\vcvars32"
:: This is for VC.NET 2003.
call "C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\bin\vcvars32"
:: This is for VC Toolkit 2003.
call "C:\Program Files\Microsoft Visual C++ Toolkit 2003\vcvars32"

:: Set environment variables for Platform SDK:
SET INCLUDE=%INCLUDE%;C:\Program Files\Microsoft SDK\Include;
SET LIB=%LIB%;C:\Program Files\Microsoft SDK\Lib;

%0\
cd %0\..
cd /d %0\..

cls

cl /GF /W3 %1 disphelper.c KERNEL32.LIB USER32.LIB urlmon.lib

@del *.obj
@echo.
@pause