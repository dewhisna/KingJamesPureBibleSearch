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
#include "convert.h"

/* Global variable to record if we are running in UNICODE mode. */
BOOL dh_g_bIsUnicodeMode;


/* **************************************************************************
 * dhInvokeArray:
 *   This function is used to wrap calls to IDispatch::GetIdsOfNames and 
 * IDispatch::Invoke. It does not handle argument identifiers or sub objects.
 * These are handled by the higher level function dhInvoke.
 *
 * Parameter Info:
 *   invokeType - Method, property-get, property-put or property-putref.
 *   pDisp      - The dispatch interface on which to invoke the member.
 *   szMember   - The method or property name to invoke.
 *   pvResult   - Receives the result. This should be NULL if no result is desired.
 *   cArgs      - The number of arguments in the array.
 *   pArgs      - The array of VARIANT args. The arguments must be packed in reverse order.
 * For DISPATCH_PROPERTYPUT the last argument in the array is used as the property value.
 *
 * Example(s):
 *   dhInvokeArray(DISPATCH_METHOD, &vtResult, 2, pDisp, L"DoSomething", &vtArgs);
 *
 ============================================================================ */
HRESULT dhInvokeArray(int invokeType, VARIANT * pvResult, UINT cArgs,
                         IDispatch * pDisp, LPCOLESTR szMember, VARIANT * pArgs)
{
	DISPPARAMS dp       = { 0 };
	EXCEPINFO excep     = { 0 };
	DISPID dispidNamed  = DISPID_PROPERTYPUT;
	DISPID dispID;
	UINT uiArgErr;
	HRESULT hr;

	DH_ENTER(L"InvokeArray");

	if(!pDisp || !szMember || (cArgs != 0 && !pArgs)) return DH_EXIT(E_INVALIDARG, szMember);

	/* Get DISPID for name passed */
	hr = pDisp->lpVtbl->GetIDsOfNames(pDisp, &IID_NULL, (LPOLESTR *) &szMember, 1, LOCALE_USER_DEFAULT, &dispID);

	if(FAILED(hr)) return DH_EXITEX(hr, TRUE, szMember, szMember, NULL, 0);

	if (pvResult != NULL) VariantInit(pvResult);

	/* Build DISPPARAMS */
	dp.cArgs  = cArgs;
	dp.rgvarg = pArgs;

	/* Handle special-case for property-puts */
	if(invokeType & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
	{
		dp.cNamedArgs = 1;
		dp.rgdispidNamedArgs = &dispidNamed;
	}

	/* Make the call */
	hr = pDisp->lpVtbl->Invoke(pDisp, dispID, &IID_NULL, LOCALE_USER_DEFAULT, (WORD) invokeType, &dp, pvResult, &excep, &uiArgErr);

	return DH_EXITEX(hr, TRUE, szMember, szMember, &excep, uiArgErr);
}



/* **************************************************************************
 * dhCallMethodV:
 *   This function will attempt to execute a method. No value is returned from
 * the method. To return a value from a method dhGetValue should be used.
 *
 * Example(s):
 *   dhCallMethod(wdApp, L"Selection.TypeText(%S)", L"Testing 123");
 *   dhCallMethod(wdApp, L"Documents(%d).Activate", 2);
 *   dhCallMethod(xlApp, L"Worksheets(%d).Range(%s).Clear", 2, "A1:A10");
 *
 ============================================================================ */
HRESULT dhCallMethodV(IDispatch * pDisp, LPCOLESTR szMember, va_list * marker)
{
	DH_ENTER(L"CallMethodV");

	return DH_EXIT(dhInvokeV(DISPATCH_METHOD, VT_EMPTY, NULL, pDisp, szMember, marker), szMember);
}



/* **************************************************************************
 * dhPutValueV:
 *   This function will attempt to set a property value. The property value
 * is the last argument.
 *
 * Example(s):
 *   xlApp.ActiveSheet.Cells(1,3).Value = "test"
 *   ---> dhPutValue(xlApp, L"ActiveSheet.Cells(%d, %d).Value = %s", 1, 3, "test");
 *   wdApp.Documents(3).ShowSpellingErrors = True
 *   ---> dhPutValue(wdApp, L"Documents(%d).ShowSpellingErrors = %b", 3, TRUE);
 *
 ============================================================================ */
HRESULT dhPutValueV(IDispatch * pDisp, LPCOLESTR szMember, va_list * marker)
{
	DH_ENTER(L"PutValueV");

	return DH_EXIT(dhInvokeV(DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, pDisp, szMember, marker), szMember);
}



/* **************************************************************************
 * dhPutRefV:
 *   This function will attempt to set a property to an object reference.
 *
 * Example(s):
 *   dhPutRef(tts, L".Voice = %o", spVoice);
 *
 ============================================================================ */
HRESULT dhPutRefV(IDispatch * pDisp, LPCOLESTR szMember, va_list * marker)
{
	DH_ENTER(L"PutRefV");

	return DH_EXIT(dhInvokeV(DISPATCH_PROPERTYPUTREF, VT_EMPTY, NULL, pDisp, szMember, marker), szMember);
}



/* **************************************************************************
 * dhGetValueV:
 *   This function will attempt to retrieve a property value or the value returned
 * by a method. The returned value is placed in the object pointed to by pResult.
 * The type of the object to return is determined by the szIdentifier argument.
 *
 * Example(s):
 *   LPSTR szCaption = xlApp.Worksheets(1).Name
 *   ---> dhGetValue(L"%s", &szCaption, xlApp, L"Worksheets(%d).Name", 1);
 *   IDispatch * wdDoc = wdApp.Documents.Add
 *   ---> dhGetValue(L"%o", &wdDoc, wdApp, L"Documents.Add");
 *   SYSTEMTIME creationDate = file.datecreated
 *   ---> dhGetValue(L"%T", &creationDate, file, L"datecreated");
 *
 ============================================================================ */
HRESULT dhGetValueV(LPCWSTR szIdentifier, void * pResult, IDispatch * pDisp, LPCOLESTR szMember, va_list * marker)
{
	VARIANT vtResult;
	VARTYPE returnType;
	HRESULT hr;

	DH_ENTER(L"GetValueV");

	if (!pResult || !szIdentifier) return DH_EXIT(E_INVALIDARG, szMember);

	/* Skip % if it starts the identifier string. eg. "%d" */
	if (*szIdentifier == L'%') szIdentifier++;

	/* Set the return type that we want based on the identifier */
	switch(*szIdentifier)
	{
		case L'd': returnType = VT_I4;       break;
		case L'u': returnType = VT_UI4;      break;
		case L'e': returnType = VT_R8;       break;
		case L'b': returnType = VT_BOOL;     break;
		case L'v': returnType = VT_EMPTY;    break;
		case L'B': returnType = VT_BSTR;     break;
		case L'S': returnType = VT_BSTR;     break;
		case L's': returnType = VT_BSTR;     break;
		case L'T': returnType = VT_BSTR;     break;
		case L'o': returnType = VT_DISPATCH; break;
		case L'O': returnType = VT_UNKNOWN;  break;
		case L't': returnType = VT_DATE;     break;
		case L'W': returnType = VT_DATE;     break;
		case L'f': returnType = VT_DATE;     break;
		case L'D': returnType = VT_DATE;     break;
#ifndef _WIN64
		case L'p': returnType = VT_I4;       break;
#else
		case L'p': returnType = VT_I8;       break;
#endif
		default:
			DEBUG_NOTIFY_INVALID_IDENTIFIER(*szIdentifier);
			return DH_EXIT(E_INVALIDARG, szMember);
	}

	/* Delegate to get the value in a variant(vtResult) */
	hr = dhInvokeV(DISPATCH_PROPERTYGET|DISPATCH_METHOD, returnType, &vtResult, pDisp, szMember, marker);
	if (FAILED(hr)) return DH_EXIT(hr, szMember);

	/* dhInvokeV will only succeed if it can return a variant of
	 * the type we specified in returnType.
	 * This means we can safely extract the return value
	 * from the corresponding VARIANT member. */

	switch(*szIdentifier)
	{
		case L'd': 
			*((LONG *) pResult) = V_I4(&vtResult);
			break;

		case L'u':
			*((ULONG *) pResult) = V_UI4(&vtResult);
			break;

		case L'e':
			*((DOUBLE *) pResult) = V_R8(&vtResult);
			break;

		case L'b':
			*((BOOL *) pResult) = V_BOOL(&vtResult);
			break;

		case L'v':
			*((VARIANT *) pResult) = vtResult;
			break;

		case L'B':
			*((BSTR *) pResult) = V_BSTR(&vtResult);
			break;

		case L'S': 
			*((LPWSTR *) pResult) = V_BSTR(&vtResult);
			break;

		case L's':
			hr = ConvertBStrToAnsiStr(V_BSTR(&vtResult), (LPSTR *) pResult);
			SysFreeString(V_BSTR(&vtResult));
			break;

		case L'T':
			if (dh_g_bIsUnicodeMode)
			{
				*((LPWSTR *) pResult) = V_BSTR(&vtResult);
			}
			else
			{
				hr = ConvertBStrToAnsiStr(V_BSTR(&vtResult), (LPSTR *) pResult);
				SysFreeString(V_BSTR(&vtResult));
			}
			break;

		case L'o':
			*((IDispatch **) pResult) = V_DISPATCH(&vtResult);
			if (V_DISPATCH(&vtResult) == NULL) hr = E_NOINTERFACE;
			break;

		case L'O':
			*((IUnknown **) pResult) = V_UNKNOWN(&vtResult);
			if (V_UNKNOWN(&vtResult) == NULL) hr = E_NOINTERFACE;
			break;

		case L't':
			hr = ConvertVariantTimeToTimeT(V_DATE(&vtResult), (time_t *) pResult);
			break;

		case L'W':
			hr = ConvertVariantTimeToSystemTime(V_DATE(&vtResult), (SYSTEMTIME *) pResult);
			break;

		case L'f':
			hr = ConvertVariantTimeToFileTime(V_DATE(&vtResult), (FILETIME *) pResult);
			break;

		case L'D':
			*((DATE *) pResult) = V_DATE(&vtResult);
			break;

		case L'p': /* Note: Could use V_INTPTR if defined */
#ifndef _WIN64
			*((LPVOID *) pResult) = (LPVOID) V_I4(&vtResult);
#else
			*((LPVOID *) pResult) = (LPVOID) V_I8(&vtResult);
#endif
			break;
	}

	return DH_EXIT(hr, szMember);
}
