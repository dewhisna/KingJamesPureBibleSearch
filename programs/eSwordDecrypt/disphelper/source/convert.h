/* This file is part of the source code for the DispHelper COM helper library.
 * DispHelper allows you to call COM objects with an extremely simple printf style syntax.
 * DispHelper can be used from C++ or even plain C. It works with most Windows compilers
 * including Dev-CPP, Visual C++ and LCC-WIN32. Including DispHelper in your project
 * couldn't be simpler as it is available in a compacted single file version.
 *
 * Included with DispHelper are over 20 samples that demonstrate using COM objects
 * including ADO, CDO, Outlook, Eudora, Excel, Word, Internet Explorer, MSHTML,
 * PocketSoap, Word Perfect, MS Agent, SAPI, MSXML, WIA, dexplorer and WMI.
 *
 * DispHelper is free open source software provided under the BSD license.
 *
 * Find out more and download DispHelper at:
 * http://sourceforge.net/projects/disphelper/
 * http://disphelper.sourceforge.net/
 */


#ifndef CONVERT_H_INCLUDED
#define CONVERT_H_INCLUDED

#include <objbase.h>
#include <time.h>

HRESULT ConvertFileTimeToVariantTime(FILETIME * pft, DATE * pDate);
HRESULT ConvertVariantTimeToFileTime(DATE date, FILETIME * pft);

HRESULT ConvertVariantTimeToSystemTime(DATE date, SYSTEMTIME * pSystemTime);
HRESULT ConvertSystemTimeToVariantTime(SYSTEMTIME * pSystemTime, DATE * pDate);

HRESULT ConvertTimeTToVariantTime(time_t timeT, DATE * pDate);
HRESULT ConvertVariantTimeToTimeT(DATE date, time_t * pTimeT);

HRESULT ConvertAnsiStrToBStr(LPCSTR szAnsiIn, BSTR * lpBstrOut);
HRESULT ConvertBStrToAnsiStr(BSTR bstrIn, LPSTR * lpszOut);

#endif /* ----- CONVERT_H_INCLUDED ----- */
