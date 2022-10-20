#pragma once

#include "TimeSequence.h"

#include "Systems/Execution/ResourceHolder.h"

#include "Math/floaty.h"

namespace Sequence
{
	struct TerminalSequencev2;

	struct TerminalSequenceElement
	{
		TerminalSequenceElement(TerminalSequencev2 *term) : Terminal(term) {}
		virtual ~TerminalSequenceElement() {}
		virtual double GetTextString(double deltatime, std::string &out) = 0;
	protected:
		TerminalSequencev2 *Terminal;
	};

	struct TerminalSequencev2 : ITimedSequenceElement, virtual FullResourceHolder, TextBrushHolder, NormalFontHolder
	{
		TerminalSequencev2(CommonResources *resources) : FullResourceHolder(resources), TextBrushHolder("Terminal"), NormalFontHolder("Terminal") { EnsureFormat(); EnsureBrush(); CaretTimer.Reset(); CaretTimer.Start(); }
		//TerminalSequencev2(CommonResources *resources, std::vector<std::unique_ptr<TerminalSequenceElement>>&& vec) : FullResourceHolder(resources), TextBrushHolder(L"Terminal"), NormalFontHolder(L"Terminal"), Elements(vec) { EnsureFormat(); EnsureBrush(); }

		virtual double Apply(double localtime) override;

		template<class T, typename ... Args>
		std::enable_if_t<std::is_base_of_v<TerminalSequenceElement, T>, T* const> Add(Args&&... args)
		{
			auto ptr = new T(this, std::forward<Args>(args)...);
			Elements.emplace_back(ptr);
			return ptr;
		}


		void EnsureFormat();
		void EnsureBrush();
		void DrawTheText(std::string amount);
		void DrawCursor(floaty2 here);
		void DrawFlickerCursor(floaty2 here);

		inline void DisableCursor() { DisplayCursor = false; }
		inline void EnableCursor() { DisplayCursor = true; }
		
	protected:
		double InternalApply(double deltatime);

		double LastTime{ 0.0 };
		size_t CurrentIndex{ 0ul };
		size_t LastStringSize{ 0ul };
		std::vector<std::unique_ptr<TerminalSequenceElement>> Elements;
		std::string CompletedString;
		GameTimer CaretTimer;
		bool DisplayCursor{ true };
	};

	struct NormalTextTerminalElement : TerminalSequenceElement
	{
		NormalTextTerminalElement(TerminalSequencev2 *term, std::string content = "") : TerminalSequenceElement(term), Omaewamoushindeiru(content), duration(0.03125 * double(content.size())) {}
		NormalTextTerminalElement(TerminalSequencev2 *term, std::string content, double time) : TerminalSequenceElement(term), Omaewamoushindeiru(content), duration(time) {  }

		virtual double GetTextString(double deltatime, std::string &out) override;

	protected:
		size_t GetIndexTo(double deltatime);
		std::string GetCulledString(double time);

		double duration{ 2.f };
		std::string Omaewamoushindeiru;
	};
	
	struct PauseTerminalElement : TerminalSequenceElement
	{
		PauseTerminalElement(TerminalSequencev2 *term, double duration = 3.0) : TerminalSequenceElement(term), Duration(duration) {}

		double GetTextString(double deltatime, std::string &out) override;

	protected:
		double Duration{ 3.0 };
	};

	struct LoadingTerminalElement : TerminalSequenceElement
	{
		LoadingTerminalElement(TerminalSequencev2 *term, double duration = 3.f) : TerminalSequenceElement(term), Duration(duration) {}

		double GetTextString(double deltatime, std::string &out) override;

	protected:
		double Duration{ 3.f };
		double ChangeInterval{ 0.25 };
	};

	struct Loading2TerminalElement : TerminalSequenceElement
	{
		Loading2TerminalElement(TerminalSequencev2 *term, double duration = 3.f, size_t length = 30ul) : TerminalSequenceElement(term), Duration(duration), numloadingsymbols(length) { TextDuration = 0.015625 * double(length + 2ul); }
		Loading2TerminalElement(TerminalSequencev2 *term, double duration, size_t length, double texttime) : TerminalSequenceElement(term), Duration(duration), numloadingsymbols(length), TextDuration(texttime) {}

		double GetTextString(double deltatime, std::string &out) override;

	protected:
		static const char *LoadSymbol;
		static const char *NonLoadSymbol;
		static const char *LoadingLoadBarSymbol;

		size_t GetTextIndex(double time);

		std::string GetNonLoadSymbols(size_t count);
		std::string GetLoadSymbols(size_t count);
		std::string GetLoadLoadSymbols(size_t count);

		double Duration{ 3.f };
		double TextDuration{ 1.5f };
		size_t numloadingsymbols{ 30ul };
	};
}