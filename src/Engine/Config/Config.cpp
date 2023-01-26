#include "Config.h"

#include "Helpers/DebugHelper.h"

#include <list>
#include <forward_list>
#include <unordered_map>
#include <vector>

namespace Config
{
	std::any DeadVar = 0.f;

	struct KeyInternal
	{
		std::string Name;
		size_t Index = 0ull;

		inline bool operator==(const KeyInternal &other) const { return Name == other.Name && Index == other.Index; }
	};

	bool IConfigThrone::HasType(Key key, const std::type_info &type)
	{
		const std::any *target = this->GetN(key);
		if (!target)
			return false;
		return target->type() == type;
	}
	void IChangeListener::Reset()
	{ 
		if (m_Overlord) 
			m_Overlord->RemoveListener(this); 
		m_Overlord = nullptr;
	}
	void DefaultConfigValues::operator()(IConfigThrone *config) const
	{
		config->SafeSet(Config::Key{ "InverseMouseSensitivity" }, 1.f / 250.f);
		config->SafeSet(Config::Key{ "MouseSensitivity" }, 250.f);
		config->SafeSet(Config::Key{ "MultiSampleLevel" }, 0ull);
		config->SafeSet(Config::Key{ "FOV" }, 140.f);
	}
}

namespace std
{
	template<>
	struct hash<Config::KeyInternal>
	{
		inline size_t operator()(const Config::KeyInternal &val) const
		{
			size_t shash = hash<std::string>()(val.Name);
			shash >>= 1;
			shash <<= (sizeof(size_t) * 8) / 2;
			size_t ihash = val.Index >> 1;
			ihash &= 0x00000000ffffffff;
			return shash + ihash;
		}
	};

	template<>
	struct hash<Config::Key>
	{
		inline size_t operator()(const Config::Key &val) const
		{
			size_t chash = (size_t)val.Container << ((sizeof(size_t) - 1) * 8);
			size_t shash = hash<std::string>()(val.Name);
			shash <<= ((sizeof(size_t) / 2) - 1) * 8;
			size_t ihash = val.Index & 0x0000000000ffffff;
			return chash + shash + ihash;
		}
	};

	template<>
	struct hash<std::pair<Config::KeyInternal, std::any>>
	{
		inline size_t operator()(const std::pair<Config::KeyInternal, std::any> &val) const
		{
			return hash<Config::KeyInternal>()(val.first);
		}
	};
}

namespace Config
{

	class ConfigOverlord : public Config::IConfigThrone
	{
		enum LockType
		{
			Shared = 0,
			Exclusive = 1,
			Lifetime = 2,
		};

		struct LockInfoInternal
		{
			LockType Type;
			size_t Count = 0ull;
		};

		std::list<std::pair<KeyInternal, std::any>> DoubleListVariables;
		std::forward_list<std::pair<KeyInternal, std::any>> ForwardListVariables;
		std::vector<std::pair<KeyInternal, std::any>> VectorVariables;
		std::unordered_map<KeyInternal, std::any> HashmapVariables;

		std::unordered_map<Key, LockInfoInternal> Locks;
		std::unordered_map<Key, std::vector<IChangeListener *>> Listeners;

	protected:
		virtual void RemoveSharedLock(Key key) noexcept override;
		virtual void RemoveExclusiveLock(Key key) noexcept override;
		virtual void RemoveLifetimeLock(Key key) noexcept override;
		virtual void RemoveListener(IChangeListener *listener) override;

		std::any *GetInternal(Key key);
		void RemoveVariable(Key key); // Does not check for locks, currently assumed this will only be called when the last Lifetime lock on it is destroyed

		std::list<std::pair<KeyInternal, std::any>>::iterator FindDoubleListVar(KeyInternal key);
		std::forward_list<std::pair<KeyInternal, std::any>>::iterator FindForwardListVar(KeyInternal key);
		std::vector<std::pair<KeyInternal, std::any>>::iterator FindVectorVar(KeyInternal key);
		std::list<std::pair<KeyInternal, std::any>>::const_iterator FindDoubleListVar(KeyInternal key) const;
		std::forward_list<std::pair<KeyInternal, std::any>>::const_iterator FindForwardListVar(KeyInternal key) const;
		std::vector<std::pair<KeyInternal, std::any>>::const_iterator FindVectorVar(KeyInternal key) const;

	public:

		virtual void Set(Key key, const std::any &val) override;
		virtual void SafeSet(Key key, const std::any &val) noexcept override;

		virtual const std::any &Get(Key key) const override;
		virtual const std::any *GetN(Key key) const noexcept override;

		virtual SharedVariableLock GetShared(Key key) noexcept override;
		virtual ExclusiveVariableLock GetExclusive(Key key) noexcept override;
		virtual bool GetLifetimeExisting(Key key, SharedLifetimeLock &out) noexcept override;
		virtual bool GetLifetime(Key key, SharedLifetimeLock &out) noexcept override;
		virtual SharedLifetimeLock StartLifetime(Key key) override;

		virtual bool RegisterListener(IChangeListener *listener) override;

		virtual size_t Count(Key key) override;

		virtual bool Has(Key key) override;
	};
}

namespace Config
{

	ExclusiveVariableLock::~ExclusiveVariableLock()
	{
		if (m_Overlord)
		{
			m_Overlord->RemoveExclusiveLock(this->m_Key);
		}
		else
		{
			assert(!m_Variable);
		}
	}

	ExclusiveVariableLock &ExclusiveVariableLock::operator=(ExclusiveVariableLock &&other)
	{
		Reset();
		this->m_Key = other.m_Key;
		this->m_Overlord = other.m_Overlord;
		this->m_Variable = other.m_Variable;

		other.m_Overlord = nullptr;
		other.m_Variable = nullptr;
		return *this;
	}

	void ExclusiveVariableLock::Reset()
	{
		if (m_Overlord)
		{
			m_Overlord->RemoveExclusiveLock(this->m_Key);
			m_Overlord = nullptr;
			m_Variable = nullptr;
		}
		else
		{
			assert(!m_Variable);
		}
	}

	SharedVariableLock::SharedVariableLock(const SharedVariableLock &other) : SharedVariableLock((other.m_Overlord ? other.m_Overlord->GetShared(other.m_Key) : std::move(SharedVariableLock{ nullptr }))) {}

	SharedVariableLock::~SharedVariableLock()
	{
		if (m_Overlord)
		{
			m_Overlord->RemoveSharedLock(this->m_Key);
		}
		else
		{
			assert(!m_Variable);
		}
	}

	SharedVariableLock &SharedVariableLock::operator=(const SharedVariableLock &other)
	{
		if (!other.m_Overlord)
		{
			Reset();
			return *this;
		}

		SharedVariableLock tmp = other.m_Overlord->GetShared(other.m_Key);

		if (tmp.m_Overlord)
			return (*this) = std::move(tmp);
		else
		{
			DWARNING("Failed to copy Shared lock");
			return *this;
		}
	}

	SharedVariableLock &SharedVariableLock::operator=(SharedVariableLock &&other)
	{
		Reset();
		this->m_Key = other.m_Key;
		this->m_Overlord = other.m_Overlord;
		this->m_Variable = other.m_Variable;

		other.m_Overlord = nullptr;
		other.m_Variable = nullptr;
		return *this;
	}

	void SharedVariableLock::Reset()
	{
		if (m_Overlord)
		{
			m_Overlord->RemoveSharedLock(m_Key);
			m_Overlord = nullptr;
			m_Variable = nullptr;
		}
		else
		{
			assert(!m_Variable);
		}
	}

	SharedLifetimeLock::SharedLifetimeLock(const SharedLifetimeLock &other) : m_Key(other.m_Key), m_Variable{nullptr}, m_Overlord{nullptr}
	{
		if (other.m_Overlord)
		{
			other.m_Overlord->GetLifetimeExisting(other.m_Key, *this);
		}
		else
			m_Key = {};
	}

	SharedLifetimeLock::~SharedLifetimeLock()
	{
		if (m_Overlord)
		{
			m_Overlord->RemoveLifetimeLock(m_Key);
			m_Overlord = nullptr;
			m_Variable = nullptr;
		}
		else
		{
			assert(!m_Variable);
		}
	}
	SharedLifetimeLock &SharedLifetimeLock::operator=(const SharedLifetimeLock &other)
	{
		if (!other.m_Overlord)
		{
			Reset();
			return *this;
		}

		SharedLifetimeLock tmp{ nullptr };
		assert(other.m_Overlord->GetLifetimeExisting(other.m_Key, tmp));

		if (tmp.m_Overlord)
			return (*this) = std::move(tmp);
		else
		{
			DWARNING("Failed to copy Lifetime lock");
			return *this;
		}
	}
	SharedLifetimeLock &SharedLifetimeLock::operator=(SharedLifetimeLock &&other) noexcept
	{
		Reset();
		this->m_Key = other.m_Key;
		this->m_Overlord = other.m_Overlord;
		this->m_Variable = other.m_Variable;

		other.m_Overlord = nullptr;
		other.m_Variable = nullptr;
		return *this;
	}
	void SharedLifetimeLock::Reset()
	{
		if (m_Overlord)
		{
			m_Overlord->RemoveSharedLock(m_Key);
			m_Overlord = nullptr;
			m_Variable = nullptr;
		}
		else
		{
			assert(!m_Variable);
		}
	}

	void ConfigOverlord::RemoveSharedLock(Key key) noexcept
	{
		if (!Has(key))
		{
			DWARNING("Can not remove Shared Lock on non-existant variable");
			return;
		}

		auto it = Locks.find(key);
		if (it != Locks.end())
		{
			if (it->second.Type == Shared)
			{
				if (it->second.Count == 1)
					Locks.erase(it);
				else
					it->second.Count -= 1;
			}
			else
			{
				DWARNING("Attempt at removing a non-shared lock as if it was a Shared lock, ignoring");
			}
		}
	}

	void ConfigOverlord::RemoveExclusiveLock(Key key) noexcept
	{
		if (!Has(key))
		{
			DWARNING("Can not remove Exclusive Lock on non-existant variable");
			return;
		}

		auto it = Locks.find(key);
		if (it != Locks.end())
		{
			if (it->second.Type == Exclusive)
			{
				Locks.erase(it);
			}
			else
			{
				DWARNING("Attempt at removing a non-exclusive lock as if it was an Exclusive lock, ignoring");
			}
		}
	}

	void ConfigOverlord::RemoveLifetimeLock(Key key) noexcept
	{
		if (!Has(key))
		{
			DWARNING("Can not remove lifetime lock on a non-existant variable");
			return;
		}

		auto it = Locks.find(key);
		if (it != Locks.end())
		{
			if (it->second.Type == Lifetime)
			{
				if (it->second.Count == 1)
				{
					Locks.erase(it);
					RemoveVariable(key);
				}
				else
					it->second.Count -= 1;
			}
			else
			{
				DWARNING("Given key is locked by something other than a lifetime lock, cannot remove");
			}
		}
	}

	void ConfigOverlord::RemoveListener(IChangeListener *listener)
	{
		auto it = Listeners.find(listener->GetKey());
		if (it == Listeners.end())
		{
			DWARNING("Not removing non-existant listener");
			return;
		}
		auto vecit = std::find(it->second.begin(), it->second.end(), listener);
		if (vecit != it->second.end())
		{
			DINFO("Removing Config Change Listener");
			std::swap(*vecit, it->second.back());
			it->second.pop_back();
		}
	}

	std::any *ConfigOverlord::GetInternal(Key key)
	{
		KeyInternal key_int = { key.Name, key.Index };
		switch (key.Container)
		{
		case HashMap:
		{
			auto it = HashmapVariables.find(key_int);
			if (it == HashmapVariables.cend())
				return nullptr;
			return &it->second;
		}
		break;
		case List:
		{
			auto it = FindForwardListVar(key_int);
			if (it == ForwardListVariables.cend())
				return nullptr;
			return &it->second;
		}
		break;
		case DoubleList:
		{
			auto it = FindDoubleListVar(key_int);
			if (it == DoubleListVariables.cend())
				return nullptr;
			return &it->second;
		}
		break;
		case Vector:
		{
			auto it = FindVectorVar(key_int);
			if (it == VectorVariables.cend())
				return nullptr;
			return &it->second;
		}
		break;
		default:
		case Unknown:
		{
			{
				auto it = HashmapVariables.find(key_int);
				if (it != HashmapVariables.end())
				{
					return &it->second;
				}
			}
			{
				auto it = FindForwardListVar(key_int);
				if (it != ForwardListVariables.end())
				{
					return &it->second;
				}
			}
			{
				auto it = FindDoubleListVar(key_int);
				if (it != DoubleListVariables.end())
				{
					return &it->second;
				}
			}
			{
				auto it = FindVectorVar(key_int);
				if (it != VectorVariables.end())
				{
					return &it->second;
				}
			}
			DWARNING("Could not find existing value (" + key.Name + ", " + std::to_string(key.Index) + ") with Unknown Container");
		}
		break;
		}
		DWARNING("Could not find (" + key.Name + ", " + std::to_string(key.Index) + ")'s value in specified Container");
		return nullptr;
	}

	void ConfigOverlord::RemoveVariable(Key key)
	{
		KeyInternal key_int = { key.Name, key.Index };

		auto mapit = HashmapVariables.find(key_int);
		if (mapit != HashmapVariables.end())
		{
			HashmapVariables.erase(mapit);
			return;
		}

		auto doublelistit = FindDoubleListVar(key_int);
		if (doublelistit != DoubleListVariables.end())
		{
			DoubleListVariables.erase(doublelistit);
			return;
		}

		std::forward_list<std::pair<Config::KeyInternal, std::any>>::iterator listit = ForwardListVariables.begin(), previt = ForwardListVariables.before_begin();
		for (; listit != ForwardListVariables.end(); ++listit)
		{
			if (listit->first == key_int)
			{
				ForwardListVariables.erase_after(previt);
				return;
			}
			previt = listit;
		}

		auto vecit = FindVectorVar(key_int);
		if (vecit != VectorVariables.end())
		{
			VectorVariables.erase(vecit);
			return;
		}

		DWARNING("Key not found in any container");
	}

	std::list<std::pair<KeyInternal, std::any>>::iterator ConfigOverlord::FindDoubleListVar(KeyInternal key)
	{
		auto it = DoubleListVariables.begin();
		for (; it != DoubleListVariables.end(); ++it)
		{
			if (it->first == key)
				return it;
		}
		return it;
	}

	std::forward_list<std::pair<KeyInternal, std::any>>::iterator ConfigOverlord::FindForwardListVar(KeyInternal key)
	{
		auto it = ForwardListVariables.begin();
		for (; it != ForwardListVariables.end(); ++it)
		{
			if (it->first == key)
				return it;
		}
		return it;
	}

	std::vector<std::pair<KeyInternal, std::any>>::iterator ConfigOverlord::FindVectorVar(KeyInternal key)
	{
		auto it = VectorVariables.begin();
		for (; it != VectorVariables.end(); ++it)
		{
			if (it->first == key)
				return it;
		}
		return it;
	}

	std::list<std::pair<KeyInternal, std::any>>::const_iterator ConfigOverlord::FindDoubleListVar(KeyInternal key) const
	{
		auto it = DoubleListVariables.cbegin();
		for (; it != DoubleListVariables.cend(); ++it)
		{
			if (it->first == key)
				return it;
		}
		return it;
	}

	std::forward_list<std::pair<KeyInternal, std::any>>::const_iterator ConfigOverlord::FindForwardListVar(KeyInternal key) const
	{
		auto it = ForwardListVariables.cbegin();
		for (; it != ForwardListVariables.cend(); ++it)
		{
			if (it->first == key)
				return it;
		}
		return it;
	}

	std::vector<std::pair<KeyInternal, std::any>>::const_iterator ConfigOverlord::FindVectorVar(KeyInternal key) const
	{
		auto it = VectorVariables.cbegin();
		for (; it != VectorVariables.cend(); ++it)
		{
			if (it->first == key)
				return it;
		}
		return it;
	}

	void ConfigOverlord::Set(Key key, const std::any &val)
	{
		auto lockit = Locks.find(key);
		if (lockit != Locks.end())
		{
			throw LockedException();
		}

		SafeSet(key, val);
	}

	void ConfigOverlord::SafeSet(Key key, const std::any &val) noexcept
	{
		auto lockit = Locks.find(key);
		if (lockit != Locks.end())
		{
			return;
		}

		auto listenit = Listeners.find(key);
		if (listenit != Listeners.end())
		{
			for (auto &listener : listenit->second)
			{
				listener->VariableChanged(val);
			}
		}

		KeyInternal key_int = { key.Name, key.Index };
		switch (key.Container)
		{
		case HashMap:
		{
			HashmapVariables[key_int] = val;
		}
		break;
		case List:
		{
			auto it = FindForwardListVar(key_int);
			if (it != ForwardListVariables.end())
				it->second = val;
			else
				ForwardListVariables.emplace_front(std::make_pair(key_int, val));
		}
		break;
		case DoubleList:
		{
			auto it = FindDoubleListVar(key_int);
			if (it != DoubleListVariables.end())
				it->second = val;
			else
				DoubleListVariables.emplace_back(std::make_pair(key_int, val));
		}
		break;
		case Vector:
		{
			auto it = FindVectorVar(key_int);
			if (it != VectorVariables.end())
				it->second = val;
			else
				VectorVariables.emplace_back(std::make_pair(key_int, val));
		}
		break;
		default:
		case Unknown:
		{
			{
				auto it = HashmapVariables.find(key_int);
				if (it != HashmapVariables.end())
				{
					it->second = val;
					break;
				}
			}
			{
				auto it = FindForwardListVar(key_int);
				if (it != ForwardListVariables.end())
				{
					it->second = val;
					break;
				}
			}
			{
				auto it = FindDoubleListVar(key_int);
				if (it != DoubleListVariables.end())
				{
					it->second = val;
					break;
				}
			}
			{
				auto it = FindVectorVar(key_int);
				if (it != VectorVariables.end())
				{
					it->second = val;
					break;
				}
			}
			DWARNING("Could not find existing value (" + key.Name + ", " + std::to_string(key.Index) + ") with Unknown Container");
		}
		break;
		}
	}

	const std::any &ConfigOverlord::Get(Key key) const
	{
		const std::any *p = GetN(key);
		if (!p)
			throw CouldNotFindException();
		else
			return *p;
	}

	const std::any *ConfigOverlord::GetN(Key key) const noexcept
	{
		KeyInternal key_int = { key.Name, key.Index };
		switch (key.Container)
		{
		case HashMap:
		{
			auto it = HashmapVariables.find(key_int);
			if (it == HashmapVariables.cend())
				return nullptr;
			return &it->second;
		}
		break;
		case List:
		{
			auto it = FindForwardListVar(key_int);
			if (it == ForwardListVariables.cend())
				return nullptr;
			return &it->second;
		}
		break;
		case DoubleList:
		{
			auto it = FindDoubleListVar(key_int);
			if (it == DoubleListVariables.cend())
				return nullptr;
			return &it->second;
		}
		break;
		case Vector:
		{
			auto it = FindVectorVar(key_int);
			if (it == VectorVariables.cend())
				return nullptr;
			return &it->second;
		}
		break;
		default:
		case Unknown:
		{
			{
				auto it = HashmapVariables.find(key_int);
				if (it != HashmapVariables.end())
				{
					return &it->second;
				}
			}
			{
				auto it = FindForwardListVar(key_int);
				if (it != ForwardListVariables.end())
				{
					return &it->second;
				}
			}
			{
				auto it = FindDoubleListVar(key_int);
				if (it != DoubleListVariables.end())
				{
					return &it->second;
				}
			}
			{
				auto it = FindVectorVar(key_int);
				if (it != VectorVariables.end())
				{
					return &it->second;
				}
			}
			DWARNING("Could not find existing value (" + key.Name + ", " + std::to_string(key.Index) + ") with Unknown Container");
		}
		break;
		}
		DWARNING("Could not find (" + key.Name + ", " + std::to_string(key.Index) + ")'s value in specified Container");
		return nullptr;
	}

	SharedVariableLock ConfigOverlord::GetShared(Key key) noexcept
	{
		auto it = Locks.find(key);
		if (it != Locks.end())
		{			
			if (it->second.Type == Shared)
			{
				auto ptr = GetInternal(key);
				if (!ptr)
				{
					DWARNING("Lock was found for a variable that didn't exist when creating new lock");
					return nullptr;
				}
				++it->second.Count;
				return { key, ptr, this };
			}
			DINFO("Shared lock denied");
			return nullptr;
		}
		else
		{
			auto ptr = GetInternal(key);
			if (!ptr)
			{
				return nullptr;
			}
			Locks[key] = { Shared, 1 };
			return { key, ptr, this };
		}
	}

	ExclusiveVariableLock ConfigOverlord::GetExclusive(Key key) noexcept
	{
		auto it = Locks.find(key);
		if (it != Locks.end())
		{
			DINFO("Exclusive lock denied");
			return nullptr;
		}
		else
		{
			auto ptr = GetInternal(key);
			if (!ptr)
			{
				return nullptr;
			}
			Locks[key] = { Exclusive, 1 };
			return { key, ptr, this };
		}
	}

	bool ConfigOverlord::GetLifetimeExisting(Key key, SharedLifetimeLock &out) noexcept
	{
		auto it = Locks.find(key);
		if (it == Locks.end())
		{
			return false;
		}
		else
		{
			auto ptr = GetInternal(key);
			if (!ptr)
			{
				return false;
			}
			++Locks[key].Count;
			out = { key, ptr, this };
			return true;
		}
	}

	bool ConfigOverlord::GetLifetime(Key key, SharedLifetimeLock &out) noexcept
	{
		auto it = Locks.find(key);
		if (it == Locks.end())
		{
			auto ptr = GetInternal(key);
			Locks[key] = { Exclusive, 1 };
			out = { key, ptr, this };
			return false;
		}
		else
		{
			auto ptr = GetInternal(key);
			if (!ptr)
			{
				out = nullptr;
				return false;
			}
			return true;
		}
	}

	SharedLifetimeLock ConfigOverlord::StartLifetime(Key key)
	{
		auto it = Locks.find(key);
		if (it != Locks.end())
			throw IConfigThrone::AlreadyControlled();

		auto ptr = GetInternal(key);
		if (!ptr)
			return nullptr;
		Locks[key] = { Lifetime, 1 };
		return { key, ptr, this };
	}

	bool ConfigOverlord::RegisterListener(IChangeListener *listener)
	{
		Key key = listener->GetKey();

		if (Locks.find(key) != Locks.end())
		{
			DWARNING("Not registering listener for locked variable");
			return false;
		}

		Listeners[key].emplace_back(listener);
		return true;
	}

	size_t ConfigOverlord::Count(Key key)
	{
		DINFO("Count function not implemented");
		return 0ull;
	}

	bool ConfigOverlord::Has(Key key)
	{
		return GetInternal(key);
	}

	std::unique_ptr<IConfigThrone> GetDefaultConfigThrone()
	{
		return std::make_unique<ConfigOverlord>();
	}

}