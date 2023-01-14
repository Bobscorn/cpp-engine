#pragma once

#include "Helpers/TransferHelper.h"

#include "Systems/Events/Events.h"

#include <vector>
#include <string>

namespace InputAttach
{
	struct IAttachable
	{
		friend class AttachmentManager;
	private:
		AttachmentManager *manager = nullptr;

	public:
		virtual ~IAttachable();

		// Input methods
		inline virtual bool Receive(Event::KeyInput* e) { (void)e;  return false; }
		inline virtual bool Receive(Event::MouseMove* e) { (void)e; return false; }
		inline virtual bool Receive(Event::MouseButton* e) { (void)e; return false; }
		inline virtual bool Receive(Event::MouseWheel* e) { (void)e; return false; }
		inline virtual bool Receive(Event::MouseWheelButton* e) { (void)e; return false; };

		// Attachment event methods
		inline virtual void Reinstated() {}
		inline virtual void Superseded() {}

		virtual Stringy GetName() const = 0;
	};

	class AttachmentManager : public Events::IInputListener
	{
		std::vector<IAttachable*> Attachments;
		Stringy Name;
	public:
		AttachmentManager(Stringy name = "AttachmentManager") : Name(name) {}
		virtual ~AttachmentManager() {}

		inline bool Receive(Events::IEvent *e) override { (void)e; return false; }
		inline bool Receive(Event::KeyInput *e) override
		{
			if (Attachments.size())
				return Attachments.back()->Receive(e);
			return false;
		}
		inline bool Receive(Event::MouseMove *e) override
		{
			if (Attachments.size())
				return Attachments.back()->Receive(e);
			return false;
		}
		inline bool Receive(Event::MouseButton *e) override
		{
			if (Attachments.size())
				return Attachments.back()->Receive(e);
			return false;
		}
		inline bool Receive(Event::MouseWheel *e) override
		{
			if (Attachments.size())
				return Attachments.back()->Receive(e);
			return false;
		}
		inline bool Receive(Event::MouseWheelButton *e) override
		{
			if (Attachments.size())
				return Attachments.back()->Receive(e);
			return false;
		}
		//inline bool Receive(Event::ControllerAxis *e) override;


		inline void Add(IAttachable *e)
		{
			if (e)
			{
				bool dupe = false;
				for (size_t i = Attachments.size(); i-- > 0; )
				{
					if (Attachments[i] == e)
					{
						DINFO("Duplicate Attachment detected, pushing to top");
						e->manager = nullptr;
						Attachments.erase(Attachments.begin() + i);
						Attachments.push_back(e);
						e->manager = this;
						dupe = true;
						break;
					}
				}
				if (!dupe)
				{
					e->manager = this;
					if (Attachments.size())
						Attachments.back()->Superseded();
					Attachments.push_back(e);
				}
			}
			else
				DWARNING("Attempt at adding null IAttachable to '" + GetName() + "' ");
		}

		inline void Remove(IAttachable *e)
		{
			if (!e)
			{
				DWARNING("Attempted removal from '" + GetName() + "' with a null IAttachable");
				return;
			}
			e->manager = nullptr;
			if (!Attachments.size())
			{
				DWARNING("Attachment being removed from '" + GetName() + "' that did not have it in the first place");
			}

			for (auto i = Attachments.size(); i-- > 0; )
			{
				if (Attachments[i] == e)
				{
					Attachments.erase(Attachments.begin() + i);
				}
			}
		}

		inline virtual Stringy GetName() const override { return Name; }
	};
}