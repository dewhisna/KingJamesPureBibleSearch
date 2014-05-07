/* This file contains sample code that demonstrates use of the DispHelper COM helper library.
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
 *
 * You can browse the source code and samples, including a C++ version of this sample, online at:
 * http://cvs.sourceforge.net/viewcvs.py/disphelper/disphelper/
 */


/* NEWS FLASH: Microsoft is retiring the MSSoap toolkit in favor of .NET.
 * Please consider using PocketSoap instead.
 *
 * Note: If you get an error "Loading of the WSDL file failed, ..." when 
 * running these samples please download the WSDL file and update the path
 * to the local copy in the call to mssoapinit(). There seems to be an error
 * in the MSXML http client that sometimes causes this error.
 */


/* --
soap.c:
  Demonstrates using the MSSoap toolkit to utilise several web services.
Web services demonstrated include Google search, spell checker and cache viewer.

  The population web service sample demonstrates how to deal with a returned
safe array.

IMPORTANT NOTE - READ FIRST:
  To limit excessive use, Google requires that those who wish to use the Google Web
service acquire a unique license key (which is free to obtain). This license key
is used to limit individuals to no more than 1,000 calls to the Google Web
service per day.

  I have left the license key in below. You must ONLY use this license key to run
these samples. If you wish to develop these samples or any other code that uses the 
Google web service you MUST register to obtain your own key.

  If others have failed to do this or stolen the key, the provided key may no
longer work and you will have to acquire your own key to run these samples.

  PLEASE only use the key to run these samples a couple of times. If you need to run
the samples more please acquire your own key.

  The Google web api home page, where you can acquire a key, is at
http://www.google.com/apis/

MSSOAP NOTES:
  These samples use the high-level functionality of the MSSoap toolkit. This is a
three step process.
  1. Create the MSSoap.SoapClient object.
  2. Load the WSDL file for the web service. WSDL stands for web services
description language and describes the methods and types exposed by the web service.
  3. Call the web service methods which have been added to the COM object in step 2.

  Generally you should download the wsdl file to disk and load it from there for
greater speed and efficiency. If you are using multiple calls to a web service you
should only load the WSDL file once.

  A large listing of web services can be found at http://www.xmethods.com/

  If you require the MSSoap toolkit search for 'soap' at the Windows Download Centre or try
http://www.microsoft.com/downloads/details.aspx?familyid=c943c0dd-ceec-4088-9753-86f052ec8450
 -- */


#include "disphelper.h"
#include <stdio.h>
#include <wchar.h>
#include <tchar.h>

/* DO NOT USE THIS KEY EXCEPT TO RUN THESE SAMPLES A LIMITED NUMBER OF TIMES */
const WCHAR * szGoogleKey = L"pre9s/xQFHKRSutekE8K5bS7p0nD/+T4";

#define HR_TRY(func) if (FAILED(func)) { printf("\n## Fatal error on line %d.\n", __LINE__); goto cleanup; }
#define HR_REPORT(func) if (FAILED(func)) { printf("\n## Non-fatal error on line %d.\n", __LINE__); }


/* **************************************************************************
 * GoogleSpellChecker:
 *   Demonstrates using Google's spell checking service.
 *
 ============================================================================ */
void GoogleSpellChecker(LPCTSTR szWords)
{
	DISPATCH_OBJ(SoapClient);
	LPTSTR szSuggestions = NULL;

	_tprintf(TEXT("Checking spelling of '%s'\n"), szWords);

	if (SUCCEEDED(dhCreateObject(L"MSSoap.SoapClient", NULL, &SoapClient)) &&
	    SUCCEEDED(dhCallMethod(SoapClient, L".mssoapinit(%S)", L"http://api.google.com/GoogleSearch.wsdl")) &&
	    SUCCEEDED(dhGetValue(L"%T", &szSuggestions, SoapClient, L".doSpellingSuggestion(%S, %T)", szGoogleKey , szWords)))
	{
		_tprintf(TEXT("Suggestion(s): %s\n"), szSuggestions);
		dhFreeString(szSuggestions);
	}

	SAFE_RELEASE(SoapClient);
}


/* **************************************************************************
 * StockQuote:
 *   Demonstrates using XMethod's delayed stock quote service to receive
 * a stock price (delayed by 20 minutes). Note: Uses port 9090.
 * See http://services.xmethods.net/ve2/ViewListing.po;?key=uuid:889A05A5-5C03-AD9B-D456-0E54A527EDEE
 *
 ============================================================================ */
void StockQuote(LPCTSTR szStockSymbol)
{
	DISPATCH_OBJ(SoapClient);
	double dblStockPrice;

	if (SUCCEEDED(dhCreateObject(L"MSSoap.SoapClient", NULL, &SoapClient)) &&
	    SUCCEEDED(dhCallMethod(SoapClient, L".mssoapinit(%S)", L"http://services.xmethods.net/soap/urn:xmethods-delayed-quotes.wsdl")) &&
	    SUCCEEDED(dhGetValue(L"%e", &dblStockPrice, SoapClient, L".getQuote(%T)", szStockSymbol)))
	{
		_tprintf(TEXT("Stock Price(%s): $%.2f\n"), szStockSymbol, dblStockPrice);
	}

	SAFE_RELEASE(SoapClient);
}


/* **************************************************************************
 * GoogleCacheViewer:
 *   Demonstrates using the Google web api to get the cached copy of a page
 * and display it in Internet Explorer.
 *
 ============================================================================ */
void GoogleCacheViewer(LPCTSTR szWords)
{
	DISPATCH_OBJ(SoapClient);
	BSTR bstrCache;

	if (SUCCEEDED(dhCreateObject(L"MSSoap.SoapClient", NULL, &SoapClient)) &&
	    SUCCEEDED(dhCallMethod(SoapClient, L".mssoapinit(%S)", L"http://api.google.com/GoogleSearch.wsdl")) &&
	    SUCCEEDED(dhGetValue(L"%B", &bstrCache, SoapClient, L".doGetCachedPage(%S, %T)", szGoogleKey , szWords)))
	{
		DISPATCH_OBJ(ieApp);

		if (SUCCEEDED(dhCreateObject(L"InternetExplorer.Application", NULL, &ieApp)))
		{
			BOOL bIsBusy;

			dhPutValue(ieApp, L".Visible = %b", TRUE);
			dhCallMethod(ieApp, L".Navigate(%S)", L"about:blank");

			/* Wait for blank page to load */
			while (SUCCEEDED(dhGetValue(L"%b", &bIsBusy, ieApp, L".Busy")) && bIsBusy) Sleep(150);

			/* Note that the text in bstrCache is in UTF8 format.
			 * We should convert it to unicode and pass that in. However, for
			 * simplicity we will treat it as ansi(%s). This will produce incorrect
			 * results for pages that contain non-ascii characters. */
			dhCallMethod(ieApp, L".Document.Write(%s)", (LPSTR) bstrCache);

			SAFE_RELEASE(ieApp);
		}

		dhFreeString(bstrCache);
	}

	SAFE_RELEASE(SoapClient);
}


/* **************************************************************************
 * Population:
 *   Demonstrates using the population web service provided by AbudantTech.
 * See http://www.xmethods.net/ve2/ViewListing.po;?key=uuid:16C429C7-5724-8335-4ABB-B459F7C55371
 *
 *   This demonstrates handling the return of more complex data. If szCountry
 * is NULL we request a list of supported countries using getCountries(). This
 * method returns an array of strings. MSSoap handles this by returning a safe
 * array of BSTRs in the returned VARIANT which we then print out.
 *
 *   If szCountry is not NULL we call getPopulation(szCountry). The return
 * value of this method is a complex type containing three members(similar to a
 * C struct). MSSoap handles this by returning an IXMLDOMNodeList COM object.
 * In this sample we enumerate the nodes(which are IXMLDOMNode objects)
 * of the list and print out their name/value pairs.
 *
 *   For more information see the documentation for safe array and the
 * IXMLDOMNodeList interface in MSDN.
 *
 ============================================================================ */
void Population(LPCTSTR szCountry)
{
	DISPATCH_OBJ(SoapClient);
	DISPATCH_OBJ(xmlNodeList);
	VARIANT vtCountries;

	if (FAILED(dhCreateObject(L"MSSoap.SoapClient", NULL, &SoapClient)) ||
	    FAILED(dhCallMethod(SoapClient, L".mssoapinit(%S)", L"http://www.abundanttech.com/webservices/population/population.wsdl")))
	{
		SAFE_RELEASE(SoapClient);
		return;
	}

	if (!szCountry && SUCCEEDED(dhGetValue(L"%v", &vtCountries, SoapClient, L".getCountries()")))
	{
		if (vtCountries.vt == (VT_ARRAY | VT_BSTR))
		{
			BSTR * pbstr;
			UINT i;

			_tprintf(TEXT("Listing supported countries...\n\n"));

			if (SUCCEEDED(SafeArrayAccessData(vtCountries.parray, (void **) &pbstr)))
			{
				for (i=0;i < vtCountries.parray->rgsabound[0].cElements;i++)
				{
					wprintf(L"%s\n", pbstr[i]);
				}

				SafeArrayUnaccessData(vtCountries.parray);
			}
		}
		else
		{
			_tprintf(TEXT("## Unexpected data type returned!"));
		}

		VariantClear(&vtCountries);
	}
	else if (szCountry && SUCCEEDED(dhGetValue(L"%o", &xmlNodeList, SoapClient, L".getPopulation(%T)", szCountry)))
	{
		FOR_EACH(xmlNode, xmlNodeList, NULL)
		{
			LPTSTR szNodeName, szNodeText;

			if (SUCCEEDED(dhGetValue(L"%T", &szNodeName, xmlNode, L".nodeName")))
			{
				_tprintf(TEXT("%s: "), szNodeName);
				dhFreeString(szNodeName);
			}

			if (SUCCEEDED(dhGetValue(L"%T", &szNodeText, xmlNode, L".text")))
			{
				_tprintf(TEXT("%s\n"), szNodeText);
				dhFreeString(szNodeText);
			}

		} NEXT(xmlNode);

		SAFE_RELEASE(xmlNodeList);
	}

	SAFE_RELEASE(SoapClient);
}


/* **************************************************************************
 * GoogleSearch:
 *   Demonstrates using the Google web api to perform a web search. This sample
 * dumps the result to a file, 'search.htm' in basic html format.
 *
 *   The result of doGoogleSearch is returned as a IXMLDOMNodeList. We use some
 * trickery to get the result as an IXMLDOMNode. This allows us to use the 
 * selectSingleNode to get members by name.
 *
 *   We then enumerate all the child nodes of the resultElements node
 * which are the individual search results and and print out their contents.
 *
 *   It should be noted that while I have elected to get members by name it is
 * far more efficient(although not as robust) to get them by index. For example
 * you can get the documentFiltering member with:
 * dhGetValue(L"%b", &bDocumentFiltering, xmlNodeList, L".item(%d).text", 1);
 * See http://www.codeproject.com/asp/googleapisinasp.asp
 *
 *   This is a sample only, if you simply wish to convert the results
 * to a html page or other text format it is far cleaner and more efficient to
 * use an XSL transform. Please see http://momche.net/res/google/ for a
 * sample.
 *
 ============================================================================ */
void GoogleSearch(LPCTSTR szQuery, UINT iFirstResult)
{
	DISPATCH_OBJ(SoapClient);
	DISPATCH_OBJ(xmlNodeList);
	DISPATCH_OBJ(xmlParentNode);
	FILE * hFile = NULL;

	struct { /* Structure to hold the returned search meta information */
		BOOL bDocumentFiltering, bEstimateIsExact;
		int cEstTotalResults, nEndIndex, nStartIndex;
		double dblSearchTime;
		LPSTR szSearchTips, szSearchComments, szSearchQuery;
	} GoogleSearch = { 0 };

	struct { /* Structure to hold an individual search result */
		LPSTR szSnippet, szHostName, szURL, szCachedSize;
		LPSTR szTitle, szSummary, szDirectoryTitle;
		BOOL bRelatedInformation;
	} SearchResult;

	/* Open output file */
	if (!(hFile = fopen("search.htm", "w")))
	{
		MessageBox(NULL, TEXT("Failed to open output file for search results."), NULL, MB_ICONSTOP);
		goto cleanup;
	}

	HR_TRY( dhCreateObject(L"MSSoap.SoapClient", NULL, &SoapClient) );
	HR_TRY( dhCallMethod(SoapClient, L".mssoapinit(%S)", L"http://api.google.com/GoogleSearch.wsdl") );

	/* Do the search and get the result as an IXMLDOMNodeList */
	HR_TRY( dhGetValue(L"%o", &xmlNodeList, SoapClient, L".doGoogleSearch(%S, %T, %d, %d, %b, %S, %b, %S, %S, %S)",
			szGoogleKey, szQuery, iFirstResult, 10, TRUE, L"", FALSE, L"", L"", L"") );

	/* Turn the IXMLDOMNodeList into an IXMLDOMNode so we can use selectSingleNode */
	HR_TRY( dhGetValue(L"%o", &xmlParentNode, xmlNodeList, L".item(%d).parentNode", 0) );

	/* Now get the returned information. */
	HR_REPORT( dhGetValue(L"%b", &GoogleSearch.bDocumentFiltering, xmlParentNode, L".selectSingleNode(%S).text", L"documentFiltering") );
	HR_REPORT( dhGetValue(L"%b", &GoogleSearch.bEstimateIsExact,   xmlParentNode, L".selectSingleNode(%S).text", L"estimateIsExact") );
	HR_REPORT( dhGetValue(L"%d", &GoogleSearch.nEndIndex,          xmlParentNode, L".selectSingleNode(%S).text", L"endIndex") );
	HR_REPORT( dhGetValue(L"%d", &GoogleSearch.nStartIndex,        xmlParentNode, L".selectSingleNode(%S).text", L"startIndex") );
	HR_REPORT( dhGetValue(L"%e", &GoogleSearch.dblSearchTime,      xmlParentNode, L".selectSingleNode(%S).text", L"searchTime") );
	HR_REPORT( dhGetValue(L"%s", &GoogleSearch.szSearchTips,       xmlParentNode, L".selectSingleNode(%S).text", L"searchTips") );
	HR_REPORT( dhGetValue(L"%s", &GoogleSearch.szSearchComments,   xmlParentNode, L".selectSingleNode(%S).text", L"searchComments") );
	HR_REPORT( dhGetValue(L"%s", &GoogleSearch.szSearchQuery,      xmlParentNode, L".selectSingleNode(%S).text", L"searchQuery") );
	HR_REPORT( dhGetValue(L"%d", &GoogleSearch.cEstTotalResults,   xmlParentNode, L".selectSingleNode(%S).text", L"estimatedTotalResultsCount") );

	fprintf(hFile, "<html><body>\n");
	fprintf(hFile, "Searched the web for <u>%s</u>.<br />\n"
	               "Results %d to %d of %s %d.<br />\n"
	               "Search took %.3f seconds.<br />\n"
	               "Results are %sfiltered.<br />\n"
	               "Search Tips: %s<br />\n"
	               "Search Comments: %s<br />\n",
	     GoogleSearch.szSearchQuery,
	     GoogleSearch.nStartIndex, GoogleSearch.nEndIndex, GoogleSearch.bEstimateIsExact ? "exactly" : "about", GoogleSearch.cEstTotalResults,
	     GoogleSearch.dblSearchTime,
	     GoogleSearch.bDocumentFiltering ? "" : "not ",
	     GoogleSearch.szSearchTips, GoogleSearch.szSearchComments);

	dhFreeString(GoogleSearch.szSearchTips);
	dhFreeString(GoogleSearch.szSearchComments);
	dhFreeString(GoogleSearch.szSearchQuery);

	/* Enumerate the childNodes of the resultElements node which are the individual results */
	FOR_EACH1(xmlResult, xmlParentNode, L".selectSingleNode(%S).childNodes", L"resultElements")
	{
		int nNodeType;

		if (SUCCEEDED(dhGetValue(L"%d", &nNodeType, xmlResult, L".nodeType")) && nNodeType == 1) /* NODE_ELEMENT - Ignore comment and text nodes. */
		{
			ZeroMemory(&SearchResult, sizeof(SearchResult));

			HR_REPORT( dhGetValue(L"%s", &SearchResult.szCachedSize,        xmlResult, L".selectSingleNode(%S).text", L"cachedSize") );
			HR_REPORT( dhGetValue(L"%s", &SearchResult.szSnippet,           xmlResult, L".selectSingleNode(%S).text", L"snippet") );
			HR_REPORT( dhGetValue(L"%s", &SearchResult.szHostName,          xmlResult, L".selectSingleNode(%S).text", L"hostName") );
			HR_REPORT( dhGetValue(L"%s", &SearchResult.szURL,               xmlResult, L".selectSingleNode(%S).text", L"URL") );
			HR_REPORT( dhGetValue(L"%s", &SearchResult.szTitle,             xmlResult, L".selectSingleNode(%S).text", L"title") );
			HR_REPORT( dhGetValue(L"%s", &SearchResult.szSummary,           xmlResult, L".selectSingleNode(%S).text", L"summary") );
			HR_REPORT( dhGetValue(L"%s", &SearchResult.szDirectoryTitle,    xmlResult, L".selectSingleNode(%S).text", L"directoryTitle") );
			HR_REPORT( dhGetValue(L"%b", &SearchResult.bRelatedInformation, xmlResult, L".selectSingleNode(%S).text", L"relatedInformationPresent") );

			fprintf(hFile, "\n\n<br /><b>Title:</b> %s<br />\n"
			               "<b>Description:</b> %s<br />\n"
			               "<b>Snippet:</b> %s<br />\n"
			               "<b>Directory Title:</b> %s<br />\n"
			               "<b>Host Name:</b> %s<br />\n"
			               "<b>Similar Pages Available:</b> %s<br />\n"
			               "<a href=\"%s\">%s</a> - %s<br />\n",
			    SearchResult.szTitle, SearchResult.szSummary, SearchResult.szSnippet,
			    SearchResult.szDirectoryTitle, SearchResult.szHostName,
			    SearchResult.bRelatedInformation ? "True" : "False",
			    SearchResult.szURL, SearchResult.szURL, SearchResult.szCachedSize);

			dhFreeString(SearchResult.szCachedSize);
			dhFreeString(SearchResult.szSnippet);
			dhFreeString(SearchResult.szHostName);
			dhFreeString(SearchResult.szURL);
			dhFreeString(SearchResult.szTitle);
			dhFreeString(SearchResult.szSummary);
			dhFreeString(SearchResult.szDirectoryTitle);
		}

	} NEXT(xmlResult);

cleanup:
	if (hFile)
	{
		fprintf(hFile, "</body></html>");
		fclose(hFile);
	}

	SAFE_RELEASE(xmlParentNode);
	SAFE_RELEASE(xmlNodeList);
	SAFE_RELEASE(SoapClient);
}


/* ============================================================================ */
int main(void)
{
	dhInitialize(TRUE);
	dhToggleExceptions(TRUE);

	printf("Running Google Spell Checking sample...\n");
	GoogleSpellChecker(TEXT("Thiis is a teest of Googel spelll checking."));

	printf("\nRunning Stock Quote sample...\n");
	StockQuote(TEXT("YHOO"));
	StockQuote(TEXT("TIVO"));

	printf("\nRunning Google Cache Viewer sample...\n");
	GoogleCacheViewer(TEXT("http://sourceforge.net"));

	printf("\nRunning Population sample...\n");
	Population(TEXT("United States"));
	Population(TEXT("Western Sahara"));
	Population(NULL); /* List supported countries. */

	printf("\nRunning Google Search sample(output to search.htm)...\n");
	GoogleSearch(TEXT("test"), 500);

	printf("\nPress ENTER to exit...\n");
	getchar();

	dhUninitialize(TRUE);
	return 0;
}




