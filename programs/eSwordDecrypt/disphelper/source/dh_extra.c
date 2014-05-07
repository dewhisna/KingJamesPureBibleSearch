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


/* Note: The functions in this file are not available in the DispHelper
 * single file version and are considered extras.
 */


#define DISPHELPER_INTERNAL_BUILD
#include "disphelper.h"


/* **************************************************************************
 * dhAutoWrap:
 *   This function is included as a replacement for the AutoWrap function used by
 * the Microsoft B2C utility.
 *
 * Notes:
 *   B2C is available from: http://support.microsoft.com/default.aspx?scid=kb;EN-US;216388
 *
 * Example(s):
 *   dhAutoWrap(DISPATCH_METHOD, NULL, wdApp, L"DoSomething", 3, vtArg2, vtArg1, vtArg0);
 *   dhAutoWrap(DISPATCH_PROPERTYGET, &vtResult, wdApp, L"Caption", 0);
 *
 ============================================================================ */
HRESULT dhAutoWrap(int invokeType, VARIANT * pvResult, IDispatch * pDisp, LPCOLESTR szMember, UINT cArgs, ...)
{
	VARIANT vtArgs[DH_MAX_ARGS];
	HRESULT hr;
	UINT iArg;
	va_list marker;

	DH_ENTER(L"AutoWrap");

	if (cArgs > ARRAYSIZE(vtArgs)) return DH_EXIT(E_INVALIDARG, szMember);

	/* Begin variable-argument list */
	va_start(marker, cArgs);

	/* Extract arguments */
	for(iArg = 0; iArg < cArgs; iArg++)
	{
		vtArgs[iArg] = va_arg(marker, VARIANT);
	}

	/* Delegate to actual work function */
	hr = dhInvokeArray(invokeType, pvResult, cArgs, pDisp, szMember, vtArgs);

	/* End variable-argument section */
	va_end(marker);

	return DH_EXIT(hr, szMember);
}


#ifdef _MSC_VER
#pragma warning(disable : 4701) /* Possible use of variable before initialization.
                                   For szParamValue in function below. */
#endif


/* **************************************************************************
 * dhParseProperties:
 *   This function is used to set several properties on a COM object in one call.
 * Each name/value pair is extracted from the string and invoked seperately. As each value
 * is invoked as a string this function takes advantage of automatic IDispatch type coercing.
 * For example, if we have "Visible=True" the string value "True" will be coerced to the
 * numeric value VARIANT_TRUE if that property expects a boolean value.
 *
 * Parameter Info:
 *   pDisp        - The IDispatch interface on which to set the properties.
 *   szProperties - The name/value pairs. Each property name must be followed with an
 * equal and each property value, including the last, must be followed by a colon. Property
 * values are not quoted and can not include a colon.
 *   lpcPropsSet  - If this is not NULL, the number of properties correctly set will be returned.
 *
 * Return Value:
 *   This function returns the hr of the last assignment to fail or S_OK if all assignments
 * were successful.
 *
 * Notes:
 *   This function does not handle argument identifiers or sub objects. Automatic coercion
 * from strings to other types is limited so setting properties that do not natively take
 * strings should be tested.
 *
 * Example(s):
 *   dhParseProperties(ieApp, L"Visible=True;Top=120;Left=30;", NULL);
 *   dhParseProperties(wdDoc, L"Name=My Pet Rock List;ShowSpellingErrors=False;", &cPropsSet);
 *
 ============================================================================ */
HRESULT dhParseProperties(IDispatch * pDisp, LPCWSTR szProperties, UINT * lpcPropsSet)
{
	BOOL bInValue = FALSE;
	HRESULT hr, hrRet = S_OK;
	LPWSTR szParamName, szParamValue;
	VARIANT vtNewVal;
	BSTR bstrCopy;        /* Copy of input string that we can modify. */
	LPWSTR szParams;      /* Modifiable pointer to bstrCopy contents. */

	DH_ENTER(L"ParseProperties");

	/* Set successful assignment count to zero */
	if (lpcPropsSet) *lpcPropsSet = 0;

	if (!szProperties) return DH_EXIT(E_INVALIDARG, szProperties);

	/* Make a modifiable copy of the input string. We use SysAllocString in place of wcsdup. */
	if (!(bstrCopy = SysAllocString(szProperties))) return DH_EXIT(E_OUTOFMEMORY, szProperties);

	/* Set first param name to start at start of input string */
	szParamName = szParams = bstrCopy;

	while (*szParams) {

		if (*szParams == L'=' && !bInValue)
		{
			/* An equal sign signifies the start of the parameter value
			 * and the end of the param name if we are not already
			 * in the param value... */

			bInValue = TRUE;
			szParamValue = szParams + 1;  /* Param value starts after the equal */
			*szParams = L'\0';            /* Terminate param name at equal */
		}

		else if (*szParams == L';' && bInValue)
		{
			/* A colon signifies the end of a param value and the
			 * start of a new param name */

			bInValue = FALSE;
			*szParams = L'\0';   /* Terminate param value at colon */

			/* DEBUGUSE - MessageBoxW(NULL,szParamValue,szParamName,0); */

			vtNewVal.vt = VT_BSTR;
			vtNewVal.bstrVal = SysAllocString(szParamValue);

			if (vtNewVal.bstrVal == NULL) hr = E_OUTOFMEMORY;
			else hr = dhInvokeArray(DISPATCH_PROPERTYPUT, NULL, 1, pDisp, szParamName, &vtNewVal);

			SysFreeString(vtNewVal.bstrVal);

			if (FAILED(hr)) hrRet = hr;              /* We return the HR of the last assignment to fail or S_OK if no failures */
			else if (lpcPropsSet) (*lpcPropsSet)++;  /* We keep count of the number of successful assignments */

			szParamName = szParams + 1;  /* Start new param name after colon */
		}

		szParams++;  /* Move to next character */

	} /* End while */

	SysFreeString(bstrCopy);

	return DH_EXIT(hrRet, szProperties);
}


#ifdef _MSC_VER
#pragma warning(default : 4701) 
#endif
