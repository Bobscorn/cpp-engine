#pragma once

#include "Helpers/VectorHelper.h"
#include "Helpers/IntVectorHelper.h"

#include "ProgramReference.h"
#include "Texture.h"

#include <vector>
#include <array>
#include <memory>
#include <unordered_map>
#include <string>

namespace Drawing
{
	using namespace Vector;

	// Temporary class
	class EpicoTexture
	{

	};

	enum TextureType
	{
		DIFFUSE = 0,
		OPACITY = 1,
		AMBIENT = 2,
		EMISSIVE = 3,
		SPECULAR = 4,
		SPECULAR_POWER = 5,
		NORMAL = 6,
		BUMP = 7,
	};

	struct TypedTexture
	{
		EpicoTexture Texture;
		TextureType Type;
	};

	enum class MaterialSize
	{
		ONE = 1,
		TWO = 2,
		THREE = 3,
		FOUR = 4
	};

	enum class PropertyType
	{
		INT = 0,
		FLOAT = 1,
	};

	int MatSizeToInt(MaterialSize size);
	MaterialSize IntToMatSize(int size);

	struct MaterialProperty
	{
		std::string name;
		std::array<char, 32> data;
		MaterialSize size;
		PropertyType type;

		template<class T>
		T as() const; // Used with T = int, inty2, inty3, inty4, float, floaty2, floaty3 and floaty4

		template<class T>
		void set_from(T val); // Used with T's the same as the as() method

		template<class T>
		static MaterialProperty from(std::string name, T val, MaterialSize size, PropertyType type = PropertyType::FLOAT)
		{
			MaterialProperty me;
			me.name = name;
			me.size = size;
			me.type = type;
			me.set_from(val);
			return me;
		}
	};

	struct MaterialDescription
	{
		std::vector<MaterialProperty> Properties; // An ordered list of properties

		int CalculateMaterialByteSize() const;
	};

	struct Material
	{
		ProgramReference Program;

		std::unordered_map<std::string, TextureReference> Textures;

		std::unordered_map<std::string, MaterialProperty> Properties;

		// Uses the Program's MaterialDescription to convert to byte form
		std::vector<char> ToByteForm();

		// Converts a composition based Material to byte form using the MaterialDescription
		static std::vector<char> ConvertBytesViaDescription(const Material& mat, const MaterialDescription& desc);
	};

	struct SerializableMaterial
	{
		std::string MaterialName;
		std::string ProgramName;
		std::vector<std::pair<std::string, std::string>> Textures;
		std::vector<MaterialProperty> Properties;

		Material ToMaterial() const;
	};

	class MaterialStore
	{
		static std::unique_ptr<MaterialStore> _instance;
	public:
		class Accessor
		{
		public:
			inline operator MaterialStore& () const { return *_instance; }
			inline MaterialStore& operator()() const { return *_instance; }
		} static Instance;

	private:
		std::unordered_map<std::string, std::shared_ptr<Material>> _store;

		void LoadMaterial(const SerializableMaterial& mat);
		void LoadMaterial(std::string fileName);
		void LoadMaterialDirectory(std::string directory);
	public:
		MaterialStore(std::string materialDirectory, std::vector<SerializableMaterial> builtIn = std::vector<SerializableMaterial>());

		std::shared_ptr<Material> GetMaterial(std::string name);

		static void InitializeStore(std::string materialDirectory, std::vector<SerializableMaterial> builtIns = std::vector<SerializableMaterial>());
	};

	class MaterialReference
	{
		std::string _materialName;
		std::shared_ptr<Material> _mat;
	public:
		MaterialReference() = default;
		MaterialReference(std::string materialName);
		MaterialReference(MaterialReference&& other);

		inline bool HasMaterial() const { return _mat.get(); }

		std::shared_ptr<Material> GetMaterial();

		std::string GetMaterialName() const;

		void Reset();

		MaterialReference& operator=(const std::string& matName);
		MaterialReference& operator=(MaterialReference&& other);
	};
}