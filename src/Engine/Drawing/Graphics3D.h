#pragma once

#include "Math/floaty.h"
#include "Math/matrix.h"
#include "Helpers/PointerHelper.h"

#include "Drawing/Image.h"

#include <memory>

struct Material
{
	constexpr Material() :
		AmbientColor(0.2f, 0.2f, 0.2f, 1.0f),
		EmissiveColor(0.f, 0.f, 0.f, 1.0f),
		DiffuseColor(0.5f, 0.5f, 0.5f, 1.0f),
		SpecularColor(0.75f, 0.75f, 0.7f, 12.0f),
		IndexOfRefraction(1.0f),
		TextureBitmap(0x0),
		Padding(0.f, 0.f)
	{
	}

	constexpr Material(floaty4 Ambient, floaty4 Emissive, floaty4 Diffuse, floaty4 Specular, float IOR, UINT TextureBitmap) :
		AmbientColor(Ambient),
		EmissiveColor(Emissive),
		DiffuseColor(Diffuse),
		SpecularColor(Specular),
		IndexOfRefraction(IOR),
		TextureBitmap(TextureBitmap),
		Padding(0.f, 0.f) {}

	floaty4 AmbientColor;
	// ------------------------------ 16 Bytes
	floaty4 EmissiveColor;
	// ------------------------------ 16 Bytes
	floaty4 DiffuseColor; // w = Opacity
	// ------------------------------ 16 Bytes
	floaty4 SpecularColor; // w = SpecularPower/SpecularScale for SpecularTexture
	// ------------------------------ 16 Bytes

	// For transparent materials, IOR > 0.
	float IndexOfRefraction;
	// Bit:
	// 0x1 1: Ambient
	// 0x2 2: Emissize
	// 0x4 3: Diffuse
	// 0x8 4: Specular
	// 0x10 5: Normal
	// 0x20 6: Bump
	// 0x40 7: Opacity
	UINT TextureBitmap;
	floaty2 Padding;
	// ------------------------------ 16 Bytes
}; // 16 * 5 = 80 Bytes

#define LIGHT_POINT 0u
#define LIGHT_SPOT 1u
#define LIGHT_DIRECTION 2u

#define LIGHT_COUNT 32u

enum LightType : uint32_t
{
	PointLight = LIGHT_POINT,
	SpotLight = LIGHT_SPOT,
	DirectionalLight = LIGHT_DIRECTION,
};

#define COLOR_NORMAL 0u
#define COLOR_NEGATIVE 1u
#define COLOR_INVERTED 2u
#define COLOR_INFRARED_FAKE 3u

constexpr auto BIT_AMBIENT = 0b1;
constexpr auto BIT_EMISSIZE = 0b10;
constexpr auto BIT_DIFFUSE = 0b100;
constexpr auto BIT_SPECULAR = 0b1000;
constexpr auto BIT_SPECULARPOWER = 0b10000;
constexpr auto BIT_NORMAL = 0b100000;
constexpr auto BIT_BUMP = 0b1000000;
constexpr auto BIT_OPACITY = 0b10000000;

struct ColorVertex3D
{
	floaty3 PosL;
	floaty3 Color;
};

struct TexVertex3D
{
	floaty3 PosL;
	floaty2 Tex;
};

struct ColorVertex2D
{
	floaty2 PosL;
	floaty3 Color;
};

struct TexVertex2D
{
	floaty2 PosL;
	floaty2 Tex;
};

struct FullVertex
{
	floaty3 PosL;
	floaty3 TangentL;
	floaty3 BinormalL;
	floaty3 NormalL;
	floaty2 Tex;
};

struct Light
{
	// Position for point and spot lights (World space, must end with w = 1.f).
	floaty4 PositionWS;

	//--------------------------------------------------------------( 16 bytes )

	// Direction for spot and directional lights (World space, w is not used).
	floaty4 DirectionWS;

	//--------------------------------------------------------------( 16 bytes )

	// Position for point and spot lights (View space).
	floaty4 PositionVS;

	//--------------------------------------------------------------( 16 bytes )

	// Direction for spot and directional lights (View space).
	floaty4 DirectionVS;

	//--------------------------------------------------------------( 16 bytes )

	// Color of the light. Diffuse and specular colors are not seperated.
	floaty3 Color;

	// The intensity of the Light
	float Intensity;

	//--------------------------------------------------------------( 16 bytes )
	floaty3 Attenuation;

	// The half angle of the spotlight cone. (in radians)
	float SpotlightAngle;

	// -------------------------------------------------------------( 16 bytes )
	// The range of the light.
	float Range;
	// Disable or enable the light.
	int Enabled;

	// The type of the light.
	unsigned int Type;

	float Padding;
	//--------------------------------------------------------------( 16 bytes )
}; // 96 Bytes

struct ShapeDesc
{
	std::vector<FullVertex> Vertices;
	std::vector<unsigned int> Indices;
	Pointer::observing_ptr<Matrixy4x4> Trans;
	Pointer::observing_ptr<Material> Mat;
};

struct Textures
{
	std::shared_ptr<Drawing::SDLImage>  DiffuseTexture;
	std::shared_ptr<Drawing::SDLImage>  OpacityTexture;
	std::shared_ptr<Drawing::SDLImage>  AmbientTexture;
	std::shared_ptr<Drawing::SDLImage>  EmissiveTexture;
	std::shared_ptr<Drawing::SDLImage>  SpecularTexture;
	std::shared_ptr<Drawing::SDLImage>  SpecularPowerTexture;
	std::shared_ptr<Drawing::SDLImage>  NormalTexture;
	std::shared_ptr<Drawing::SDLImage>  BumpTexture;
};

struct IRen3D;

struct GeoThing
{
	GeoThing(IRen3D *ren, size_t key) : m_Ren(ren), m_Key(key) {}
	GeoThing(GeoThing &&other) : m_Ren(other.m_Ren), m_Key(other.m_Key) { other.m_Key = 0ull; other.m_Ren = nullptr; }
	GeoThing(const GeoThing &other) = delete;
	~GeoThing() { Reset(); }

	inline GeoThing &operator=(GeoThing &&other)
	{
		Reset();

		this->m_Key = other.m_Key;
		this->m_Ren = other.m_Ren;
		other.m_Key = 0ull;
		other.m_Ren = nullptr;

		return *this;
	}

	GeoThing &operator=(const GeoThing &other) = delete;

	inline size_t Get() const noexcept { return m_Key; }
	void Reset();

protected:
	IRen3D *m_Ren;
	size_t m_Key;
};

constexpr unsigned int DefaultTextureLoc = 0u;
