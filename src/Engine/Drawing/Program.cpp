#include "Program.h"

#include <filesystem>
#include <yaml-cpp/yaml.h>

#include "Helpers/StringHelper.h"
#include "Helpers/YAMLHelper.h"

#include "GLRen2.h"
#include "BindingManager.h"

namespace Drawing
{
	using namespace YAMLNames::Program;

	std::unique_ptr<ProgramStore> ProgramStore::_instance = nullptr;
	ProgramStore::Accessor ProgramStore::Instance{};

	GLenum StringToShaderType(const std::string& str)
	{
		if (str == "VERTEX")
			return GL_VERTEX_SHADER;
		if (str == "FRAGMENT")
			return GL_FRAGMENT_SHADER;
		if (str == "GEOMETRY")
			return GL_GEOMETRY_SHADER;
		if (str == "TESS_EVAL")
			return GL_TESS_EVALUATION_SHADER;
		if (str == "TESS_CONTRL")
			return GL_TESS_CONTROL_SHADER;
		return GL_INVALID_VALUE;
	}

	MaterialDescription ProcessMaterialYAML(const YAML::Node& node);
	std::vector<TextureMapping> ProcessTextureMappings(const YAML::Node& node);

	void ProgramStore::LoadProgram(ProgramDescription desc)
	{
		if (_store.find(desc.ProgramName) != _store.end())
			DWARNING("Overwriting existing program!");

		_store[desc.ProgramName] = std::make_shared<Program>(desc);
	}

	void ProgramStore::LoadDirectory(std::string directory)
	{
		// TODO: Load files, Create ProgramDescriptions from files
		auto prependDirectory = [&directory](const std::string& shaderPath)
		{
			auto path = std::filesystem::path(directory);
			path /= shaderPath;
			auto pathStr = path.string();
			return pathStr;
		};
		if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory))
		{
			DERROR("Program directory does not exist!");
			DERROR("Please copy programs to correct directory!");
			return;
		}
		for (auto& dir_entry : std::filesystem::directory_iterator(directory))
		{
			if (dir_entry.exists() && dir_entry.is_regular_file() && dir_entry.path().extension() == ".prog")
			{
				try
				{
					auto fileName = dir_entry.path().string();
					DINFO("Loading program: '" + fileName + "' from disk");

					YAML::Node yaml = YAML::LoadFile(fileName);

					ProgramDescription desc;

					if (!yaml[NameTag])
					{
						DWARNING("Processing: " + dir_entry.path().string() + " as shader program, no '" + NameTag + "' entry found!");
						continue;
					}

					desc.ProgramName = yaml[NameTag].as<std::string>();
					DINFO("Program name: '" + desc.ProgramName + "'");

					auto perObjectNode = yaml[PerObjectBufTag];
					if (!perObjectNode || !perObjectNode.IsScalar())
					{
						DWARNING("Processing: " + dir_entry.path().string() + " as shader program, no '" + PerObjectBufTag + "' entry found!");
						DWARNING("Every program needs a per object buffer!");
						continue;
					}

					desc.PerObjectBufferName = perObjectNode.Scalar();

					auto materialBufNode = yaml[MaterialBufTag];
					if (!materialBufNode || !materialBufNode.IsScalar())
					{
						DINFO("Loading program '" + desc.ProgramName + "' with no material buffer");
					}
					else
					{
						desc.MaterialBufferName = materialBufNode.Scalar();

						if (desc.MaterialBufferName.empty())
						{
							DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): Found empty material buffer! Remove the materialbuf tag if there is no material buffer!");
						}
						else
						{
							auto materialNode = yaml[MaterialTag];
							if (!materialNode)
							{
								DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): Found material buffer but no material tag!");
							}
							desc.MaterialDesc = ProcessMaterialYAML(materialNode);
						}
					}

					desc.IsLit = false;
					auto lightBufNode = yaml[LightBufTag];
					if (lightBufNode)
					{
						if (lightBufNode.IsScalar())
						{
							desc.LightBufferName = lightBufNode.Scalar();

							if (desc.LightBufferName.size())
								desc.IsLit = true;
						}
						else
						{
							auto mark = lightBufNode.Mark();
							DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): Invalid lightbuf tag at line : " + std::to_string(mark.line) + " col : " + std::to_string(mark.column));
						}
					}

					auto shaderNode = yaml[ShadersTag];
					if (!shaderNode)
					{
						DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): expected shaders tag, none was found.");
						continue;
					}

					if (!shaderNode.IsMap())
					{
						DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): expected shaders entry to be a map.");
						continue;
					}

					for (auto it = shaderNode.begin(); it != shaderNode.end(); ++it)
					{
						std::string shaderPath = it->first.as<std::string>();
						GLenum shaderType = StringToShaderType(it->second.as<std::string>());

						switch (shaderType)
						{
						case GL_VERTEX_SHADER:
							if (desc.VertexShaderPath.size())
								DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): Overwriting existing Vertex Shader (existing: '" + desc.VertexShaderPath + "') with '" + shaderPath + "'");

							desc.VertexShaderPath = prependDirectory(shaderPath);
							break;
						case GL_TESS_CONTROL_SHADER:
							if (desc.TessControlShaderPath.size())
								DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): Overwriting existing Tesselation Control Shader (existing: '" + desc.TessControlShaderPath + "') with '" + shaderPath + "'");

							desc.TessControlShaderPath = prependDirectory(shaderPath);
							break;
						case GL_TESS_EVALUATION_SHADER:
							if (desc.TessEvalShaderPath.size())
								DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): Overwriting existing Tesselation Evaluation Shader (existing: '" + desc.TessEvalShaderPath + "') with '" + shaderPath + "'");

							desc.TessEvalShaderPath = prependDirectory(shaderPath);
							break;
						case GL_GEOMETRY_SHADER:
							if (desc.GeometryShaderPath.size())
								DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): Overwriting existing Geometry Shader (existing: '" + desc.GeometryShaderPath + "') with '" + shaderPath + "'");

							desc.GeometryShaderPath = prependDirectory(shaderPath);
							break;
						case GL_FRAGMENT_SHADER:
							if (desc.FragmentShaderPath.size())
								DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): Overwriting existing Fragment Shader (existing: '" + desc.FragmentShaderPath + "') with '" + shaderPath + "'");

							desc.FragmentShaderPath = prependDirectory(shaderPath);
							break;
						}
					}

					if (desc.VertexShaderPath.empty())
					{
						DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): No Vertex Shader found! Every program requires at least a Vertex Shader!");
						continue;
					}


					// Load geometry description
					auto geo = yaml[GeometryTag];
					if (!geo)
					{
						DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): No geometry description found. Program requires a geometry description");
						continue;
					}


					GeometryDescription geoDesc;

					if (!geo[PositionTag])
					{
						DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): No position geometry component found!");
						continue;
					}

					auto getCompOrderSize = [&fileName, &desc](const YAML::Node& node, size_t& order, size_t& size, std::string compName)
					{
						if (!node)
							return;
						auto orderNode = node["order"];
						size_t orderTmp = 0;
						if (!orderNode || !orderNode.IsScalar() || !StringHelper::IfSizeT(orderNode.Scalar(), &orderTmp))
						{
							DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): Found geometry component '" + compName + "' without a valid order tag!");
							return;
						}
						auto sizeNode = node["size"];
						size_t sizeTmp = 0;
						if (!sizeNode || !sizeNode.IsScalar() || !StringHelper::IfSizeT(sizeNode.Scalar(), &sizeTmp))
						{
							DWARNING("When Processing '" + fileName + "' as Program (" + desc.ProgramName + "): Found geometry component '" + compName + "' without a valid size tag!");
							return;
						}
						order = orderTmp;
						size = sizeTmp;
					};

					getCompOrderSize(geo[PositionTag], geoDesc.PositionOrder, geoDesc.PositionSize, "position");
					getCompOrderSize(geo[BinormalTag], geoDesc.BinormalOrder, geoDesc.BinormalSize, "binormal");
					getCompOrderSize(geo[TangentTag], geoDesc.TangentOrder, geoDesc.TangentSize, "tangent");
					getCompOrderSize(geo[NormalTag], geoDesc.NormalOrder, geoDesc.NormalSize, "normal");
					getCompOrderSize(geo[TexCoordTag], geoDesc.TexCoordOrder, geoDesc.TexCoordSize, "tex coords");

					desc.InputDesc = geoDesc;

					desc.TextureMappings = ProcessTextureMappings(yaml["textures"]);

					LoadProgram(desc);
				}
				catch (YAML::Exception& e)
				{
					DERROR("Failed to parse program file '" + dir_entry.path().string() + "' with YAML exception: " + e.what());
				}
			}
		}
	}

	std::vector<TextureMapping> ProcessTextureMappings(const YAML::Node& node)
	{
		if (!node || !node.IsSequence())
		{
			if (node)
				DWARNING("When processing program: texture node was not a sequence!");
			return {};
		}

		std::vector<TextureMapping> mapping{};

		for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
		{
			auto seqNode = *it;
			if (!seqNode)
				continue;
			if (!seqNode.IsMap())
			{
				DWARNING("When processing program at line " + std::to_string(seqNode.Mark().line) + ": entry in texture sequence is not a map!");
				continue;
			}

			auto matNameNode = seqNode["mat-name"];
			auto glNameNode = seqNode["gl-name"];

			if (!matNameNode || !matNameNode.IsScalar())
			{
				if (matNameNode)
					DWARNING("When processing program at line '" + std::to_string(matNameNode.Mark().line) + "': 'mat-name' was not a scalar!");
				else
					DWARNING("When processing program: at line '" + std::to_string(seqNode.Mark().line) + "', expected tag 'mat-name'!");
				continue;
			}

			if (!glNameNode || !glNameNode.IsScalar())
			{
				if (matNameNode)
					DWARNING("When processing program at line '" + std::to_string(glNameNode.Mark().line) + "': 'gl-name' tag was not a scalar!");
				else
					DWARNING("When processing program: at line '" + std::to_string(seqNode.Mark().line) + "', expected tag 'gl-name'!");
				continue;
			}

			mapping.emplace_back(TextureMapping{ matNameNode.Scalar(), glNameNode.Scalar() });
		}

		return mapping;
	}

	MaterialDescription ProcessMaterialYAML(const YAML::Node& node)
	{
		MaterialDescription desc{};

		auto props = node["properties"];
		if (!props.IsSequence())
			return desc;

		for (YAML::iterator it = props.begin(); it != props.end(); ++it)
		{
			auto propNode = *it;
			MaterialProperty prop;
			
			auto nameNode = propNode["name"];
			if (!nameNode || !nameNode.IsScalar())
			{
				DWARNING("When Processing Material Description: Found property with no/invalid name!");
				continue;
			}

			prop.name = nameNode.Scalar();

			auto sizeNode = propNode["size"];
			int size = 1;
			if (!sizeNode || !sizeNode.IsScalar() || !StringHelper::IfINT(sizeNode.Scalar(), &size))
			{
				DWARNING("When processing Material Description: Found property with invalid or no size! Specify the size of the property as either 1, 2, 3 or 4");
				continue;
			}

			prop.size = IntToMatSize(size);

			auto typeFunc = [](const YAML::Node& typeNode, PropertyType& type) -> bool
			{
				if (!typeNode || !typeNode.IsScalar())
					return false;
				
				auto str = typeNode.Scalar();
				
				if (str == "int")
				{
					type = PropertyType::INT;
					return true;
				}
				if (str == "float")
				{
					type = PropertyType::FLOAT;
					return true;
				}

				return false;
			};

			auto typeNode = propNode["type"];
			if (!typeFunc(typeNode, prop.type))
			{
				DWARNING("When processing program's Material Description: Found property with invalid or no type! Specify the type of the property as either int or float");
				continue;
			}

			auto defaultNode = propNode["default"];

			if (!defaultNode)
			{
				prop.set_from(inty4{ 0, 0, 0, 0 }); // The default is considered the property's actual value as this is the description and not an actual material
				desc.Properties.emplace_back(std::move(prop));
				continue;
			}

			if (!(defaultNode.IsMap() || defaultNode.IsSequence()))
			{
				if (defaultNode.IsScalar())
				{
					if (prop.type == PropertyType::INT)
					{
						int defaultInt = 0;
						if (!StringHelper::IfINT(defaultNode.Scalar(), &defaultInt))
						{
							DWARNING("When processing program's Material Description: Found property with type = int with a default value that was not an int! Ensure the value is numeric!");
							DWARNING("This property will still be processed with the default set to 0.");
						}
						prop.set_from(inty4{ defaultInt, defaultInt, defaultInt, defaultInt });
					}
					else // prop.type == PropertyType::FLOAT
					{
						float defaultFloat = 0.f;
						if (!StringHelper::IfFloat(defaultNode.Scalar(), &defaultFloat))
						{
							DWARNING("When processing program's Material Description: Found property with type = float with a default value that was not a floating point! Ensure the value is decimal!");
							DWARNING("This property will still be processed with the default set to 0.f.");
						}
						prop.set_from(floaty4{ defaultFloat, defaultFloat, defaultFloat, defaultFloat });
					}
					desc.Properties.emplace_back(std::move(prop));
					continue;
				}
				DWARNING("When processing program's Material Description: Found property with invalid default node!");
				DWARNING("This property will still be processed with the default set to 0.f.");
				if (prop.type == PropertyType::INT)
					prop.set_from(inty4{ 0, 0, 0, 0 });
				else
					prop.set_from(floaty4{ 0.f, 0.f, 0.f, 0.f });

				desc.Properties.emplace_back(std::move(prop));
				continue;
			}

			constexpr std::array<const char*, 4> vectorComps = { "x", "y", "z", "w" };
			constexpr std::array<const char*, 4> colorComps = { "r", "g", "b", "a" };

			auto processDefaultFunc = [&vectorComps, &colorComps](YAML::Node& defaultNode, MaterialProperty& prop, auto defaultVal, auto defaultSingleVal, auto isTypeFunc)
			{
				decltype(defaultVal) val = defaultVal;
				int size = MatSizeToInt(prop.size);
				for (int i = 0; i < size; ++i)
				{
					auto compNode = defaultNode[vectorComps[i]];
					if (!compNode)
						compNode = defaultNode[colorComps[i]];
					if (!compNode)
					{
						DWARNING("When processing program's Material Description: Default value didn't contain either '" + vectorComps[i] + "' or '" + colorComps[i] + "' when expected due to property size (" + std::to_string(MatSizeToInt(prop.size)) + ")!");
						DWARNING("This property will still be processed with the default set to 0 (or 0.f).");
						prop.set_from(defaultVal);
						return;
					}
					decltype(defaultSingleVal) compVal = 0;
					if (!compNode.IsScalar() || !isTypeFunc(compNode.Scalar(), &compVal))
					{
						DWARNING("When processing program's Material Description: Found property with a default with a malformed component '" + vectorComps[i] + "' or '" + colorComps[i] + "'!");
						DWARNING("This property will still be processed with the default set to 0 (or 0.f).");
						prop.set_from(defaultVal);
						return;
					}

					val.m[i] = compVal;
				}
				prop.set_from(val);
				return;
			};

			if (prop.type == PropertyType::INT)
			{
				processDefaultFunc(defaultNode, prop, inty4{ 0, 0, 0, 0 }, (int)0, StringHelper::IfINT);
				desc.Properties.emplace_back(std::move(prop));
			}
			else
			{
				processDefaultFunc(defaultNode, prop, floaty4{ 0.f, 0.f, 0.f, 0.f }, (float)0.f, StringHelper::IfFloat);
				desc.Properties.emplace_back(std::move(prop));
			}
		}

		return desc;
	}

	ProgramStore::ProgramStore(std::string directory)
	{
		LoadDirectory(directory);
	}

	std::shared_ptr<Program> ProgramStore::GetProgram(std::string name)
	{
		auto find_it = _store.find(name);
		if (find_it == _store.end())
			return nullptr;

		return find_it->second;
	}

	void ProgramStore::InitializeStore(std::string directory)
	{
		if (_instance)
			return;

		_instance = std::make_unique<ProgramStore>(directory);
	}

	std::vector<GLShaderPair> Program::GenerateShaders(const ProgramDescription& desc)
	{
		std::vector<GLShaderPair> shaders;

		if (desc.VertexShaderPath.size())
			shaders.emplace_back(GLShaderPair{ desc.VertexShaderPath, GL_VERTEX_SHADER });

		if (desc.TessControlShaderPath.size())
			shaders.emplace_back(GLShaderPair{ desc.TessControlShaderPath, GL_TESS_CONTROL_SHADER });

		if (desc.TessEvalShaderPath.size())
			shaders.emplace_back(GLShaderPair{ desc.TessEvalShaderPath, GL_TESS_EVALUATION_SHADER });

		if (desc.GeometryShaderPath.size())
			shaders.emplace_back(GLShaderPair{ desc.GeometryShaderPath, GL_GEOMETRY_SHADER });

		if (desc.FragmentShaderPath.size())
			shaders.emplace_back(GLShaderPair{ desc.FragmentShaderPath, GL_FRAGMENT_SHADER });

		return shaders;
	}

	void Program::EnsureMaterialBuffer()
	{
		if (!_matBuffer)
		{
			int byteSize = _matDesc.CalculateMaterialByteSize();
			if (byteSize > 0)
			{
				CHECK_GL_ERR("Before Creating Material Buffer");
				GLuint buf = 0;
				glGenBuffers(1, &buf);

				if (!_matBufBinding)
					_matBufBinding = BindingManager::GetNext();

				glBindBufferBase(GL_UNIFORM_BUFFER, _matBufBinding, buf);
				glBufferData(GL_UNIFORM_BUFFER, byteSize, nullptr, GL_STREAM_DRAW);
				CHECK_GL_ERR("Binding and uploading buffer");

				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				_matBuffer.Reset(buf);
			}
		}
	}

	Program::Program(ProgramDescription desc)
		: _program(GenerateShaders(desc))
		, _desc(desc)
		, _matDesc(desc.MaterialDesc)
		, _textureMappings()
	{
		glUseProgram(_program.Get());
		CHECK_GL_ERR("Pre-program initialization");
		GLuint vao = 0;
		glGenVertexArrays(1, &vao);
		_inputVAO.Reset(vao);

		GLuint matBuf = 0;
		glGenBuffers(1, &matBuf);
		_matBuffer.Reset(matBuf);

		_matBufBinding = BindingManager::GetNext();

		auto byteSize = _matDesc.CalculateMaterialByteSize();

		std::vector<char> blankBuffer{ (size_t)byteSize, (char)0, std::allocator<char>()};

		glBindBufferBase(GL_UNIFORM_BUFFER, _matBufBinding, matBuf);
		glBufferData(GL_UNIFORM_BUFFER, byteSize, (GLvoid*)blankBuffer.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		/*GLint index = glGetUniformBlockIndex(_program.Get(), _desc.PerObjectBufferName.c_str());
		glUniformBlockBinding(_program.Get(), index, DrawCallRenderer::PerObjectBufBinding);*/

		//Textures v
		_textureMappings.reserve(desc.TextureMappings.size());
		int textureBind = 0;
		for (int i = 0; i < desc.TextureMappings.size(); ++i)
		{
			BoundTextureMapping mapping;
			mapping.GLName = desc.TextureMappings[i].GLName;
			mapping.MatName = desc.TextureMappings[i].MaterialName;
			mapping.Binding = 0;
			auto location = glGetUniformLocation(_program.Get(), mapping.GLName.c_str());
			if (location != GL_INVALID_INDEX)
			{
				mapping.Binding = textureBind;
				glUniform1i(location, textureBind);
				++textureBind;
			}
			else
			{
				DWARNING("When creating program '" + _desc.ProgramName + "': could not find uniform sampler '" + mapping.GLName + "'!");
			}
			_textureMappings.emplace_back(std::move(mapping));
		}

		// Buffers v

		if (_desc.LightBufferName.size())
		{
			GLint index = glGetUniformBlockIndex(_program.Get(), _desc.LightBufferName.c_str());
			if (index != GL_INVALID_INDEX)
				glUniformBlockBinding(_program.Get(), index, DrawCallRenderer::GetLightBufBinding());
			else
				DERROR("Failed to find program '" + _desc.ProgramName + "'s Light Buffer UniformBlockIndex!");
		}

		GLint blockIndex = glGetUniformBlockIndex(_program.Get(), _desc.MaterialBufferName.c_str());
		if (blockIndex != GL_INVALID_INDEX)
			glUniformBlockBinding(_program.Get(), blockIndex, _matBufBinding);
		else
			DERROR("Failed to find program '" + _desc.ProgramName + "'s Material Buffer's UniformBlockIndex!");
		CHECK_GL_ERR("Getting and binding uniform");

		blockIndex = glGetUniformBlockIndex(_program.Get(), _desc.PerObjectBufferName.c_str());
		if (blockIndex != GL_INVALID_INDEX)
			glUniformBlockBinding(_program.Get(), blockIndex, DrawCallRenderer::GetPerObjectBufBinding());
		else
			DERROR("Failed to find program '" + _desc.ProgramName + "'s Per Object Buffer's UniformBlockIndex!");

		CHECK_GL_ERR("Post-program initialization");
		glUseProgram(0);
	}

	bool Program::CanBindTo(const VertexBuffer& buf) const
	{
		return _desc.InputDesc == buf.GetDescription();
	}

	void Program::BindTo(const VertexBuffer& buf)
	{
		_inputVAO.ResetState();

		GLuint vertexAttrib = 0;
		GLuint offset = 0;
		int nextOrder = 1;

		auto& desc = buf.GetDescription();
		glBindBuffer(GL_ARRAY_BUFFER, buf.GetVBO().Get());

		std::vector<VertexComponent> comps{ VertexComponents.cbegin(), VertexComponents.cend() };

		GLsizei compStride = (GLsizei)desc.GetVertexByteSize();
		int emergencyBreak = 0;
		auto emergencyLimit = comps.size() * comps.size();
		while (emergencyBreak++ < emergencyLimit)
		{
			if (comps.empty())
				break;

			bool found = false;
			for (int i = 0; i < comps.size(); ++i)
			{
				auto& comp = comps[i];

				if (desc.OrderOfComponent(comp) == nextOrder)
				{
					GLVertexArray::ArrayAttribState state;
					state.Enabled = true;
					state.Size = (GLint)desc.SizeOfComponent(comp);
					state.Type = GL_FLOAT;
					state.Normalized = IsVectorVertexComponent(comp);
					state.Stride = compStride;
					state.Offset = reinterpret_cast<GLvoid*>(sizeof(GLfloat) * ((size_t)offset));

					_inputVAO.SetState(vertexAttrib, state);

					vertexAttrib++;
					offset += state.Size;
					nextOrder++;
					found = true;
				}
			}
			if (!found)
				break;
		}
	}

	void Program::SetMaterial(const Material& material)
	{
		EnsureMaterialBuffer();

		if (!_matBuffer)
			return;

		auto bytes = Material::ConvertBytesViaDescription(material, _matDesc);

		glBindBuffer(GL_UNIFORM_BUFFER, _matBuffer.Get());
		glBufferSubData(GL_UNIFORM_BUFFER, 0, (GLsizeiptr)bytes.size(), (GLvoid*)bytes.data());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}