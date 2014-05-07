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
 * dhCreateObjectEx:
 *   This function is used to create a COM object based on a program id.
 *
 * Notes:
 *   We use CoGetClassObject/CreateInstance as it is supported on Windows 95
 * without DCOM while CoCreateInstanceEx is not. (The pServerInfo argument
 * to CoGetClassObject is reserved and must be NULL in this scenario).
 *
 ============================================================================ */
HRESULT dhCreateObjectEx(LPCOLESTR szProgId, REFIID riid, DWORD dwClsContext,
			    COSERVERINFO * pServerInfo, void ** ppv)
{
	CLSID clsid;
	HRESULT hr;
	IClassFactory * pCf = NULL;

	DH_ENTER(L"CreateObjectEx");

	if (!szProgId || !riid || !ppv) return DH_EXIT(E_INVALIDARG, szProgId);

	if (L'{' == szProgId[0])
		hr = CLSIDFromString((LPOLESTR) szProgId, &clsid);
	else
		hr = CLSIDFromProgID(szProgId, &clsid);

	if (SUCCEEDED(hr)) hr = CoGetClassObject(&clsid, dwClsContext, pServerInfo, &IID_IClassFactory, (void **) &pCf);
	if (SUCCEEDED(hr)) hr = pCf->lpVtbl->CreateInstance(pCf, NULL, riid, ppv);

	if (pCf) pCf->lpVtbl->Release(pCf);

	return DH_EXIT(hr, szProgId);
}



/* **************************************************************************
 * dhGetObjectEx:
 *   If szPathName is not NULL this function will load it in a com object.
 * Otherwise, this function will attempt to retrieve a running instance
 * of the object specified by szProgId.
 * See KB Item Q122288.
 *
 ============================================================================ */
HRESULT dhGetObjectEx(LPCOLESTR szPathName, LPCOLESTR szProgId, REFIID riid,
		         DWORD dwClsContext, LPVOID lpvReserved, void ** ppv)
{
	HRESULT hr;

	DH_ENTER(L"GetObjectEx");

	if ((!szProgId && !szPathName) || !riid || !ppv || lpvReserved) return DH_EXIT(E_INVALIDARG, szProgId);

	if (szPathName)
	{
		/* Attempt to load szPathName in a com object */

		if (!szProgId)
		{
			hr = CoGetObject(szPathName, NULL, riid, ppv);
		}
		else
		{
			IPersistFile * ppf = NULL;

			hr = dhCreateObjectEx(szProgId, &IID_IPersistFile, dwClsContext, NULL, (void **) &ppf);

			if (SUCCEEDED(hr)) hr = ppf->lpVtbl->Load(ppf, szPathName, 0);
			if (SUCCEEDED(hr)) hr = ppf->lpVtbl->QueryInterface(ppf, riid, ppv);

			if (ppf) ppf->lpVtbl->Release(ppf);
		}
	}
	else
	{
		/* Attempt to get an instance of szProgId from the running object table */

		CLSID clsid;
		IUnknown * pUnk = NULL;

		if (L'{' == szProgId[0])
			hr = CLSIDFromString((LPOLESTR) szProgId, &clsid);
		else
			hr = CLSIDFromProgID(szProgId, &clsid);

		if (SUCCEEDED(hr)) hr = GetActiveObject(&clsid, NULL, &pUnk);
		if (SUCCEEDED(hr)) hr = pUnk->lpVtbl->QueryInterface(pUnk, riid, ppv);

		if (pUnk) pUnk->lpVtbl->Release(pUnk);
	}

	return DH_EXIT(hr, szProgId);
}



/* **************************************************************************
 * dhCreateObject / dhGetObject:
 *   Shorthand versions of above functions. szMachine can specify a remote
 * machine on which to create the object(NULL for local).
 *
 ============================================================================ */
HRESULT dhCreateObject(LPCOLESTR szProgId, LPCWSTR szMachine, IDispatch ** ppDisp)
{
	COSERVERINFO si = { 0 };

	DH_ENTER(L"CreateObject");

	si.pwszName = (LPWSTR) szMachine;

	return DH_EXIT(dhCreateObjectEx(szProgId, &IID_IDispatch, 
			  szMachine ? CLSCTX_REMOTE_SERVER : CLSCTX_LOCAL_SERVER|CLSCTX_INPROC_SERVER,
			  szMachine ? &si : NULL, (void **) ppDisp), szProgId);
}



/* ========================================================================== */
HRESULT dhGetObject(LPCOLESTR szPathName, LPCOLESTR szProgId, IDispatch ** ppDisp)
{
	DH_ENTER(L"GetObject");

	return DH_EXIT(dhGetObjectEx(szPathName, szProgId, &IID_IDispatch,
			  CLSCTX_LOCAL_SERVER|CLSCTX_INPROC_SERVER, NULL, (void **) ppDisp), szProgId);
}



/* **************************************************************************
 * dhCallMethod / dhPutValue / dhPutRef / dhGetValue / dhInvoke
 *
 *   These functions are the accessor functions that initialise the va_list
 * and call their corresponding V versions.
 *
 * Notes:
 *   Please see the V versions of these functions for further information and
 * samples. The architecture of DispHelper requires that the va_list is passed by
 * reference so that multiple different functions can extract arguments.
 *
 ============================================================================ */
HRESULT dhCallMethod(IDispatch * pDisp, LPCOLESTR szMember, ... )
{
	HRESULT hr;
	va_list marker;

	DH_ENTER(L"CallMethod");

	/* Begin variable-argument list */
	va_start(marker, szMember);

	/* Delegate to actual work function which takes a pointer to a va_list */
	hr = dhCallMethodV(pDisp, szMember, &marker);

	/* End variable-argument section */
	va_end(marker);

	return DH_EXIT(hr, szMember);
}



/* ======================================================================== */
HRESULT dhPutValue(IDispatch * pDisp, LPCOLESTR szMember, ...)
{
	HRESULT hr;
	va_list marker;

	DH_ENTER(L"PutValue");

	va_start(marker, szMember);

	hr = dhPutValueV(pDisp, szMember, &marker);

	va_end(marker);

	return DH_EXIT(hr, szMember);
}


/* ======================================================================== */
HRESULT dhPutRef(IDispatch * pDisp, LPCOLESTR szMember, ...)
{
	HRESULT hr;
	va_list marker;

	DH_ENTER(L"PutRef");

	va_start(marker, szMember);

	hr = dhPutRefV(pDisp, szMember, &marker);

	va_end(marker);

	return DH_EXIT(hr, szMember);
}


/* ======================================================================== */
HRESULT dhGetValue(LPCWSTR szIdentifier, void * pResult, IDispatch * pDisp, LPCOLESTR szMember, ...)
{
	HRESULT hr;
	va_list marker;

	DH_ENTER(L"GetValue");

	va_start(marker, szMember);

	hr = dhGetValueV(szIdentifier, pResult, pDisp, szMember, &marker);

	va_end(marker);

	return DH_EXIT(hr, szMember);
}



/* ======================================================================== */
HRESULT dhInvoke(int invokeType, VARTYPE returnType, VARIANT * pvResult, IDispatch * pDisp, LPCOLESTR szMember, ...)
{
	HRESULT hr;
	va_list marker;

	DH_ENTER(L"Invoke");

	va_start(marker, szMember);

	hr = dhInvokeV(invokeType, returnType, pvResult, pDisp, szMember, &marker);

	va_end(marker);

	return DH_EXIT(hr, szMember);
}
