#include "DrawingHelper.h"

#include "ProfileHelper.h"

#include "Drawing/IRen3D.h"

void Drawing::DrawCallHolder::FreeDrawCall()
{
	if (DrawKey != DeadKey)
		mResources->Ren3->RemoveDrawCall(DrawKey);
	DrawKey = DeadKey;
}

size_t Drawing::DrawCaller::Add(DrawCall call)
{
	DrawCalls[LastKey] = call;
	Keys.emplace_back(LastKey);

	return LastKey++;
}

void Drawing::DrawCaller::Remove(size_t key)
{
	if (key == 0)
		return;
	auto it = DrawCalls.find(key);
	if (it == DrawCalls.end())
	{
		DWARNING("Could not find DrawCall with key: " + std::to_string(key));
		return;
	}

	DrawCalls.erase(key);

	auto ass = std::find(Keys.cbegin(), Keys.cend(), key);
	if (ass != Keys.cend())
		Keys.erase(ass);
	else
		DWARNING("Key to remove not found in Keys vector");
}

Drawing::DrawCall *Drawing::DrawCaller::Get(size_t key)
{
	if (DrawCalls.find(key) != DrawCalls.end())
		return &DrawCalls[key];
	return nullptr;
}

std::vector<size_t> &Drawing::DrawCaller::GetKeys()
{
	return Keys;
}

void Drawing::DrawCaller::Apply(const std::vector<size_t> &keys)
{
	PROFILE_PUSH("Apply Draw Calls");
	
	DrawCall lastcall;
	std::vector<Matrixy4x4> instancedata;
	if (DrawGuy)
		DrawGuy->Begin();
	bool done_transparent = false;
	bool do_transparent_pass = false;
	for (auto &index : keys)
	{
		auto &call = DrawCalls[index];
		if (call.Enabled)
		{
			if (done_transparent)
			{
				if (call.Material->DiffuseColor.w == 1.f)
				{
					DINFO("Opaque Material Drawn after Transparent Pass was enabled");
				}
			}
			else
			{
				if (call.Material->DiffuseColor.w < 1.f)
				{
					do_transparent_pass = true;
				}
			}
			bool Different = (TexGuy && call.textures != lastcall.textures) || (MatGuy && (call.Material.get() != lastcall.Material.get()));

			if (!Different && call.Geometry == lastcall.Geometry)
			{
				if (call.Matrix)
					instancedata.emplace_back(*call.Matrix);
				else
					instancedata.emplace_back(Matrixy4x4::Identity());
			}
			else
			{
				if (GeoGuy)
					GeoGuy->DrawInstanced(lastcall.Geometry, instancedata);
				instancedata.clear();
				if (call.Matrix)
					instancedata.emplace_back(*call.Matrix);
				else
					instancedata.emplace_back(Matrixy4x4::Identity());
			}
			if (Different)
			{
				if (MatGuy && call.Material != lastcall.Material)
					MatGuy->ApplyMaterial(call.Material.get());
				if (TexGuy && call.textures != lastcall.textures)
					TexGuy->ApplyTextures(call.textures.get());
			}
			lastcall = call;
			if (do_transparent_pass)
			{
				done_transparent = true;
				DrawGuy->DoTransparentPass();
			}
		}
	}
	if (keys.size())
	{
		if (instancedata.size())
		{
			if (instancedata.size() == 1)
				GeoGuy->DrawInstanced(lastcall.Geometry, instancedata);
			else
				GeoGuy->DrawInstanced(lastcall.Geometry, instancedata);
		}
	}

	if (DrawGuy)
		DrawGuy->End();
	PROFILE_POP();
}

std::vector<size_t> Drawing::DrawCaller::Cull(const std::vector<size_t> &keys, const Voxel::CameraFrustum &f)
{
	PROFILE_PUSH("Cull");
	std::vector<size_t> out;

	out.clear();
	out.reserve(keys.size());
	for (auto &key : keys)
	{
		auto it = DrawCalls.find(key);
		if (it != DrawCalls.end() && !it->second.FrustumCuller || it->second.FrustumCuller->InsideFrustum(f))
			out.push_back(key);
	}
	PROFILE_POP();

	return out;
}

void Drawing::DrawCaller::Sort(std::vector<size_t> &keys)
{
	PROFILE_PUSH("Sort");
	if (Sorter)
		Sorter->Sort(keys, DrawCalls);
	PROFILE_POP();
}
