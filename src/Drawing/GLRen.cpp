#include "GLRen.h"

#include "Helpers/ProfileHelper.h"
#include "Helpers/GLHelper.h"

#include <algorithm>

#include "BindingManager.h"

void GLRen::DrawImage(Drawing::SDLImage * im, PointRect target)
{
	DidWork = true;
	if (!im->CanDraw())
		return;
	if (!im->HasGLTex())
	{
		im->LoadGL();
	}

	glUseProgram(ImageProgram.Get());

	// Setup
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, im->Get());
	glUniform1i(ImageSamplerLoc, 0);

	std::array<const floaty2, 4u> vertices =
	{
		floaty2{target.left, target.bottom }, // Bottom Left
		floaty2{target.right, target.bottom }, // Bottom Right
		floaty2{target.right, target.top }, // Top Right
		floaty2{target.left, target.top }, // Top Left
	};

	UpdateImageVertices(vertices);
	UpdateTexTrans(im->GetWidthScale(), im->GetHeightScale());

	glBindVertexArray(ImageVAO.Get());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ImageIBO.Get());

	// Draw
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

	// Clean up
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);

	CHECK_GL_ERR("DrawImage");
}

void GLRen::DrawImage(Drawing::SDLImage * im, floaty2 target)
{
	DrawImage(im, { target.x, target.y + (float)im->GetHeight(), target.x + (float)im->GetWidth(), target.y });
}

void GLRen::DrawImage(Drawing::SDLImage * im, PointRect src, PointRect target)
{
	DidWork = true;
	// TODO: my dood
	DINFO("Draw Image with src rectangle not supported yet (soz I'm lazy)");
	(void)im;
	(void)src;
	(void)target;
}

void GLRen::SetTransform(Matrixy2x3 trans)
{
	// Convert 2D matrix to 4x4
	CHECK_GL_ERR("Before Transform Update");
	TransformMatrix = trans;
	glBindBuffer(GL_UNIFORM_BUFFER, Trans2DBuffer.Get());
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrixy4x4), TransformMatrix.ma);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	CHECK_GL_ERR("Updating Transform");
}

void GLRen::SetScissorRect(GLint x, GLint y, GLint width, GLint height)
{
	DWARNING("Scissor Rect not implemented yet");
}

void GLRen::ClearScissor()
{
	DWARNING("Scissor Rect not implemented yet");
}

void GLRen::Begin()
{
	CHECK_GL_ERR("Pre Applying 3D Properties");
	glUseProgram(Program3D.Get());
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDepthMask(true);
	CHECK_GL_ERR("Applying 3D Properties");
}

void GLRen::End()
{
	glUseProgram(Program3D.Get());
	glDisable(GL_CULL_FACE);
	glDepthMask(true);
}

void GLRen::DoTransparentPass()
{
	glDepthMask(false);
	//glDisable(GL_CULL_FACE);
}

void GLRen::SetLight(unsigned int index, Light light)
{
	Lights.SetLight(index, light);
}

Light * GLRen::GetLight(unsigned int index)
{
	return Lights.GetLight(index);
}

const Light *GLRen::GetLight(unsigned int Index) const
{
	return Lights.GetLight(Index);
}

Drawing::DrawCall GLRen::CreateDrawCall(std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Material mat, Matrixy4x4 * matrix)
{
	DINFO("Supplying Material to CreateDrawCall deprecated");
	Drawing::DrawCall out;
	out.Geometry = AddGeometry(std::move(vertices), std::move(indices));
	out.Matrix = matrix;
	out.Material = std::make_shared<Material>(mat);
	out.Enabled = true;
	return out;
}

Drawing::DrawCall GLRen::CreateDrawCall(std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Matrixy4x4 * matrix)
{
	Drawing::DrawCall out;
	out.Geometry = AddGeometry(std::move(vertices), std::move(indices));
	out.Matrix = matrix;
	out.Enabled = true;
	return out;
}

size_t GLRen::AddDrawCall(Drawing::DrawCall call)
{
	return DrawCallGuy.Add(call);
}

void GLRen::RemoveDrawCall(size_t Key)
{
	DrawCallGuy.Remove(Key);
}

Drawing::DrawCall * GLRen::GetDrawCall(size_t key)
{
	return DrawCallGuy.Get(key);
}

std::shared_ptr<GeoThing> GLRen::AddGeometry(std::vector<FullVertex> vertices, std::vector<unsigned int> indices)
{
	size_t key = GeoMan.AddGeometry(vertices, indices);
	return std::make_shared<GeoThing>(GeoThing{ this, key });
}

void GLRen::RemoveGeometry(size_t key)
{
	GeoMan.RemoveGeometry(key);
}

void GLRen::DrawCalls(Matrixy4x4 View, Matrixy4x4 Proj, Voxel::CameraFrustum frustum)
{
	CHECK_GL_ERR("Pre-Applying Draw Calls");
	Projection3D = Proj;
	DidWork = true;

	// Prep OpenGL
	Lights.UpdateLightVS(View);
	Lights.Apply();

	CHECK_GL_ERR("Pre-Enabling Depth Test");
	glEnable(GL_DEPTH_TEST);
	CHECK_GL_ERR("Enabling Depth Test");
	View3D = View;
	PerObjectInstanceDesc gpu;
	gpu.View = View3D;
	gpu.Proj = Projection3D;
	UpdatePerObjectInstance(&gpu);

	// Prep Draw Calls
	std::vector<size_t> final_keys = DrawCallGuy.Cull(DrawCallGuy.GetKeys(), frustum);
	DrawCallGuy.Sort(final_keys);

	// Apply
	DrawCallGuy.Apply(final_keys);

	m_Renv2.Draw(View, Proj, frustum);
	
	glDisable(GL_DEPTH_TEST);
}

void GLRen::ApplyMaterial(Material *mat)
{
	if (!mat)
		return;

	glBindBuffer(GL_UNIFORM_BUFFER, m_MaterialBuf.Get());
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), mat);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GLRen::Sort(std::vector<size_t>& keys, const std::unordered_map<size_t, Drawing::DrawCall> &calls)
{
	struct DrawAfterGuy
	{
		DrawAfterGuy(const std::unordered_map<size_t, Drawing::DrawCall> &calls, const Matrixy4x4 &view, bool compare_distance_opaque = true) : calls(calls), View(view), m_CompareDistanceOpaque(compare_distance_opaque) {}

		const std::unordered_map<size_t, Drawing::DrawCall> &calls;
		const Matrixy4x4 &View;
		const float Margin = 0.015f;
		const bool m_CompareDistanceOpaque = true;

		// Returns whether an object should be drawn after or before
		inline bool operator()(const size_t &akey, const size_t &bkey) const
		{
			auto a_it = calls.find(akey);
			if (a_it == calls.end())
				return false;
			auto b_it = calls.find(bkey);
			if (b_it == calls.end())
				return false;

			const auto &a = a_it->second;
			const auto &b = b_it->second;

			if (!a.Material || !b.Material)
				return false;

			const auto &m1 = *a.Material;
			const auto &m2 = *b.Material;

			/*if (m1.DiffuseColor.w < m2.DiffuseColor.w)
				return true;
			else if (m1.DiffuseColor.w == m2.DiffuseColor.w)
			{
				if (a.Geometry->Get() < b.Geometry->Get())
					return true;
				else if (a.Geometry->Get() == b.Geometry->Get())
				{
					if (a.Matrix && b.Matrix)
					{
						floaty3 av = View.Transform({ a.Matrix->dx, a.Matrix->dy, a.Matrix->dz }), bv = View.Transform({ b.Matrix->dx, b.Matrix->dy, b.Matrix->dz });
						if (av.z < (bv.z - Margin))
							return false;
						else if (av.z > (bv.z + Margin))
							return true;
					}
					if (&m1 < &m2)
						return true;
					else if (&m1 == &m2)
					{
						if (a.Textures.get() < b.Textures.get())
							return true;
						else if (a.Textures.get() == b.Textures.get())
							return akey < bkey;
					}
				}
			}

			return false;*/

			if (m1.DiffuseColor.w < 1.f)
			{
				if (m2.DiffuseColor.w == 1.f)
					return true;

				floaty3 al = { 0.f, 0.f, 0.f }, bl = { 0.f, 0.f, 0.f }; // A Local, B local
				if (a.Matrix)
					al = { a.Matrix->dx, a.Matrix->dy, a.Matrix->dz };
				if (b.Matrix)
					bl = { b.Matrix->dx, b.Matrix->dy, b.Matrix->dz };

				floaty3 av = View.Transform(al), bv = View.Transform(bl); // A View space, B View space
				if (av.z < (bv.z - Margin))
					return false;
				else if (av.z > (bv.z + Margin))
					return true;

				if (a.Material.get() < b.Material.get())
					return true;
				else if (a.Material.get() > b.Material.get())
					return false;

				if (a.Geometry->Get() < b.Geometry->Get())
					return true;
				else 
					return false;
			}
			else
			{
				if (m2.DiffuseColor.w < 1.f)
					return false;

				if (m_CompareDistanceOpaque)
				{
					floaty3 al = { 0.f, 0.f, 0.f }, bl = { 0.f, 0.f, 0.f }; // A Local, B local
					if (a.Matrix)
						al = { a.Matrix->dx, a.Matrix->dy, a.Matrix->dz };
					if (b.Matrix)
						bl = { b.Matrix->dx, b.Matrix->dy, b.Matrix->dz };

					floaty3 av = View.Transform(al), bv = View.Transform(bl); // A View space, B View space

					if (av.z < (bv.z - Margin))
						return true;
					else if (av.z > (bv.z + Margin))
						return false;
				}

				if (a.Material.get() < b.Material.get())
					return true;
				else if (a.Material.get() > b.Material.get())
					return false;

				if (a.Geometry->Get() < b.Geometry->Get())
					return true;
				else
					return false;
			}
			return false;
		}
	};

	std::vector<size_t> transparentKeys{};
	std::vector<size_t> opaqueKeys{};

	transparentKeys.reserve(keys.size());
	opaqueKeys.reserve(keys.size());

	for (int i = 0; i < keys.size(); ++i)
	{
		const auto& key = keys[i];
		auto findIt = calls.find(key);
		if (findIt == calls.end())
			continue;
		
		const auto& call = findIt->second;
		const auto& mat = *call.Material;

		if (mat.DiffuseColor.w < 1.f)
			transparentKeys.push_back(key);
		else
			opaqueKeys.push_back(key);
	}

	std::sort(transparentKeys.rbegin(), transparentKeys.rend(), DrawAfterGuy(calls, View3D, false));

	int key_index = 0;
	for (int i = 0; i < transparentKeys.size(); ++i, ++key_index)
	{
		keys[key_index] = transparentKeys[i];
	}
	for (int i = 0; i < opaqueKeys.size(); ++i, ++key_index)
	{
		keys[key_index] = opaqueKeys[i];
	}
	for (; keys.size() > key_index; ++key_index)
		keys.pop_back();
}

void GLRen::Drawv2(Matrixy4x4 view, Matrixy4x4 proj, Voxel::CameraFrustum frustum)
{
	m_Renv2.Draw(view, proj, frustum);
}

void GLRen::Resize(unsigned int NewWidth, unsigned int NewHeight)
{
	ProjectionMatrix = Matrixy4x4::OrthoProject((float)NewWidth, (float)NewHeight, 1.f, -1.f);
	glBindBuffer(GL_UNIFORM_BUFFER, Proj2DBuffer.Get());
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrixy4x4), ProjectionMatrix.ma);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glViewport(0, 0, (GLsizei)NewWidth, (GLsizei)NewHeight);
}

void GLRen::Present(SDL_Window *win)
{
	PROFILE_PUSH("GLRen Present");
	CHECK_GL_ERR("Uncaught Error before Presenting");
	if (DidWork) 
		SDL_GL_SwapWindow(win); 
	DidWork = false;
	PRINT_GL_ERR("After Presenting (most likely some accumulated errors being reported on present, rather than a problem with present)");
	PROFILE_POP();
}

GLProgram GLRen::InitImage()
{
	return GLProgram(
		{
			{"Shaders/image.glvs", GL_VERTEX_SHADER},
			{"Shaders/image.glfs", GL_FRAGMENT_SHADER}
		}
		);
}

GLuint GLRen::InitImageSampler()
{
	GLuint out = glGetUniformLocation(ImageProgram.Get(), "TextureThing");
	if (!out)
		throw GL_EXCEPT("glGetUniformLocation");
	return out;
}

GLuint GLRen::InitImageVAO()
{
	GLuint out;
	glGenVertexArrays(1, &out);
	CHECK_GL_ERR("glGenVertexArrays");
	return out;
}

GLuint GLRen::InitImageVBO()
{
	GLuint VBOs[2] = { 0, 0 };

	floaty2 initPos[4] =
	{  // 2D Coords,   Tex Coords
		{ -1.f, -1.f }, // Bottom Left
		{ +1.f, -1.f }, // Bottom Right
		{ +1.f, +1.f }, // Top Right
		{ -1.f, +1.f },  // Top Left
	};

	floaty2 initTex[4] =
	{
		{ 0.f, 1.f },
		{ 1.f, 1.f },
		{ 1.f, 0.f },
		{ 0.f, 0.f }
	};

	glBindVertexArray(ImageVAO.Get());

	glGenBuffers(2, VBOs);
	CHECK_GL_ERR("glGenBuffers");
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(floaty2), initPos, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(ImagePosLoc);
	glVertexAttribPointer(ImagePosLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	CHECK_GL_ERR("glBuffer things");

	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(floaty2), initTex, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ImageTexLoc);
	glVertexAttribPointer(ImageTexLoc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	CHECK_GL_ERR("glBuffer things");


	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	ImageVBOTex = VBOs[1];
	return VBOs[0];
}

GLuint GLRen::InitImageIBO()
{
	GLuint IBO = 0;

	unsigned int initDat[] =
	{
		0,
		1,
		2,
		3,
	};

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(unsigned int), initDat, GL_STATIC_DRAW);
	CHECK_GL_ERR("glBuffer things");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return IBO;
}

GLuint GLRen::InitImageTexTrans()
{
	GLuint out = glGetUniformLocation(ImageProgram.Get(), "TexTrans");
	if (out == -1)
		throw GL_EXCEPT("glGetUniformLocation");
	return out;
}

GLProgram GLRen::Init2D()
{
	return GLProgram(
		{
			{"Shaders/draw.glvs", GL_VERTEX_SHADER},
			{"Shaders/draw.glfs", GL_FRAGMENT_SHADER}
		}
		);
}

GLuint GLRen::InitDrawVAO()
{
	GLuint out;
	glGenVertexArrays(1, &out);
	CHECK_GL_ERR("glGenVertexArrays");
	return out;
}

GLuint GLRen::InitDrawVBO()
{
	GLuint VBO = 0;

	floaty2 initPos[4] =
	{  // 2D Coords,   Tex Coords
		{ -1.f, -1.f }, // Bottom Left
		{ +1.f, -1.f }, // Bottom Right
		{ +1.f, +1.f }, // Top Right
		{ -1.f, +1.f },  // Top Left
	};

	glBindVertexArray(DrawVAO.Get());

	glGenBuffers(1, &VBO);
	CHECK_GL_ERR("glGenBuffers");
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(floaty2), initPos, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	CHECK_GL_ERR("glBuffer things");

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return VBO;
}

GLuint GLRen::InitDrawIBO()
{
	GLuint IBO = 0;

	unsigned int initDat[] =
	{
		0,
		1,
		2,
		3
	};

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(unsigned int), initDat, GL_STATIC_DRAW);
	CHECK_GL_ERR("glBuffer things");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return IBO;
}

GLuint GLRen::InitDrawColor()
{
	GLuint out = glGetUniformLocation(Program2D.Get(), "DrawColor");
	if (out == -1)
		throw GL_EXCEPT("glGetUniformLocation");
	return out;
}

GLuint GLRen::InitProjMatrix()
{
	ProjMat2DBinding = Drawing::BindingManager::GetNext();

	GLint index1 = glGetUniformBlockIndex(Program2D.Get(), "SharedProjData");
	GLint index2 = glGetUniformBlockIndex(this->ImageProgram.Get(), "SharedProjData");
	glUniformBlockBinding(Program2D.Get(), index1, ProjMat2DBinding);
	glUniformBlockBinding(ImageProgram.Get(), index2, ProjMat2DBinding);
	CHECK_GL_ERR("Binding Projection Matrix Buffer");

	GLuint out = 0;
	glGenBuffers(1, &out);
	glBindBufferBase(GL_UNIFORM_BUFFER, ProjMat2DBinding, out);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrixy4x4), ProjectionMatrix.ma, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	CHECK_GL_ERR("Initializing Projection Matrix Buffer");

	return out;
}

GLuint GLRen::InitTransMatrix()
{
	TransMat2DLoc = Drawing::BindingManager::GetNext();

	GLint index1 = glGetUniformBlockIndex(Program2D.Get(), "SharedTransformData");
	GLint index2 = glGetUniformBlockIndex(this->ImageProgram.Get(), "SharedTransformData");
	glUniformBlockBinding(Program2D.Get(), index1, TransMat2DLoc);
	glUniformBlockBinding(ImageProgram.Get(), index2, TransMat2DLoc);
	CHECK_GL_ERR("Binding Object Transform Buffer");

	GLuint out = 0;
	glGenBuffers(1, &out);
	glBindBufferBase(GL_UNIFORM_BUFFER, TransMat2DLoc, out);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrixy4x4), TransformMatrix.ma, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	CHECK_GL_ERR("Initializing Object Transform Buffer");

	return out;
}

GLProgram GLRen::Init3D()
{
	return GLProgram(
		{
			{"Shaders/3DI.glvs", GL_VERTEX_SHADER},
			{"Shaders/3D.glfs", GL_FRAGMENT_SHADER}
		}
	);
}

GLuint GLRen::InitPerObjectBuffer()
{
	PerObjectLoc = Drawing::BindingManager::GetNext();

	GLint index = glGetUniformBlockIndex(Program3D.Get(), "PerObject");
	glUniformBlockBinding(Program3D.Get(), index, PerObjectLoc);
	CHECK_GL_ERR("Binding Buffer");

	GLuint out = 0;
	glGenBuffers(1, &out);
	glBindBufferBase(GL_UNIFORM_BUFFER, PerObjectLoc, out);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PerObjectDesc), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	CHECK_GL_ERR("Initializing Per Object Buffer");

	return out;
}

GLuint GLRen::InitMaterialBuffer()
{
	MaterialBufLoc = Drawing::BindingManager::GetNext();

	GLint index = glGetUniformBlockIndex(Program3D.Get(), "MaterialBuffer");
	glUniformBlockBinding(Program3D.Get(), index, MaterialBufLoc);

	GLuint out = 0;
	glGenBuffers(1, &out);
	glBindBufferBase(GL_UNIFORM_BUFFER, (GLuint)MaterialBufLoc, out);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), nullptr, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return out;
}

void GLRen::UpdateImageVertices(std::array<const floaty2, 4u> vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, ImageVBOPos.Get());
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floaty2) * 4u, vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
}

void GLRen::UpdateTexTrans(float x, float y)
{
	glUniform2f(ImageTexTransName, x, y);
}

void GLRen::UpdateDrawVerts(std::array<floaty2, 4u> vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, DrawVBO.Get());
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floaty2) * 4u, vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
}

void GLRen::UpdateDrawColor(floaty4 color)
{
	glUniform4f(DrawColorName, color.x, color.y, color.z, color.w);
}

void GLRen::UpdatePerObjectInstance(PerObjectInstanceDesc * desc)
{
	if (!desc)
		return;
	CHECK_GL_ERR("Pre-Updating Per Object Instance");
	glBindBuffer(GL_UNIFORM_BUFFER, PerObjectBuf.Get());
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerObjectInstanceDesc), desc);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	CHECK_GL_ERR("Updating Per Object Instance");
}

void GLRen::UpdatePerObject(Matrixy4x4 * mat)
{
	if (!mat)
		return;
	CHECK_GL_ERR("Pre-Updating Per Object Transform");
	glBindBuffer(GL_UNIFORM_BUFFER, PerObjectBuf.Get());
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrixy4x4), mat);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	CHECK_GL_ERR("Updating Per Object Transform");
}

void GLRen::UpdatePerObject(PerObjectDesc * desc)
{
	if (!desc)
		return;

	CHECK_GL_ERR("Pre-Updating Per Object Desc");
	glBindBuffer(GL_UNIFORM_BUFFER, PerObjectBuf.Get());
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerObjectDesc), desc);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	CHECK_GL_ERR("Updating Per Object Desc");
}

GLRen::GLRen(CommonResources *resources)
	: FullResourceHolder(resources)
	, m_DefaultTex(std::make_shared<Drawing::SDLFileImage>(resources, "default.jpg"))
	, DrawCallGuy(this, &TexMan, this, &GeoMan, this, resources)
	, ImageProgram(InitImage())
	, ImageVAO(InitImageVAO())
	, ImageVBOPos(InitImageVBO())
	, ImageIBO(InitImageIBO())
	, ImageSamplerLoc(InitImageSampler())
	, ImageTexTransName(InitImageTexTrans())
	, Program2D(Init2D())
	, DrawVAO(InitDrawVAO())
	, DrawVBO(InitDrawVBO())
	, DrawIBO(InitDrawIBO())
	, DrawColorName(InitDrawColor())
	, Proj2DBuffer(InitProjMatrix())
	, Trans2DBuffer(InitTransMatrix())
	, Program3D(Init3D())
	, PerObjectBuf(InitPerObjectBuffer())
	, m_MaterialBuf(InitMaterialBuffer())
	, TexMan(resources, m_DefaultTex, Program3D)
	, ParticleMan(resources, m_DefaultTex)
	, Lights(Program3D)
	, m_Renv2(resources)
{
	glUseProgram(Program3D.Get());
	TexMan.BlankifyTextues();

	glUseProgram(ImageProgram.Get());
	SetTransform(Matrixy2x3::Identity());
	OutputProgramLog(Program2D.Get());
}

void GLRen::DrawPoint(floaty2 point, floaty4 color)
{
	DidWork = true;
	glUseProgram(Program2D.Get());

	// Bind
	glBindVertexArray(DrawVAO.Get());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DrawIBO.Get());

	// Update Vertices
	std::array<floaty2, 4u> vertes =
	{
		point, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}
	};
	UpdateDrawVerts(vertes);
	UpdateDrawColor(color);

	// Draw
	glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, NULL);

	// Clean up
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void GLRen::DrawLine(floaty2 a, floaty2 b, floaty4 color)
{
	DidWork = true;
	glUseProgram(Program2D.Get());

	// Bind
	glBindVertexArray(DrawVAO.Get());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DrawIBO.Get());

	// Update Vertices
	std::array<floaty2, 4u> vertes =
	{
		a, b, {0.f, 0.f}, {0.f, 0.f}
	};
	UpdateDrawVerts(vertes);
	UpdateDrawColor(color);

	// Draw
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, NULL);

	// Clean up
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void GLRen::DrawRectangle(floaty2 topleft, floaty2 bottomright, floaty4 color)
{
	DidWork = true;
	glUseProgram(Program2D.Get());

	// Bind
	glBindVertexArray(DrawVAO.Get());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DrawIBO.Get());

	// Update Vertices
	std::array<floaty2, 4u> vertes =
	{
		floaty2{topleft.x, bottomright.y}, {bottomright.x, bottomright.y}, {bottomright.x, topleft.y}, {topleft.x, topleft.y}
	};
	UpdateDrawVerts(vertes);
	UpdateDrawColor(color);

	// Draw
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, NULL);

	// Clean up
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void GLRen::DrawRectangle(floaty2 a, floaty2 b, floaty2 c, floaty2 d, floaty4 color)
{
	DidWork = true;
	glUseProgram(Program2D.Get());

	// Bind
	glBindVertexArray(DrawVAO.Get());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DrawIBO.Get());

	// Update Vertices
	std::array<floaty2, 4u> vertes =
	{
		a, b, c, d
	};
	UpdateDrawVerts(vertes);
	UpdateDrawColor(color);

	// Draw
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, NULL);

	// Clean up
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void GLRen::FillRectangle(floaty2 topleft, floaty2 bottomright, floaty4 color)
{
	DidWork = true;
	glUseProgram(Program2D.Get());

	// Bind
	glBindVertexArray(DrawVAO.Get());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DrawIBO.Get());

	// Update Vertices
	std::array<floaty2, 4u> vertes =
	{
		floaty2{topleft.x, bottomright.y}, {bottomright.x, bottomright.y}, {bottomright.x, topleft.y}, {topleft.x, topleft.y}
	};
	UpdateDrawVerts(vertes);
	UpdateDrawColor(color);

	// Draw
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

	// Clean up
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void GLRen::FillRectangle(floaty2 a, floaty2 b, floaty2 c, floaty2 d, floaty4 color)
{
	DidWork = true;
	glUseProgram(Program2D.Get());

	// Bind
	glBindVertexArray(DrawVAO.Get());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DrawIBO.Get());

	// Update Vertices
	std::array<floaty2, 4u> vertes =
	{
		a, b, c, d
	};
	UpdateDrawVerts(vertes);
	UpdateDrawColor(color);

	// Draw
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

	// Clean up
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void GLRen::DrawTriangle(floaty2 a, floaty2 b, floaty2 c, floaty4 color)
{
	DidWork = true;
	glUseProgram(Program2D.Get());

	// Bind
	glBindVertexArray(DrawVAO.Get());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DrawIBO.Get());

	// Update Vertices
	std::array<floaty2, 4u> vertes =
	{
		a, b, c, { 0.f, 0.f }
	};
	UpdateDrawVerts(vertes);
	UpdateDrawColor(color);

	// Draw
	glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, NULL);

	// Clean up
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void GLRen::FillTriangle(floaty2 a, floaty2 b, floaty2 c, floaty4 color)
{
	DidWork = true;
	glUseProgram(Program2D.Get());

	// Bind
	glBindVertexArray(DrawVAO.Get());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DrawIBO.Get());

	// Update Vertices
	std::array<floaty2, 4u> vertes =
	{
		a, b, c, { 0.f, 0.f }
	};
	UpdateDrawVerts(vertes);
	UpdateDrawColor(color);

	// Draw
	glDrawElements(GL_TRIANGLE_FAN, 3, GL_UNSIGNED_INT, NULL);

	// Clean up
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void GLRen::DrawVertices(std::vector<floaty2> vertices, floaty4 color)
{
	DidWork = true;
	glUseProgram(Program2D.Get());

	// Bind
	glBindVertexArray(DrawVAO.Get());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DrawIBO.Get());

	// Update Vertices
	std::array<floaty2, 4u> vertes;
	UpdateDrawColor(color);

	for (size_t i = 0; (i + 1) * 3 <= vertices.size(); ++i)
	{
		vertes[0] = vertices[i * 3 + 0];
		vertes[1] = vertices[i * 3 + 1];
		vertes[2] = vertices[i * 3 + 2];

		UpdateDrawVerts(vertes);

		glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, NULL);
	}

	// Draw

	// Clean up
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void GLRen::FillVertices(std::vector<floaty2> vertices, floaty4 color)
{
	DidWork = true;
	glUseProgram(Program2D.Get());

	// Bind
	glBindVertexArray(DrawVAO.Get());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DrawIBO.Get());

	// Update Vertices
	std::array<floaty2, 4u> vertes;
	UpdateDrawColor(color);

	for (size_t i = 0; (i + 1) * 3 <= vertices.size(); ++i)
	{
		vertes[0] = vertices[i * 3 + 0];
		vertes[1] = vertices[i * 3 + 1];
		vertes[2] = vertices[i * 3 + 2];

		UpdateDrawVerts(vertes);

		glDrawElements(GL_TRIANGLE_FAN, 3, GL_UNSIGNED_INT, NULL);
	}

	// Draw

	// Clean up
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void GLRen::DrawVertices(std::vector<floaty2> vertices, floaty4 color, GLenum mode)
{
	DWARNING("Fucka de you, no draw vertices for you");
}

void GLRen::FillVertices(std::vector<floaty2> vertices, floaty4 color, GLenum mode)
{
	DWARNING("Fucka de you, no fill vertices for you");
}

void GLRen::DrawImage(Drawing::SDLImage * im)
{
	DidWork = true;
	DrawImage(im, { 0.f, 0.f });
}

GeometryManager::~GeometryManager()
{
	CleanUp();
}

size_t GeometryManager::AddGeometry(std::vector<FullVertex> vertices, std::vector<unsigned int> indices)
{
	if (vertices.empty())
		DWARNING("No Vertices are being added in this AddGeometry call");
	// Create Description
	GeometryInternalDesc desc;
	desc.Key = LastKey;
	desc.VertexStart = (unsigned int)this->Vertices.size();
	desc.VertexCount = (unsigned int)vertices.size();
	desc.IndexCount = (unsigned int)indices.size();
	desc.IndexStart = (unsigned int)this->Indices.size();

	// Add Geometry
	Vertices.insert(Vertices.end(), vertices.begin(), vertices.end());
	Indices.insert(Indices.end(), indices.begin(), indices.end());

	Descriptions.emplace_back(desc);

	DataChanged = true;
	
	return LastKey++;
}

void GeometryManager::RemoveGeometry(size_t key)
{
	if (key == 0)
		return;
	size_t index = 0ull;
	bool Found = false;
	for (size_t i = Descriptions.size(); i-- > 0; )
	{
		auto &desc = Descriptions[i];
		if (desc.Key == key)
		{
			index = i;
			Found = true;
			break;
		}
	}
	if (!Found)
	{
		DWARNING("Could not find Geometry key to remove");
		return;
	}
	else
		DINFO("Removing Geometry with key: " + std::to_string(key));

	auto &desc = Descriptions[index];

	// Decrement the following descriptions
	for (auto it = Descriptions.begin() + (index + 1); it < Descriptions.end(); ++it)
	{
		it->VertexStart -= desc.VertexCount;
		it->IndexStart -= desc.IndexCount;
	}

	// Erase the vertices and indices
	Vertices.erase(Vertices.begin() + desc.VertexStart, Vertices.begin() + (desc.VertexStart + desc.VertexCount));
	Indices.erase(Indices.begin() + desc.IndexStart, Indices.begin() + (desc.IndexStart + desc.IndexCount));

	// Remove the description

	Descriptions.erase(Descriptions.begin() + index);

	DataChanged = true;
}

void GeometryManager::Purge()
{
	VAO.Reset();
	VertexBuffer.Reset();
	IndexBuffer.Reset();
	Vertices.clear();
	Indices.clear();
	Descriptions.clear();
}

void GeometryManager::DrawInstanced(std::shared_ptr<GeoThing> geo, const std::vector<Matrixy4x4>& matrices)
{
	if (!geo)
		return;
	auto key = geo->Get();
	if (key == 0 || matrices.empty())
		return;
	if (DataChanged)
		UpdateGPUData();
	
	GeometryInternalDesc desc;
	if (Get(key, desc))
	{
		UpdateInstanceBuffer(matrices);

		glBindVertexArray(VAO.Get());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer.Get());

		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, (GLsizei)desc.IndexCount, GL_UNSIGNED_INT, (GLvoid*)(desc.IndexStart * sizeof(GLuint)), (GLsizei)matrices.size(), (GLint)desc.IndexOffset);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}

void GeometryManager::CleanUp()
{
	if (Descriptions.size())
	{
		DWARNING("Improper clean up of Geometry, manually cleaning up (if this is not at the end of execution (in the destructor) or at the end of a scene something may be wrong)");
		for (auto it = Descriptions.begin(); Descriptions.size(); it = Descriptions.begin())
		{
			RemoveGeometry(it->Key);
		}
		if (Descriptions.size())
			DWARNING("Wtf this warning should not happen if it did somehow I can't help you man (There's still geometry descriptions ");
	}
	else
	{
		DINFO("Proper clean up of Geometry was done, Good job");
	}
}

void GeometryManager::UpdateGPUData()
{
	VAO.Reset();
	VertexBuffer.Reset();
	IndexBuffer.Reset();

	if (Vertices.empty())
		return;

	CHECK_GL_ERR("Before Updating GPU Geometry Data");
	GLuint buffers[2] = { 0, 0 };
	glGenBuffers(2, buffers);
	VertexBuffer.Reset(buffers[0]);
	IndexBuffer.Reset(buffers[1]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer.Get());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), Indices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer.Get());
	glBufferData(GL_ARRAY_BUFFER, sizeof(FullVertex) * Vertices.size(), Vertices.data(), GL_STATIC_DRAW);

	// VAO
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	VAO.Reset(vao);
	m_RedoInstanceAttribs = true;

	// Vertex Buffer is already bound
	glBindVertexArray(VAO.Get());

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FullVertex), (GLvoid*)offsetof(FullVertex, PosL));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(FullVertex), (GLvoid*)offsetof(FullVertex, TangentL));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(FullVertex), (GLvoid*)offsetof(FullVertex, BinormalL));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, sizeof(FullVertex), (GLvoid*)offsetof(FullVertex, NormalL));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(FullVertex), (GLvoid*)offsetof(FullVertex, Tex));
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	CHECK_GL_ERR("Updating GPU Geometry Data");

	DataChanged = false;
}

void GeometryManager::UpdateInstanceBuffer(const std::vector<Matrixy4x4>& matrices)
{
	CHECK_GL_ERR("Pre-Instance buffer stuff");
	if (matrices.size() > InstanceBufferSize || InstanceBuffer.Get() == 0 || m_RedoInstanceAttribs)
	{
		InstanceBufferSize = (InstanceBufferSize > matrices.size() ? InstanceBufferSize : matrices.size());
		// recreate instance buffer
		glBindVertexArray(VAO.Get());
		GLuint newbuf = 0u;
		glGenBuffers(1, &newbuf);
		glBindBuffer(GL_ARRAY_BUFFER, newbuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Matrixy4x4) * InstanceBufferSize, matrices.data(), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Matrixy4x4), (GLvoid*)offsetof(Matrixy4x4, _11));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Matrixy4x4), (GLvoid*)offsetof(Matrixy4x4, _12));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Matrixy4x4), (GLvoid*)offsetof(Matrixy4x4, _13));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(Matrixy4x4), (GLvoid*)offsetof(Matrixy4x4, _14));

		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		InstanceBuffer.Reset(newbuf);
		glBindVertexArray(0);

		m_RedoInstanceAttribs = false;
	}
	else
	{
		// just copy over new stuff
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBuffer.Get());
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Matrixy4x4) * matrices.size(), matrices.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	CHECK_GL_ERR("Instance buffer stuff");
}

bool GeometryManager::Get(size_t Key, GeometryInternalDesc & out)
{
	size_t Index = Key + 1;
	if (Index >= Descriptions.size())
		Index = Descriptions.size();

	for (; Index-- > 0; )
	{
		if (Descriptions[Index].Key == Key)
		{
			out = Descriptions[Index];
			return true;
		}
	}

	return false;
}

bool LightManager::Apply()
{
	// Assume lights have been transformed, meaning they need transfer
	CHECK_GL_ERR("Pre-Updating Lights");
	glBindBuffer(GL_UNIFORM_BUFFER, LightBuffer.Get());
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * Lights.size(), Lights.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	CHECK_GL_ERR("Updating Lights");

	return false; // Can't be screwed doing this lol
}

void LightManager::UpdateLightVS(Matrixy4x4 View)
{
	for (unsigned int i = LightCount; i-- > 0; )
	{
		auto &Light = Lights[i];
		if (Light.Enabled)
		{
			Light.DirectionVS = View.TransformNormal(Light.DirectionWS);
			Light.PositionVS = View.Transform(Light.PositionWS);
		}
	}
}

GLuint LightManager::InitLightBuffer()
{
	memset(Lights.data(), 0, sizeof(Lights));

	m_LightBufferLoc = Drawing::BindingManager::GetNext();

	GLint index = glGetUniformBlockIndex(m_Program.Get(), "Lights");
	glUniformBlockBinding(m_Program.Get(), index, m_LightBufferLoc);

	CHECK_GL_ERR("Pre-Creating Light buffer");
	GLuint out = 0;
	glGenBuffers(1, &out);
	glBindBufferBase(GL_UNIFORM_BUFFER, m_LightBufferLoc, out);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * LightCount, Lights.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	CHECK_GL_ERR("Creating Light buffer");

	return out;
}

void TextureManager::ApplyTextures(Textures *textures)
{
	if (!textures)
		return;
	auto &tex = *textures;
	CHECK_GL_ERR("Pre-Applying Texture Info");
	TexScaleDesc scaling;
	if (tex.DiffuseTexture)
	{
		tex.DiffuseTexture->LoadGL();
		glActiveTexture(GL_TEXTURE0 + DiffuseTexName);
		glBindTexture(GL_TEXTURE_2D, tex.DiffuseTexture->Get());
		glUniform1i(DiffuseSamplerLoc, DiffuseTexName);
		scaling.DiffuseScale.x = tex.DiffuseTexture->GetWidthScale();
		scaling.DiffuseScale.y = tex.DiffuseTexture->GetHeightScale();
	}
	else
	{
		// Set to default texture
		glUniform1i(DiffuseSamplerLoc, 0);
		scaling.DiffuseScale.x = scaling.DiffuseScale.y = 1.f;
	}
	if (tex.OpacityTexture)
	{
		tex.OpacityTexture->LoadGL();
		glActiveTexture(GL_TEXTURE0 + OpacityTexName);
		glBindTexture(GL_TEXTURE_2D, tex.OpacityTexture->Get());
		glUniform1i(OpacitySamplerLoc, OpacityTexName);
		scaling.OpacityScale.x = tex.OpacityTexture->GetWidthScale();
		scaling.OpacityScale.y = tex.OpacityTexture->GetHeightScale();
	}
	else
	{
		// Set to default texture
		glUniform1i(OpacitySamplerLoc, 0);
		scaling.OpacityScale.x = scaling.OpacityScale.y = 1.f;
	}
	if (tex.AmbientTexture)
	{
		tex.AmbientTexture->LoadGL();
		glActiveTexture(GL_TEXTURE0 + AmbientTexName);
		glBindTexture(GL_TEXTURE_2D, tex.AmbientTexture->Get());
		glUniform1i(AmbientSamplerLoc, AmbientTexName);
		scaling.AmbientScale.x = tex.AmbientTexture->GetWidthScale();
		scaling.AmbientScale.y = tex.AmbientTexture->GetHeightScale();
	}
	else
	{
		// Set to default texture
		glUniform1i(AmbientSamplerLoc, 0);
		scaling.AmbientScale.x = scaling.AmbientScale.y = 1.f;
	}
	if (tex.EmissiveTexture)
	{
		tex.EmissiveTexture->LoadGL();
		glActiveTexture(GL_TEXTURE0 + EmissiveTexName);
		glBindTexture(GL_TEXTURE_2D, tex.EmissiveTexture->Get());
		glUniform1i(EmissiveSamplerLoc, EmissiveTexName);
		scaling.EmissiveScale.x = tex.EmissiveTexture->GetWidthScale();
		scaling.EmissiveScale.y = tex.EmissiveTexture->GetHeightScale();
	}
	else
	{
		// Set to default texture
		glUniform1i(EmissiveSamplerLoc, 0);
		scaling.EmissiveScale.x = scaling.EmissiveScale.y = 1.f;
	}
	if (tex.SpecularTexture)
	{
		tex.SpecularTexture->LoadGL();
		glActiveTexture(GL_TEXTURE0 + SpecularTexName);
		glBindTexture(GL_TEXTURE_2D, tex.SpecularTexture->Get());
		glUniform1i(SpecularSamplerLoc, SpecularTexName);
		scaling.SpecularScale.x = tex.SpecularTexture->GetWidthScale();
		scaling.SpecularScale.y = tex.SpecularTexture->GetHeightScale();
	}
	else
	{
		// Set to default texture
		glUniform1i(SpecularSamplerLoc, 0);
		scaling.SpecularScale.x = scaling.SpecularScale.y = 1.f;
	}
	if (tex.SpecularPowerTexture)
	{
		tex.SpecularPowerTexture->LoadGL();
		glActiveTexture(GL_TEXTURE0 + SpecularPowerTexName);
		glBindTexture(GL_TEXTURE_2D, tex.SpecularPowerTexture->Get());
		glUniform1i(SpecularPowerSamplerLoc, SpecularPowerTexName);
		scaling.SpecularPowerScale.x = tex.SpecularPowerTexture->GetWidthScale();
		scaling.SpecularPowerScale.y = tex.SpecularPowerTexture->GetHeightScale();
	}
	else
	{
		// Set to default texture
		glUniform1i(SpecularPowerSamplerLoc, 0);
		scaling.SpecularPowerScale.x = scaling.SpecularPowerScale.y = 1.f;
	}
	if (tex.NormalTexture)
	{
		tex.NormalTexture->LoadGL();
		glActiveTexture(GL_TEXTURE0 + NormalTexName);
		glBindTexture(GL_TEXTURE_2D, tex.NormalTexture->Get());
		glUniform1i(NormalSamplerLoc, NormalTexName);
		scaling.NormalScale.x = tex.NormalTexture->GetWidthScale();
		scaling.NormalScale.y = tex.NormalTexture->GetHeightScale();
	}
	else
	{
		// Set to default texture
		glUniform1i(NormalSamplerLoc, 0);
		scaling.NormalScale.x = scaling.NormalScale.y = 1.f;
	}
	if (tex.BumpTexture)
	{
		tex.BumpTexture->LoadGL();
		glActiveTexture(GL_TEXTURE0 + BumpTexName);
		glBindTexture(GL_TEXTURE_2D, tex.BumpTexture->Get());
		glUniform1i(BumpSamplerLoc, BumpTexName);
		scaling.BumpScale.x = tex.BumpTexture->GetWidthScale();
		scaling.BumpScale.y = tex.BumpTexture->GetHeightScale();
	}
	else
	{
		// Set to default texture
		glUniform1i(BumpSamplerLoc, 0);
		scaling.BumpScale.x = scaling.BumpScale.y = 1.f;
	}
	// Update GPU's Texture Scale Buffer (because textures are scaled to fit power 2 dimensions)
	glBindBuffer(GL_UNIFORM_BUFFER, m_TexScaleBuffer.Get());
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(TexScaleDesc), &scaling);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	CHECK_GL_ERR("Applying Texture Info");
}

void TextureManager::BlankifyTextues()
{
	CHECK_GL_ERR("Before Blanking Textures");
	glUniform1i(DiffuseSamplerLoc, 0);
	glUniform1i(OpacitySamplerLoc, 0);
	glUniform1i(AmbientSamplerLoc, 0);
	glUniform1i(EmissiveSamplerLoc, 0);
	glUniform1i(SpecularSamplerLoc, 0);
	glUniform1i(SpecularPowerSamplerLoc, 0);
	glUniform1i(NormalSamplerLoc, 0);
	glUniform1i(BumpSamplerLoc, 0);
	CHECK_GL_ERR("glUniform - Blanking Textures");
}

void TextureManager::InitializeLocations()
{
	CHECK_GL_ERR("Before getting Sampler Locations");
	DiffuseSamplerLoc = glGetUniformLocation(m_Program.Get(), "DiffuseTexture");
	OpacitySamplerLoc = glGetUniformLocation(m_Program.Get(), "OpacityTexture");
	AmbientSamplerLoc = glGetUniformLocation(m_Program.Get(), "AmbientTexture");
	EmissiveSamplerLoc = glGetUniformLocation(m_Program.Get(), "EmissiveTexture");
	SpecularSamplerLoc = glGetUniformLocation(m_Program.Get(), "SpecularTexture");
	SpecularPowerSamplerLoc = glGetUniformLocation(m_Program.Get(), "SpecularPowerTexture");
	NormalSamplerLoc = glGetUniformLocation(m_Program.Get(), "NormalTexture");
	BumpSamplerLoc = glGetUniformLocation(m_Program.Get(), "BumpTexture");
	
	CHECK_GL_ERR("glGetUniformLocation");
}

GLuint TextureManager::InitTexScaleBuffer()
{
	TexScaleLoc = Drawing::BindingManager::GetNext();

	GLint index = glGetUniformBlockIndex(m_Program.Get(), "TextureScales");
	glUniformBlockBinding(m_Program.Get(), index, TexScaleLoc);
	CHECK_GL_ERR("Binding Tex Scale Buffer");

	GLuint out = 0;
	glGenBuffers(1, &out);
	glBindBufferBase(GL_UNIFORM_BUFFER, TexScaleLoc, out);
	TexScaleDesc emptybuf;
	glBufferData(GL_UNIFORM_BUFFER, sizeof(TexScaleDesc), &emptybuf, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	CHECK_GL_ERR("gl uniform buffer stuff");
	return out;
}

size_t GLRen::AddExternalParticle(std::shared_ptr<Drawing::SDLImage> texture, Particles::BasicParticle particle)
{
	return ParticleMan.AddExternalParticle(texture, particle);
}

std::vector<size_t> GLRen::AddExternalParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<Particles::BasicParticle> particles)
{
	return ParticleMan.AddExternalParticles(texture, particles);
}

void GLRen::RemoveExternalParticle(Drawing::SDLImage *texture, size_t key)
{
	ParticleMan.RemoveExternalParticle(texture, key);
}

void GLRen::RemoveExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> keys)
{
	ParticleMan.RemoveExternalParticles(texture, keys);
}

void GLRen::AddVelocityParticle(std::shared_ptr<Drawing::SDLImage> texture, Particles::BasicParticle basic, Particles::VelocityParticleData velodat)
{
	ParticleMan.AddVelocityParticle(texture, basic, velodat);
}

void GLRen::AddVelocityParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<Particles::BasicParticle> basic, std::vector<Particles::VelocityParticleData> props)
{
	ParticleMan.AddVelocityParticles(texture, basic, props);
}

void GLRen::AddVelocityParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<std::pair<Particles::BasicParticle, Particles::VelocityParticleData>> particles)
{
	ParticleMan.AddVelocityParticles(texture, particles);
}

Particles::BasicParticle *GLRen::GetExternalParticle(Drawing::SDLImage *texture, size_t key)
{
	return ParticleMan.GetExternalParticle(texture, key);
}

std::vector<Particles::BasicParticle *> GLRen::GetExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> key)
{
	return ParticleMan.GetExternalParticles(texture, key);
}

const Particles::BasicParticle *GLRen::GetExternalParticle(Drawing::SDLImage *texture, size_t key) const
{
	return ParticleMan.GetExternalParticle(texture, key);
}

std::vector<const Particles::BasicParticle *> GLRen::GetExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> keys) const
{
	return ParticleMan.GetExternalParticles(texture, keys);
}
