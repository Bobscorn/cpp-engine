#pragma once

#include <string>

#include <memory>

#include <any>
#include <functional>
#include <assert.h>
#include <exception>
#include <typeinfo>

namespace Config
{
	enum ContainerType
	{
		NoContainer = -1,
		Unknown = -1,
		HashMap = 0,
		FastAccess = 0,
		List = 1,
		FastInsert = 1,
		DoubleList = 2,
		FastInsertRemove = 2,
		Vector = 3,
		FastLowMemAccess = 3,
	};


	struct Key
	{
		std::string Name;
		ContainerType Container = NoContainer;
		size_t Index = 0ull; // Differentiate between keys in the same container and name

		Key() = default;
		Key(std::string name) : Name(name), Container(HashMap), Index(0ull) {}
		Key(std::string name, ContainerType container, size_t index = 0ull) : Name(name), Container(container), Index(index) {}
		Key(std::string name, size_t index) : Name(name), Container(HashMap), Index(index) {}

		inline bool operator==(const Key &other) const
		{
			return Container == other.Container && Index == other.Index && Name == other.Name;
		}
	};

	struct KeyC
	{
		const char* Name = "";
		ContainerType Container;
		size_t Index = 0ull;

		constexpr KeyC(const char* name) : Name(name), Container(HashMap), Index(0ull) {}
		constexpr KeyC(const char* name, ContainerType container, size_t index = 0ull) : Name(name), Container(container), Index(0ull) {}
		constexpr KeyC(const char* name, size_t index) : Name(name), Container(HashMap), Index(index) {}

		inline operator Key() const { return { Name, Container, Index }; }
	};
	
	extern std::any DeadVar;

	/*
	template<class T>
	struct LockHome;

	template<class T>
	struct ILockDestroyer
	{
		virtual void DestroyHomeLock(LockHome<T> *lock) = 0;
	};
	
	template<class T>
	struct LockHome
	{
	private:
		LockHome();
		~LockHome();
	public:
		T m_Data;

		static void CreateLockHome();
	};

	template<class T>
	struct LockAway
	{
	private:
		LockHome *m_Home;
		ILockDestroyer *m_Destroyer;
		LockAway(LockHome &home, ILockDestroyer &destroy);
	public:
		LockAway(LockAway &&other) : m_Home(other.m_Home), m_Destroyer(other.m_Destroyer) { other.m_Home = nullptr; other.m_Destroyer = nullptr; }
		~LockAway() { if (m_Destroyer) m_Destroyer->DestroyHomeLock(m_Home); else assert(!m_Home); }
	};*/


	class IConfigThrone;

	// Exclusive locks block access to a variable unless another Exclusive lock or Shared lock has locked that variable
	// Does not block read access (yet?)
	struct ExclusiveVariableLock
	{
	private:
		Key m_Key;
		std::any *m_Variable = nullptr;
		IConfigThrone *m_Overlord = nullptr;
	public:
		ExclusiveVariableLock(std::nullptr_t) : m_Key(), m_Variable(nullptr), m_Overlord(nullptr) {}
		ExclusiveVariableLock(Key key, std::any *var, IConfigThrone *overlord) : m_Key(key), m_Variable(var), m_Overlord(overlord) {}
		ExclusiveVariableLock(const ExclusiveVariableLock &other) = delete;
		ExclusiveVariableLock(ExclusiveVariableLock &&other) : m_Key(other.m_Key), m_Overlord(other.m_Overlord), m_Variable(other.m_Variable) {}
		~ExclusiveVariableLock();

		ExclusiveVariableLock &operator=(const ExclusiveVariableLock &other) = delete;
		ExclusiveVariableLock &operator=(ExclusiveVariableLock &&other);

		inline bool Alive() const { return m_Variable && m_Variable->has_value(); }

		void Reset();

		inline void Set(const std::any &val) { if (m_Variable) *m_Variable = val; }

		inline operator std::any &() { if (m_Variable) return *m_Variable; else return DeadVar; }
	};

	// Shared locks do block access to a variable unless they get a SharedVariableLock on it
	// Shared locks can be blocked by an existing Exclusive lock, however Exclusive locks can be blocked by existing Shared locks
	// No lock can block read access to a variable
	// Mainly because these aren't proper locks and don't have any fancy 'lemme finish with this variable' method
	struct SharedVariableLock
	{
	private:
		Key m_Key;
		std::any *m_Variable;
		IConfigThrone *m_Overlord = nullptr;
	public:
		SharedVariableLock(std::nullptr_t) : m_Key(), m_Variable(nullptr), m_Overlord(nullptr) {}
		SharedVariableLock(Key key, std::any *var, IConfigThrone *overlord) : m_Key(key), m_Variable(var), m_Overlord(overlord) {}
		SharedVariableLock(const SharedVariableLock &other);
		SharedVariableLock(SharedVariableLock &&other) : m_Key(other.m_Key), m_Variable(other.m_Variable), m_Overlord(other.m_Overlord) { other.m_Key = {}; other.m_Variable = nullptr; other.m_Overlord = nullptr; }
		~SharedVariableLock();

		SharedVariableLock &operator=(const SharedVariableLock &other);
		SharedVariableLock &operator=(SharedVariableLock &&other);

		inline bool Alive() const { return m_Variable && m_Variable->has_value(); }

		void Reset();

		inline void Set(const std::any &val) { if (m_Variable) *m_Variable = val; }

		inline operator std::any &() { if (m_Variable) return *m_Variable; else return DeadVar; }

	};

	struct SharedLifetimeLock
	{
	private:
		Key m_Key;
		std::any *m_Variable;
		IConfigThrone *m_Overlord = nullptr;
	public:
		SharedLifetimeLock(std::nullptr_t) : m_Key(), m_Variable(nullptr), m_Overlord(nullptr) {}
		SharedLifetimeLock(Key key, std::any *var, IConfigThrone *overlord) : m_Key(key), m_Variable(var), m_Overlord(overlord) {}
		SharedLifetimeLock(const SharedLifetimeLock &other);
		SharedLifetimeLock(SharedLifetimeLock &&other) : m_Key(other.m_Key), m_Variable(other.m_Variable), m_Overlord(other.m_Overlord) { other.m_Key = {}; other.m_Variable = nullptr; other.m_Overlord = nullptr; }
		~SharedLifetimeLock();

		SharedLifetimeLock &operator=(const SharedLifetimeLock &other);
		SharedLifetimeLock &operator=(SharedLifetimeLock &&other);

		inline bool Alive() const { return m_Variable && m_Variable->has_value(); }

		void Reset();

		inline void Set(const std::any &val) noexcept { if (m_Variable) *m_Variable = val; }

		inline operator std::any &() noexcept { if (m_Variable) return *m_Variable; else return DeadVar; }
		inline operator bool() const noexcept { return Alive(); }
		inline const std::any *Get() const noexcept { return m_Variable; }
	};

	struct CouldNotFindException : std::exception
	{
		inline virtual const char *what() const noexcept override { return "Could not find given variable in an exception throwing search"; }
	};

	struct LockedException : std::exception
	{
		inline virtual const char *what() const noexcept override { return "Could not establish lock, or modify variable because another lock blocked access"; }
	};

	struct IChangeListener
	{
		IChangeListener() = default;
		IChangeListener(Key key) : m_Key(key) {}
		IChangeListener(IChangeListener &&other) = delete;
		virtual ~IChangeListener() { Reset(); }

		virtual void VariableChanged(const std::any &newval) = 0;
		inline Key GetKey() const noexcept { return m_Key; }
	protected:
		friend class IConfigThrone;

		void Reset();

	private:
		Key m_Key;
		IConfigThrone *m_Overlord = nullptr;

		inline void Release() { m_Overlord = nullptr; }
	};

	class IConfigThrone
	{
	protected:
		friend struct ExclusiveVariableLock;
		friend struct SharedVariableLock;
		friend struct SharedLifetimeLock;
		friend struct IChangeListener;

		virtual void RemoveSharedLock(Key key) noexcept = 0;
		virtual void RemoveExclusiveLock(Key key) noexcept = 0;
		virtual void RemoveLifetimeLock(Key key) noexcept = 0;
		virtual void RemoveListener(IChangeListener *listener) = 0;

		inline void ReleaseListener(IChangeListener *listen) { listen->Release(); }
	public:

		virtual void Set(Key key, const std::any &val) = 0;
		virtual void SafeSet(Key key, const std::any &val) noexcept = 0;

		virtual const std::any &Get(Key key) const = 0;
		virtual const std::any *GetN(Key key) const noexcept = 0;

		virtual SharedVariableLock GetShared(Key key) noexcept = 0;
		virtual ExclusiveVariableLock GetExclusive(Key key) noexcept = 0;
		virtual bool GetLifetimeExisting(Key key, SharedLifetimeLock &out) noexcept = 0; // Should not modify out parameter if there is no existing key (returns false), will modify out parameter if it does exist (returns true)
		virtual bool GetLifetime(Key key, SharedLifetimeLock &out) noexcept = 0; // Returns whether the key existed before this call, will create targetted entry if did not, out will always be modified
		virtual SharedLifetimeLock StartLifetime(Key key) = 0; // May (should) throw exception if key already has a lifetime lock on it

		virtual bool RegisterListener(IChangeListener *listener) = 0;

		struct AlreadyControlled : std::exception
		{
			inline virtual const char *what() const noexcept override { return "Specified Key already has a lifetime lock on it!"; }
		};

		virtual size_t Count(Key key) = 0;

		inline virtual bool Has(Key key) { return Count(key); }
		virtual bool HasType(Key key, const std::type_info &type);
	};

	
	struct DefaultChangeListener : IChangeListener
	{
		DefaultChangeListener() = default;
		DefaultChangeListener(Key key) : IChangeListener(key) {}

		inline virtual void VariableChanged(const std::any &newval) override { (void)newval; m_Changed = true; }

		inline bool GetChange() { auto tmp = m_Changed; m_Changed = false; return tmp; } // Resets to false when called
	private:

		bool m_Changed = false;
	};

	std::unique_ptr<IConfigThrone> GetDefaultConfigThrone();


	struct DefaultConfigValues
	{
		void operator()(IConfigThrone *config) const;
	};
}