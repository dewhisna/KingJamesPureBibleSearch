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
 * dhEnumBeginV:
 *   This function is used to begin an enumeration. It returns an IEnumVARIANT
 * interface.
 *
 * Example(s):
 *   dhEnumBegin(&pEnum, wdDocs, NULL);
 *   dhEnumBegin(&pEnum, wdApp, L".Documents(%d).Words", 2);
 *
 ============================================================================ */
HRESULT dhEnumBeginV(IEnumVARIANT ** ppEnum, IDispatch * pDisp, LPCOLESTR szMember, va_list * marker)
{
	DISPPARAMS dp    = { 0 };
	EXCEPINFO excep  = { 0 };
	VARIANT vtResult;
	IDispatch * pDispObj;
	HRESULT hr;

	DH_ENTER(L"EnumBeginV");

	if (!ppEnum || !pDisp) return DH_EXIT(E_INVALIDARG, szMember);

	if (szMember != NULL) /* Retrieve the object */
	{
		hr = dhGetValueV(L"%o", &pDispObj, pDisp, szMember, marker);
		if (FAILED(hr)) return DH_EXIT(hr, szMember);
	}
	else
	{
		pDispObj = pDisp;
	}

	/* Now ask the object for an enumerator */
	hr = pDispObj->lpVtbl->Invoke(pDispObj, DISPID_NEWENUM, &IID_NULL, LOCALE_USER_DEFAULT,
				 DISPATCH_METHOD | DISPATCH_PROPERTYGET, &dp, &vtResult, &excep, NULL);

	if (szMember != NULL) pDispObj->lpVtbl->Release(pDispObj);

	if (FAILED(hr)) return DH_EXITEX(hr, TRUE, L"_NewEnum", szMember, &excep, 0);

	/* Retrieve an IEnumVariant interface from the returned interface */
	if (vtResult.vt == VT_DISPATCH)
		hr = vtResult.pdispVal->lpVtbl->QueryInterface(vtResult.pdispVal, &IID_IEnumVARIANT, (void **) ppEnum);
	else if (vtResult.vt == VT_UNKNOWN)
		hr = vtResult.punkVal->lpVtbl->QueryInterface(vtResult.punkVal, &IID_IEnumVARIANT, (void **) ppEnum);
	else
		hr = E_NOINTERFACE;

	VariantClear(&vtResult);

	return DH_EXIT(hr, szMember);
}



/* **************************************************************************
 * dhEnumNextVariant:
 *   This function retrieves the next VARIANT from an IEnumVariant interface.
 *
 ============================================================================ */
HRESULT dhEnumNextVariant(IEnumVARIANT * pEnum, VARIANT * pvResult)
{
	DH_ENTER(L"EnumNextVariant");

	if (!pEnum || !pvResult) return DH_EXIT(E_INVALIDARG, L"Enumerator");

	return DH_EXIT(pEnum->lpVtbl->Next(pEnum, 1, pvResult, NULL), L"Enumerator");
}



/* **************************************************************************
 * dhEnumNextObject:
 *   This function retrieves the next IDispatch object from an IEnumVariant
 * interface.
 *
 * Example(s):
 *   while(dhEnumNextObject(pEnum, &wdPara) == NOERROR)
 *
 ============================================================================ */
HRESULT dhEnumNextObject(IEnumVARIANT * pEnum, IDispatch ** ppDisp)
{
	VARIANT vtResult;
	HRESULT hr;

	DH_ENTER(L"EnumNextObject");

	if (!pEnum || !ppDisp) return DH_EXIT(E_INVALIDARG, L"Enumerator");

	hr = pEnum->lpVtbl->Next(pEnum, 1, &vtResult, NULL);

	if (hr == S_OK)
	{
		if (vtResult.vt == VT_DISPATCH)
		{
			*ppDisp = vtResult.pdispVal;
		}
		else
		{
			hr = VariantChangeType(&vtResult, &vtResult, 0, VT_DISPATCH);
			if (SUCCEEDED(hr)) *ppDisp = vtResult.pdispVal;
			else VariantClear(&vtResult);
		}
	}

	return DH_EXIT(hr, L"Enumerator");
}



/* =========================================================================== */
HRESULT dhEnumBegin(IEnumVARIANT ** ppEnum, IDispatch * pDisp, LPCOLESTR szMember, ...)
{
	HRESULT hr;
	va_list marker;

	DH_ENTER(L"EnumBegin");

	va_start(marker, szMember);

	hr = dhEnumBeginV(ppEnum, pDisp, szMember, &marker);

	va_end(marker);

	return DH_EXIT(hr, szMember);
}
