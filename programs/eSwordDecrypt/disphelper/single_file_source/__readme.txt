DispHelper single file source:

To use DispHelper, simply add disphelper.c and disphelper.h to your project.
For Visual C++, Borland C++ and LCC-Win32 import libraries are included via
pragma directives. For other compilers you may need to add ole32, oleaut32 and uuid.
To do this in Dev-CPP add "-lole32 -loleaut32 -luuid" to the linker box under
Project->Project Options->Parameters.

If you are using Dev-CPP and get errors when compiling disphelper.c:
Make sure disphelper.c is set to compile as C and not C++ under Project->Project Options->Files.





---------
DispHelper COM Helper Library:

DispHelper allows you to call COM objects with an extremely simple printf style syntax.
DispHelper can be used from C++ or even plain C. It works with most Windows compilers
including Dev-CPP, Visual C++ and LCC-WIN32. Including DispHelper in your project
couldn't be simpler as it is available in a compacted single file version.

Included with DispHelper are over 20 samples that demonstrate using COM objects
including ADO, CDO, Outlook, Eudora, Excel, Word, Internet Explorer, MSHTML,
PocketSoap, Word Perfect, MS Agent, SAPI, MSXML, WIA, dexplorer and WMI.

DispHelper is free open source software provided under the BSD license.

Find out more and download DispHelper at:
http://sourceforge.net/projects/disphelper/
http://disphelper.sourceforge.net/
