#pragma once

#include "Helpers/VectorHelper.h"
#include "Helpers/DrawingHelper.h"

#include "Drawing/Graphics2D.h"
#include "Drawing/Graphics3D.h"
#include "Drawing/IRen3D.h"
#include "Drawing/Particles.h"

// GLRen2 Stuff
#include "GLRen2.h"

#ifndef __glew_h__
#include <GL/glew.h>
#endif
#ifdef __linux__
#include <SDL2/SDL_opengl.h>
#else
#include <SDL_opengl.h>
#endif

template<class T>
struct KeyedThing
{
	T &Get() { return Thing; }

	size_t Key;
	T Thing;
};

struct GeometryInternalDesc
{
	size_t Key = 0ull;
	union
	{
		unsigned int VertexStart = 0u;
		unsigned int IndexOffset;
	};
	unsigned int VertexCount = 0u;
	unsigned int IndexStart = 0u;
	unsigned int IndexCount = 0u;
};

struct GeometryManager : Drawing::IGeometryGuy
{
	~GeometryManager();
	// Returns Index to use when drawing this geometry
	size_t AddGeometry(std::vector<FullVertex> vertices, std::vector<unsigned int> indices);
	void RemoveGeometry(size_t key); // Does nothing when index is out of range

	void Purge(); // Remove all geometry

	//void Draw(size_t key) override;
	void DrawInstanced(std::shared_ptr<GeoThing> geo, const std::vector<Matrixy4x4> &matrices) override;

protected:
	void CleanUp();

	void UpdateGPUData();
	void UpdateInstanceBuffer(const std::vector<Matrixy4x4> &matrices);

	bool Get(size_t Key, GeometryInternalDesc &out);

	std::vector<FullVertex> Vertices;
	std::vector<unsigned int> Indices;
	std::vector<GeometryInternalDesc> Descriptions;
	size_t LastKey = 1ull;
	GLVertexArray VAO;
	GLBuffer VertexBuffer;
	GLBuffer IndexBuffer;
	GLBuffer InstanceBuffer;
	size_t InstanceBufferSize = 0ull;
	bool m_RedoInstanceAttribs = false;
	bool DataChanged = false;
};

struct TexScaleDesc
{
	floaty2 DiffuseScale = { 1.f, 1.f };
	floaty2 OpacityScale = { 1.f, 1.f };
	floaty2 AmbientScale = { 1.f, 1.f };
	floaty2 EmissiveScale = { 1.f, 1.f };
	floaty2 SpecularScale = { 1.f, 1.f };
	floaty2 SpecularPowerScale = { 1.f, 1.f };
	floaty2 NormalScale = { 1.f, 1.f };
	floaty2 BumpScale = { 1.f, 1.f };
};

struct TextureManager : Drawing::ITextureGuy
{
	TextureManager(CommonResources *resources, std::shared_ptr<Drawing::SDLImage> defaulttex, GLProgram &program) : m_Program(program), m_TexScaleBuffer(InitTexScaleBuffer()), m_DefaultTex(defaulttex) {
		// Set Default texture (at slot 0)
		glUseProgram(program.Get());
		InitializeLocations();
		m_DefaultTex->LoadGL();
		if (m_DefaultTex->HasGLTex())
		{
			DINFO("Default Texture bound successfully");
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_DefaultTex->GetGLTexName());
			BlankifyTextues();
		}
		else
			DINFO("Default Texture failed to bind, you might get some odd textures");
	}
	
	void ApplyTextures(Textures *tex) override;
	void BlankifyTextues();

protected:
	void InitializeLocations();
	GLuint InitTexScaleBuffer();
	bool Get(size_t Key, Textures &mat);

	/*constexpr static GLuint DiffuseSamplerLoc		= 4u;*/
	GLint DiffuseSamplerLoc = 0;
	constexpr static GLuint DiffuseTexName = 1u; // Offset added to GL_TEXTURE0 when activating texture

	/*constexpr static GLuint OpacitySamplerLoc		= 5u;*/
	GLint OpacitySamplerLoc = 0;
	constexpr static GLuint OpacityTexName = 2u;

	/*constexpr static GLuint AmbientSamplerLoc		= 6u;*/
	GLint AmbientSamplerLoc = 0;
	constexpr static GLuint AmbientTexName = 3u;

	/*constexpr static GLuint EmissiveSamplerLoc		= 7u;*/
	GLint EmissiveSamplerLoc = 0;
	constexpr static GLuint EmissiveTexName = 4u;

	/*constexpr static GLuint SpecularSamplerLoc		= 8u;*/
	GLint SpecularSamplerLoc = 0;
	constexpr static GLuint SpecularTexName = 5u;

	/*constexpr static GLuint SpecularPowerSamplerLoc	= 9u;*/
	GLint SpecularPowerSamplerLoc = 0;
	constexpr static GLuint SpecularPowerTexName = 6u;

	/*constexpr static GLuint NormalSamplerLoc		= 10u;*/
	GLint NormalSamplerLoc = 0;
	constexpr static GLuint NormalTexName = 7u;

	/*constexpr static GLuint BumpSamplerLoc			= 11u;*/
	GLint BumpSamplerLoc = 0;
	constexpr static GLuint BumpTexName = 8u;

	constexpr static GLuint TexScaleLoc = 3u;
	//GLint TexScaleLoc = 0;

	GLProgram &m_Program;
	GLBuffer m_TexScaleBuffer;
	size_t m_LastKey = 1ull;
	std::shared_ptr<Drawing::SDLImage> m_DefaultTex;
};

/*struct MaterialManager
{
	MaterialManager(GLProgram &program) : m_Program(program), MaterialBuffer(InitBuffer()) {}

	size_t AddMaterial(Material mat);
	Material *GetMaterial(size_t key);
	void RemoveMaterial(size_t key) override;

	void ApplyMaterial(size_t key) override;

protected:
	GLuint InitBuffer();
	bool Get(size_t Key, Material &mat);

	GLProgram &m_Program;
	std::unordered_map<size_t, Material> Materials;
	size_t LastKey = 1ull;
	static constexpr GLint m_MaterialBufferLoc = 1;
	GLBuffer MaterialBuffer;

	constexpr static GLuint MaterialBufferLoc = 1u;
};*/

struct ShapeInternalDesc
{
	ShapeInternalDesc() = default;
	ShapeInternalDesc(Pointer::observing_ptr<Matrixy4x4> t, Pointer::observing_ptr<Material> m, unsigned int i) : Trans(std::move(t)), Mat(std::move(m)), Index(i) {}

	Pointer::observing_ptr<Matrixy4x4> Trans;
	Pointer::observing_ptr<Material> Mat;
	unsigned int Index{ 0u };
};

struct GPU3DDesc
{
	Matrixy4x4 Transform;
	Material Mat;
};

struct PerObjectDesc
{
	Matrixy4x4 WorldView;
	Matrixy4x4 WorldViewProj;
};

struct PerObjectInstanceDesc
{
	Matrixy4x4 View;
	Matrixy4x4 Proj;
};

struct LightManager
{
	LightManager(GLProgram &program) : m_Program(program), LightBuffer(InitLightBuffer()) {}

	bool Apply(); // Returns whether another run needs to happen

	constexpr static unsigned int LightCount = LIGHT_COUNT;

	inline void SetLight(unsigned int index, Light light)
	{
		if (index >= LightCount)
			return;

		NeedUpdate = true;

		Lights[index] = light;
	}
	inline const Light *GetLight(unsigned int index) const
	{
		if (index >= LightCount)
			return nullptr;

		return &Lights[index];
	}
	inline Light *GetLight(unsigned int index)
	{
		if (index >= LightCount)
			return nullptr;
		return &Lights[index];
	}

	void UpdateLightVS(Matrixy4x4 View);

protected:
	GLuint InitLightBuffer();

	GLProgram &m_Program;
	std::array<Light, LightCount> Lights;
	static constexpr GLint m_LightBufferLoc = 7;
	GLBuffer LightBuffer;

	constexpr static GLuint LightBufferLoc = 2u;

	bool NeedUpdate = false;
};

struct GLRen : IRen2D, IRen3D, Particles::IParticleDrawer, virtual FullResourceHolder, Drawing::IDrawGuy, Drawing::ISorterGuy, Drawing::IMaterialGuy//, Drawing::IMatrixGuy
{
	GLRen(CommonResources *resources);

	/// -------------------------
	// 2D
	void DrawPoint(floaty2 point, floaty4 color) override;
	void DrawLine(floaty2 a, floaty2 b, floaty4 color) override;

	// A ---- \
	// |      |
	// \ ---- B
	void DrawRectangle(floaty2 topleft, floaty2 bottomright, floaty4 color) override;

	// D ---- C
	// |      |
	// A ---- B
	void DrawRectangle(floaty2 a, floaty2 b, floaty2 c, floaty2 d, floaty4 color) override;

	// A ---- \
	// |      |
	// \ ---- B
	void FillRectangle(floaty2 topleft, floaty2 bottomright, floaty4 color) override;

	// D ---- C
	// |      |
	// A ---- B
	void FillRectangle(floaty2 a, floaty2 b, floaty2 c, floaty2 d, floaty4 color) override;

	virtual void DrawTriangle(floaty2 a, floaty2 b, floaty2 c, floaty4 color) override;
	virtual void FillTriangle(floaty2 a, floaty2 b, floaty2 c, floaty4 color) override;
	virtual void DrawVertices(std::vector<floaty2> vertices, floaty4 color) override; // Draw them as a Triangle List
	virtual void FillVertices(std::vector<floaty2> vertices, floaty4 color) override; // Fill them as a Triangle List
	virtual void DrawVertices(std::vector<floaty2> vertices, floaty4 color, GLenum mode) override;
	virtual void FillVertices(std::vector<floaty2> vertices, floaty4 color, GLenum mode) override; // Mode is vertex listing mode

	void DrawImage(Drawing::SDLImage *im) override;

	/// Draw *all* of an image to target Rect/point (will be transformed)
	void DrawImage(Drawing::SDLImage *im, PointRect target) override;
	void DrawImage(Drawing::SDLImage *im, floaty2 target) override;

	/// Draw part of an image to target Rect
	/// <param name="src">The Portion of the image to fit into target</param>
	/// <param name="target">The target Rectangle to fit the image into</param>
	void DrawImage(Drawing::SDLImage *im, PointRect src, PointRect target) override;

	// Set the transform matrix to be used for rendering
	void SetTransform(Matrixy2x3 trans) override;
	inline floaty2 Transform(floaty2 me) override
	{
		return ((Matrixy2x3)TransformMatrix).TransformPoint(me);
	}
	inline PointRect Transform(PointRect me) override
	{
		Matrixy2x3 tmp = TransformMatrix;
		PointRect out;
		floaty2 tmpa = tmp.TransformPoint({ me.left, me.top });
		floaty2 tmpb = tmp.TransformPoint({ me.right, me.bottom });
		return { tmpa.x, tmpa.y, tmpb.x, tmpb.y };
	}

	virtual void SetScissorRect(GLint x, GLint y, GLint width, GLint height) override;
	virtual void ClearScissor() override;
	// 2D
	/// --------------------------------------
	// 3D
	
	void Begin() override;
	void End() override;
	void DoTransparentPass() override;
	
	void SetLight(unsigned int index, Light light) override;
	const Light *GetLight(unsigned int Index) const override;
	Light *GetLight(unsigned int index) override;

	Drawing::DrawCall CreateDrawCall(std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Material mat, Matrixy4x4 *matrix = nullptr) override;
	Drawing::DrawCall CreateDrawCall(std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Matrixy4x4 *matrix = nullptr) override;
	

	size_t AddDrawCall(Drawing::DrawCall call) override;
	std::shared_ptr<GeoThing> AddGeometry(std::vector<FullVertex> vertices, std::vector<unsigned int> indices) override;
	void RemoveDrawCall(size_t Key) override;
	void RemoveGeometry(size_t key) override;
	Drawing::DrawCall *GetDrawCall(size_t key) override;

	void DrawCalls(Matrixy4x4 View, Matrixy4x4 Proj, Voxel::CameraFrustum frustum) override;
	void ApplyMaterial(Material *mat);

	//void SetMatrix(Matrixy4x4 *mat) override;
	void Sort(std::vector<size_t> &keys, const std::unordered_map<size_t, Drawing::DrawCall> &calls);

	inline void DidSomething() override { DidWork = true; }

	// 3D
	// ---------------------------------------
	// 3D v2

	Drawing::DrawCallReference AddDrawCallv2(Drawing::DrawCallv2 call);
	bool RemoveDrawCallv2(size_t key);

	const Drawing::DrawCallv2* GetDrawCallv2(size_t key) const;
	bool SetDrawCallv2(size_t key, Drawing::DrawCallv2 updatedValue);

	void Drawv2(Matrixy4x4 view, Matrixy4x4 proj, Voxel::CameraFrustum frustum);

	inline Drawing::DrawCallRenderer* Getv2() { return &m_Renv2; }

	// 3D v2
	/// --------------------------------------
	// Particles

	size_t AddExternalParticle(std::shared_ptr<Drawing::SDLImage> texture, Particles::BasicParticle particle) override;
	std::vector<size_t> AddExternalParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<Particles::BasicParticle> particles) override;
	void RemoveExternalParticle(Drawing::SDLImage *texture, size_t key) override;
	void RemoveExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> keys) override;

	void AddVelocityParticle(std::shared_ptr<Drawing::SDLImage> texture, Particles::BasicParticle basic, Particles::VelocityParticleData velodat) override;
	void AddVelocityParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<Particles::BasicParticle> basic, std::vector<Particles::VelocityParticleData> props) override;
	void AddVelocityParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<std::pair<Particles::BasicParticle, Particles::VelocityParticleData>> particles) override;

	// Observers

	Particles::BasicParticle *GetExternalParticle(Drawing::SDLImage *texture, size_t key) override;
	std::vector<Particles::BasicParticle *> GetExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> key) override;

	const Particles::BasicParticle *GetExternalParticle(Drawing::SDLImage *texture, size_t key) const override;
	std::vector<const Particles::BasicParticle *> GetExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> keys) const override;




	// Particles
	/// ----------------------------------------

	inline PointRect ToGLCoords(PointRect fromTopLeft) override
	{
		floaty2 topleft = ToGLCoords({ fromTopLeft.left, fromTopLeft.top });
		floaty2 bottomright = ToGLCoords({ fromTopLeft.right, fromTopLeft.bottom });
		return { topleft.x, topleft.y, bottomright.x, bottomright.y };
	}
	inline floaty2 ToGLCoords(floaty2 fromTopLeft) override
	{
		return { fromTopLeft.x - (*mResources->HalfWindowWidth), (*mResources->HalfWindowHeight) - fromTopLeft.y };
	}

	inline bool IsGood() noexcept override { return ImageProgram.Get(); }
	void Resize(unsigned int NewWidth, unsigned int NewHeight) override;

	inline void Engage() override
	{
		glUseProgram(ImageProgram.Get());
	}

	virtual void Clear(SDL_Color color) override { DidWork = true; glClearColor((float)color.r / 255.f, (float)color.g / 255.f, (float)color.b / 255.f, (float)color.a / 255.f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
	virtual void Present(SDL_Window *win) override;

protected:
	// Init methods
	GLProgram InitImage();
	GLuint InitImageVAO();
	GLuint InitImageVBO();
	GLuint InitImageIBO();
	GLuint InitImageTexTrans();
	GLuint InitImageSampler();

	GLProgram Init2D(); // 2D and Draw refer to the same shader/program
	GLuint InitDrawVAO();
	GLuint InitDrawVBO();
	GLuint InitDrawIBO();
	GLuint InitDrawColor();

	GLuint InitProjMatrix();
	GLuint InitTransMatrix();

	GLProgram Init3D();
	GLuint InitPerObjectBuffer();
	GLuint InitMaterialBuffer();

	// Update methods
	void UpdateImageVertices(std::array<const floaty2, 4u> vertices); // Must be 4 vertices
	void UpdateTexTrans(float x, float y);

	void UpdateDrawVerts(std::array<floaty2, 4u> vertices);
	void UpdateDrawColor(floaty4 color);

	void UpdatePerObjectInstance(PerObjectInstanceDesc *desc);
	void UpdatePerObject(Matrixy4x4 *mat);
	void UpdatePerObject(PerObjectDesc *desc);

	Matrixy4x4 TransformMatrix;
	Matrixy4x4 ProjectionMatrix;

	std::shared_ptr<Drawing::SDLImage> m_DefaultTex;

	GLProgram ImageProgram;
	GLVertexArray ImageVAO;
	GLBuffer ImageVBOTex;
	GLBuffer ImageVBOPos;
	GLBuffer ImageIBO;
	static constexpr GLuint ImagePosLoc = 0u;
	static constexpr GLuint ImageTexLoc = 1u;
	GLuint ImageTexTransName;
	GLuint ImageSamplerLoc;

	GLProgram Program2D;
	GLVertexArray DrawVAO;
	GLBuffer DrawVBO;
	GLBuffer DrawIBO;
	GLuint DrawColorName;

	GLBuffer Proj2DBuffer;
	GLBuffer Trans2DBuffer;

	// New 3D
	Drawing::DrawCallRenderer m_Renv2;

	Matrixy4x4 View3D;
	Matrixy4x4 Projection3D;
	PerObjectDesc PerObject;

	GLProgram Program3D; // Not done
	constexpr static GLint m_PerObjectLoc = 2u;
	GLBuffer PerObjectBuf;
	GLBuffer m_MaterialBuf;
	constexpr static unsigned int MaterialBufLoc = 1u;
	GeometryManager GeoMan;
	//MaterialManager MatMan;
	LightManager Lights;
	TextureManager TexMan;
	Particles::ParticleManager ParticleMan;
	Drawing::DrawCaller DrawCallGuy;

	bool DidWork = false;
};