DispHelper C++ samples:
Each of the .cpp files in this directory contains a sample program demonstrating using
one or more COM objects.

Compiling the samples:
For simplicity in compiling the samples, I have provided compile batch files.
The idea is to drag the file you want to compile onto the appropriate batch file.
Before you do that you may have to do two things:
1. Put a copy of disphelper.c and disphelper.h in this directory if they are not present.
2. Edit the appropriate batch file to set the correct paths. Typically, this will not be needed.

Alternatively, you can set up a new project in your IDE. Simply add
disphelper.c, disphelper.h and the sample you want to compile to a new project.

Samples List:

word.cpp
  Demonstrates outputting formatted text to a Word document and getting user feed
back with the help of the office assistant. Demonstrates using Word as a spell checker.
--
excel.cpp
  Demonstrates outputting formatted data to Excel and using it to create a chart.
Demonstrates using a safe array to efficiently insert data into Excel.
--
email.cpp
  Demonstrates sending an email with CDO, Outlook and Eudora.
--
ado.cpp
  Demonstrates reading and manipulating data from a data source using ActiveX Data Objects. 
--
corel.cpp
  Demonstrates outputting formatted text to a WordPerfect document. 
--
speech.cpp
  Demonstrates using Microsoft Agent and SAPI to provide text-to-speech. 
--
MSHTML.cpp
  Demonstrates ui-less html parsing and manipulation of the html document object model(DOM) 
using MSHTML. Provides functions to parse html from a string, a website or a file. 
--
regexp.cpp
  Demonstrates using the VBScript.RegExp object to provide support for regular expressions.
Provides a function to extract hrefs from a web page using a regular expression.
--
xml.cpp
  Demonstrates using MSMXL to download a web page, read an RSS feed and read 
and manipulate XML with the XML document object model(DOM).
--
wmi.cpp
  Demonstrates using Windows Management Instrumentation(WMI). Samples include enumerating 
installed hot fixes, purging print queues, starting a program, monitoring starting
and terminating processes, and monitoring file creation. All of these samples can
target the local or a remote computer.
--
pocketsoap.cpp
  Demonstrates using the PocketSoap toolkit to utilise several web services. 
Play the 'Who wants to be a millionaire' quiz with the help of a web service. 
--
soap.cpp
  Demonstrates using the MSSoap toolkit to utilise several web services. 
Web services demonstrated include Google search, spell checker and cache viewer.
--
iexplore.cpp
  Demonstrates controlling Internet Explorer via COM. Demonstrates using an
Internet Explorer window to display or retrieve information from the user.
--
scriptctl.cpp
  Demonstrates using the MSScriptControl to run a VBScript or JScript.
--
dexplore.cpp
  Demonstrates controlling Microsoft's new help system for developers, dexplore.
--
dcom_alt_creds.cpp
  Demonstrates one way of accessing a remote COM object using alternate credentials.
--
wia.cpp
  Demonstrates using Windows Image Acquisition(WIA) to manipulate images. Demonstrates
taking a snapshot from a video device.





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
