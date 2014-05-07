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


/* Some of the code in this file is based loosely on VB code by William Rowe found at:
 * http://groups.google.com/groups?selm=uwJJuh2I%24GA.234%40cppssbbsa05
 */


#include "convert.h"
#include <math.h>

  /* Number of 100 nannosecond units in a FILETIME day */
static const LONGLONG FILE_TIME_ONE_DAY           = 864000000000;

  /* VARIANT DATE 0 (1899-Dec-30) as a FILETIME */
static const LONGLONG FILE_TIME_VARIANT_DAY0      = 94353120000000000;

  /* FILETIME of day 1, year 10,000 */
static const ULONGLONG FILE_TIME_VARIANT_OVERFLOW  = 2650467744000000000;

  /* FILETIME date 0 (1601-Jan-1) as a VARIANT DATE */
static const DATE      VARIANT_FILE_TIME_DAY0      = -109205;

  /* time_t date 0 (1970-Jan-1) as a VARIANT DATE */
static const DATE      VARIANT_TIMET_DAY0          = 25569;

  /* Number of seconds in a time_t day */
static const LONG      TIMET_ONE_DAY               = 86400;

#ifndef _WIN64
  /* VARIANT DATE of 2038-Jan-19 */
static const DATE      VARIANT_TIMET_MAX           = 50424.13480;
#else
  /* time_t of day 1, year 10,000 */
static const time_t    TIMET_VARIANT_OVERFLOW      = 253402300800;
#endif


/* ======================================================================== */
HRESULT ConvertFileTimeToVariantTime(FILETIME * pft, DATE * pDate)
{
	ULONGLONG ftScalar;

	if (!pft || !pDate) return E_INVALIDARG;

	ftScalar = *((ULONGLONG *) pft) + 500; /* Add 500 to counter double bit errors */

	if (ftScalar >= FILE_TIME_VARIANT_OVERFLOW) return E_INVALIDARG;   /* Date is too late for a variant */
	*pDate = (LONGLONG) (ftScalar - FILE_TIME_VARIANT_DAY0) / (double) FILE_TIME_ONE_DAY;
	if (*pDate < 0) *pDate = floor(*pDate) + (floor(*pDate) - *pDate); /* Fix negative dates */

	return NOERROR;
}


/* ======================================================================== */
HRESULT ConvertVariantTimeToFileTime(DATE date, FILETIME * pft)
{
	ULONGLONG ftScalar;

	if (!pft) return E_INVALIDARG;

	if (date < 0) date = ceil(date) + (ceil(date) - date);  /* Fix negative dates */

	if (date < VARIANT_FILE_TIME_DAY0) return E_INVALIDARG; /* Date is too early for a FILETIME */
	ftScalar = (ULONGLONG) ((date * FILE_TIME_ONE_DAY) + FILE_TIME_VARIANT_DAY0);

	*pft = *((FILETIME *) &ftScalar);

	return NOERROR;
}


/* ======================================================================== */
HRESULT ConvertVariantTimeToSystemTime(DATE date, SYSTEMTIME * pSystemTime)
{
	HRESULT hr;
	FILETIME fileTime;

	if (!pSystemTime) return E_INVALIDARG;
	if (FAILED(hr = ConvertVariantTimeToFileTime(date, &fileTime))) return hr;
	return (FileTimeToSystemTime(&fileTime, pSystemTime) ? NOERROR : HRESULT_FROM_WIN32( GetLastError() ));
}


/* ======================================================================== */
HRESULT ConvertSystemTimeToVariantTime(SYSTEMTIME * pSystemTime, DATE * pDate)
{
	FILETIME fileTime;

	if (!pSystemTime || !pDate) return E_INVALIDARG;
	if (!SystemTimeToFileTime(pSystemTime, &fileTime)) return HRESULT_FROM_WIN32( GetLastError() );
	return ConvertFileTimeToVariantTime(&fileTime, pDate);
}


/* ======================================================================== */
HRESULT ConvertVariantTimeToTimeT(DATE date, time_t * pTimeT)
{
	struct tm * ptm;

	if (!pTimeT) return E_INVALIDARG;

	/* Check if date is in the range of a time_t */
#ifndef _WIN64
	if (date < VARIANT_TIMET_DAY0 || date > VARIANT_TIMET_MAX) return E_INVALIDARG;
#else
	if (date < VARIANT_TIMET_DAY0) return E_INVALIDARG;
#endif

	/* Convert variant DATE to 'local' time_t */
	*pTimeT = (time_t) (((date - VARIANT_TIMET_DAY0) * TIMET_ONE_DAY) + 0.5);

	/* Now convert 'local' time_t to normal gmt time_t */
	if ( (ptm = gmtime(pTimeT)) == NULL || !(ptm->tm_isdst = -1) ||
	     (*pTimeT = mktime(ptm)) == (time_t) -1 ) return E_FAIL;

	return NOERROR;
}


/* ======================================================================== */
HRESULT ConvertTimeTToVariantTime(time_t timeT, DATE * pDate)
{
	struct tm localtm, utctm, * ptm;
	time_t timeTLocal, timeTUtc;

	if (!pDate) return E_INVALIDARG;

	/* Convert timeT to local time and gmt so we can get the difference */
	if ( (ptm = localtime(&timeT)) == NULL) return E_FAIL;
	localtm = *ptm;

	if ( (ptm = gmtime(&timeT)) == NULL) return E_FAIL;
	utctm = *ptm;

	localtm.tm_isdst = 0;
	utctm.tm_isdst   = 0;

	/* Convert tm structs to time_ts so we can calculate difference */
	if ( (timeTLocal = mktime(&localtm)) == (time_t) -1 ||
	     (timeTUtc   = mktime(&utctm))   == (time_t) -1) return E_FAIL;

	/* Convert timeT to 'local' time_t by adding local offset */
	timeT += timeTLocal - timeTUtc;

#ifdef _WIN64
	if (timeT >= TIMET_VARIANT_OVERFLOW) return E_INVALIDARG;   /* Too late for a variant DATE */
#endif
	*pDate = (DATE)  (timeT / (double) TIMET_ONE_DAY) + VARIANT_TIMET_DAY0;

	return NOERROR;
}


/* ======================================================================== */
HRESULT ConvertAnsiStrToBStr(LPCSTR szAnsiIn, BSTR * lpBstrOut)
{
	DWORD dwSize;

	if (lpBstrOut == NULL) return E_INVALIDARG;
	if (szAnsiIn == NULL) { *lpBstrOut = NULL; return NOERROR; }

	/* Get the number of unicode characters needed to convert szAnsiIn */
	dwSize = MultiByteToWideChar(CP_ACP, 0, szAnsiIn, -1, NULL, 0);
	if (dwSize == 0) return HRESULT_FROM_WIN32( GetLastError() );

	/* Allocate a BSTR of the required length.
	 * Note we minus one as dwSize includes space for the null terminator
	 * while SysAllocStringLen adds space for the null terminator */
	*lpBstrOut = SysAllocStringLen(NULL, dwSize - 1);
	if (*lpBstrOut == NULL) return E_OUTOFMEMORY;

	/* Convert szAnsiIn into the BSTR we allocated */
	if ( !MultiByteToWideChar(CP_ACP, 0, szAnsiIn, -1, *lpBstrOut, dwSize) )
	{
		SysFreeString(*lpBstrOut);
		return HRESULT_FROM_WIN32( GetLastError() );
	}

	return NOERROR;
}


/* ======================================================================== */
HRESULT ConvertBStrToAnsiStr(BSTR bstrIn, LPSTR * lpszOut)
{
	DWORD dwSize;

	if (lpszOut == NULL) return E_INVALIDARG;
	if (bstrIn == NULL) { *lpszOut = NULL; return NOERROR; }

	/* Get the number of characters needed to convert bStrIn */
	dwSize = WideCharToMultiByte(CP_ACP, 0, bstrIn, -1, NULL, 0, NULL, NULL);
	if (dwSize == 0) return HRESULT_FROM_WIN32( GetLastError() );

	/* Allocate memory of the required length */
	*lpszOut = (LPSTR) SysAllocStringByteLen(NULL, dwSize - 1);
	if (*lpszOut == NULL) return E_OUTOFMEMORY;

	/* Convert bstrIn into the memory we allocated */
	if ( !WideCharToMultiByte(CP_ACP, 0, bstrIn, -1, *lpszOut, dwSize, NULL, NULL) )
	{
		SysFreeString((BSTR) *lpszOut);
		return HRESULT_FROM_WIN32( GetLastError() );
	}

	return NOERROR;
}
