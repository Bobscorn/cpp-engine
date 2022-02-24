//#include "Thing.h"
#include "Options.h"

Options::Options::Options(std::vector<std::pair<OptionID, size_t>> options)
{
	for (auto& option : options)
	{
		mOptions[option.first] = (double)option.second;
	}
}
