#pragma once

#include "Helpers/GLHelper.h"
#include "VertexBuffer.h"
#include "Mesh.h"
#include "Material.h"
#include "Math/matrix.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <array>

namespace Drawing
{
	struct ShadowCascadeBuffer
	{
		Matrixy4x4 ProjectionViewMatrices[3];
		float Distances[3];
	};

	struct TextureMapping
	{
		std::string MaterialName;
		std::string GLName;
	};

	// Describes what a program can do
	// Texture names that are empty ("") are considered not used
	struct ProgramDescription
	{
		std::string ProgramName;

		// Buffer Names
		std::string PerObjectBufferName;
		std::string MaterialBufferName;
		std::string LightBufferName;

		// Shaders
		std::string VertexShaderPath;
		std::string TessControlShaderPath;
		std::string TessEvalShaderPath;
		std::string GeometryShaderPath;
		std::string FragmentShaderPath;

		// Inputs
		GeometryDescription InputDesc;

		// Material
		MaterialDescription MaterialDesc;

		// Textures
		std::vector<TextureMapping> TextureMappings;

		// ShadowMaps
		bool SupportsShadows;
		std::string RegularShadowMapArrayName;
		std::string CubemapShadowMapArrayName;
		std::string CascadeShadowMapArrayName;
		std::string ShadowMatrixBufferName;
		std::string ShadowCascadeBufferName;

		// True means this Program's Fragment shader has a Light buffer named by the LightBufferName string
		// The format of this Light buffer must be this:
		// layout(binding = [any number], std140) uniform [LightBufferName]
		// {
		//	   Light lights[MAX_LIGHT_COUNT];
		// };
		// Where MAX_LIGHT_COUNT is identical to the defined LIGHT_COUNT in Graphics3D.h
		//
		// False means this Program does not use Lighting information
		bool IsLit;
		bool IsInstanced;
	};

	struct BoundTextureMapping
	{
		GLint Binding;
		std::string MatName;
		std::string GLName;
	};

	class Program
	{
		GLProgram _program;

		ProgramDescription _desc;

		MaterialDescription _matDesc;
		GLBuffer _matBuffer;
		GLuint _matBufBinding;

		GLVertexArray _inputVAO;
		GeometryDescription _lastGeoDesc;
		GLuint _lastVertexBuffer;

		std::vector<BoundTextureMapping> _textureMappings;
		GLint _shadowMap2DBinding = -1;
		GLint _shadowMapCubemapBinding = -1;
		GLint _shadowCascadeMapBinding = -1;
		GLBuffer _shadowMatrixBuffer;
		GLuint _shadowMatrixBinding = 0;
		GLBuffer _shadowCascadeBuffer;
		GLuint _shadowCascadeBinding = 0;
		bool _supportsShadows = false;

		std::vector<GLShaderPair> GenerateShaders(const ProgramDescription& desc);

		void EnsureMaterialBuffer();
		
	public:
		Program(ProgramDescription desc);

		inline void SetActive() { glUseProgram(_program.Get()); } // Sets this Program as the active OpenGL shader program

		inline const MaterialDescription& GetMatDesc() const { return _matDesc; }

		inline const std::string& GetName() const { return _desc.ProgramName; }

		bool CanBindTo(const VertexBuffer& buf) const; // Checks whether this Program can bind to a VertexBuffer (whether they have compatible GeometryDescriptions)
		void BindTo(const VertexBuffer& buf); // Configures the Input VAO to point to a given VertexBuffer 

		inline void BindVAO() { CHECK_GL_ERR("Before Binding VAO"); glBindVertexArray(_inputVAO.Get()); CHECK_GL_ERR("After Binding VAO"); }

		void SetMaterial(Material& material, BufferUpdateMode mode);

		inline const std::vector<BoundTextureMapping>& GetTexMappings() const { return _textureMappings; }

		inline GLint GetShadowMap2DBinding() const { return _shadowMap2DBinding; }
		inline GLint GetShadowMapCubemapBinding() const { return _shadowMapCubemapBinding; }
		inline GLint GetShadowCascadeBinding() const { return _shadowCascadeMapBinding; }
		inline bool GetShadowSupport() const { return _supportsShadows; }
		void SetShadowMatrices(const std::vector<Matrixy4x4>& matrices, ShadowCascadeBuffer cascadeData, BufferUpdateMode updateMode);
	};

	/// <summary>
	/// This class stores references to named <see cref="Program"/>s
	/// </summary>
	/// <remarks>
	/// This class is expected to be loaded with Programs before any calls to <see cref="GetProgram"/> can be made.
	/// <para>
	/// Likely this loading will occur on start-up loading program from disk.
	/// </para>
	/// </remarks>
	class ProgramStore
	{
		static std::unique_ptr<ProgramStore> _instance;
	public:
		class Accessor
		{
		public:
			inline ProgramStore& operator()() const { return *_instance; }
			inline operator ProgramStore& () const { return *_instance; }
		} static Instance;

	private:
		std::unordered_map<std::string, std::shared_ptr<Program>> _store;

		void LoadProgram(ProgramDescription desc);
		void LoadDirectory(std::string directory);
	public:
		ProgramStore(std::string directory);

		std::shared_ptr<Program> GetProgram(std::string name);

		static void InitializeStore(std::string directory); // Static initializer that instantiates the static _instance member and loads all programs in a directory
	};
}