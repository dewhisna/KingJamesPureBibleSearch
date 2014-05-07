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


#define DISPHELPER_INTERNAL_BUILD
#include "disphelper.h"


/* **************************************************************************
 * dhInitializeImp:
 *   dhInitialize should be called at the start of each thread. The global
 * unicode mode is set depending on whether UNICODE is defined or not.
 * This funcion optionally initializes COM. CoInitialize may be changed
 * to OleInitialize in a future version.
 *
 ============================================================================ */
HRESULT dhInitializeImp(BOOL bInitializeCOM, BOOL bUnicode)
{
	dh_g_bIsUnicodeMode = bUnicode;

	if (bInitializeCOM) return CoInitialize(NULL);

	return NOERROR;
}



/* **************************************************************************
 * dhUninitialize:
 *   This function should be called at the end of every thread. Frees
 * the thread's exception if it exists and uninitializes COM if requested. 
 *
 ============================================================================ */
void dhUninitialize(BOOL bUninitializeCOM)
{
#ifndef DISPHELPER_NO_EXCEPTIONS
	dhCleanupThreadException();
#endif
	if (bUninitializeCOM) CoUninitialize();
}
