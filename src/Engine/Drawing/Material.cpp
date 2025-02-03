#include "Material.h"

#include <cstring>
#include <filesystem>

#pragma warning(push)
#pragma warning(disable:4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning(pop)

#include "Program.h"

#include "Helpers/StringHelper.h"
#include "Math/floaty.h"
#include "Math/inty.h"

namespace Drawing
{
	using namespace Vector;
	
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

	const ProgramReference& Material::GetProgram() const
	{
		return m_Program;
	}

	void Material::SetProgram(ProgramReference program)
	{
		m_Program = std::move(program);
		m_Dirty = true;
	}

	const std::unordered_map<std::string, TextureReference>& Material::GetTextures() const
	{
		return m_Textures;
	}

	void Material::SetTexture(const std::string& name, TextureReference tex)
	{
		m_Textures[name] = std::move(tex);
		m_Dirty = true;
	}

	const std::unordered_map<std::string, MaterialProperty>& Material::GetProperties() const
	{
		return m_Properties;
	}

	void Material::SetProperty(const std::string& name, MaterialProperty prop)
	{
		m_Properties[name] = std::move(prop);
		m_Dirty = true;
	}

	const std::vector<char>& Material::ToByteForm()
	{
		if (!m_Dirty)
			return m_CachedByteForm;
		auto prog = m_Program.GetProgram();
		if (!prog)
			return m_CachedByteForm;
		m_CachedByteForm = ConvertBytesViaDescription(*this, prog->GetMatDesc());
		m_Dirty = false;
		return m_CachedByteForm;
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
			auto it = mat.m_Properties.find(prop.name);
			
			auto value = (it != mat.m_Properties.end() ? it->second.data : prop.data);

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

	Material Material::Clone() const
	{
		return *this;
	}

	Material Material::CloneWithProperty(const std::string& name, MaterialProperty prop) const
	{
		Material newMat = *this;
		newMat.SetProperty(name, std::move(prop));
		return newMat;
	}

	Material Material::CloneWithTexture(const std::string& name, TextureReference texRef) const
	{
		Material newMat = *this;
		newMat.SetTexture(name, std::move(texRef));
		return newMat;
	}

	Material& Material::WithProperty(const std::string& name, MaterialProperty prop)
	{
		SetProperty(name, prop);
		return *this;
	}

	Material& Material::WithTexture(const std::string& name, TextureReference texRef)
	{
		SetTexture(name, std::move(texRef));
		return *this;
	}

	Material SerializableMaterial::ToMaterial() const
	{
		auto prog = ProgramReference(ProgramName);
		std::unordered_map<std::string, TextureReference> textures;
		for (int i = 0; i < textures.size(); ++i)
		{
			// Handle special case Voxel Atlas textures
			auto& name = this->textures[i].second;
			if (name.size() > 6 && name.rfind("atlas-", 0) == 0)
			{
				Voxel::AtlasType type = Voxel::AtlasType::DIFFUSE;
				std::string atlasName = name.substr(5);
				if (name.compare(name.length() - 8, 8, "-diffuse") == 0)
				{
					atlasName = atlasName.substr(0, atlasName.length() - 8);
					type = Voxel::AtlasType::DIFFUSE;
				}
				else if (name.compare(name.length() - 7, 7, "-normal") == 0)
				{
					atlasName = atlasName.substr(0, atlasName.length() - 7);
					type = Voxel::AtlasType::NORMAL;
				}
				else if (name.compare(name.length() - 9, 9, "-specular") == 0)
				{
					atlasName = atlasName.substr(0, atlasName.length() - 9);
					type = Voxel::AtlasType::SPECULAR;
				}
				else if (name.compare(name.length() - 9, 9, "-emissive") == 0)
				{
					atlasName = atlasName.substr(0, atlasName.length() - 9);
					type = Voxel::AtlasType::EMISSIVE;
				}
				else if (name.compare(name.length() - 8, 8, "-ambient") == 0)
				{
					atlasName = atlasName.substr(0, atlasName.length() - 8);
					type = Voxel::AtlasType::AMBIENT; // Emissive and Ambient are the same
				}
				else if (name.compare(name.length() - 5, 5, "-bump") == 0)
				{
					atlasName = atlasName.substr(0, atlasName.length() - 5);
					type = Voxel::AtlasType::BUMP;
				}
				else
				{
					DWARNING("When Deserializing material found atlas texture reference without a proper suffix! Valid suffixes are: -diffuse -normal -specular -emissive or -bump");
				}
				if (atlasName.size() && atlasName[0] == '-')
					atlasName = atlasName.substr(1);
				textures[this->textures[i].first] = TextureReference(Voxel::AtlasTextureName{ std::move(atlasName), type });
				continue;
			}

			// Not an atlas texture :(
			textures[this->textures[i].first] = this->textures[i].second;
		}

		std::unordered_map<std::string, MaterialProperty> properties;
		for (int i = 0; i < Properties.size(); ++i)
			properties[Properties[i].name] = Properties[i];
		return Material(std::move(prog), std::move(textures), std::move(properties));
	}

	void MaterialStore::LoadMaterial(const SerializableMaterial& smat)
	{
		auto it = _store.find(smat.MaterialName);
		if (it != _store.end())
		{
			DINFO("Overwriting material '" + smat.MaterialName + "'");
			*it->second = smat.ToMaterial();
			return;
		}
				
		_store.emplace(smat.MaterialName, std::make_shared<Material>(smat.ToMaterial()));
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

				auto texNode = yaml["textures"];
				if (texNode)
				{
					if (texNode.IsMap())
					{
						auto& tex = texNode;
						for (auto it = tex.begin(); it != tex.end(); ++it)
						{
							if (!it->second.IsScalar())
							{
								DWARNING("File '" + fileName + "' being parsed as a material, invalid texture entry found in 'textures' tag! (at line: " + std::to_string(it->second.Mark().line) + ")");
								continue;
							}
							mat.textures.emplace_back(std::make_pair(it->first.as<std::string>(), it->second.as<std::string>()));
						}
					}
					else
					{
						DWARNING("File '" + fileName + "' being parsed as a material, 'textures' tag exists but isn't a map!");
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

	void MaterialStore::Reload(std::string materialDirectory, std::vector<SerializableMaterial> builtIns)
	{
		for (int i = 0; i < builtIns.size(); ++i)
		{
			LoadMaterial(builtIns[i]);
		}
		LoadMaterialDirectory(materialDirectory);
	}

	std::shared_ptr<Material> MaterialStore::GetMaterial(const std::string& name) const
	{
		auto it = _store.find(name);
		if (it == _store.end())
		{
			DWARNING("Could not find material '" + name + "'!");
			return nullptr;
		}
		return it->second;
	}

	std::shared_ptr<Material>& MaterialStore::SetMaterial(const std::string& name)
	{
		return _store[name];
	}

	std::shared_ptr<Material> MaterialStore::operator[](const std::string& name) const
	{
		return GetMaterial(name);
	}

	std::shared_ptr<Material>& MaterialStore::operator[](const std::string& name)
	{
		return SetMaterial(name);
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
