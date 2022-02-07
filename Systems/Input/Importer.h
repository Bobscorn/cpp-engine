#pragma once

#include "Helpers/DebugHelper.h"

#include <string>
#include <set>
#include <vector>
#include <stack>
#include <map>
#include <memory>
#include <unordered_map>
#include <initializer_list>

namespace Importing
{
	typedef unsigned int UINT;
	
	inline bool IsWhiteSpace(wchar_t a) { return (a == L' ' || a == L'\t' || a == L'\n'); }

	class IImporter
	{

	public:
		virtual ~IImporter() {};

		virtual bool Import(Stringy FromFile) = 0;

		virtual operator bool() = 0;
	};

	class IExporter
	{

	public:
		virtual ~IExporter() {};

		virtual bool Export(Stringy ToFile) = 0;

		virtual operator bool() = 0;
	};

	struct XMLKey
	{
		Stringy Name;
		size_t id;

		inline bool operator<(const XMLKey & other) const { return (Name < other.Name || id < other.id); }
		inline bool operator==(const XMLKey & other) const { return (Name == other.Name && id == other.id); }

		struct equal
		{
			inline bool operator() (const XMLKey & a, const XMLKey & b) const { return a == b; }
		};
	};
}

namespace std
{
	template<>
	struct hash<Importing::XMLKey>
	{
		size_t operator() (const Importing::XMLKey& e) const
		{
			std::hash<Stringy> hashy;
			return (hashy(e.Name) ^ e.id);
		}
	};
}

namespace Importing
{
	template<class T>
	struct bob
	{
		bob() {}
		bob(const bob<T>& other) : _ptr(std::make_unique<T>(*other._ptr)) {}
		bob(bob<T> &&other) : _ptr(std::move(other._ptr)) { other._ptr.reset(); }
		bob(T *ptr) : _ptr(ptr) {}

		inline bob<T>& operator=(const bob<T>& other)
		{
			this->_ptr = std::make_unique<T>(*other._ptr);
			return *this;
		}
		inline bob<T>& operator=(bob<T>&& other)
		{
			this->_ptr = std::move(other._ptr);
			other._ptr = nullptr;
		}

		inline T& operator*() { return *_ptr; }
		inline const T& operator*() const { return *_ptr; }
		inline T* operator->() const noexcept { return _ptr.get(); }
		inline T* get() noexcept { return _ptr.get(); }
		inline const T* get() const noexcept { return _ptr.get(); }


	protected:
		std::unique_ptr<T> _ptr;
	};

	template<class T, class ...Args>
	bob<T> make_bob(Args... args)
	{
		return bob<T>(new T(std::forward<Args>(args)...));
	}


	struct XMLElement
	{
		friend struct XMLExportData;
	protected:
		Stringy name;
		std::vector<XMLKey> ChildKeys;
		std::unordered_map<XMLKey, bob<XMLElement>> Children;

	public:
		inline Stringy &Name() { return name; }
		inline Stringy &Type() { return name; }
		inline const Stringy &Name() const { return name; }
		inline const Stringy &Type() const { return name; }

		inline std::unordered_map<XMLKey, bob<XMLElement>> &GetChildren() { return Children; }
		inline std::unordered_map<XMLKey, bob<XMLElement>> const &GetChildren() const { return Children; }

		std::map<Stringy, Stringy> Properties;

		XMLElement &push_back(Stringy name);

		XMLElement *Parent;

		XMLElement(Stringy type = "", XMLElement *Parent = nullptr) : name(type), Parent(Parent) {};
		XMLElement(const XMLElement& other) : name(other.name), Parent(other.Parent), Children(other.Children), Properties(other.Properties) {}
		XMLElement(XMLElement&& other) noexcept
		{
			name = other.name;
			other.name.clear();

			Parent = other.Parent;
			other.Parent = nullptr;

			Children = std::move(other.Children);
			Properties = std::move(other.Properties);

			other.Children.clear();
			other.Properties.clear();
		}

		XMLElement& operator=(const XMLElement& other)
		{
			this->name = other.name;
			this->Parent = other.Parent;
			this->Children = other.Children;
			this->Properties = other.Properties;
			return *this;
		}

		XMLElement& operator=(XMLElement&& other) noexcept
		{
			name = other.name;
			other.name.clear();

			Parent = other.Parent;
			other.Parent = nullptr;

			Children = std::move(other.Children);
			Properties = std::move(other.Properties);

			other.Children.clear();
			other.Properties.clear();

			return *this;
		}

		inline bool operator<(const XMLElement& other) { return this->name < other.name; }
	};

	struct XMLThing : public XMLElement
	{
	private:
		using XMLElement::Parent;
		using XMLElement::Name;
		using XMLElement::Type;
		using XMLElement::push_back;
	public:

		XMLElement & push_back(Stringy name);

		XMLElement *Find(Stringy Name, std::initializer_list<std::pair<Stringy, Stringy>> Properties = {});
		std::vector<XMLElement*> FindAll(Stringy Name, std::initializer_list<std::pair<Stringy, Stringy>> Properties = {});
	};

	template<class R, class M>
	struct FindClass
	{
		static R *Find(Stringy Name, std::initializer_list<std::pair<Stringy, Stringy>>, M &elements);
	};

	const auto QP = std::make_pair<Stringy, Stringy>; // Quick shortcut for pair of wstrings, works better than an initializer list like { "bob", "dylan" }
	inline XMLElement *Find(Stringy Name, std::initializer_list<std::pair<Stringy, Stringy>> Properties, std::unordered_map<XMLKey, bob<XMLElement>> &Elements)
	{
		return FindClass<XMLElement, std::unordered_map<XMLKey, bob<XMLElement>>>::Find(Name, Properties, Elements);
	}
	inline const XMLElement *Find(Stringy Name, std::initializer_list<std::pair<Stringy, Stringy>> Properties, const std::unordered_map<XMLKey, bob<XMLElement>> &Elements)
	{
		return FindClass<const XMLElement, const std::unordered_map<XMLKey, bob<XMLElement>>>::Find(Name, Properties, Elements);
	}

	std::vector<XMLElement*> FindAll(Stringy Name, std::initializer_list<std::pair<Stringy, Stringy>> Properties, std::unordered_map<XMLKey, bob<XMLElement>> &Elements);

	class BasicXMLImporter : IImporter
	{
		friend struct XMLExportData;
	protected:
		std::unique_ptr<XMLThing> Output;

		std::stack<XMLElement*> Elems;
		bool midTag = false;
		bool startTag = false;
		bool endTag = false;
		bool midText = false;
		bool midPropertyName = false;
		bool midPropertyValue = false;
		bool PropertyValueHasQuotes = false;
		Stringy Text;
		Stringy ElementName;
		Stringy EndTagName;
		Stringy PropertyName;
		Stringy PropertyValue;

		size_t LineNumber = 0ull;
		size_t IndexOffset = 0ull;

		void Reset();
		void CreateElement();
		void AddProperty();
		void FinishElement();
		void RemoveEndlessTag(XMLElement * from);
		XMLElement *CurrentElement(); // Throws error if there is none

		bool SkipComments(Stringy *data, size_t& i); // Returns if there was a comment
		Stringy LineAndChar(size_t index);
		void TrackLineAndChar(wchar_t c, size_t index);
		Stringy LineAndCharAlt(Stringy& data, size_t index);

		bool IsSupportedChar(wchar_t c);

	public:
		BasicXMLImporter(Stringy openfile = "");
		virtual bool Import(Stringy FromFile) override;
		inline const XMLThing& GetOutput() const { return *Output; };
		inline XMLThing& GetOutput() { if (!Output) Output = std::make_unique<XMLThing>(); return *Output; }

		operator bool() override { return Output.get(); }
	};


	struct ExportElement
	{
		Stringy Name;
		std::vector<std::pair<Stringy, Stringy>> Properties;
		std::vector<ExportElement> Children;
	};

	struct XMLExportData
	{
	private:
		ExportElement ConvertElement(XMLElement *el);
		std::vector<ExportElement> CreateChildren(XMLElement *el);

	public:
		std::vector<ExportElement> RootElements;

		XMLExportData(const BasicXMLImporter& imp);
		XMLExportData(const std::vector<ExportElement>& e = {});
	};

	class XMLExporter : public IExporter
	{
		void ExportElement(const ExportElement *el, std::ofstream *out, UINT indent = 0u, bool ParentLine = false);

		XMLExportData data;
	public:
		XMLExporter(XMLExportData data = XMLExportData()) : data(data) {};



		static bool Export(XMLExportData ass, Stringy ToFile);
		bool Export(Stringy ToFile) override;

		inline operator bool() { return true; }
	};

	class GSpaceImporter
	{
		const Stringy ShapeType = "shape";


	public:
		bool Import(Stringy FromFile);
	};
}