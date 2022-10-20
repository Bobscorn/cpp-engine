#include "DebugPrintHelper.h"



namespace DebugStrings
{
	Stringy LastErrorFile;
	Stringy LastErrorLine;
	Stringy LastError;
}

#ifdef WIN32
#include <Windows.h>
#include <comdef.h>

bool HRFunc(Stringy file, Stringy line, long hr)
{
	if (FAILED(hr))
	{
		_com_error error(hr);
		DebugStrings::LastError = error.ErrorMessage();
		DebugStrings::LastErrorFile = file;
		DebugStrings::LastErrorLine = line;
		Stringy errorText = Stringy(Stringy("In file ") + file + "(") + line + Stringy("):") + DebugStrings::LastError;
		MessageBoxA(NULL, errorText.c_str(), NULL, NULL);
		return true;
	}
	return false;
}

long HHRFunc(Stringy file, Stringy line, long hr)
{
	if (FAILED(hr))
	{
		_com_error error(hr);
		DebugStrings::LastError = error.ErrorMessage();
		DebugStrings::LastErrorFile = file;
		DebugStrings::LastErrorLine = line;
		Stringy errorText = Stringy(Stringy("In file ") + file + "(") + line + Stringy("):") + DebugStrings::LastError;
		MessageBoxA(NULL, errorText.c_str(), NULL, NULL);
		return hr;
	}
	return S_OK;
}
#endif //WIN32

#ifdef DEBUG_OUTPUT
#	if defined(DEBUG_OUTPUT_DEBUGSTRING)
#		include <Windows.h>
#	endif

#	include <string>

#	ifdef DEBUG_OUTPUT_COUT
#		include <iostream>
#	endif

void OutFunc(const Charry* a)
{
#	ifdef DEBUG_OUTPUT_DEBUGSTRING
	OutputDebugStringA(a);
#	endif
#	ifdef DEBUG_OUTPUT_COUT
#		ifdef _UNICODE
	std::wcout << a;
#		else
	std::cout << a;
#		endif
#	endif
#	ifdef DEBUG_OUTPUT_LOG
	logger << a;
#	endif
}


#if defined(_UNICODE) & defined(WIN32)
void ErrorExit(wchar_t* lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}
#endif //WIN32
#endif

