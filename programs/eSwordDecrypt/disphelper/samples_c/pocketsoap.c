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


/* --
pocketsoap.c:
  Demonstrates using the PocketSoap toolkit to utilise several web services.
Play the 'Who wants to be a millionaire' quiz with the help of a web service.
  PocketSoap is available from http://www.pocketsoap.com/
 -- */


#include "disphelper.h"
#include <stdio.h>
#include <wchar.h>
#include <tchar.h>

#define HR_TRY(func) if (FAILED(func)) { printf("\n## Fatal error on line %d.\n", __LINE__); goto cleanup; }


/* **************************************************************************
 * Temperature:
 *   Demonstrates using XMethod's Temperature service to get the temperature for
 * a specific US zip code.
 * http://xmethods.net/ve2/ViewListing.po;?key=uuid:477CEED8-1EDD-89FA-1070-6C2DBE1685F8
 *
 ============================================================================ */
void Temperature(LPCTSTR szZipCode)
{
	DISPATCH_OBJ(psEnvelope);
	DISPATCH_OBJ(psTransport);
	LPWSTR szEnvelope =  NULL;
	double dblTemperature;

	/* Create a PocketSoap envelope */
	HR_TRY( dhCreateObject(L"PocketSOAP.Envelope.2", NULL, &psEnvelope) );

	/* Set the methodName and namespace URI */
	HR_TRY( dhCallMethod(psEnvelope, L".SetMethod(%S, %S)",
	                          L"getTemp", L"urn:xmethods-Temperature") );

	/* Create a new parameter */
	HR_TRY( dhCallMethod(psEnvelope, L".Parameters.Create(%S, %T)", L"zipcode", szZipCode) );

	/* Create and set up the transport object */
	HR_TRY( dhCreateObject(L"PocketSoap.HTTPTransport.2", NULL, &psTransport) );
	HR_TRY( dhPutValue(psTransport, L".SOAPAction = %S", L"") );

	/* Serialize the envelope into an xml string and send it to the end point */
	HR_TRY( dhGetValue(L"%S", &szEnvelope, psEnvelope, L".serialize") );
	HR_TRY( dhCallMethod(psTransport, L".Send(%S, %S)",
	               L"http://services.xmethods.net:80/soap/servlet/rpcrouter", szEnvelope) );

	/* Parse the returned xml back into the COM object model */
	HR_TRY( dhCallMethod(psEnvelope, L".Parse(%o)", psTransport) );

	/* Extract the return result */
	HR_TRY( dhGetValue(L"%e", &dblTemperature, psEnvelope, L".Parameters.Item(%d).Value", 0) );

	_tprintf(TEXT("The temperature for zipcode %s is %.2f degrees fahrenheit.\n"), szZipCode, dblTemperature);

cleanup:
	dhFreeString(szEnvelope);
	SAFE_RELEASE(psEnvelope);
	SAFE_RELEASE(psTransport);
}


/* **************************************************************************
 * ExchangeRate:
 *   Demonstrates using XMethod's Exchange Rate service to get the exchange
 * rate between the currencies of two countries.
 * http://xmethods.net/ve2/ViewListing.po;?key=uuid:D784C184-99B2-DA25-ED45-3665D11A12E5
 *
 ============================================================================ */
void ExchangeRate(LPCTSTR szCountry1, LPCTSTR szCountry2)
{
	DISPATCH_OBJ(psEnvelope);
	DISPATCH_OBJ(psTransport);
	LPWSTR szEnvelope =  NULL;
	double dblRate;

	HR_TRY( dhCreateObject(L"PocketSOAP.Envelope.2", NULL, &psEnvelope) );
	HR_TRY( dhCallMethod(psEnvelope, L".SetMethod(%S, %S)",
	                          L"getRate", L"urn:xmethods-CurrencyExchange") );

	HR_TRY( dhCallMethod(psEnvelope, L".Parameters.Create(%S, %T)", L"country1", szCountry1) );
	HR_TRY( dhCallMethod(psEnvelope, L".Parameters.Create(%S, %T)", L"country2", szCountry2) );

	HR_TRY( dhCreateObject(L"PocketSoap.HTTPTransport.2", NULL, &psTransport) );
	HR_TRY( dhPutValue(psTransport, L".SOAPAction = %S", L"") );

	HR_TRY( dhGetValue(L"%S", &szEnvelope, psEnvelope, L".serialize") );
	HR_TRY( dhCallMethod(psTransport, L".Send(%S, %S)",
	               L"http://services.xmethods.net:80/soap", szEnvelope) );

	HR_TRY( dhCallMethod(psEnvelope, L".Parse(%o)", psTransport) );

	HR_TRY( dhGetValue(L"%e", &dblRate, psEnvelope, L".Parameters.Item(%d).Value", 0) );

	_tprintf(TEXT("The exchange rate between %s and %s is %.2f.\n"), szCountry1, szCountry2, dblRate);

cleanup:
	dhFreeString(szEnvelope);
	SAFE_RELEASE(psEnvelope);
	SAFE_RELEASE(psTransport);
}


/* **************************************************************************
 * Millionaire:
 *   Demonstrates using Konrad Wulf's Millionaire Quiz service.
 * http://xmethods.net/ve2/ViewListing.po?key=uuid:0D5D2C03-91DD-EFEA-4167-3554ED3F6FDD
 *
 * This sample first gets the question with alternate answers and then when
 * the user is ready uses a different method to get the correct answer.
 * For added realism we should add 'Sleep(240000)' between question and answer.
 *
 ============================================================================ */
void Millionaire(void)
{
	DISPATCH_OBJ(psEnvelope);
	DISPATCH_OBJ(psTransport);
	LPWSTR szEnvelope =  NULL;
	struct MillionaireQuestion {
		LPWSTR szQuestion, szAnswerA, szAnswerB, szAnswerC, szAnswerD, szCorrectAnswer;
		int id, level;
	} Question = { 0 };

	HR_TRY( dhCreateObject(L"PocketSOAP.Envelope.2", NULL, &psEnvelope) );
	HR_TRY( dhCallMethod(psEnvelope, L".SetMethod(%S, %S)",
	                          L"getRandomQuestion", L"http://tempuri.org/QuizService") );

	HR_TRY( dhCreateObject(L"PocketSoap.HTTPTransport.2", NULL, &psTransport) );
	HR_TRY( dhPutValue(psTransport, L".SOAPAction = %S", L"") );

	HR_TRY( dhGetValue(L"%S", &szEnvelope, psEnvelope, L".serialize") );
	HR_TRY( dhCallMethod(psTransport, L".Send(%S, %S)",
	               L"http://java.rus.uni-stuttgart.de:80/quiz/quiz", szEnvelope) );

	SAFE_FREE_STRING(szEnvelope);

	HR_TRY( dhCallMethod(psEnvelope, L".Parse(%o)", psTransport) );

	WITH1(psNodes, psEnvelope, L".Parameters.item(%d).Nodes", 0)
	{
		dhGetValue(L"%S", &Question.szQuestion, psNodes, L".ItemByName(%S).Value", L"question");
		dhGetValue(L"%S", &Question.szAnswerA,  psNodes, L".ItemByName(%S).Value", L"answerA");
		dhGetValue(L"%S", &Question.szAnswerB,  psNodes, L".ItemByName(%S).Value", L"answerB");
		dhGetValue(L"%S", &Question.szAnswerC,  psNodes, L".ItemByName(%S).Value", L"answerC");
		dhGetValue(L"%S", &Question.szAnswerD,  psNodes, L".ItemByName(%S).Value", L"answerD");
		dhGetValue(L"%d", &Question.id,         psNodes, L".ItemByName(%S).Value", L"id");
		dhGetValue(L"%d", &Question.level,      psNodes, L".ItemByName(%S).Value", L"difficultyLevel");

	} END_WITH(psNodes);

	wprintf(L"Question No. %d - $%d\n%s\nA: %s\nB: %s\nC: %s\nD: %s\n\n",
	          Question.id, Question.level, Question.szQuestion,
	          Question.szAnswerA, Question.szAnswerB, Question.szAnswerC, Question.szAnswerD);

	dhFreeString(Question.szQuestion);
	dhFreeString(Question.szAnswerA);
	dhFreeString(Question.szAnswerB);
	dhFreeString(Question.szAnswerC);
	dhFreeString(Question.szAnswerD);

	/* Now get the answer */
	printf("Press ENTER to get the answer...\n");
	getchar();

	HR_TRY( dhCallMethod(psEnvelope, L".Parameters.Clear") );

	HR_TRY( dhPutValue(psEnvelope, L".MethodName = %S", L"getCorrectAnswerForQuestionById") );
	HR_TRY( dhCallMethod(psEnvelope, L".Parameters.Create(%S, %d)", L"id", Question.id) );

	HR_TRY( dhGetValue(L"%S", &szEnvelope, psEnvelope, L".serialize") );
	HR_TRY( dhCallMethod(psTransport, L".Send(%S, %S)",
	               L"http://java.rus.uni-stuttgart.de:80/quiz/quiz", szEnvelope) );

	HR_TRY( dhCallMethod(psEnvelope, L".Parse(%o)", psTransport) );

	HR_TRY( dhGetValue(L"%S", &Question.szCorrectAnswer, psEnvelope, L".Parameters.Item(%d).Value", 0) );

	wprintf(L"The answer is %s.\n", Question.szCorrectAnswer);

	dhFreeString(Question.szCorrectAnswer);

cleanup:
	dhFreeString(szEnvelope);
	SAFE_RELEASE(psEnvelope);
	SAFE_RELEASE(psTransport);
}


/* ============================================================================ */
int main(void)
{
	dhInitialize(TRUE);
	dhToggleExceptions(TRUE);

	printf("\nRunning Temperature sample...\n\n");
	Temperature(TEXT("94107"));

	printf("\nRunning ExchangeRate sample...\n\n");
	ExchangeRate(TEXT("Fiji"), TEXT("Egypt"));

	printf("\nRunning Millionaire sample...\n\n");
	Millionaire();

	printf("\nPress ENTER to exit...\n");
	getchar();

	dhUninitialize(TRUE);
	return 0;
}




