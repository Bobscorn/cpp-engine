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
