#pragma once

#include <string>
#include <memory>

namespace Drawing
{
	class Program;

	class ProgramReference
	{
		std::string _programName;
		std::shared_ptr<Program> _program;

	public:
		ProgramReference() = default;
		ProgramReference(std::string programName);
		ProgramReference(ProgramReference&& other);

		inline bool HasProgram() const { return _programName.size() && _program.get(); }

		std::shared_ptr<Program> GetProgram();

		std::string GetProgramName() const;

		ProgramReference& operator=(std::string newProgramName);
		ProgramReference& operator=(ProgramReference&& other);
	};
}