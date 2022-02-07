#pragma once

#include "Helpers/TransferHelper.h"
#include "Helpers/PointerHelper.h"

#include "Camera.h"
#include "Systems/Events/EventsBase.h"
#include "Systems/Requests/Requestable.h"
#include "Systems/Execution/ResourceHolder.h"

#include "Structure/BulletCollisionListener.h"

#include <vector>
#include <unordered_map>

#ifdef USE_BULLET_STUFF
#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)
#endif


namespace G1
{
	class IGSpace;
	struct IShape;

	using namespace Pointer;
		
#ifdef POLYMORPHIC_SHAPE_CHILDREN
	struct IShapeIterator;

	struct ShapeIterator;

	struct ShapeIterator
	{
		typedef IShape& reference;
		typedef IShape* pointer;

		ShapeIterator(IShapeIterator *ass) : Iter(ass) {}
		ShapeIterator(std::unique_ptr<IShapeIterator> ass) : Iter(std::move(ass)) {}
		ShapeIterator(const ShapeIterator& other);
		ShapeIterator(ShapeIterator&& other);

		inline ShapeIterator& operator=(const ShapeIterator& other);
		inline ShapeIterator& operator=(ShapeIterator&& other);

		[[nodiscard]] inline  reference operator*();
		[[nodiscard]] inline  pointer operator->();
		inline ShapeIterator& operator++(); // pre-increment
		inline ShapeIterator operator++(int); // post-increment
		inline ShapeIterator& operator--(); // pre-decrement
		inline ShapeIterator operator--(int); // post-decrement
		inline ShapeIterator& operator+=(size_t move);
		inline ShapeIterator& operator-=(size_t move);
		[[nodiscard]] inline reference operator[](size_t dif);

		inline bool operator==(const ShapeIterator& other);
		inline bool operator!=(const ShapeIterator& other);

	protected:
		std::unique_ptr<IShapeIterator> Iter;
	};

	struct ShapeChildrenIterable
	{
		ShapeChildrenIterable(IShape *shape) : Shape(shape) {}

		inline ShapeIterator begin();
		inline ShapeIterator end();

		IShape *Shape;
	};

	struct ConstShapeChildrenIterable
	{
		ConstShapeChildrenIterable(IShape *shape) : Shape(shape) {}

		inline ShapeIterator begin() const;
		inline ShapeIterator end() const;

		IShape *Shape;
	};
#endif // Polymorphic shape children

	struct IShapeProperty
	{
		virtual ~IShapeProperty() {}
	};

	struct BasicProp
	{
		inline constexpr operator bool() const { return Val != 0.f; }
		inline constexpr operator float() const { return Val; }
		inline constexpr operator uint32_t() const { return uint32_t(Val); }

		[[nodiscard]] inline constexpr bool IsTrue() const { return *this; }
		[[nodiscard]] inline constexpr bool IsFalse() const { return Val == 0.f; }
		[[nodiscard]] inline constexpr float Value() const { return *this; }
		[[nodiscard]] inline constexpr uint32_t ValueU() const { return *this; }

		inline constexpr void SetTrue() { Val = 1.f; }
		inline constexpr void SetFalse() { Val = 0.f; }
		inline constexpr void SetValue(float val) { Val = val; }
		inline constexpr void SetValue(uint32_t val) { Val = float(val); }

		inline constexpr BasicProp& operator=(float val) { Val = val; return *this; }
		inline constexpr BasicProp& operator=(uint32_t val) { Val = float(val); return *this; }
		inline constexpr BasicProp& operator=(bool val) { Val = (val ? 1.f : 0.f); return *this; }
	protected:
		float Val;
	};

	struct IShapeProperties
	{
		virtual ~IShapeProperties() {}

		std::unordered_map<std::string, BasicProp> BasicProperties;
		std::unordered_map<std::string, std::unique_ptr<IShapeProperty>> AdvancedProperties;
	};

	struct ShapePositionProperties : IShapeProperties
	{
		Matrixy4x4 ToWorld{ Matrixy4x4::Identity() };
		floaty3 Rotation{ 0.f, 0.f, 0.f }; // Initial Rotation only, not updated during physics execution (the ToWorld matrix is though)
		floaty3 Position{ 0.f, 0.f, 0.f }; // Initial position only, not updated during physics execution

		inline void SetPosition(floaty3 NewPos)
		{
			MovePosition({ NewPos.x - Position.x, NewPos.y - Position.y, NewPos.z - Position.z });
		}

		inline void MovePosition(floaty3 ByThisMuch)
		{
			ToWorld = Matrixy4x4::Translate(ByThisMuch.x, ByThisMuch.y, ByThisMuch.z);
		}
	};

	struct IShapeThings
	{
		IGSpace *Container = nullptr;
		CommonResources *Resources = nullptr;
		std::string Name = "Unnamed";

		inline IShapeThings & WithName(std::string newname)
		{
			Name = newname;
			return *this;
		}
	};

	struct IShape : virtual FullResourceHolder
	{
		friend class IGSpace;
	protected:
		std::string Name;
		bool Initialized = false;
		
		IGSpace *Container;
	public:
		IShape *Parent = nullptr;

		IShape(std::string name = "") : Name(name) {};
		IShape(IGSpace *container, std::string name = "") : Container(container), Name(name) {};
		IShape(IShapeThings things) : Container(things.Container), Name(things.Name) {}
		virtual ~IShape() {};


		virtual void BeforeDraw() = 0;
		virtual void Draw() {}
		virtual void AfterDraw() = 0;

#ifdef POLYMORPHIC_SHAPE_CHILDREN
		virtual void AddChild(IShape *toadd) = 0;
		virtual void AddChild(selfish_ptr<IShape> &&toadd) = 0;
		virtual void RemoveChild(IShape *toremove) = 0;

		inline ShapeChildrenIterable Children() { return ShapeChildrenIterable(this); }
		[[nodiscard]] virtual ShapeIterator GetChildrenBegin() = 0;
		[[nodiscard]] virtual ShapeIterator GetChildrenEnd() = 0;
#else
		// AddChild Takes ownership of the lifetime of any supplied IShape pointer
		inline Pointer::f_ptr<IShape> AddChild(IShape* toadd) { children.emplace_back(toadd); children.back()->Parent = this; return children.back(); }
		template<class Shape, class ... Args>
		inline Pointer::f_ptr<Shape> AddChild(Args... args) { children.emplace_back(new Shape(this->Container, this->mResources, args...)); children.back()->Parent = this; return (Pointer::f_ptr<IShape>(children.back()).SketchyCopy<Shape>()); }
		inline bool RemoveChild(IShape *culprit) { for (size_t i = children.size(); i-- > 0;) if (children[i].get() == culprit) { std::swap(children[i], children.back()); children.pop_back(); return true; } return false; }
		std::vector<selfish_ptr<IShape>> children;
		inline std::vector<selfish_ptr<IShape>>& Children() { return children; }
		inline const std::vector<selfish_ptr<IShape>>& Children() const { return children; }
#endif // Polymorphic shape children

		inline IGSpace *GetContainer() const { return Container; }
		inline virtual std::string GetName() const { return Name; }
		inline void SetName(const std::string& newname) { Name = newname; }
	};
	

#ifdef POLYMORPHIC_SHAPE_CHILDREN
	struct IShapeIterator
	{
		typedef IShape& reference;
		typedef IShape* pointer;

		IShapeIterator() : _ptr(nullptr) {};
		IShapeIterator(pointer _p) : _ptr(_p) {};
		virtual ~IShapeIterator() {};

		[[nodiscard]] virtual reference operator*() { return *_ptr; }
		[[nodiscard]] virtual pointer operator->() { return _ptr; }
		virtual IShapeIterator& operator++() { ++_ptr; return *this; }
		virtual IShapeIterator& operator--() { --_ptr; return *this; }
		virtual IShapeIterator& operator+=(size_t move) { _ptr += move; return *this; }
		virtual IShapeIterator& operator-=(size_t move) { _ptr -= move; return *this; }
		[[nodiscard]] virtual reference operator[](size_t dif) { return _ptr[dif]; }
		virtual IShapeIterator* clone() { return DBG_NEW IShapeIterator(_ptr); }

		inline bool operator==(const IShapeIterator& other) { return *this == &other; }
		inline bool operator!=(const IShapeIterator& other) { return *this != &other; }

		virtual inline bool operator==(const IShapeIterator *other) { return this->_ptr == other->_ptr; }
		virtual inline bool operator!=(const IShapeIterator *other) { return this->_ptr != other->_ptr; }

	protected:
		IShape * _ptr;
	};
#endif

	

#ifdef POLYMORPHIC_SHAPE_CHILDREN
	struct SelfishVectorChildrenIterator : IShapeIterator
	{
		typedef SelfishVectorChildrenIterator Self;
		SelfishVectorChildrenIterator(std::vector<selfish_ptr<IShape>>::iterator _p) : ity(_p) {}

		[[nodiscard]] inline reference operator*() { return *ity->get(); }
		[[nodiscard]] inline pointer operator->() { return &(*ity->get()); }
		inline IShapeIterator& operator++() override { ++ity; return *this; }
		inline IShapeIterator& operator--() override { --ity; return *this; }
		inline IShapeIterator& operator+=(size_t move) override { ity += move; return *this; }
		inline IShapeIterator& operator-=(size_t move) override { ity -= move; return *this; }
		[[nodiscard]] inline reference operator[](size_t dif) override { return *(ity[dif].get()); }

		inline bool operator==(const IShapeIterator *other) override { if (auto *o = dynamic_cast<const Self*>(other)) return this->ity == o->ity; else return false; }
		inline bool operator!=(const IShapeIterator *other) override { return !(*this == other); }

		std::vector<selfish_ptr<IShape>>::iterator ity;
	};

	struct SelfishVectorChildrenShape : virtual IShape
	{
		std::vector<selfish_ptr<IShape>> Children;

		[[nodiscard]] ShapeIterator GetChildrenBegin() override { return ShapeIterator(DBG_NEW SelfishVectorChildrenIterator(Children.begin())); }
		[[nodiscard]] ShapeIterator GetChildrenEnd() override { return ShapeIterator(DBG_NEW SelfishVectorChildrenIterator(Children.end())); }

		void AddChild(IShape *e) override { Children.emplace_back(e); }
		void AddChild(selfish_ptr<IShape> &&e) override { Children.emplace_back(std::move(e)); }

		void RemoveChild(IShape *e) override { for (auto& child : Children) if (child.get() == e) std::swap(child, Children.back()); Children.pop_back(); }
	};
#endif

#pragma warning(disable:4250)
	struct RootShape : virtual IShape
#ifdef POLYMORPHIC_SHAPE_CHILDREN
		, SelfishVectorChildrenShape
#endif
	{
	private:
		using IShape::Parent;
	public:
		RootShape(CommonResources *resources, IGSpace *container) : IShape(container, "Root"), FullResourceHolder(resources) {};

#pragma warning(disable:4100)
		void BeforeDraw() override {};
		void AfterDraw() override {};
#pragma warning(default:4100)

	};
#pragma warning(default:4250)

	class IGSpace : public Requests::IRequestable, public virtual FullResourceHolder
	{
	protected:
		struct find_struct
		{
			inline bool operator() (std::string key, Pointer::selfish_ptr<IShape>& in)
			{
				if (!in)
					return false;

				return key == in->GetName();
			}
		};

		struct child_struct
		{
			inline decltype(auto) operator() (Pointer::selfish_ptr<IShape>& in)
			{
				return in->Children();
			}
		};

		RootShape Root;

		// Physics v
		void PhysicsCull(std::vector<std::shared_ptr<btCollisionObject>>& objs, std::vector<std::shared_ptr<btRigidBody>>& bodies);

		virtual void AddCollisionObject(btCollisionObject *obj, int collisionFilterGroup = btBroadphaseProxy::StaticFilter, int collisionFilterMask = btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter) = 0;
		virtual void AddRigidBody(btRigidBody *body) = 0;
		virtual void AddRigidBody(btRigidBody *body, int group, int mask) = 0;
		virtual void AddContactListener(BulletThingo::IContactListener *listen) = 0;
		virtual void RemoveContactListener(BulletThingo::IContactListener *listen) = 0;

		virtual void SimulatePhysics(double interval) = 0;
		// Physics ^

		void BeforeDrawShapes(IShape *shape);
		void DrawShapes(IShape *shape);
		void AfterDrawShapes(IShape *shape);
		virtual void BeforeDrawI() = 0;
		virtual void DrawI() = 0;
		virtual void AfterDrawI() = 0;
	public:
		IGSpace(CommonResources *resources) : Root(resources, this) {};
		virtual ~IGSpace() { BeingDestroyed = true; };
		
		void BeforeDraw();
		void Draw();
		void AfterDraw();
		
		virtual void DoPhysics() = 0;

		const static std::string RootShapeName;

		IShape *FindShapeyRaw(std::string name);
		Pointer::observing_ptr<IShape> FindShapey(std::string);
		Pointer::f_ptr<IShape> FindShapeFPtr(std::string name);

		// Methods to be call by Shapes
		virtual void SetCamera(const observing_ptr<Camera>& c) = 0;
		virtual Camera *GetCamera() = 0;
		virtual const Camera *GetCamera() const = 0;
		virtual bool CameraIsExternal() const = 0;
		
		virtual std::weak_ptr<btDynamicsWorld> GetPhysicsWorld() = 0;

		virtual void RequestPhysicsCall(std::shared_ptr<btRigidBody> body) = 0;
		virtual void RequestPhysicsCall(std::shared_ptr<btRigidBody> body, int collisionFilterGroup, int collisionFilterMask) = 0;
		virtual void RequestPhysicsCall(std::shared_ptr<btCollisionObject> obj, int collisionFilterGroup = btBroadphaseProxy::StaticFilter, int collisionFilterMask = btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter) = 0;
		virtual void RequestPhysicsRemoval(btRigidBody *body) = 0;
		virtual void RequestPhysicsRemoval(btCollisionObject *obj) = 0;
		virtual void RemoveCollisionObject(btCollisionObject *obj) = 0;
		virtual void RemoveRigidBody(btRigidBody *body) = 0;
		bool BeingDestroyed = false;
	};

	struct GSpaceData : Requests::IRequestData
	{
		// TODO: Give this struct necessary data
	};


	/// <summary>Convenient Pointer that ensures it always points to a shape of a given name if any of the name exist</summary>
	/// <param name="container"> Pointer to the container that will contain the moving/changing shape </param>
	/// <param name="name"> The name of the shape to search for initially and when the cached one expires </param>
	/// <param name="ptr"> The cached pointer of a search for the name in the container </param>
	struct ShapePointer
	{
		IGSpace *const container = nullptr;
		const std::string name;
		Pointer::observing_ptr<IShape> ptr;

		ShapePointer(IGSpace *container, std::string name) : container(container), name(name) { EnsurePtr(); }

		inline void EnsurePtr()
		{
			if (ptr)
				return;

			if (container)
				ptr = container->FindShapey(name);
		}

		inline operator bool() { EnsurePtr(); return ptr; }

		inline IShape *operator->() { EnsurePtr();  return ptr.get(); }
		inline IShape& operator*() { EnsurePtr(); return *ptr.get(); }
	};

	template<class T>
	struct ShapePointerT : ShapePointer
	{
	private:
		using ShapePointer::EnsurePtr;
		using ShapePointer::operator*;
		using ShapePointer::operator->;
	public:
		ShapePointerT(IGSpace *container, std::string name) : ShapePointer(container, name) { EnsurePtr(); }

		inline void EnsurePtr()
		{
			if (ptr)
				return;

			if (container)
			{
				Pointer::observing_ptr<IShape> tmp = container->FindShapey(name);
				if (dynamic_cast<T*>(tmp.get()))
					ptr = std::move(tmp);
			}
		}

		inline T *operator->() { EnsurePtr(); return static_cast<T*>(ptr.get()); }
		inline T &operator*() { EnsurePtr(); return static_cast<T&>(*ptr.get()); }

	};
}