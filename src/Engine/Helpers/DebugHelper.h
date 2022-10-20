#pragma once



// Debug Macros/Function vv
// Debug Output vvvv
#include "DebugPrintHelper.h"

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
		explicit DebugReturn(INT val = 0) : Code(val) {};
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