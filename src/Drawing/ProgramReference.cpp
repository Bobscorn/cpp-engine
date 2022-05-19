#include "ProgramReference.h"

#include "Program.h"

namespace Drawing
{
	ProgramReference::ProgramReference(std::string programName)
		: _programName(programName)
	{
	}

	ProgramReference::ProgramReference(ProgramReference&& other)
		: _programName(std::move(other._programName))
		, _program(std::move(other._program))
	{
		other._programName = "";
		other._program = nullptr;
	}

	std::shared_ptr<Program> ProgramReference::GetProgram()
	{
		if (!_program && _programName.size())
			_program = ProgramStore::Instance().GetProgram(_programName);
		return _program;
	}

	std::string ProgramReference::GetProgramName() const
	{
		return _programName;
	}

	ProgramReference& ProgramReference::operator=(std::string newProgramName)
	{
		_programName = std::move(newProgramName);
		_program = nullptr;
		return *this;
	}

	ProgramReference& ProgramReference::operator=(ProgramReference&& other)
	{
		_programName = std::move(other._programName);
		_program = std::move(other._program);

		other._programName = "";
		other._program = nullptr;

		return *this;
	}
}