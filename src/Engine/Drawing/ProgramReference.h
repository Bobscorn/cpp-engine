#pragma once

#include <string>
#include <memory>
#include <functional>

namespace Drawing
{
	class Program;

	class ProgramReference
	{
		std::string _programName;

		// Mutable as this is really just a cache member to cache the result of ProgramStore::Get(_programName);
		mutable std::shared_ptr<Program> _program;

	public:
		ProgramReference() = default;
		ProgramReference(std::string programName);
		ProgramReference(ProgramReference&& other);
		ProgramReference(const ProgramReference& other);

		inline bool HasProgram() const { return _programName.size() && _program.get(); }

		const std::shared_ptr<Program>& GetProgram() const;

		const std::string& GetProgramName() const;

		ProgramReference& operator=(std::string newProgramName);
		ProgramReference& operator=(ProgramReference&& other);
		ProgramReference& operator=(const ProgramReference& other);

		bool operator==(const ProgramReference& other) const;
		inline bool operator!=(const ProgramReference& other) const { return !(*this == other); }

		// Will try to grab the program if it does not have it stored at _program
		// However grabbing the program is const (via _program being mutable)
		bool IsValid() const;
	};
}

namespace std
{
	template<>
	struct hash<Drawing::ProgramReference>
	{
		inline size_t operator()(const Drawing::ProgramReference& reference) const
		{
			return hash<string>()(reference.GetProgramName());
		}
	};
}