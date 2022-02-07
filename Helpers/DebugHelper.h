#pragma once



// Debug Macros/Function vv
// Debug Output vvvv

#include <string>

#ifdef WIDE_CHARS
typedef std::wstring Stringy;
typedef wchar_t Charry;
#define FILE_THING __FILEW__
#define LINE_THING __LINE__
#else
typedef std::string Stringy;
typedef char Charry;
#define FILE_THING __FILE__
#define LINE_THING __LINE__
#endif //_UNICODE

#if defined(_DEBUG)


namespace DebugStrings
{
	extern Stringy LastErrorFile;
	extern Stringy LastErrorLine;
	extern Stringy LastError;
}

#endif

#ifdef WIN32
#define __FUNCTION_NAME__ __FUNCTION__
#elif defined(__linux__)
#define __FUNCTION_NAME__ __func__
#else
#error "So its not 'nix or win32, can't be screwed defining the function macro"
#endif


#if defined(_DEBUG) & defined(WIN32)
bool HRFunc(Stringy file, Stringy line, long hr);
long HHRFunc(Stringy file, Stringy line, long hr);

#	ifndef HR
// HR(x) prints any errors from a Failed HRESULT
#		define HR(x) HRFunc(__FILE_THING__, std::to_string(LINE_THING), x)
#	endif
#	ifndef HRRF
// HRRF(x) prints any errors from a Failed HRESULT, and returning false if there is one
#		define HRRF(x) { if (HR(x)) return false; }
#	endif
#	ifndef HRRH
// HRRF(x) prints any errors from a Failed HRESULT and returns the failed HRESULT
#		define HRRH(x) { long h = HHRFunc(FILE_THING, std::to_string(LINE_THING), x); if (h != 0ul) return h; }
#	endif
#	ifndef HRRE
// HRRE(x) prints any errors from a Failed HRESULT and returns a wstring containing the error message
#		define HRRE(x) { long h = HHRFunc(FILE_THING, std::to_string(LINE_THING), x); if (h != 0ul) return DebugStrings::LastError; }
#	endif HRRE

#else // RELEASE NO OUTPUT

#	ifndef HR
#		define HR(x) (x)
#	endif // HR

#	ifndef HRRF
#		define HRRF(x) { if (x) return false; }
#	endif

#	ifndef HRRH
#		define HRRH(x) { long r = x; if (r) return r; }
#	endif

#	ifndef HRRE
#		define HRRE(x) { if (x) return L"Error occured, however HRESULTs have been turned off and this exists simply to prevent compiler errors"; }
#	endif

#endif // _DEBUG

// ^^ Debug Function
// Debug Output vv


#define DEBUG_OUTPUT_COUT
//#define DEBUG_OUTPUT_LOG

#if defined(_DEBUG) & !defined(DEBUG_OUTPUT_DEBUGSTRING) & defined(WIN32)
#	define DEBUG_OUTPUT_DEBUGSTRING
#endif

#if (defined(DEBUG_OUTPUT_DEBUGSTRING) | defined(DEBUG_OUTPUT_COUT) | defined(DEBUG_OUTPUT_LOG)) & !defined(DEBUG_OUTPUT)
#	define DEBUG_OUTPUT
#endif

#ifdef DEBUG_OUTPUT

void OutFunc(const Charry *msg);

#	ifdef _UNICODE

#		ifndef DEBUGBASETHING
#			ifdef DEBUG_OUTPUT_LINES_FILES
#				define DEBUGBASETHING std::wstring(L" in function ") + __FUNCTIONW__ + L" at " + __FILEW__ + L"(" + __LINEW__ + L") "
#			else
#				define DEBUGBASETHING std::wstring(L" at ") + __FUNCTIONW__ + L" "
#			endif
#		endif // DEBUGBASETHING

#		ifndef WARNINGTHING
#			define WARNINGTHING std::wstring(L"WARNING: ") + DEBUGBASETHING
#		endif

#		ifndef ERRORTHING
#			define ERRORTHING std::wstring(L"ERROR: ") + DEBUGBASETHING
#		endif

#		ifndef CRITICALTHING
#			define CRITICALTHING std::wstring(L"CRITICAL: ") + DEBUGBASETHING
#		endif


#		ifndef DINFO
#			define DINFO(x) OutFunc((std::wstring(L"Info: ") + x + L"\n").c_str())
#		endif

#		ifndef DWARNING
#			define DWARNING(x) OutFunc((WARNINGTHING + x + L"\n").c_str())
#		endif // !DWARNING

#		ifndef DERROR
#			define DERROR(x) OutFunc((ERRORTHING + x + L"\n").c_str())
#		endif // !DERROR

#		ifndef DCRITICAL
#			define DCRITICAL(x) OutFunc((CRITICALTHING + x + L"\n").c_str())
#		endif // !DCRITICAL

#	else // UNICODE / TEXT

#		ifndef DEBUGBASETHING
#			ifdef DEBUG_OUTPUT_LINES_FILES
#				define DEBUGBASETHING std::string(" in fuction ") + __FUNCTION_NAME__ + " at " + __FILE__ + "(" + __LINE__ + ") "
#			else
#				define DEBUGBASETHING std::string(" at ") + __FUNCTION_NAME__ + " "
#			endif
#		endif // DEBUGBASETHING

#		ifndef WARNINGTHING
#			define WARNINGTHING std::string("WARNING: ") + DEBUGBASETHING
#		endif

#		ifndef ERRORTHING
#			define ERRORTHING std::string("ERROR: ") + DEBUGBASETHING
#		endif

#		ifndef CRITICALTHING
#			define CRITICALTHING std::string("CRITICAL: ") + DEBUGBASETHING
#		endif


#		ifndef DINFO
#			define DINFO(x) OutFunc((std::string("Info: ") + x + "\n").c_str())
#		endif

#		ifndef DWARNING
#			define DWARNING(x) OutFunc((WARNINGTHING + x + "\n").c_str())
#		endif // !DWARNING

#		ifndef DCRITICAL
#			define DCRITICAL(x) OutFunc((CRITICALTHING + x + "\n").c_str())
#		endif // !DCRITICAL

#		ifndef DERROR
#			define DERROR(x) OutFunc((ERRORTHING + x + "\n").c_str())
#		endif // !DERROR
#	endif
#endif

#if defined(_UNICODE) & defined(WIN32)
void ErrorExit(wchar_t *lpszFunction);
#endif

typedef int INT;
typedef unsigned int UINT;

#include <vector>

namespace Debug
{
	struct DebugReturn
	{
		INT Code;
		std::vector<Stringy> Errors;

		DebugReturn(bool val) : Code(val ? 0 : -1) {};
		DebugReturn(INT val = 0) : Code(val) {};
		DebugReturn(Stringy thing) : Code(-1), Errors({ thing }) {};
		DebugReturn(std::vector<Stringy> errors) : Code(-1), Errors(errors) {}

		DebugReturn(const DebugReturn &other) : Code(other.Code), Errors(other.Errors) {}
		DebugReturn(DebugReturn &&tmp) noexcept : Code(tmp.Code), Errors(std::move(tmp.Errors)) { tmp.Code = 0u; }

		inline Stringy GetError() const { if (HasErrors()) return Errors.front(); }
		inline bool HasErrors() const { return Errors.size(); }

		inline decltype(auto) begin() { return Errors.begin(); }
		inline decltype(auto) end() { return Errors.end(); }

		inline operator bool() { return !Code; }
		inline bool operator==(const DebugReturn & other)
		{
			return Code == other.Code && Errors == other.Errors;
		}
		inline Stringy AsString() const
		{
			if (HasErrors())
			{
				Stringy out;
				for (auto& error : Errors)
				{
					out += error + ", ";
				}
				out.pop_back();
				out.pop_back();
				return out;
			}
			else
				return "No Errors";
		}


		inline void Append(Stringy error)
		{
			Errors.push_back(std::move(error));
		}
		inline DebugReturn operator+(const DebugReturn &other) const
		{
			auto tmp = Errors;
			tmp.insert(tmp.end(), other.Errors.begin(), other.Errors.end());
			return tmp;
		}
		inline DebugReturn &operator+=(const DebugReturn &other)
		{
			Errors.insert(Errors.end(), other.Errors.begin(), other.Errors.end());
			return *this;
		}
		inline DebugReturn &operator=(const DebugReturn &other)
		{
			this->Code = other.Code;
			this->Errors = other.Errors;
			return *this;
		}
		inline DebugReturn &operator=(DebugReturn &&other) noexcept
		{
			this->Code = other.Code;
			other.Code = 0u;
			this->Errors = std::move(other.Errors);
			return *this;
		}
	};

	void ReportError(const Charry *title, const Charry *msg); // Report an Error to the user, convenience function that tries a simple message box, and console if that fails
	bool ReportRetry(const Charry *title, const Charry *msg); // Same as ReportError except returns whether user wants retry or not

	constexpr int RetryID = 0;
	constexpr int ContinueID = 1;
	constexpr int CancelID = 2;
	constexpr int ExitID = 3;
	int ReportErrorCancelTryContinue(const Charry *title, const Charry *msg);
	int ReportErrorCancelTry(const Charry *title, const Charry *msg);
	int ReportErrorCancelContinue(const Charry *title, const Charry *msg);
}

#define FILE_DATA_THING std::string(__FILE__) + "(" + std::to_string(__LINE__) + ")"

#define MEMBER_OFFSET(x, y) ((size_t)&reinterpret_cast<char const volatile&>((((x*)0)->y)))