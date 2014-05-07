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

static HRESULT TraverseSubObjects(IDispatch ** ppDisp, LPWSTR * lpszMember, va_list * marker);
static HRESULT CreateArgumentArray(LPWSTR szTemp, VARIANT * pArgs, BOOL * pbFreeList, UINT * pcArgs, va_list * marker);
static HRESULT InternalInvokeV(int invokeType, VARTYPE returnType, VARIANT * pvResult, IDispatch * pDisp, LPOLESTR szMember, va_list * marker);
static HRESULT ExtractArgument(VARIANT * pvArg, WCHAR chIdentifier, BOOL * pbFreeArg, va_list * marker);


/* **************************************************************************
 * dhInvokeV:
 *   This function can be called externally. This function strips the parent
 * objects from the member using TraverseSubObjects and then passes the result
 * to InternalInvokeV to continue.
 *
 * Example(s):
 *   dhInvoke(DISPATCH_PROPERTYGET, VT_BSTR, &vtResult, wdApp, L"Documents(%d).Caption", 1);
 *   dhInvoke(DISPATCH_METHOD, VT_EMPTY, NULL, wdApp, L"Documents.Add");
 *
 ============================================================================ */
HRESULT dhInvokeV(int invokeType, VARTYPE returnType, VARIANT * pvResult,
                     IDispatch * pDisp, LPCOLESTR szMember, va_list * marker)
{
	WCHAR szCopy[DH_MAX_MEMBER];
	LPWSTR szTemp                  = szCopy;
	SIZE_T cchDest                 = ARRAYSIZE(szCopy);
	HRESULT hr;

	DH_ENTER(L"InvokeV");

	if (!pDisp || !szMember || !marker) return DH_EXIT(E_INVALIDARG, szMember);

	do /* Copy input string into buffer so we can modify it */
	{
		if (cchDest-- == 0) return DH_EXIT(E_INVALIDARG, szMember);
	}
	while( (*szTemp++ = *szMember++) );

	szTemp = szCopy;

	/* Get sub object in pDisp and sub member in szTemp */
	hr = TraverseSubObjects(&pDisp, &szTemp, marker);

	if (SUCCEEDED(hr))
	{
		/* This function extracts the arguments and invokes the member */
		hr = InternalInvokeV(invokeType, returnType, pvResult, pDisp, szTemp, marker);

		/* Release the object returned by TraverseSubObjects */
		pDisp->lpVtbl->Release(pDisp);
	}

	return DH_EXIT(hr, szMember);
}



/* **************************************************************************
 * TraverseSubObjects:
 *   This function is designed to traverse the object path to get each sub object
 * until no more sub objects are available.
 *
 *   eg. Assume pDisp = wdApp and szMember = "Selection.Range.Font.Bold"
 * In this case upon successful return pDisp will point to the 'Font'
 * object(the last sub object) and szMember will point to "Bold".
 *
 ============================================================================ */
static HRESULT TraverseSubObjects(IDispatch ** ppDisp, LPWSTR * lpszMember, va_list * marker)
{
	/* NOTE: Assumes that lpszMember string is modifiable. */
	/* NOTE: Assumes arguments have been validated.        */

	LPWSTR szSeperator, szTemp;
	VARIANT vtObject;
	HRESULT hr;

	DH_ENTER(L"TraverseSubObjects");

	/* Skip initial dot if it starts the input string */
	if (**lpszMember == L'.') (*lpszMember)++;

	/* AddRef on our dispatch pointer so we can return it or release it later */
	(*ppDisp)->lpVtbl->AddRef(*ppDisp);

	/* Find first '.' in szMember string */
	szSeperator = wcschr(*lpszMember, L'.');

	/* No dots equals no sub objects - we are finished */
	if (szSeperator == NULL) return DH_EXIT(NOERROR, *lpszMember);

	szTemp = *lpszMember;  	/* Convenience variable */

	/* eg. wdApp, "Selection.Range.Font.Bold" */

	do
	{
		/* Terminate object name at dot */
		*szSeperator = L'\0';

		/* eg. szTemp equals "Selection" so we are getting the 'Selection' object */
		hr = InternalInvokeV(DISPATCH_METHOD|DISPATCH_PROPERTYGET, VT_DISPATCH,
		                     &vtObject, *ppDisp, szTemp, marker);

		if (!vtObject.pdispVal && SUCCEEDED(hr)) hr = E_NOINTERFACE;

		/* Release old object in *ppDisp */
		(*ppDisp)->lpVtbl->Release(*ppDisp);

		if (FAILED(hr)) break;

		/* Copy new object into *ppDisp */
		*ppDisp = vtObject.pdispVal; 

		/* Point next object name to start at character after '.'.
		 * eg. szTemp will now equal "Range.Font.Bold" */
		szTemp = szSeperator + 1;

	} /* Find next dot seperator */
	while ( (szSeperator = wcschr(szTemp, L'.') ) != NULL); 

	/* eg. if input was "ActiveDocument.Select.TypeText(%S)" then
	 * szMember will point to "TypeText(%S)". */

	*lpszMember = szTemp;

	return DH_EXIT(hr, *lpszMember);
}



/* **************************************************************************
 * InternalInvokeV:
 *   This function is responsible for invoking a member with no parent objects.
 * Example input: 'Navigate(%S, %d)', 'Visible = %b', 'Cells(%d,%d)'
 *
 ============================================================================ */
static HRESULT InternalInvokeV(int invokeType, VARTYPE returnType, VARIANT * pvResult,
                               IDispatch * pDisp, LPOLESTR szMember, va_list * marker)
{
	/* NOTE: Assumes that the szMember string is modifiable. */
	/* NOTE: Assumes arguments have been validated.          */

	VARIANT vtArgs[DH_MAX_ARGS];           /* Argument array */
	BOOL bFreeList[DH_MAX_ARGS];           /* List of which arguments need to be freed */
	HRESULT hr;
	UINT cArgs, iArg;

	DH_ENTER(L"InternalInvokeV");

	/* This function also terminates member name at start of arguments */
	hr = CreateArgumentArray(szMember, vtArgs, bFreeList, &cArgs, marker);

	if (SUCCEEDED(hr))
	{
		/* Invoke member */
		hr = dhInvokeArray(invokeType, pvResult, cArgs, pDisp, szMember, &vtArgs[DH_MAX_ARGS - cArgs]);

		/* Free the variants in the argument array as needed */
		for (iArg = DH_MAX_ARGS - cArgs;iArg < DH_MAX_ARGS;iArg++)
		{
			if (bFreeList[iArg]) VariantClear(&vtArgs[iArg]);
		}

		/* Coerce result (if it exists) into the desired type. 
		 * Note that VT_EMPTY means do not coerce the return value. */
		if (SUCCEEDED(hr) && pvResult != NULL &&
	            pvResult->vt != returnType && returnType != VT_EMPTY)
		{
			hr = VariantChangeType(pvResult, pvResult, 16 /* = VARIANT_LOCALBOOL */, returnType);
			if (FAILED(hr)) VariantClear(pvResult);
		}
	}

	return DH_EXIT(hr, szMember);
}



/* **************************************************************************
 * CreateArgumentArray:
 *   Fills the argument array for a member. Also terminates the member name
 * at the start of the arguments.
 *
 *   eg. If szMember is "Navigate(%S, %d)" then pArgs will contain one BSTR
 * variant and one VT_I4 variant, *pcArgs will equal two and szMember will 
 * equal "Navigate" upon successful return.
 * 
 ============================================================================ */
static HRESULT CreateArgumentArray(LPWSTR szMember, VARIANT * pArgs, BOOL * pbFreeList,
				   UINT * pcArgs, va_list * marker)
{
	/* NOTE: Assumes that the szMember string is modifiable. */
	/* NOTE: Assumes arguments have been validated.          */

	HRESULT hr        = NOERROR;
	INT iArg          = DH_MAX_ARGS;
	BOOL bInArguments = FALSE;

	DH_ENTER(L"CreateArgumentArray");

	/* Note: As we have to pack the arguments in reverse order
	 * iArg starts at DH_MAX_ARGS and we work backwards. */

	while (*szMember)
	{
		if (!bInArguments &&          /* Check for start of arguments.  */
                   (*szMember == L'(' || *szMember == L' ' || *szMember == L'=') )
		{
			bInArguments = TRUE;

			/* Terminate the member name string at start of arguments */
			*szMember = L'\0';
		}
		else if  (*szMember == L'%') /* Prepends argument identifiers */
		{ 
			if (!bInArguments) /* % also signifies the start of arguments */
			{
				bInArguments = TRUE;
				*szMember = L'\0';
			}

			iArg--;

			/* Check if we have ran out of argument slots */
			if (iArg == -1) { hr = E_INVALIDARG; break; }

			szMember++; /* Move forward to actual identifier */

			/* Extract argument based on identifier */
			hr = ExtractArgument(&pArgs[iArg], *szMember, &pbFreeList[iArg], marker);

			if (FAILED(hr)) break;
		}

		/* Move to next character in input string */
		szMember++;
	}

	*pcArgs = DH_MAX_ARGS - iArg;  /* Return argument count */

	if (FAILED(hr))
	{
		/* Free arguments that have already been allocated */
		for (++iArg;iArg < DH_MAX_ARGS; iArg++)
		{
			if (pbFreeList[iArg]) VariantClear(&pArgs[iArg]);
		}
	}

	return DH_EXIT(hr, szMember);
}



/* **************************************************************************
 * ExtractArgument:
 *   Extract (and convert if needed) an argument from the va_list based on an
 * identifier and pack it in a VARIANT.
 *
 ============================================================================ */
static HRESULT ExtractArgument(VARIANT * pvArg, WCHAR chIdentifier, BOOL * pbFreeArg, va_list * marker)
{
	HRESULT hr = NOERROR;

	/* By default, the argument does not need to be freed */
	*pbFreeArg = FALSE;

	/* Change 'T' identifier to 'S' or 's' based on UNICODE mode */
	if (chIdentifier == L'T') chIdentifier = (dh_g_bIsUnicodeMode ? L'S' : L's');

	switch (chIdentifier)
	{
		case L'd':   /* LONG */
			V_VT(pvArg)  = VT_I4;
			V_I4(pvArg)  = va_arg(*marker, LONG);
			break;

		case L'u':   /* ULONG */
			V_VT(pvArg)  = VT_UI4;
			V_UI4(pvArg) = va_arg(*marker, ULONG);
			break;

		case L'e':   /* DOUBLE */
			V_VT(pvArg)  = VT_R8;
			V_R8(pvArg)  = va_arg(*marker, DOUBLE);
			break;

		case L'b':   /* BOOL */
			V_VT(pvArg)   = VT_BOOL;
			V_BOOL(pvArg) = ( va_arg(*marker, BOOL) ? VARIANT_TRUE : VARIANT_FALSE );
			break;

		case L'v':   /*  VARIANT *  */
			*pvArg  = *va_arg(*marker, VARIANT *);
			break;

		case L'm':   /* Missing optional argument */
			V_VT(pvArg)    = VT_ERROR;
			V_ERROR(pvArg) = DISP_E_PARAMNOTFOUND;
			break;

		case L'B':   /* BSTR */
			V_VT(pvArg)   = VT_BSTR;
			V_BSTR(pvArg) = va_arg(*marker, BSTR);
			break;

		case L'S':   /* LPCOLESTR (aka LPCWSTR) */
		{
			LPOLESTR szTemp = va_arg(*marker, LPOLESTR);

			V_VT(pvArg)   = VT_BSTR;
			V_BSTR(pvArg) = SysAllocString(szTemp);

			if (V_BSTR(pvArg) == NULL && szTemp != NULL) hr = E_OUTOFMEMORY;

			*pbFreeArg = TRUE;   /* We must free this argument */
			break;
		}

		case L's':   /* LPCSTR */
			V_VT(pvArg) = VT_BSTR;
			hr = ConvertAnsiStrToBStr(va_arg(*marker, LPSTR), &V_BSTR(pvArg));
			*pbFreeArg = TRUE;   /* We must free this argument */
			break;

		case L'o':   /* IDispatch *   */
			V_VT(pvArg)       = VT_DISPATCH;
			V_DISPATCH(pvArg) = va_arg(*marker, IDispatch *);
			break;

		case L'O':   /* IUnknown *    */
			V_VT(pvArg)      = VT_UNKNOWN;
			V_UNKNOWN(pvArg) = va_arg(*marker, IUnknown *);
			break;

		case L'D':   /* DATE (Variant Date) */
			V_VT(pvArg)   = VT_DATE;
			V_DATE(pvArg) = va_arg(*marker, DATE);
			break;

		case L't':   /* time_t */
			V_VT(pvArg) = VT_DATE;
			hr = ConvertTimeTToVariantTime(va_arg(*marker, time_t), &V_DATE(pvArg));
			break;

		case L'W':   /* SYSTEMTIME *    */
			V_VT(pvArg) = VT_DATE;
			hr = ConvertSystemTimeToVariantTime(va_arg(*marker, SYSTEMTIME *), &V_DATE(pvArg));
			break;

		case L'f':   /* FILETIME *   */
			V_VT(pvArg) = VT_DATE;
			hr = ConvertFileTimeToVariantTime(va_arg(*marker, FILETIME *), &V_DATE(pvArg));
			break;

		case L'p':   /* Pointers, handles, etc */
#ifndef _WIN64
			V_VT(pvArg) = VT_I4;
			V_I4(pvArg) = (LONG) va_arg(*marker, LPVOID);
#else
			V_VT(pvArg) = VT_I8;
			V_I8(pvArg) = (LONGLONG) va_arg(*marker, LPVOID);
#endif
			break;

		default:    /* Invalid identifier */
			hr = E_INVALIDARG;
			DEBUG_NOTIFY_INVALID_IDENTIFIER(chIdentifier);
			break;
	}

	return hr;
}
