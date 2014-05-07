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
speech.c:
  Demonstrates using Microsoft Agent and SAPI to provide text-to-speech.
 -- */


#include "disphelper.h"
#include <stdio.h>


/* ============================================================================ */
static HRESULT Sapi5(IDispatch * tts, LPCTSTR szPhrase)
{
	dhCallMethod(tts, L".Speak(%S)", L"SAPI Version 5 was found on your computer.");

	/* Speak the phrase and the name with each available voice */
	FOR_EACH(spVoice, tts, L".GetVoices")
	{
		LPSTR szDescription;

		dhPutRef(tts, L".Voice = %o", spVoice);

		if (SUCCEEDED(dhGetValue(L"%s", &szDescription, spVoice, L".GetDescription")))
		{
			char szBuf[128];
			sprintf(szBuf, "My name is %.100s", szDescription);

			dhCallMethod(tts, L".Speak(%s)", szBuf);
			dhFreeString(szDescription);
		}

		dhCallMethod(tts, L".Speak(%T)", szPhrase);

	} NEXT(spVoice);

	return NOERROR;	
}


/* ============================================================================ */
static HRESULT Sapi4(IDispatch * tts, LPCTSTR szPhrase)
{
	BOOL bIsSpeaking;

	dhCallMethod(tts, L".Register(%S, %S)", L"", L"DispHelper Sample Application");
	dhPutValue(tts, L".Enabled = %b", TRUE);

	dhCallMethod(tts, L".Speak(%S, %d)", L"SAPI Version 4 was found on your computer.", 1);
	
	/* Pause while it is speaking the version */
	while (SUCCEEDED(dhGetValue(L"%b", &bIsSpeaking, tts, L".IsSpeaking")) && bIsSpeaking) Sleep(150);

	return dhCallMethod(tts, L".Speak(%T, %d)", szPhrase, 1);
}


/* **************************************************************************
 * Speak:
 *   Demonstrates using SAPI for text to speech. This sample attempts to use Sapi 5.
 * If not available it reverts to using Sapi 4.
 *
 ============================================================================ */
HRESULT Speak(LPCTSTR szPhrase)
{
	DISPATCH_OBJ(tts);
	HRESULT hr;

	dhToggleExceptions(FALSE);

		/* Attempt to use Sapi 5 */
		hr = dhCreateObject(L"Sapi.SpVoice", NULL, &tts);

	dhToggleExceptions(TRUE);

	if (SUCCEEDED(hr))
	{
		/* Sapi 5 is available */
		hr = Sapi5(tts, szPhrase);
	}
	else
	{
		/* No Sapi 5 - Try Sapi 4 */
		hr = dhCreateObject(L"Speech.VoiceText", NULL, &tts);
		if (SUCCEEDED(hr)) hr = Sapi4(tts, szPhrase);
	}

	SAFE_RELEASE(tts);

	return hr;
}


/* **************************************************************************
 * Agent:
 *   Demonstrates using the MSAgent. Note: MSAgent's text-to-speech does not work
 * on XP out of the box. It required downloads and registry edits.
 * See 'http://www.microsoft.com/msagent/support/user/tts.asp' under Windows XP or 
 * do a search on "Microsoft Agent uses SAPI 4.0 to provide speech "
 *
 ============================================================================ */
int Agent(LPCTSTR szPhrase)
{
	DISPATCH_OBJ(agent);
	DISPATCH_OBJ(genie);

	dhCreateObject(L"Agent.Control.1", NULL, &agent);

	dhPutValue(agent, L".Connected = %b", TRUE);
	dhCallMethod(agent, L".Characters.Load(%S)", L"Genie");

	dhGetValue(L"%o", &genie, agent, L".Characters(%S)", L"Genie");
	dhCallMethod(genie, L".Show");

	dhCallMethod(genie, L".Play(%S)", L"Greet");

	dhCallMethod(genie, L".Speak(%T)", szPhrase);

	dhCallMethod(genie, L".Speak(%S)", L"Ahh, that bottle was small.");

	SAFE_RELEASE(genie);
	/* SAFE_RELEASE(agent); */ /* We leave the agent alive */

	return 0;
}


/* ============================================================================ */
int main(void)
{
	dhInitialize(TRUE);
	dhToggleExceptions(TRUE);

	printf("Running MS Agent sample...\n");
	Agent(TEXT("Hello, how are you today? I am the Microsoft Agent."));

	printf("\nPress ENTER to run SAPI Text to Speech sample...");
	getchar();
	Speak(TEXT("Hello, I am a sample of text to speech. Is it lunch time yet? Please don't throw me out the window!"));

	dhUninitialize(TRUE);
	return 0;
}




