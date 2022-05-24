#include "Material.h"

#include <cstring>
#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "Program.h"

#include "Helpers/StringHelper.h"

namespace Drawing
{
	std::unique_ptr<MaterialStore> MaterialStore::_instance = nullptr;
	MaterialStore::Accessor MaterialStore::Instance{};

	template<class T>
	T MaterialProperty::as() const
	{
		return *reinterpret_cast<const T*>(data.data());
	}

	template<class T>
	void MaterialProperty::set_from(T val)
	{
		(*reinterpret_cast<T*>(data.data())) = val;
	}

	template void MaterialProperty::set_from<floaty4>(floaty4 val);
	template void MaterialProperty::set_from<floaty3>(floaty3 val);
	template void MaterialProperty::set_from<floaty2>(floaty2 val);
	template void MaterialProperty::set_from<float>(float val);
	template void MaterialProperty::set_from<inty4>(inty4 val);
	template void MaterialProperty::set_from<inty3>(inty3 val);
	template void MaterialProperty::set_from<inty2>(inty2 val);
	template void MaterialProperty::set_from<int>(int val);
	template floaty4 MaterialProperty::as<floaty4>() const;
	template floaty3 MaterialProperty::as<floaty3>() const;
	template floaty2 MaterialProperty::as<floaty2>() const;
	template float MaterialProperty::as<float>() const;
	template inty4 MaterialProperty::as<inty4>() const;
	template inty3 MaterialProperty::as<inty3>() const;
	template inty2 MaterialProperty::as<inty2>() const;
	template int MaterialProperty::as<int>() const;

	std::vector<char> Material::ToByteForm()
	{
		auto prog = Program.GetProgram();
		if (!prog)
			return std::vector<char>{};
		return ConvertBytesViaDescription(*this, prog->GetMatDesc());
	}

	std::vector<char> Material::ConvertBytesViaDescription(const Material& mat, const MaterialDescription& desc)
	{
		std::vector<char> bytes{ (size_t)desc.CalculateMaterialByteSize(), (char)0, std::allocator<char>() };
		int offset = 0;
		for (int i = 0; i < desc.Properties.size(); ++i)
		{
			auto& prop = desc.Properties[i];
			int size = MatSizeToInt(prop.size) * 4;
			if (bytes.size() < offset + size)
				bytes.resize(offset + size);
			auto it = mat.Properties.find(prop.name);
			
			auto value = (it != mat.Properties.end() ? it->second.data : prop.data);

			memcpy(bytes.data() + offset, &value[0], size);
			offset += size;
		}
		bytes.resize(offset);

		// Insert padding (should this be a material parameter?)
		int extra = bytes.size() % 16;
		if (extra)
			//bytes.insert(bytes.end(), (size_t)16 - extra, (char)0);
			DWARNING("Material is not a multiple of 16!");

		return bytes;
	}

	Material SerializableMaterial::ToMaterial() const
	{
		Material mat;
		mat.Program = ProgramReference(ProgramName);
		for (int i = 0; i < Textures.size(); ++i)
			mat.Textures[Textures[i].first] = Textures[i].second;
		for (int i = 0; i < Properties.size(); ++i)
			mat.Properties[Properties[i].name] = Properties[i];
		return mat;
	}

	void MaterialStore::LoadMaterial(const SerializableMaterial& smat)
	{
		if (_store.find(smat.MaterialName) != _store.end())
			return;

		std::shared_ptr<Material> mat = std::make_shared<Material>();

		mat->Program = ProgramReference(smat.ProgramName);
		
		for (int i = 0; i < smat.Properties.size(); ++i)
		{
			auto& prop = smat.Properties[i];
			mat->Properties[prop.name] = prop;
		}

		for (int i = 0; i < smat.Textures.size(); ++i)
		{
			auto& tex = smat.Textures[i];
			mat->Textures[tex.first] = TextureReference(tex.second);
		}

		_store.emplace(smat.MaterialName, std::move(mat));
	}

	void MaterialStore::LoadMaterial(std::string fileName)
	{
		if (std::filesystem::exists(fileName))
		{
			try
			{
				DINFO("Loading material from file '" + fileName + "'");

				auto yaml = YAML::LoadFile(fileName);

				if (!yaml["name"])
				{
					DWARNING("File '" + fileName + "' being parsed as a material, no name entry found!");
					return;
				}

				SerializableMaterial mat;
				mat.MaterialName = yaml["name"].as<std::string>();
				DINFO("Material name '" + mat.MaterialName + "'");

				if (!yaml["program"])
				{
					DWARNING("File '" + fileName + "' being parsed as a material, no program entry found!");
					return;
				}

				mat.ProgramName = yaml["program"].as<std::string>();

				if (yaml["textures"] && yaml["textures"].IsMap())
				{
					auto tex = yaml["textures"];
					for (auto it = tex.begin(); it != tex.end(); ++it)
					{
						mat.Textures.emplace_back(std::make_pair(it->first.as<std::string>(), it->second.as<std::string>()));
					}
				}

				auto propertiesNode = yaml["properties"];
				if (propertiesNode && propertiesNode.IsMap())
				{
					auto get_vec_comp = [](YAML::Node& n, const std::string& opt1, const std::string& opt2, const std::string& opt3, auto default_value)
					{
						if (n[opt1])
							return n[opt1].as<decltype(default_value)>();
						if (n[opt2])
							return n[opt2].as<decltype(default_value)>();
						if (opt3.size() && n[opt3])
							return n[opt3].as<decltype(default_value)>();
						return default_value;
					};

					for (auto it = propertiesNode.begin(); it != propertiesNode.end(); ++it)
					{
						MaterialProperty prop{};
						prop.name = it->first.Scalar();
						prop.type = PropertyType::FLOAT;

						auto propType = it->second["type"];
						if (propType && propType.IsScalar() && propType.Scalar() == "int" || propType.Scalar() == "float")
							prop.type = (propType.Scalar() == "int" ? PropertyType::INT : PropertyType::FLOAT);

						if (prop.type == PropertyType::FLOAT)
						{
							auto x = get_vec_comp(it->second, "x", "r", "u", 0.f);
							auto y = get_vec_comp(it->second, "y", "g", "v", 0.f);
							auto z = get_vec_comp(it->second, "z", "b", "", 0.f);
							auto w = get_vec_comp(it->second, "w", "a", "", 0.f);
							prop.set_from(floaty4{ x, y, z, w });
						}
						else
						{
							auto x = get_vec_comp(it->second, "x", "r", "u", (int)0);
							auto y = get_vec_comp(it->second, "y", "g", "v", (int)0);
							auto z = get_vec_comp(it->second, "z", "b", "", (int)0);
							auto w = get_vec_comp(it->second, "w", "a", "", (int)0);
							prop.set_from(inty4{ x, y, z, w });
						}

						auto sizeNode = it->second["size"];
						int size = 0;
						if (!sizeNode || !sizeNode.IsScalar())
						{
							DERROR("When Processing '" + fileName + "' as a Material (" + mat.MaterialName + "): Found property '" + prop.name + "' with malformed size tag!");
							continue;
						}

						if (!StringHelper::IfINT(sizeNode.Scalar(), &size) || (size != 1 && size != 2 && size != 3 && size != 4))
						{
							DERROR("When Processing '" + fileName + "' as a Material (" + mat.MaterialName + "): Found property with invalid size '" + sizeNode.Scalar() + "' expected either 1, 2, 3, or 4");
							continue;
						}

						prop.size = IntToMatSize(size);

						mat.Properties.emplace_back(std::move(prop));
					}
				}

				LoadMaterial(mat);
			}
			catch (YAML::Exception& e)
			{
				DERROR("Failed to process material file: '" + fileName + "' with YAML Exception: " + e.what());
			}
		}
	}

	void MaterialStore::LoadMaterialDirectory(std::string directory)
	{
		if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory))
		{
			for (auto& entry : std::filesystem::directory_iterator{ directory })
			{
				if (entry.exists() && entry.is_regular_file())
				{
					LoadMaterial(entry.path().string());
				}
			}
		}
	}

	MaterialStore::MaterialStore(std::string materialDirectory, std::vector<SerializableMaterial> builtIn)
	{
		for (int i = 0; i < builtIn.size(); ++i)
		{
			LoadMaterial(builtIn[i]);
		}
		LoadMaterialDirectory(materialDirectory);
	}

	std::shared_ptr<Material> MaterialStore::GetMaterial(std::string name)
	{
		auto it = _store.find(name);
		if (it == _store.end())
			return nullptr;
		return it->second;
	}

	void MaterialStore::InitializeStore(std::string materialDirectory, std::vector<SerializableMaterial> builtIns)
	{
		if (_instance)
			return;

		_instance = std::make_unique<MaterialStore>(materialDirectory, builtIns);
	}

	MaterialReference::MaterialReference(std::string materialName)
		: _materialName(materialName)
		, _mat()
	{
	}

	MaterialReference::MaterialReference(MaterialReference&& other)
		: _materialName(std::move(other._materialName))
		, _mat(std::move(other._mat))
	{
		other._materialName.clear();
		other._mat = nullptr;
	}

	std::shared_ptr<Material> MaterialReference::GetMaterial()
	{
		if (!_mat)
			_mat = MaterialStore::Instance().GetMaterial(_materialName);
		return _mat;
	}

	std::string MaterialReference::GetMaterialName() const
	{
		return _materialName;
	}

	void MaterialReference::Reset()
	{
		_mat = nullptr;
		_materialName.clear();
	}

	MaterialReference& MaterialReference::operator=(const std::string& matName)
	{
		Reset();
		_materialName = matName;

		return *this;
	}

	MaterialReference& MaterialReference::operator=(MaterialReference&& other)
	{
		Reset();
		_materialName = std::move(other._materialName);
		_mat = std::move(other._mat);

		return *this;
	}

	int MaterialDescription::CalculateMaterialByteSize() const
	{
		int size = 0;

		for (int i = 0; i < Properties.size(); ++i)
		{
			size += MatSizeToInt(Properties[i].size);
		}

		return size * 4;
	}

	int MatSizeToInt(MaterialSize size)
	{
		switch (size)
		{
		default:
		case MaterialSize::ONE:
			return 1;
		case MaterialSize::TWO:
			return 2;
		case MaterialSize::THREE:
			return 3;
		case MaterialSize::FOUR:
			return 4;
		}
	}

	MaterialSize IntToMatSize(int size)
	{
		if (size < 1)
			return MaterialSize::ONE;
		if (size > 4)
			return MaterialSize::FOUR;
		switch (size)
		{
		default:
		case 1: return MaterialSize::ONE;
		case 2: return MaterialSize::TWO;
		case 3: return MaterialSize::THREE;
		case 4: return MaterialSize::FOUR;
		}
	}
}
