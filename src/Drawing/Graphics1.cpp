//#include "Thing.h"
#include "Graphics1.h"

#include "Helpers/ProfileHelper.h"

#include "Helpers/IteratorHelper.h"

const std::string G1::IGSpace::RootShapeName = "";

//void G1::IGSpace::EraseEmptyDrawCalls(G1::DrawGroup& drawcalls)
//{
//	// Check for any null draw calls
//	for (size_t i = drawcalls.size(); i-- > 0; )
//	{
//		if (!drawcalls[i])
//		{
//			std::swap(drawcalls[i], drawcalls.back());
//			drawcalls.pop_back();
//		}
//	}
//}
//
//void G1::IGSpace::EraseEmptyDrawCalls(DrawGroup & drawcalls, std::vector<Drawing::DrawCallDebugInfo>& debug)
//{
//	for (size_t i = drawcalls.size(); i-- > 0; )
//	{
//		if (!drawcalls[i])
//		{
//			auto &indices = debug[i].i;
//			if (debug[i].Enabled)
//				DINFO(L"Removing DrawCall with '" + std::to_wstring(indices.VertexCount) + L"' vertices and '" + std::to_wstring(indices.IndexCount) + L"' indices that was enabled");
//			else
//				DINFO(L"Removing DrawCall with '" + std::to_wstring(indices.VertexCount) + L"' vertices and '" + std::to_wstring(indices.IndexCount) + L"' indices that was disabled");
//
//			std::swap(drawcalls[i], drawcalls.back());
//			std::swap(debug[i], debug.back());
//			drawcalls.pop_back();
//			debug.pop_back();
//		}
//	}
//}
//
//void G1::IGSpace::SortDrawingCalls(G1::DrawGroup& drawcalls)
//{
//	std::stable_sort(drawcalls.begin(), drawcalls.end());
//}

void G1::IGSpace::PhysicsCull(std::vector<std::shared_ptr<btCollisionObject>>& objs, std::vector<std::shared_ptr<btRigidBody>>& bodies)
{
	for (auto i = objs.size(); i-- > 0; )
	{
		auto& obj = objs[i];
		if (obj.use_count() < 2)
		{
			RemoveCollisionObject(obj.get());
			std::swap(objs[i], objs.back());
			objs.pop_back();
		}
		else if (obj.get() == nullptr)
		{
			std::swap(objs[i], objs.back());
			objs.pop_back();
		}
	}
	for (auto i = bodies.size(); i-- > 0; )
	{
		auto& body = bodies[i];
		if (body.use_count() == 1)
		{
			RemoveRigidBody(body.get());
			std::swap(bodies[i], bodies.back());
			bodies.pop_back();
		}
		else if (body.get() == nullptr)
		{
			std::swap(bodies[i], bodies.back());
			bodies.pop_back();
		}
	}
}

void G1::IGSpace::BeforeDrawShapes(IShape * shape)
{
	PROFILE_PUSH("Shape " + shape->GetName());
	if (shape)
	{
		shape->BeforeDraw();

		for (auto& child : shape->Children())
		{
			BeforeDrawShapes(child.get());
		}
	}
	PROFILE_POP();
}

void G1::IGSpace::DrawShapes(IShape *shape)
{
	PROFILE_PUSH("Draw " + shape->GetName());
	if (shape)
	{
		shape->Draw();

		for (auto &child : shape->Children())
		{
			DrawShapes(child.get());
		}
	}
	PROFILE_POP();
}

void G1::IGSpace::AfterDrawShapes(IShape * shape)
{
	PROFILE_PUSH("Shape " + shape->GetName());
	if (shape)
	{
		shape->AfterDraw();

		for (auto& child : shape->Children())
		{
			AfterDrawShapes(child.get());
		}
	}
	PROFILE_POP();
}

G1::IShape * G1::IGSpace::FindShapeyRaw(std::string name)
{
	if (name == RootShapeName)
		return &Root;

	return FindShapey(name).get();
}

Pointer::observing_ptr<G1::IShape> G1::IGSpace::FindShapey(std::string name)
{
	Pointer::selfish_ptr<IShape> *out = nullptr;
	find_struct ass;
	child_struct assy;
	for (auto& child : Root.Children())
		if (Iteration::HierachyBase<Pointer::selfish_ptr<IShape>, std::string, find_struct, child_struct>(name, child, ass, &out, assy))
			return Pointer::observing_ptr<G1::IShape>(*out);

	return Pointer::observing_ptr<G1::IShape>(nullptr);
}

Pointer::f_ptr<G1::IShape> G1::IGSpace::FindShapeFPtr(std::string name)
{
	Pointer::selfish_ptr<IShape>* out = nullptr;
	find_struct ass;
	child_struct assy;
	for (auto& child : Root.Children())
		if (Iteration::HierachyBase<Pointer::selfish_ptr<IShape>, std::string, find_struct, child_struct>(name, child, ass, &out, assy))
			return Pointer::f_ptr<G1::IShape>(*out);

	return Pointer::f_ptr<IShape>(nullptr);
}

void G1::IGSpace::BeforeDraw()
{
	PROFILE_PUSH("BeforeDraw Shapes");
	BeforeDrawShapes(&Root);
	PROFILE_POP();
	BeforeDrawI();
}

void G1::IGSpace::Draw()
{
	DrawI();
	DrawShapes(&Root);
}

void G1::IGSpace::AfterDraw()
{
	PROFILE_PUSH("AfterDraw Shapes");
	AfterDrawShapes(&Root);
	PROFILE_POP();
	AfterDrawI();
}

#ifdef POLYMORPHIC_SHAPE_CHILDREN
G1::ShapeIterator::ShapeIterator(const ShapeIterator & other) : Iter((other.Iter ? other.Iter->clone() : nullptr)) {}

G1::ShapeIterator::ShapeIterator(ShapeIterator && other) : Iter(std::move(other.Iter)) {}

inline G1::ShapeIterator & G1::ShapeIterator::operator=(const ShapeIterator & other)
{
	Iter.reset((other.Iter ? other.Iter->clone() : nullptr)); return *this;
}

inline G1::ShapeIterator & G1::ShapeIterator::operator=(ShapeIterator && other)
{
	Iter = std::move(other.Iter); return *this;
}

inline G1::ShapeIterator::reference G1::ShapeIterator::operator*()
{
	return **Iter;
}

inline G1::ShapeIterator::pointer G1::ShapeIterator::operator->()
{
	if (Iter) 
		return Iter->operator->(); 
	else 
		return nullptr;
}

inline G1::ShapeIterator & G1::ShapeIterator::operator++()
{
	++(*Iter); return *this;
}

inline G1::ShapeIterator G1::ShapeIterator::operator++(int)
{
	auto tmp = *this; ++(*Iter); return tmp;
}

inline G1::ShapeIterator & G1::ShapeIterator::operator--()
{
	--(*Iter); return *this;
}

inline G1::ShapeIterator G1::ShapeIterator::operator--(int)
{
	auto tmp = *this; --(*Iter); return tmp;
}

inline G1::ShapeIterator & G1::ShapeIterator::operator+=(size_t move)
{
	(*Iter) += move; return *this;
}

inline G1::ShapeIterator & G1::ShapeIterator::operator-=(size_t move)
{
	(*Iter) -= move; return *this;
}

inline G1::ShapeIterator::reference G1::ShapeIterator::operator[](size_t dif)
{
	return (*Iter)[dif];
}

inline bool G1::ShapeIterator::operator==(const ShapeIterator & other)
{
	return (*Iter) == (*other.Iter);
}

inline bool G1::ShapeIterator::operator!=(const ShapeIterator & other)
{
	return (*Iter) != (*other.Iter);
}

inline G1::ShapeIterator G1::ConstShapeChildrenIterable::begin() const
{
	return Shape->GetChildrenBegin();
}

inline G1::ShapeIterator G1::ConstShapeChildrenIterable::end() const
{
	return Shape->GetChildrenEnd();
}

inline G1::ShapeIterator G1::ShapeChildrenIterable::begin()
{
	return Shape->GetChildrenBegin();
}

inline G1::ShapeIterator G1::ShapeChildrenIterable::end()
{
	return Shape->GetChildrenEnd();
}
#endif // polymorphic children*/