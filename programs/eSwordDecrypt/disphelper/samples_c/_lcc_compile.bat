:: Drag a sample file onto this batch file to compile it with LCC-WIN32
:: Edit this batch file as needed for your environment

SET PATH=%PATH%;C:\lcc\bin;C:\Program Files\lcc\bin;

%0\
cd %0\..
cd /d %0\..

cls

lcc -unused %1 -Fo%1.obj
lcc -unused disphelper.c
lcclnk -s -subsystem console %1.obj disphelper.obj urlmon.lib

@del *.obj
@echo.
@pause