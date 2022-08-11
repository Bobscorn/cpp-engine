#include "DebugHelper.h"

#ifdef __linux__
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

constexpr SDL_MessageBoxColorScheme ReportBoxScheme{
		{ /* .colors (.r, .g, .b) */
			/* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
			{ 0xCF, 0xCF, 0xCF },
			/* [SDL_MESSAGEBOX_COLOR_TEXT] */
			{ 0, 0, 0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
			{ 0x10, 0x10, 0x10 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
			{ 0xDF, 0xDF, 0xDF },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
			{ 255, 0, 255 }
		}
};

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

void OutFunc(const Charry * a)
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
void ErrorExit(wchar_t *lpszFunction)
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

void Debug::ReportError(const Charry * title, const Charry * msg)
{
	if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, nullptr) < 0)
	{
		DERROR("SDL failed to show a *SIMPLE* message box with error: " + SDL_GetError());
		DERROR("This was the error to be shown: " + msg);
	}
}

bool Debug::ReportRetry(Charry const * title, Charry const *msg)
{
	const SDL_MessageBoxButtonData buttons[] = {
	{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, RetryID, "Retry" },
	{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, ExitID, "Exit" },
	};
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_INFORMATION, /* .flags */
		NULL, /* .window */
		title, /* .title */
		msg, /* .message */
		SDL_arraysize(buttons), /* .numbuttons */
		buttons, /* .buttons */
		&ReportBoxScheme /* .colorScheme */
	};
	int buttonid;
	if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
		DERROR("SDL failed to show message box with error: " + SDL_GetError());
		return false;
	}
	return buttonid == RetryID;
}

int Debug::ReportErrorCancelTryContinue(const Charry * title, const Charry * msg)
{
	const SDL_MessageBoxButtonData buttons[] = {
	   {										0, ContinueID, "Continue" },
	   {										0, RetryID, "Retry" },
	   { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, CancelID, "Cancel" },
	};
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_INFORMATION, /* .flags */
		NULL, /* .window */
		title, /* .title */
		msg, /* .message */
		SDL_arraysize(buttons), /* .numbuttons */
		buttons, /* .buttons */
		&ReportBoxScheme /* .colorScheme */
	};
	int buttonid;
	if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
		DERROR("SDL failed to show message box with error: " + SDL_GetError());
		return -1;
	}
	return buttonid;
}

int Debug::ReportErrorCancelTry(const Charry * title, const Charry * msg)
{
	const SDL_MessageBoxButtonData buttons[] = {
	   { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, RetryID, "Retry" },
	   { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, CancelID, "Cancel" },
	};
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_INFORMATION, /* .flags */
		NULL, /* .window */
		title, /* .title */
		msg, /* .message */
		SDL_arraysize(buttons), /* .numbuttons */
		buttons, /* .buttons */
		&ReportBoxScheme /* .colorScheme */
	};
	int buttonid;
	if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
		DERROR("SDL failed to show message box with error: " + SDL_GetError());
		return -1;
	}
	return buttonid;
}

int Debug::ReportErrorCancelContinue(const Charry * title, const Charry * msg)
{
	const SDL_MessageBoxButtonData buttons[] = {
	   { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, ContinueID, "Continue" },
	   { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, CancelID, "Cancel" },
	};
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_INFORMATION, /* .flags */
		NULL, /* .window */
		title, /* .title */
		msg, /* .message */
		SDL_arraysize(buttons), /* .numbuttons */
		buttons, /* .buttons */
		&ReportBoxScheme /* .colorScheme */
	};
	int buttonid;
	if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
		DERROR("SDL failed to show message box with error: " + SDL_GetError());
		return -1;
	}
	return buttonid;
}
