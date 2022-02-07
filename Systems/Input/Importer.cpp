//#include "Thing.h"
#include "Importer.h"

#include "Helpers/DebugHelper.h"

#include "Helpers/StringHelper.h"

#include <fstream>
#ifdef __linux__
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <stack>

using StringHelper::IsWhiteSpace;
using StringHelper::IsDouble;

Importing::XMLElement & Importing::XMLThing::push_back(Stringy ename)
{
	auto it = Children.find({ ename, 0ull });
	if (it == Children.end())
	{
		ChildKeys.push_back({ ename, 0ull });
		Children.emplace(Importing::XMLKey{ename, 0ull}, Importing::make_bob<XMLElement>(ename));
		return *Children[{ename, 0ull}];
	}
	else
	{
		size_t free_id = 0ull;
		do
		{
			++free_id;
			it = Children.find({ ename, free_id });
		} while (it != Children.end());
		ChildKeys.push_back({ ename, free_id });
		Children.emplace(Importing::XMLKey{ ename, free_id }, Importing::make_bob<XMLElement>(ename));
		return *Children[{ename, free_id}];
	}
}

Importing::XMLElement * Importing::XMLThing::Find(Stringy Name, std::initializer_list<std::pair<Stringy, Stringy>> props)
{
	return Importing::Find(Name, props, this->Children);
}

std::vector<Importing::XMLElement*> Importing::XMLThing::FindAll(Stringy Name, std::initializer_list<std::pair<Stringy, Stringy>> props)
{
	return Importing::FindAll(Name, props, this->Children);
}

template<class R, class M>
R * Importing::FindClass<R, M>::Find(Stringy Name, std::initializer_list<std::pair<Stringy, Stringy>> Properties, M &Elements)
{
	if (Properties.size())
	{
		size_t i = 0ull;
		while (true)
		{
			auto it = Elements.find({ Name, i++ });
			if (it == Elements.end())
				break;

			R &el = *it->second;
			bool HasAllProps = true;
			for (auto iter = Properties.begin(); iter != Properties.end(); iter++)
			{
				const Stringy& PropName = iter->first;
				const Stringy& PropVal = iter->second;
				if (PropVal == "Any")
				{
					if (el.Properties.find(PropName) == el.Properties.end())
					{
						HasAllProps = false;
						break;
					}
				}
				else if (PropVal == "AnyNumber")
				{
					auto ity = el.Properties.find(PropName);
					if (ity == el.Properties.end())
					{
						HasAllProps = false;
						break;
					}
					else
					{
						if (!IsDouble(ity->second, nullptr))
						{
							HasAllProps = false;
							break;
						}
					}
				}
				else
				{
					auto ity = el.Properties.find(PropName);
					if (ity == el.Properties.end())
					{
						HasAllProps = false;
						break;
					}
					else
					{
						if (ity->second != PropVal)
						{
							HasAllProps = false;
							break;
						}
					}
				}
			}
			if (HasAllProps)
				return &el;
		}		
	}
	else
	{
		auto it = Elements.find({ Name, 0ull });
		if (it != Elements.end())
			return it->second.get();
	}

	// Hasn't found it, try children
	for (auto& element : Elements)
	{
		if (R *pElement = Find(Name, Properties, element.second->GetChildren()))
			return pElement;
	}

	// Didn't find anywhere
	return nullptr;
}

std::vector<Importing::XMLElement*> Importing::FindAll(Stringy Name, std::initializer_list<std::pair<Stringy, Stringy>> Properties, std::unordered_map<XMLKey, bob<XMLElement>>& Elements)
{
	std::vector<XMLElement*> Elems;
	if (Properties.size())
	{
		size_t i = 0ull;
		std::unordered_map<XMLKey, bob<XMLElement>>::iterator it;
		for (; (it = Elements.find({ Name, i++ })) != Elements.end(); )
		{
			auto &el = it->second;
			bool HasAllProps = true;
			for (auto iter = Properties.begin(); iter != Properties.end(); iter++)
			{
				const Stringy& PropName = iter->first;
				const Stringy& PropVal = iter->second;
				if (PropVal == "Any")
				{
					if (el->Properties.find(PropName) == el->Properties.end())
					{
						HasAllProps = false;
						break;
					}
				}
				else if (PropVal == "AnyNumber")
				{
					auto ity = el->Properties.find(PropName);
					if (ity == el->Properties.end())
					{
						HasAllProps = false;
						break;
					}
					else
					{
						if (!IsDouble(ity->second, nullptr))
						{
							HasAllProps = false;
							break;
						}
					}
				}
				else
				{
					auto ity = el->Properties.find(PropName);
					if (ity == el->Properties.end())
					{
						HasAllProps = false;
						break;
					}
					else
					{
						if (ity->second != PropVal)
						{
							HasAllProps = false;
							break;
						}
					}
				}
			}
			if (HasAllProps)
				Elems.push_back(el.get());
		}
	}
	else
	{
		size_t i = 0ull;
		std::unordered_map<XMLKey, bob<XMLElement>>::iterator it = Elements.find({});
		for (; (it = Elements.find({ Name, i++ })) != Elements.end(); )
		{
			auto &element = it->second;
			Elems.push_back(element.get());
		}
	}

	// Add any found in children
	for (auto& element : Elements)
	{
		std::vector<XMLElement*> ass;
		if ((ass = FindAll(Name, Properties, element.second->GetChildren())).size())
			Elems.insert(Elems.end(), ass.begin(), ass.end());
	}

	return Elems;
}

void Importing::BasicXMLImporter::Reset()
{
	for (size_t i = Elems.size(); i-- > 0; )
		Elems.pop();

	midTag = false;
	startTag = false;
	endTag = false;
	midText = false;
	midPropertyName = false;
	midPropertyValue = false;
	PropertyValueHasQuotes = false;
	Text.clear();
	ElementName.clear();
	EndTagName.clear();
	PropertyName.clear();
	PropertyValue.clear();

	LineNumber = 0ull;
	IndexOffset = 0ull;
}

void Importing::BasicXMLImporter::CreateElement()
{
	if (Elems.empty())
	{
		Elems.push(&Output->push_back(ElementName));
	}
	else
	{
		Elems.push(&Elems.top()->push_back(ElementName));
	}
}

void Importing::BasicXMLImporter::AddProperty()
{
	Elems.top()->Properties[PropertyName] = PropertyValue;
	midPropertyValue = false;
	PropertyValueHasQuotes = false;
	PropertyName.clear();
	PropertyValue.clear();
}

void Importing::BasicXMLImporter::FinishElement()
{
	midTag = false;
	startTag = false;
	midText = false;
	midPropertyName = false;
	midPropertyValue = false;
	PropertyValueHasQuotes = false;
	ElementName.clear();
	PropertyName.clear();
	PropertyValue.clear();
}

void Importing::BasicXMLImporter::RemoveEndlessTag(XMLElement * from)
{
	auto begin = from->GetChildren().begin();
	for (auto it = from->GetChildren().begin(); it != from->GetChildren().end(); ++it)
	{
		if (it->second->Type() == "endless")
			from->GetChildren().erase(it->first);

		RemoveEndlessTag(it->second.get());
	}
}

Importing::XMLElement * Importing::BasicXMLImporter::CurrentElement()
{
	return (Elems.top());
}

bool Importing::BasicXMLImporter::SkipComments(Stringy *data, size_t & i)
{
	if (!data)
		return false;
	// '//' comment
	if (i + 1 < data->size() && (*data)[i] == L'/' && (*data)[i + 1] == L'/')
	{
		size_t numSkipped(0);
		// Skip until newline
		i += 2;
		for (; i < data->size(); ++i)
		{
			++numSkipped;
			TrackLineAndChar((*data)[i], i);
			if ((*data)[i] == L'\n')
				break;
			else
				continue;
		}
		DINFO("Parsing: Skipping " + std::to_string(numSkipped) + " characters as Comment");
		return true;
	}
	// '#' comment
	else if ((*data)[i] == L'#')
	{
		++i;
		size_t numSkipped(0);
		// Skip until newline
		for (; i < data->size(); ++i)
		{
			++numSkipped;
			TrackLineAndChar((*data)[i], i);
			if ((*data)[i] == L'\n')
				break;
		}

		DINFO("Parsing: Skipping " + std::to_string(numSkipped) + " characters as Comment");
		return true;
	}
	// <!-- comment -->
	else if (i + 3 < data->size() && (*data)[i] == L'<' && (*data)[i + 1] == L'!' && (*data)[i + 2] == L'-' && (*data)[i + 3] == L'-')
	{
		i += 4;
		size_t numSkipped(0);
		// Skip until '-->' found
		for (; i + 2 < data->size(); ++i)
		{
			++numSkipped;
			TrackLineAndChar((*data)[i], i);
			if ((*data)[i] == L'-' && (*data)[i + 1] == L'-' && (*data)[i + 2] == L'>')
				break;
		}

		DINFO("Parsing: Skipping " + std::to_string(numSkipped) + " characters as Comment");
		return true;
	}

	return false;
}

Stringy Importing::BasicXMLImporter::LineAndChar(size_t index)
{
	return (Stringy("Line: ") + std::to_string(LineNumber) + " at character: " + std::to_string(index - IndexOffset));
}

void Importing::BasicXMLImporter::TrackLineAndChar(wchar_t c, size_t index)
{
	if (c == L'\n')
	{
		++LineNumber;
		IndexOffset += index;
	}
}

Stringy Importing::BasicXMLImporter::LineAndCharAlt(Stringy & data, size_t index)
{
	size_t numNewLines(0ull);
	size_t lastNewLinePos(0ull);
	for (size_t i = 0; i < data.size() && i < index; ++i)
	{
		if (data[i] == L'\n')
		{
			++numNewLines;
			lastNewLinePos = i - numNewLines;
		}
	}

	return (Stringy("Line: ") + std::to_string(numNewLines) + ", char: " + std::to_string(index - lastNewLinePos));
}

bool Importing::BasicXMLImporter::IsSupportedChar(wchar_t c)
{
	return (isalnum(c) || c == L'.');
}

void Importing::XMLExporter::ExportElement(const Importing::ExportElement * el, std::ofstream * out, UINT indent, bool ParentLine)
{
	Stringy Indent;
	for (int i = indent; i-- > 0; )
		Indent += '\t';

	if (el->Name == "text")
	{
		bool HasOwnLine = !ParentLine;

		Stringy content = "";
		for (auto& prop : el->Properties)
			if (prop.first == "content")
			{
				content = prop.second;
				break;
			}

		if (HasOwnLine)
			*out << Indent << content << std::endl;
		else
			*out << ' ' << content;

	}
	else
	{
		*out << Indent << '<' << el->Name;

		for (auto& propy : el->Properties)
			*out << ' ' << propy.first << "=\"" << propy.second << '\"';

		bool NoChildren = el->Children.begin() == el->Children.end();

		if (NoChildren)
			*out << "/";
		*out << '>';

		// Don't put end tag on separate line if: no children, or: 1 child and it is text type
		bool SameLine = NoChildren || (el->Children.size() == 1 && el->Children.begin()->Name == "text");

		if (!SameLine)
			*out << std::endl;

		// Export children
		for (auto& child : el->Children)
			ExportElement(&child, out, indent + 1, SameLine);

		// then end it
		if (!SameLine)
			*out << Indent;

		if (el->Children.size())
			*out << "</" << el->Name << ">";

		*out << std::endl;
	}
}

Importing::BasicXMLImporter::BasicXMLImporter(Stringy openfile)
{
	if (openfile.size())
		Import(openfile);
}

bool Importing::BasicXMLImporter::Import(Stringy FromFile)
{
	Reset();

	DINFO("Importing xml file: '" + FromFile + "'");

	// Check if file exists and has something in it first

	std::ifstream fin(FromFile, std::ifstream::in);

	if (!fin.good())
	{
		DWARNING("Failed to import " + FromFile + " could not find or access");
		return false;
	}

	if (!Output)
		Output = std::make_unique<XMLThing>();

	Stringy data;
	fin.seekg(0, fin.end);
	size_t length = fin.tellg();
	data.resize(length);
	fin.seekg(0, fin.beg);
	fin.read(&data[0], length);
	fin.close();

	for (size_t i = 0; i < data.size(); ++i)
	{
		auto& c = data[i];
		if (c == L'\0')
			continue; // Skip all null characters

		TrackLineAndChar(c, i);
		if (SkipComments(&data, i))
			continue;

		if (!midTag && !midText && IsWhiteSpace(c))
			continue; // Skip whitespace when not processing anything

		if (c == L'<')
		{
			if (midTag)
			{
				DERROR("Parsing: 01 Unexpected '<' found at " + LineAndCharAlt(data, i));
				continue;
			}

			if (Text.size()) // Text tag needs closing
			{
				Elems.top()->Properties["content"] = Text;
				Text.clear();
				Elems.pop();
			}
			if (i + 1 < data.size() && data[i + 1] == L'/') // If is endtag '</'
			{
				endTag = true;
				++i; // Skip the '/'
				midText = false;
			}
			else
			{	// Not endtag
				midTag = true;
				startTag = true;
				midText = false;
			}
		}
		else if (c == L'>')
		{
			if (startTag)
			{
				// Create Element (it will have no properties, but may have children)
				CreateElement();
			}
			else if (midPropertyValue && !PropertyValueHasQuotes) // Property value being ended as the tag has been closed (ignored if inside quotes)
			{
				// Add Property to element
				AddProperty();
			}
			else if (midPropertyName)
			{
				DERROR("Parsing: 02 Tag '" + CurrentElement()->Name() + "' closed before giving property '" + PropertyName + "' a value at " + LineAndCharAlt(data, i));
			}

			if (!midText) // Close any unclosed elements, unless its a text
			{
				FinishElement();
			}
		}
		else if (endTag)
		{
			Stringy endtag;
			for (; i < data.size(); ++i)
			{
				auto& c2 = data[i];
				TrackLineAndChar(c2, i);
				if (c2 == L'>')
				{
					break;
				}
				if (!IsSupportedChar(c2))
				{
					DERROR("Parsing: 03 unexpected '" + c2 + "' found at " + LineAndCharAlt(data, i));
					continue;
				}
				endtag.push_back(c2);
			}
			if (Elems.empty())
			{
				DERROR("Parsing: 05 unexpected break tag '</" + endtag + "> found at " + LineAndCharAlt(data, i));
				continue;
			}
			else if (endtag == Elems.top()->Type())
			{
				Elems.pop();
			}
			else
			{
				DERROR("Parsing: 06 unexpected break tag '</" + endtag + ">' found at " + LineAndCharAlt(data, i));
			}
			endTag = false;
		}
		else // Not '<' or '>' or an endtag
		{
			if (!midTag)
			{
				if (!midText) // Beginning of text element
				{
					// Create text element
					if (Elems.empty())
					{
						Elems.push(&Output->push_back("text"));
					}
					else
					{
						Elems.push(&Elems.top()->push_back("text"));
					}
				}
				Text.push_back(c);
				midText = true;
			}
			else
			{	// Mid Tag
				if (c == L'/' && i + 1 < data.size() && data[i + 1] == L'>') // Short closing of tag
				{
					DINFO("short tag found");
					++i; // Skip the '>' that we checked ahead for
					if (startTag)
					{
						if (Elems.empty())
						{
							Output->push_back(ElementName);
						}
						else
						{
							Elems.top()->push_back(ElementName);
						}
					}
					if (midTag)
					{
						if (midPropertyValue && PropertyValueHasQuotes)
							continue;

						if (startTag)
							CreateElement();
						Elems.pop();
					}
					FinishElement();
				}
				else if (startTag)
				{
					if (IsWhiteSpace(c))
					{
						// We have name, create element, then prepare for properties
						CreateElement();
						startTag = false;
					}
					else if (!IsSupportedChar(c))
					{
						DERROR("Parsing: 07 unexpected '" + c + "' found, ignoring at " + LineAndCharAlt(data, i));
					}
					else
					{
						ElementName.push_back(c);
					}
				}
				else // Property of tag
				{
					if (midPropertyName)
					{
						if (c == L'=')
						{
							if (PropertyName.empty())
							{
								DERROR("Parsing: 08 unexpected '=' found before any given property name at " + LineAndCharAlt(data, i));
								PropertyName = "unknown";
							}
							// Switch to assigning property value
							midPropertyName = false;
							midPropertyValue = true;
							if (i + 1 < data.size() && data[i + 1] == L'\"')
							{
								PropertyValueHasQuotes = true;
								++i; // Skip the quote
							}
							else
							{
								PropertyValueHasQuotes = false;
							}
						}
						else if (IsWhiteSpace(c))
						{
							DERROR("Parsing: 09 no value given to property at " + LineAndCharAlt(data, i));
							midPropertyName = false;
							PropertyName.clear();
						}
						else if (!IsSupportedChar(c))
						{
							DERROR("Parsing: 10 unexpected '" + c + "' found at " + LineAndCharAlt(data, i));
						}
						else
						{
							PropertyName.push_back(c);
						}
					}
					else if (midPropertyValue)
					{
						if (PropertyValueHasQuotes)
						{
							if (c == L'\"')
							{
								// Add Property to element
								AddProperty();
							}
							else
							{
								PropertyValue.push_back(c);
							}
						}
						else if (IsWhiteSpace(c))
						{
							// Add Property to element
							AddProperty();
						}
						else if (!IsSupportedChar(c))
						{
							DERROR("Parsing: 11 unexpected character '" + c + "' found when assigning value to Property at " + LineAndCharAlt(data, i));
						}
						else
						{
							PropertyValue.push_back(c);
						}
					}
					else if (IsWhiteSpace(c))
						continue;
					else if (!IsSupportedChar(c))
					{
						DERROR("Parse: 12 unexpected '" + c + "' at " + LineAndCharAlt(data, i));
					}
					else
					{
						midPropertyName = true;
						PropertyName.push_back(c);
					}
				}

			}
		}
	}

	if (midTag)
	{
		DERROR("Parsing: End of File reached before last tag was closed");
	}

	if (Elems.size() == 1 && Elems.top()->Name() == "text")
	{
		Elems.top()->Properties["content"] = Text;
		DINFO("Parsing: Closing text tag after parsing completion");
		Elems.pop();
	}

	if (Elems.size())
	{
		while (Elems.size())
		{
			auto leak = Elems.top();
			DERROR("Parsing: 13: Leaked tag '" + leak->Name() + "' found");
			Elems.pop();
		}
	}

	if (Output->GetChildren().empty())
	{
		Output = nullptr;
		return false;
	}

	return true;
}

bool Importing::XMLExporter::Export(Stringy ToFile)
{
	if (data.RootElements.empty())
	{
		DWARNING("No elements to export");
		return false;
	}

	std::ofstream fout(ToFile.c_str());

	if (!fout.good())
	{
		DERROR(" Specified File '" + ToFile + "' is inaccessible");
		return false;
	}

	for (auto& rootel : data.RootElements)
	{
		ExportElement(&rootel, &fout);
	}

	fout.close();

	return true;
}

Importing::XMLElement & Importing::XMLElement::push_back(Stringy ename)
{
	auto it = Children.find({ ename, 0ull });
	if (it == Children.end())
	{
		ChildKeys.push_back({ ename, 0ull });
		Children.emplace(Importing::XMLKey{ ename, 0ull }, Importing::make_bob<XMLElement>(ename, this));
		return *Children[{ename, 0ull}];
	}
	else
	{
		size_t free_id = 0ull;
		do
		{
			++free_id;
			it = Children.find({ ename, free_id });
		} while (it != Children.end());
		ChildKeys.push_back({ ename, free_id });
		Children.emplace(Importing::XMLKey{ ename, free_id }, Importing::make_bob<XMLElement>(ename, this));
		return *Children[{ename, free_id}];
	}
}
Importing::ExportElement Importing::XMLExportData::ConvertElement(XMLElement * el)
{
	ExportElement e;
	e.Name = el->Name();
	if (el->Properties.size())
	{
		std::vector<std::pair<Stringy, Stringy>> properties;
		properties.resize(el->Properties.size());

		for (auto it = el->Properties.begin(); it != el->Properties.end(); ++it)
		{
			auto &prop = properties[std::distance(el->Properties.begin(), it)];
			prop.first = it->first;
			prop.second = it->second;
		}

		e.Properties = std::move(properties);
	}
	return e;
}

std::vector<Importing::ExportElement> Importing::XMLExportData::CreateChildren(XMLElement * el)
{
	if (el->Children.size())
	{
		std::vector<Importing::ExportElement> children;
		children.resize(el->Children.size());

		for (size_t i = el->ChildKeys.size(); i-- > 0; )
		{
			children[i] = ConvertElement(el->Children[el->ChildKeys[i]].get());
			children[i].Children = CreateChildren(el->Children[el->ChildKeys[i]].get());
		}

		return children;
	}
	else
		return std::vector<ExportElement>();
}

Importing::XMLExportData::XMLExportData(const BasicXMLImporter & imp)
{
	if (!imp.Output)
		return;

	for (auto& e : imp.Output->ChildKeys)
	{
		RootElements.emplace_back(ConvertElement(imp.Output->Children[e].get()));
		RootElements.back().Children = CreateChildren(imp.Output->Children[e].get());
	}
}

Importing::XMLExportData::XMLExportData(const std::vector<ExportElement>& e)
{
	RootElements = e;
}

template struct Importing::FindClass<Importing::XMLElement, std::unordered_map<Importing::XMLKey, Importing::bob<Importing::XMLElement>>>;
template struct Importing::FindClass<const Importing::XMLElement, const std::unordered_map<Importing::XMLKey, Importing::bob<Importing::XMLElement>>>;