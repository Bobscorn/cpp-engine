#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>



#include <memory>

#include "Drawing/Graphics3D.h"
#include "Drawing/Frustum.h"

#include "TransferHelper.h"

#include "Systems/Execution/ResourceHolder.h"

struct Matrixy4x4;

namespace Drawing
{	
	constexpr size_t DeadKey = 0;

	/*struct IMaterialManager;

	struct MaterialReference
	{
		MaterialReference(IMaterialManager *man);
		MaterialReference(IMaterialManager *man, Material mat);
		~MaterialReference();

		// Convience Functions
		::Material &Get();
		const ::Material &Get() const;

		MaterialReference &operator=(size_t key);

		void Reset();

		inline bool Valid() const noexcept { return m_Key; }

	protected:
		size_t m_Key = 0ull;
		IMaterialManager *m_Manager = nullptr;
	};

	struct IMaterialManager
	{
		virtual void AllocateReference(MaterialReference *ref) = 0;
		virtual void AllocateReference(Material mat, MaterialReference *ref);

		virtual ::Material &Get() = 0;
		virtual const ::Material &Get() const = 0;

		virtual void RemoveRef(MaterialReference *ref) = 0;
	};*/

	enum DrawingBehaviour
	{
		FrustumCull = 0x00000001,

	};

	struct ICuller
	{
		virtual bool InsideFrustum(const Voxel::CameraFrustum &f) = 0;
	};

	struct DrawCall
	{
		std::shared_ptr<GeoThing> Geometry = nullptr; // Passed to a IGeometryGuy
		std::shared_ptr<Textures> Textures = nullptr;
		std::shared_ptr<::Material> Material = nullptr;
		Matrixy4x4 *Matrix{ nullptr };
		bool Enabled = true;
		std::string DebugString;
		ICuller *FrustumCuller = nullptr;
	};

	struct KeyedCall
	{
		size_t Key;
		DrawCall Call;
	};

	struct DrawCallHolder : virtual FullResourceHolder
	{
		DrawCallHolder(CommonResources * resources) : FullResourceHolder(resources) {}
		DrawCallHolder(size_t key, CommonResources *resources = nullptr) : DrawKey(key), FullResourceHolder(resources) {}
		~DrawCallHolder() { FreeDrawCall(); }

		inline void SetDrawCall(size_t callKey)
		{
			if (DrawKey != DeadKey)
				FreeDrawCall();
			DrawKey = callKey;
		}

		void FreeDrawCall();

	protected:
		size_t DrawKey{ DeadKey };
	};
	
	// Called at beginning and end of Draw Calls
	struct IDrawGuy
	{
		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void DoTransparentPass() = 0;
	};

	// Applies Textures based on a size_t index input
	struct ITextureGuy
	{
		virtual ~ITextureGuy() {}

		virtual void ApplyTextures(Textures *tex) = 0;
	};

	struct IGeometryGuy
	{
		virtual ~IGeometryGuy() {}

		virtual void DrawInstanced(std::shared_ptr<GeoThing> geo, const std::vector<Matrixy4x4> &instancedata) = 0;
	};

	struct IMaterialGuy
	{
		virtual ~IMaterialGuy() {}

		virtual void ApplyMaterial(Material *mat) = 0;
	};

	struct ISorterGuy
	{
		virtual ~ISorterGuy() {}

		virtual void Sort(std::vector<size_t> &keys, const std::unordered_map<size_t, DrawCall> &calls) = 0;
	};

	struct DrawCaller : FullResourceHolder
	{
		DrawCaller(IDrawGuy *draw, ITextureGuy *texguy = nullptr, IMaterialGuy *matguy = nullptr, IGeometryGuy * geoguy = nullptr, ISorterGuy *sortguy = nullptr, CommonResources *resources = nullptr) : FullResourceHolder(resources), DrawGuy(draw), TexGuy(texguy), MatGuy(matguy), GeoGuy(geoguy), Sorter(sortguy) {}
		~DrawCaller()
		{
			if (DrawCalls.size())
				DWARNING("Draw calls remain on destruction, cleaning up, remaining DrawCalls will not know they are invalid");
			for (auto it = DrawCalls.begin(); DrawCalls.size(); it = DrawCalls.begin())
			{
				Remove(it->first);
			}
		}

		inline void SetTexGuy(ITextureGuy *guy) { TexGuy = guy; }
		inline void SetMatGuy(IMaterialGuy *guy) { MatGuy = guy; }
		inline void SetGeoGuy(IGeometryGuy *guy) { GeoGuy = guy; }
		inline void SetSorter(ISorterGuy *sorter) { Sorter = sorter; }

		size_t Add(DrawCall call);
		void Remove(size_t key);

		DrawCall *Get(size_t key);

		std::vector<size_t> &GetKeys();
		void Apply(const std::vector<size_t> &keys);
		std::vector<size_t> Cull(const std::vector<size_t> &keys, const Voxel::CameraFrustum &f);
		void Sort(std::vector<size_t> &keys); // Separated from Apply to allow for sorting after GPU workload has been sent

	protected:

		std::unordered_map<size_t, DrawCall> DrawCalls;
		std::vector<size_t> Keys;
		size_t LastKey = 1ull;

		IDrawGuy *DrawGuy = 0;
		ITextureGuy *TexGuy = 0;
		IGeometryGuy *GeoGuy = 0;
		IMaterialGuy *MatGuy = 0;
		ISorterGuy *Sorter = 0;
	};

}