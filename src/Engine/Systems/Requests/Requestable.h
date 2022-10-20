#pragma once

#include <Helpers/DebugHelper.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

struct CommonResources;

namespace Requests
{
	class Requester;

	struct IRequestData
	{
		virtual ~IRequestData() {}
	};

	struct Request
	{
		Stringy Name;
		std::vector<Stringy> Params;
		Requests::IRequestData *Data = nullptr;
		CommonResources *Resources = nullptr;

		Request(Stringy e) : Name(e) {}
		Request(Stringy e, std::vector<Stringy> params, Requests::IRequestData *data = nullptr, CommonResources *resources = nullptr) : Name(e), Params(std::move(params)), Data(data), Resources(resources) {}
		Request(Stringy e, std::vector<Stringy> params, CommonResources *resources) : Name(e), Params(params), Resources(resources) {}
		Request(Stringy e, CommonResources *resources) : Name(e), Resources(resources) {}
	};

	// Request that can be sent to multiple Receivers
	struct MultiRequest
	{
		Stringy Name;
		std::vector<Stringy> Params;
		Requests::IRequestData *Data = nullptr;
		CommonResources *Resources = nullptr;

		MultiRequest(Stringy e) : Name(e) {}
		MultiRequest(Stringy e, std::vector<Stringy> params, Requests::IRequestData *data = nullptr, CommonResources *resources = nullptr) : Name(e), Params(std::move(params)), Data(data), Resources(resources) {}
		MultiRequest(Stringy e, std::vector<Stringy> params, CommonResources *resources) : Name(e), Params(params), Resources(resources) {}
		MultiRequest(Stringy e, CommonResources *resources) : Name(e), Resources(resources) {}

		inline operator Request() const
		{
			return Request(Name, Params, Data, Resources);
		}
	};


	struct IRequestable
	{
		friend class Requester;
	private:
		Requester * master = nullptr;
	protected:
		inline std::add_cv<Requester*>::type GetMaster() const { return master; }
	public:
		virtual Debug::DebugReturn Request(Request& req) = 0;
		virtual Stringy GetName() const = 0;

		virtual ~IRequestable();
	};
}