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
#include <assert.h>

#ifndef DISPHELPER_NO_EXCEPTIONS

/* Structure to store global exception options. */
static DH_EXCEPTION_OPTIONS g_ExceptionOptions;

static LONG  f_lngTlsInitBegin = -1, f_lngTlsInitEnd = -1;
static DWORD f_TlsIdxStackCount, f_TlsIdxException;

#define SetStackCount(nStackCount)   TlsSetValue(f_TlsIdxStackCount, (LPVOID) (nStackCount))
#define SetExceptionPtr(pException)  TlsSetValue(f_TlsIdxException, pException);
#define GetStackCount()       (UINT) TlsGetValue(f_TlsIdxStackCount)
#define GetExceptionPtr()            TlsGetValue(f_TlsIdxException)
#define CheckTlsInitialized()        if (f_lngTlsInitEnd != 0) InitializeTlsIndexes();



/* **************************************************************************
 * hlprStringCchCopyW:
 *   Helper function. Replaces wcsncpy.
 *
 ============================================================================ */
static void hlprStringCchCopyW(LPWSTR pszDest, SIZE_T cchDest, LPCWSTR pszSrc)
{
	assert(cchDest > 0);

	do
	{
		if (--cchDest == 0) break;
	}
	while ((*pszDest++ = *pszSrc++));

	*pszDest = L'\0';
}



/* **************************************************************************
 * InitializeTlsIndexes:
 *   Initializes the Tls indexes if needed.
 *
 ============================================================================ */
static void InitializeTlsIndexes(void)
{
	if (0 == InterlockedIncrement(&f_lngTlsInitBegin))
	{
		f_TlsIdxStackCount = TlsAlloc();
		f_TlsIdxException  = TlsAlloc();
		f_lngTlsInitEnd    = 0;
	}
	else
	{
		/* Deal with extremely unlikely race condition */
		while (f_lngTlsInitEnd != 0) Sleep(5);
	}
}



/* **************************************************************************
 * dhEnter:
 *   This function is called on entering a DispHelper function.
 *
 ============================================================================ */
void dhEnter(void)
{
	CheckTlsInitialized();
	SetStackCount(GetStackCount() + 1);
}



/* **************************************************************************
 * dhExitEx:
 *   This function is called when exiting a DispHelper function.
 *
 * Parameter Info:
 *   bDispatchError   - TRUE if the error was returned from the IDispatch interface.  
 *   szMember         - The member name that caused the error. eg. "TypeText"
 *   szCompleteMember - The complete member. eg. "Selection.TypeText(%S)"
 *   pExcepInfo       - A pointer to the EXCEPINFO structure returned by IDispatch::Invoke.
 *   iArgError        - The index of the argument that caused the error as returned by Invoke.
 *   szFunctionName   - The function which is exiting(string must have global life time).
 * 
 * Notes:
 *   szMember, szCompleteMember, pExcepInfo and iArgError are NULL or 0 if not available.
 *
 ============================================================================ */
HRESULT dhExitEx(HRESULT hr, BOOL bDispatchError, LPCWSTR szMember, LPCWSTR szCompleteMember,
                 EXCEPINFO * pExcepInfo, UINT iArgError, LPCWSTR szFunctionName)
{
	UINT nStackCount = GetStackCount();

	SetStackCount(nStackCount - 1);

	if (FAILED(hr) && !g_ExceptionOptions.bDisableRecordExceptions)
	{
		PDH_EXCEPTION pException = GetExceptionPtr();

		if (!pException) /* No exception allocated for this thread yet */
		{ 
			pException = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DH_EXCEPTION));
			if (!pException) return hr;
			SetExceptionPtr(pException);
		}
		else if (pException->bOld) /* Exception is from a former call */
		{
			SysFreeString(pException->szDescription);
			SysFreeString(pException->szSource);
			SysFreeString(pException->szHelpFile);
			ZeroMemory(pException, sizeof(DH_EXCEPTION));
		}

		if (pException->hr == 0)
		{
			/* Only record the error information the first time it is reported */

			pException->hr              = hr;
			pException->iArgError       = iArgError;
			pException->szErrorFunction = szFunctionName;
			pException->bDispatchError  = bDispatchError;

			if (szMember) hlprStringCchCopyW(pException->szMember, ARRAYSIZE(pException->szMember), szMember);

			if (pExcepInfo && hr == DISP_E_EXCEPTION)
			{
				/* Extract error info returned by IDispatch::Invoke in an EXCEPINFO */

				if (pExcepInfo->pfnDeferredFillIn &&
				    !IsBadCodePtr((FARPROC) pExcepInfo->pfnDeferredFillIn)) pExcepInfo->pfnDeferredFillIn(pExcepInfo);

				pException->szDescription = pExcepInfo->bstrDescription;
				pException->szSource      = pExcepInfo->bstrSource;
				pException->szHelpFile    = pExcepInfo->bstrHelpFile;
				pException->dwHelpContext = pExcepInfo->dwHelpContext;
				pException->swCode        = (pExcepInfo->wCode ? pExcepInfo->wCode : pExcepInfo->scode);
			}
		}

		if (nStackCount == 1) /* We are exiting the outer most function */
		{
			pException->bOld              = TRUE;
			pException->szInitialFunction = szFunctionName;

			if (szCompleteMember) hlprStringCchCopyW(pException->szCompleteMember, ARRAYSIZE(pException->szCompleteMember), szCompleteMember);

			if (g_ExceptionOptions.bShowExceptions)
				dhShowException(pException);

			if (g_ExceptionOptions.pfnExceptionCallback)
				g_ExceptionOptions.pfnExceptionCallback(pException);
		}
	}
	else if (hr == DISP_E_EXCEPTION && pExcepInfo)
	{
		/* We are responsible for cleaning up pExcepInfo even if we don't use it */
		SysFreeString(pExcepInfo->bstrDescription);
		SysFreeString(pExcepInfo->bstrSource);
		SysFreeString(pExcepInfo->bstrHelpFile);
	}

	return hr;
}



/* **************************************************************************
 * dhShowException:
 *   This function calls dhFormatException to format the provided exception
 * into a string and then shows it in a MessageBox.
 *
 ============================================================================ */
HRESULT dhShowException(PDH_EXCEPTION pException)
{
	WCHAR szMessage[512];

	dhFormatExceptionW(pException, szMessage, ARRAYSIZE(szMessage), FALSE);

	/* NOTE: MessageBoxW is one of the few unicode APIs available on Win9x. */
	MessageBoxW(g_ExceptionOptions.hwnd, szMessage, g_ExceptionOptions.szAppName,
	            MB_ICONSTOP | MB_SETFOREGROUND);

	return NOERROR;
}



/* **************************************************************************
 * dhFormatException:
 *   This function formats the information in the provided exception structure into 
 * a string and returns it in szBuffer. Up to cchBufferSize characters are placed
 * in szBuffer.
 *
 ============================================================================ */
HRESULT dhFormatExceptionW(PDH_EXCEPTION pException, LPWSTR szBuffer, UINT cchBufferSize, BOOL bFixedFont)
{
	HRESULT hr;
	UINT cch = 0;
#	define DESCRIPTION_LENGTH 255

	if (!szBuffer && cchBufferSize) return E_INVALIDARG;

	if (!pException)
	{
		dhGetLastException(&pException);
		if (!pException)
		{
			if (cchBufferSize != 0)
			{
				_snwprintf(szBuffer, cchBufferSize, L"No error information available.");
				szBuffer[cchBufferSize - 1] = L'\0';
			}

			return NOERROR;
		}
	}

	hr = (pException->hr == DISP_E_EXCEPTION && pException->swCode ?
			pException->swCode : pException->hr);

	if (!pException->szSource)
	{ 
		/* Provide a generic source if we don't yet have one */

		if (pException->bDispatchError)
			pException->szSource = SysAllocString(L"IDispatch Interface");
		else
			pException->szSource = SysAllocString(L"Application");
	}

	if (!pException->szDescription)
	{ 
		/* Provide a description if we don't yet have one */

		pException->szDescription = SysAllocStringLen(NULL, DESCRIPTION_LENGTH);

		if (pException->szDescription)
		{
			switch (hr)
			{
				/* Check for HRESULTs that we handle with custom messages */

				case E_NOINTERFACE:
					_snwprintf(pException->szDescription, DESCRIPTION_LENGTH, L"Object required");
					break;

				case DISP_E_UNKNOWNNAME:
				case DISP_E_MEMBERNOTFOUND:
					_snwprintf(pException->szDescription, DESCRIPTION_LENGTH, L"Object doesn't support this property or method: '%s'", pException->szMember);
					break;

				case DISP_E_TYPEMISMATCH:
					if (pException->szMember[0])
					{
						_snwprintf(pException->szDescription, DESCRIPTION_LENGTH, L"Type mismatch: '%s'. Argument Index: %d", pException->szMember, pException->iArgError);
						break;
					} /* else fall through */

				default:
				{
					/* If we don't have an error description yet, use FormatMessage to get one */
#ifndef UNICODE
					CHAR szDescription[DESCRIPTION_LENGTH];
#else
					LPWSTR szDescription = pException->szDescription;
#endif
					cch = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					             NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					             szDescription, DESCRIPTION_LENGTH, NULL);

					if (!cch) wcscpy(pException->szDescription, L"Unknown runtime error");
#ifndef UNICODE
					else MultiByteToWideChar(CP_ACP, 0, szDescription, -1, pException->szDescription, DESCRIPTION_LENGTH);
#endif
				}
			}
		}
	}

	if (pException->szDescription)
	{
		/* Get rid of new line on the end of the description if it is present */

		if (!cch) cch = wcslen(pException->szDescription);

		if (cch >= 2 && pException->szDescription[cch - 2] == L'\r')
			pException->szDescription[cch - 2] = L'\0';
		else if (cch >= 1 && pException->szDescription[cch - 1] == L'\n')
			pException->szDescription[cch - 1] = L'\0';
	}

	if (cchBufferSize)
	{
		if (!bFixedFont)
		{
			_snwprintf(szBuffer, cchBufferSize, L"Member:\t  %s\r\nFunction:\t  %s\t\t\r\nError In:\t  %s\r\nError:\t  %s\r\nCode:\t  %x\r\nSource:\t  %s",
				pException->szCompleteMember,
				pException->szInitialFunction, pException->szErrorFunction,
				pException->szDescription, hr,
				pException->szSource);
		}
		else
		{
			_snwprintf(szBuffer, cchBufferSize, L"Member:   %s\r\nFunction: %s\r\nError In: %s\r\nError:    %s\r\nCode:     %x\r\nSource:   %s",
				pException->szCompleteMember,
				pException->szInitialFunction, pException->szErrorFunction,
				pException->szDescription, hr,
				pException->szSource);
		}

		szBuffer[cchBufferSize - 1] = L'\0';
	}

	return NOERROR;
}


/* ============================================================================ */
HRESULT dhFormatExceptionA(PDH_EXCEPTION pException, LPSTR szBuffer, UINT cchBufferSize, BOOL bFixedFont)
{
	WCHAR szBufferW[1024];

	dhFormatExceptionW(pException, szBufferW, ARRAYSIZE(szBufferW), bFixedFont);

	if (0 == WideCharToMultiByte(CP_ACP, 0, szBufferW, -1, szBuffer, cchBufferSize, NULL, NULL))
		return HRESULT_FROM_WIN32( GetLastError() );

	return NOERROR;
}


/* **************************************************************************
 * dhGetLastException:
 *   This function places a pointer to the last dh exception to occur on this
 * thread in ppException. If no exception is available NULL is returned.
 *
 ============================================================================ */
HRESULT dhGetLastException(PDH_EXCEPTION * ppException)
{
	if (!ppException) return E_INVALIDARG;

	CheckTlsInitialized();
	*ppException = GetExceptionPtr();

	return NOERROR;
}



/* **************************************************************************
 * dhToggleExceptions:
 *   This function toggles whether exceptions are shown when an error occurs.
 *
 ============================================================================ */
HRESULT dhToggleExceptions(BOOL bShow)
{
	g_ExceptionOptions.bShowExceptions = bShow;
	if (bShow) g_ExceptionOptions.bDisableRecordExceptions = FALSE;

	return NOERROR;
}



/* **************************************************************************
 * dhSetExceptionOptions:
 *   This function sets the global exception options to those in the 
 * provided exception options structure.
 *
 ============================================================================ */
HRESULT dhSetExceptionOptions(PDH_EXCEPTION_OPTIONS pExceptionOptions)
{
	if (!pExceptionOptions) return E_INVALIDARG;

	/* Set every value individually to guarantee it is done atomically */

	g_ExceptionOptions.hwnd                     = pExceptionOptions->hwnd;
	g_ExceptionOptions.szAppName                = pExceptionOptions->szAppName;
	g_ExceptionOptions.bShowExceptions          = pExceptionOptions->bShowExceptions;
	g_ExceptionOptions.bDisableRecordExceptions = pExceptionOptions->bDisableRecordExceptions;
	g_ExceptionOptions.pfnExceptionCallback     = pExceptionOptions->pfnExceptionCallback;

	return NOERROR;
}



/* **************************************************************************
 * dhGetExceptionOptions:
 *   This function copies the global exception options to the structure
 * pointed to by pExceptionOptions.
 *
 ============================================================================ */
HRESULT dhGetExceptionOptions(PDH_EXCEPTION_OPTIONS pExceptionOptions)
{
	if (!pExceptionOptions) return E_INVALIDARG;

	pExceptionOptions->hwnd                     = g_ExceptionOptions.hwnd;
	pExceptionOptions->szAppName                = g_ExceptionOptions.szAppName;
	pExceptionOptions->bShowExceptions          = g_ExceptionOptions.bShowExceptions;
	pExceptionOptions->bDisableRecordExceptions = g_ExceptionOptions.bDisableRecordExceptions;
	pExceptionOptions->pfnExceptionCallback     = g_ExceptionOptions.pfnExceptionCallback;

	return NOERROR;
}



/* **************************************************************************
 * dhCleanupThreadException:
 *   Internal function called by dhUninitalize to cleanup this thread's
 * exception.
 *
 ============================================================================ */
void dhCleanupThreadException(void)
{
	PDH_EXCEPTION pException;

	CheckTlsInitialized();
	pException = GetExceptionPtr();
	
	if (pException)
	{
		SysFreeString(pException->szDescription);
		SysFreeString(pException->szSource);
		SysFreeString(pException->szHelpFile);

		HeapFree(GetProcessHeap(), 0, pException);

		SetExceptionPtr(NULL);
	}
}


#endif /* ----- DISPHELPER_NO_EXCEPTIONS ----- */
