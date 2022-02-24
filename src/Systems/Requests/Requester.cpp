#include "Requester.h"

Debug::DebugReturn Requests::Requester::Request(Requests::Request req)
{
	Stringy Parameters;
	for (auto& param : req.Params)
	{
		Parameters += param + ", ";
	}
	if (Parameters.size())
	{
		Parameters.pop_back();
		Parameters.pop_back();
	}
	DINFO("Requesting '" + req.Name + "' with parameters of '" + Parameters + "'");
	for (size_t i = Requestables.size(); i-- > 0; )
	{
		auto *requestable = Requestables[i];
		if (!requestable)
		{
			DWARNING("Null requestable found");
			continue;
		}
		auto out = requestable->Request(req);
		if (out)
		{
			DINFO("Request resolved by '" + requestable->GetName() + "'");
			return true;
		}
		else if (out.HasErrors())
		{
			DINFO("Requestable '" + requestable->GetName() + "' attempted to resolve (request was aborted) and failed with error: " + out.AsString());
			return out;
		}
	}
	DINFO("No Requestable found that takes Request by : (Name: '" + req.Name + "', Parameters : '" + Parameters + "')");
	return "No Requestable found that takes Request by: (Name: '" + req.Name + "', Parameters: '" + Parameters + "')";
}

Debug::DebugReturn Requests::Requester::Request(const MultiRequest && r)
{
	return Request(r);
}

Debug::DebugReturn Requests::Requester::Request(MultiRequest r)
{
	Stringy Parameters;
	for (auto& param : r.Params)
	{
		Parameters += param + ", ";
	}
	if (Parameters.size())
	{
		Parameters.pop_back();
		Parameters.pop_back();
	}
	DINFO("Requesting (Multi) '" + r.Name + "' with parameters of '" + Parameters + "'");
	
	Debug::DebugReturn Errors;
	unsigned int numsent = 0, numfailed = 0u;

	Requests::Request req = r;
	for (auto& requestable : Requestables)
	{
		auto out = requestable->Request(req);
		if (out)
			++numsent;

		Errors += out;
	}
	if (numsent)
	{
		DINFO("Request resolved by '" + std::to_string(numsent) + "' Requestable(s)");
		if (Errors.HasErrors())
		{
			DINFO("Request also failed to resolve with '" + std::to_string(Errors.Errors.size()) + "' Requestable(s), with error(s): '" + Errors.AsString() + "'");
			return "Request was resolved by '" + std::to_string(numsent) + "' and unsolved by '" + std::to_string(Errors.Errors.size()) + "' Requestable(s)";
		}
		else
			return true;
	}
	else if (Errors.HasErrors())
	{
		DINFO("Request completely failed to resolve with '" + std::to_string(Errors.Errors.size()) + "' Requestable(s) attempting, with error(s): '" + Errors.AsString() + "'");
		return "Request failed by '" + std::to_string(numfailed) + "' Requestables";
	}
	DINFO("No Requestable found that takes Request by : (Name: '" + req.Name + "', Parameters : '" + Parameters + "')");
	return "No Requestable found that takes Request by: (Name: '" + req.Name + "', Parameters: '" + Parameters + "')";
}
