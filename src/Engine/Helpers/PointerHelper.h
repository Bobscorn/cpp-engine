#pragma once

#include "DebugHelper.h"

#include <vector>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <list>

#define constyboi std::enable_if_t<std::is_const<T>::value>
#define notconstyboi std::enable_if_t<!std::is_const<T>::value>

namespace Pointer
{
	struct IObserver
	{
		virtual void Reset() = 0;
	};

	template<class T> class observing_ptr;
	template<class T> class no_expire_obs_ptr;
	template<class T> class f_ptr;
	template<class T> class nef_ptr;

	struct flexible_selfish_control_block
	{
		virtual void add(IObserver *t) = 0;
		virtual void remove(IObserver *t) = 0;
	};

	template<class T>
	class selfish_control_block : public flexible_selfish_control_block
	{
		std::vector<observing_ptr<T>*> benevolence;
		std::vector<no_expire_obs_ptr<T>*> benevolence2;
		std::vector<IObserver*> uncertain_benevolence;
	public:
		inline void add(observing_ptr<T>* ptr) { benevolence.push_back(ptr); }
		inline void add(no_expire_obs_ptr<T>* ptr) { benevolence2.push_back(ptr); }
		
		inline void add(IObserver *t) override
		{
			uncertain_benevolence.push_back(t);
		}

		inline void remove(observing_ptr<T>* ptr);
		inline void remove(no_expire_obs_ptr<T>* ptr);
		
		inline void remove(IObserver *t) override
		{
			for (size_t i = uncertain_benevolence.size(); i-- > 0; )
			{
				if (uncertain_benevolence[i] == t)
				{
					std::swap(uncertain_benevolence[i], uncertain_benevolence.back());
					uncertain_benevolence.pop_back();
					return;
				}
			}
			DWARNING("could not find requested pointer");
		}

		~selfish_control_block();
	};

	/**
	* Selfish Pointer is essentially a unique_ptr that can have weak_ptrs point to it.
	* 
	* It maintains a control block that it destroys when it goes out of scope.
	* Only 1 Selfish pointer can own a single control block, selfish pointers can not be copied.
	* When the selfish pointer is destroyed, it will destroy the control block, and any observing pointers (weak_ptr equivalents) will have their control block set to nullptr.
	* 
	*/
	template<class T>
	class selfish_ptr
	{
	public:
		friend class observing_ptr<T>;
		friend class no_expire_obs_ptr<T>;
		friend class f_ptr<T>;
		friend class nef_ptr<T>;
		using template_type = T;
		using pointer = T*;
		using reference = T&;
	private:
		pointer ptr;
		selfish_control_block<T> *control_block;
	public:
		selfish_ptr(pointer _ptr = nullptr) : ptr(_ptr), control_block(nullptr)
		{
			if (ptr)
				control_block = new selfish_control_block<T>();
		};
		~selfish_ptr() { reset(); };

		selfish_ptr(const selfish_ptr<T>&) = delete;
		selfish_ptr<T>& operator=(const selfish_ptr<T>&) = delete;

		selfish_ptr(selfish_ptr<T>&& other) : ptr(nullptr), control_block(nullptr)
		{ 
			std::swap(ptr, other.ptr);
			std::swap(control_block, other.control_block);
		};
		selfish_ptr(std::nullptr_t) : ptr(nullptr), control_block(nullptr) {};
		selfish_ptr<T>& operator=(selfish_ptr<T>&& other) 
		{ 
			if (ptr)
			{
				delete ptr;
				delete control_block;
			}

			ptr = nullptr; 
			control_block = nullptr;
			std::swap(ptr, other.ptr); 
			std::swap(control_block, other.control_block);
			return *this; 
		};
		selfish_ptr<T>& operator=(std::nullptr_t) 
		{ 
			if (ptr)
			{
				delete ptr;
				delete control_block;
			}
			ptr = nullptr; 
			control_block = nullptr;
			return *this; 
		};
		
		inline pointer get() const { return ptr; }
		inline selfish_control_block<T> *get_control() const { return control_block; }
		inline reference operator*() const noexcept { return *get(); }
		inline pointer operator->() const noexcept { return get(); }
		inline operator bool() const noexcept { return (bool)get(); }
		inline size_t count_observers() const noexcept { if (control_block) return control_block->benevolence.size(); else return 0ull; }

		inline pointer release() noexcept { pointer tmp = get(); if (control_block) delete control_block; control_block = nullptr; ptr = nullptr; return tmp; }
		inline void reset(pointer t = nullptr) noexcept 
		{ 
			if (ptr)
			{
				delete ptr;
				delete control_block;
			}
			ptr = t; 
			if (ptr)
				control_block = new selfish_control_block<T>();
			else
				control_block = nullptr;
		}
		inline void swap(selfish_ptr<T>& other) noexcept { std::swap(ptr, other.ptr); std::swap(control_block, other.control_block); }
		inline void remove_observers() noexcept { if (control_block) delete control_block; if (ptr) { control_block = new selfish_control_block<T>(); } }
	};

	/**
	* observing_ptr is essentially a weak_ptr for the selfish_ptr class.
	* It can be copied, destroyed and moved, but will not destroy the original pointer or the owning selfish_ptr.
	* When the owning selfish_ptr is destroyed, any observing_ptrs pointing to it will have their control blocks and internal pointers set to null.
	*/
	template<class T>
	class observing_ptr : IObserver
	{
		friend class selfish_control_block<T>;
	public:
		using template_type = T;
		using pointer = T*;
		using reference = T&;
	protected:
		pointer ptr;
		selfish_control_block<T>* control_block;
	public:
		observing_ptr() : ptr(nullptr), control_block(nullptr) {};
		observing_ptr(std::nullptr_t) : ptr(nullptr), control_block(nullptr) {}
		observing_ptr(const observing_ptr<T>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		};

		observing_ptr(observing_ptr<T>&& other) noexcept : ptr(other.ptr), control_block(other.control_block)
		{
			other.ptr = nullptr;
			other.control_block = nullptr;
			if (control_block)
			{
				control_block->remove(&other);
				control_block->add(this);
			}
		}

		observing_ptr(const selfish_ptr<T>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		}

		/*template<typename = constyboi>
		observing_ptr(const selfish_ptr<std::remove_const_t<T>>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		}*/

		~observing_ptr()
		{
			if (control_block)
				control_block->remove(this);
		}

		inline observing_ptr<T>& operator=(const observing_ptr<T>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		inline observing_ptr<T>& operator=(observing_ptr<T>&& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;

			other.ptr = nullptr;
			other.control_block = nullptr;

			if (control_block)
			{
				control_block->remove(&other);
				control_block->add(this);
			}

			return *this;
		}

		inline observing_ptr<T>& operator=(const selfish_ptr<T>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		/*template<typename = constyboi>
		inline observing_ptr<T>& operator=(const selfish_ptr<std::remove_const_t<T>>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}*/

		[[nodiscard]] inline pointer get() const { if (ptr && !control_block) DERROR("getting ptr when expired"); return ptr; }
		[[nodiscard]] inline reference operator*() const noexcept { if (ptr && !control_block) DERROR("getting ptr when expired"); return *get(); }
		[[nodiscard]] inline pointer operator->() const noexcept { if (ptr && !control_block) DERROR("getting ptr when expired"); return get(); }
		inline operator bool() const noexcept { return (bool)get(); }

		inline pointer release() { pointer tmp = ptr; Reset(); return tmp; }
		inline void Reset(const selfish_ptr<T>& selfish) { if (ptr) control_block->remove(this); ptr = selfish.ptr; control_block = selfish.control_block; if (control_block) control_block->add(this); }
		
		inline void Reset() override 
		{
			if (ptr)
				control_block->remove(this);

			ptr = nullptr;
			control_block = nullptr;
		}
	};

	/**
	* The no_expire_obs_ptr is identical to the observing_ptr that maintains it's reference when the original is destroyed.
	* In more detail, When a selfish_ptr is destroyed and resets all references, all no_expire_obs_ptr instances will still have their internal ptr with the original value.
	* 
	* The internal ptr will now now point to destroyed memory, and should only be used when you need the ptr's value for something *other* than accessing it.
	* Examples of such uses are keying into an unordered_map that takes a pointer, even if the original object has been destroyed.
	* This happens in this engine when looking 
	*/
	template<class T>
	class no_expire_obs_ptr : IObserver
	{
		friend class selfish_control_block<T>;
	public:
		using template_type = T;
		using pointer = T * ;
		using reference = T & ;
	protected:
		pointer ptr;
		selfish_control_block<T>* control_block;
	public:
		no_expire_obs_ptr() : ptr(nullptr), control_block(nullptr) {};
		no_expire_obs_ptr(std::nullptr_t) : ptr(nullptr), control_block(nullptr) {}
		no_expire_obs_ptr(const no_expire_obs_ptr<T>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		};

		no_expire_obs_ptr(no_expire_obs_ptr<T>&& other) : ptr(other.ptr), control_block(other.control_block)
		{
			other.ptr = nullptr;
			other.control_block = nullptr;
			if (control_block)
			{
				control_block->remove(&other);
				control_block->add(this);
			}
		}

		no_expire_obs_ptr(const selfish_ptr<T>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		}

		/*template<typename = constyboi>
		no_expire_obs_ptr(const selfish_ptr<std::remove_const_t<T>>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		}*/

		~no_expire_obs_ptr()
		{
			if (control_block)
				control_block->remove(this);
		}

		inline no_expire_obs_ptr<T>& operator=(const no_expire_obs_ptr<T>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		inline no_expire_obs_ptr<T>& operator=(no_expire_obs_ptr<T>&& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;

			other.ptr = nullptr;
			other.control_block = nullptr;

			if (control_block)
			{
				control_block->remove(&other);
				control_block->add(this);
			}

			return *this;
		}

		inline no_expire_obs_ptr<T>& operator=(const selfish_ptr<T>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		template<typename = constyboi>
		inline no_expire_obs_ptr<T>& operator=(const selfish_ptr<std::remove_const_t<T>>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		[[nodiscard]] inline pointer get() const { if (ptr && !control_block) DERROR("getting ptr when expired"); return ptr; }
		[[nodiscard]] inline reference operator*() const noexcept { if (ptr && !control_block) DERROR("getting ptr when expired"); return *get(); }
		[[nodiscard]] inline pointer operator->() const noexcept { if (ptr && !control_block) DERROR("getting ptr when expired"); return get(); }
		inline operator bool() const noexcept { return (bool)get(); }

		inline pointer release() { pointer tmp = ptr; Reset(); return tmp; }
		inline void Reset(const selfish_ptr<T>& selfish) { if (ptr) control_block->remove(this); ptr = selfish.ptr; control_block = selfish.control_block; if (control_block) control_block->add(this); }

		inline void Reset() override
		{
			if (ptr)
				control_block->remove(this);

			ptr = nullptr;
			control_block = nullptr;
		}

		inline bool expired() { return !control_block; }
	};

	/**
	* an f_ptr (short for flexible_ptr) is a type of observing_ptr that can point to types not of the owning selfish_ptr's type.
	* Besides the original type, only types that are derivatives of the type can be pointed to.
	* 
	* Basically this is a method of pointing to a selfish_ptr that contains an interface type, and not having to dynamic_cast every time you access.
	*/
	template<class T>
	class f_ptr : IObserver
	{
		template<class T2>
		friend class f_ptr;
	public:
		using template_type = T;
		using pointer = T * ;
		using reference = T & ;
	protected:
		pointer ptr;
		flexible_selfish_control_block* control_block;
	public:
		f_ptr() : ptr(nullptr), control_block(nullptr) {};
		f_ptr(std::nullptr_t) : ptr(nullptr), control_block(nullptr) {}

		f_ptr(const f_ptr<T>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		};

		f_ptr(f_ptr<T>&& other) : ptr(other.ptr), control_block(other.control_block)
		{
			other.ptr = nullptr;
			other.control_block = nullptr;
			if (control_block)
			{
				control_block->remove(&other);
				control_block->add(this);
			}
		}

		f_ptr(const selfish_ptr<T>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		}

		template<typename = constyboi>
		f_ptr(const selfish_ptr<std::remove_const_t<T>>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		}

		template<class B, typename = std::enable_if_t<std::is_base_of<T, B>::value>>
		explicit f_ptr(const selfish_ptr<B> &other) : ptr(other.get()), control_block(other.get_control())
		{
			if (control_block)
				control_block->add(this);
		}

		explicit f_ptr(T *ptr, flexible_selfish_control_block *control) : ptr(ptr), control_block(control)
		{
			if (control_block)
				control_block->add(this);
		}

		~f_ptr()
		{
			if (control_block)
				control_block->remove(this);
		}

		inline f_ptr<T>& operator=(const f_ptr<T>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		template<class _T>
		inline f_ptr<T>& operator=(const f_ptr<typename std::enable_if_t<std::is_base_of<T, _T>::value, _T>>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		inline f_ptr<T>& operator=(f_ptr<T>&& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;

			other.ptr = nullptr;
			other.control_block = nullptr;

			if (control_block)
			{
				control_block->remove(&other);
				control_block->add(this);
			}

			return *this;
		}

		template<class _T>
		inline f_ptr<T>& assign(f_ptr<typename std::enable_if_t<std::is_base_of<T, _T>::value, _T>>&& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;

			other.ptr = nullptr;
			other.control_block = nullptr;

			if (control_block)
			{
				control_block->remove(&other);
				control_block->add(this);
			}

			return *this;
		}

		inline f_ptr<T>& operator=(const selfish_ptr<T>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		template<typename = constyboi>
		inline f_ptr<T>& operator=(const selfish_ptr<std::remove_const_t<T>>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		template<class T2>
		static inline f_ptr<T> CreateFrom(const selfish_ptr<typename std::enable_if_t<std::is_base_of<T, T2>::value, T2>>& other)
		{
			f_ptr<typename std::enable_if_t<std::is_base_of<T, T2>::value, T2>> gass(other);
			f_ptr<T> ass;
			ass.assign<T2>(gass);

			return ass;
		}

		template<class U, std::enable_if_t<std::is_convertible_v<T, U>, int> = 0>
		inline operator f_ptr<U>()
		{
			auto *casted = dynamic_cast<U*>(this->ptr);
			if (casted)
			{
				return f_ptr<U>(casted, control_block);
			}
			else
				return f_ptr<U>(nullptr);
		}

		template<class U>
		inline f_ptr<U> SketchyCopy() const
		{
			U *out = dynamic_cast<U*>(ptr);
			if (out)
			{
				return f_ptr<U>{out, control_block};
			}
			else
				return f_ptr<U>(nullptr);
		}

		[[nodiscard]] inline pointer get() const { if (ptr && !control_block) DERROR("getting ptr when expired"); return ptr; }
		[[nodiscard]] inline reference operator*() const noexcept { if (ptr && !control_block) DERROR("getting ptr when expired"); return *get(); }
		[[nodiscard]] inline pointer operator->() const noexcept { if (ptr && !control_block) DERROR("getting ptr when expired"); return get(); }
		inline operator bool() const noexcept { return (bool)get(); }

		inline pointer release() { pointer tmp = ptr; reset(); return tmp; }
		inline void reset(const selfish_ptr<T>& selfish) { if (ptr) control_block->remove(this); ptr = selfish.ptr; control_block = selfish.control_block; if (control_block) control_block->add(this); }
		inline void reset() { if (ptr) control_block->remove(this); ptr = nullptr; control_block = nullptr; }

		inline bool expired() { return !control_block; }

		inline virtual void Reset() override
		{
			ptr = nullptr;
			control_block = nullptr;
		}
	};

	/**
	* a nef_ptr is both a no_expire_observing_ptr and an f_ptr
	*/
	template<class T>
	class nef_ptr : IObserver
	{
		template<class T2>
		friend class nef_ptr;
	public:
		using template_type = T;
		using pointer = T * ;
		using reference = T & ;
	protected:
		pointer ptr;
		flexible_selfish_control_block* control_block;
	public:
		nef_ptr() : ptr(nullptr), control_block(nullptr) {};
		nef_ptr(std::nullptr_t) : ptr(nullptr), control_block(nullptr) {}

		nef_ptr(const nef_ptr<T>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		};

		nef_ptr(nef_ptr<T>&& other) : ptr(other.ptr), control_block(other.control_block)
		{
			other.ptr = nullptr;
			other.control_block = nullptr;
			if (control_block)
			{
				control_block->remove(&other);
				control_block->add(this);
			}
		}

		nef_ptr(const selfish_ptr<T>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		}

		template<typename = constyboi>
		nef_ptr(const selfish_ptr<std::remove_const_t<T>>& other) : ptr(other.ptr), control_block(other.control_block)
		{
			if (control_block)
				control_block->add(this);
		}

		template<typename B, typename = std::enable_if_t<std::is_base_of<T, B>::value>>
		explicit nef_ptr(const selfish_ptr<B> &other) : ptr(other.get()), control_block(other.get_control())
		{
			if (control_block)
				control_block->add(this);
		}

		~nef_ptr()
		{
			if (control_block)
				control_block->remove(this);
		}

		inline nef_ptr<T>& operator=(const nef_ptr<T>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		template<class _T>
		inline nef_ptr<T>& operator=(const nef_ptr<typename std::enable_if_t<std::is_base_of<T, _T>::value, _T>>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		inline nef_ptr<T>& operator=(nef_ptr<T>&& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;

			other.ptr = nullptr;
			other.control_block = nullptr;

			if (control_block)
			{
				control_block->remove(&other);
				control_block->add(this);
			}

			return *this;
		}

		template<class _T>
		inline nef_ptr<T>& assign(nef_ptr<typename std::enable_if_t<std::is_base_of<T, _T>::value, _T>>&& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;

			other.ptr = nullptr;
			other.control_block = nullptr;

			if (control_block)
			{
				control_block->remove(&other);
				control_block->add(this);
			}

			return *this;
		}

		inline nef_ptr<T>& operator=(const selfish_ptr<T>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		template<typename = constyboi>
		inline nef_ptr<T>& operator=(const selfish_ptr<std::remove_const_t<T>>& other)
		{
			if (control_block)
				control_block->remove(this);

			ptr = other.ptr;
			control_block = other.control_block;
			if (control_block)
				control_block->add(this);

			return *this;
		}

		template<class T2>
		static inline nef_ptr<T> CreateFrom(const selfish_ptr<typename std::enable_if_t<std::is_base_of<T, T2>::value, T2>>& other)
		{
			nef_ptr<typename std::enable_if_t<std::is_base_of<T, T2>::value, T2>> gass(other);
			nef_ptr<T> ass;
			ass.operator=<T2>(gass);

			return ass;
		}

		[[nodiscard]] inline pointer get() const { if (ptr && !control_block) DERROR("getting ptr when expired"); return ptr; }
		[[nodiscard]] inline reference operator*() const noexcept { if (ptr && !control_block) DERROR("getting ptr when expired"); return *get(); }
		[[nodiscard]] inline pointer operator->() const noexcept { if (ptr && !control_block) DERROR("getting ptr when expired"); return get(); }
		inline operator bool() const noexcept { return (bool)get(); }

		inline pointer release() { pointer tmp = ptr; reset(); return tmp; }
		inline void reset(const selfish_ptr<T>& selfish) { if (ptr) control_block->remove(this); ptr = selfish.ptr; control_block = selfish.control_block; if (control_block) control_block->add(this); }
		inline void reset() { if (ptr) control_block->remove(this); ptr = nullptr; control_block = nullptr; }

		inline bool expired() { return !control_block; }

		inline virtual void Reset() override
		{
			control_block = nullptr;
		}
	};

	

	

	template<class T>
	inline bool operator==(selfish_ptr<T> const & l, selfish_ptr<T> const & r) { return l.get() == r.get(); }

	template<class T>
	inline bool operator!=(selfish_ptr<T> const & l, selfish_ptr<T> const & r) { return l.get() != r.get(); }

	template<class T>
	inline bool operator==(std::nullptr_t l, selfish_ptr<T> const & r) { return l == r.get(); }

	template<class T>
	inline bool operator!=(std::nullptr_t l, selfish_ptr<T> const & r) { return l != r.get(); }

	template<class T>
	inline bool operator==(selfish_ptr<T> const & l, std::nullptr_t r) { return l.get() == r; }

	template<class T>
	inline bool operator!=(selfish_ptr<T> const & l, std::nullptr_t r) { return l.get() != r; }


	template<class T>
	inline bool operator<(selfish_ptr<T> const & l, selfish_ptr<T> const & r)
	{
		return std::less<T*>() (l.get(), r.get());
	}

	template<class T>
	inline bool operator>(selfish_ptr<T> const & l, selfish_ptr<T> const & r)
	{
		return r < l;
	}

	template<class T>
	inline bool operator<=(selfish_ptr<T> const & l, selfish_ptr<T> const & r)
	{
		return !(l > r); // not greater = less/equal
	}

	template<class T>
	inline bool operator>=(selfish_ptr<T> const & l, selfish_ptr<T> const & r)
	{
		return !(l < r); // not lesser = greater/equal
	}


	template<typename T, typename... Args>
	inline selfish_ptr<T> make_selfish(Args&&... args)
	{
		return selfish_ptr<T>(new T(std::forward<Args>(args)...));
	}

	template<typename T, typename... Args>
	inline selfish_ptr<T> make_selfish_bullet(Args&&... args)
	{
		return selfish_ptr<T>(new T(std::forward<Args>(args)...));
	}

	template<class T>
	inline void selfish_control_block<T>::remove(observing_ptr<T>* ptr)
	{
		for (auto i = benevolence.size(); i-- > 0; )
		{
			if (benevolence[i] == ptr)
			{
				std::swap(benevolence[i], benevolence.back());
				benevolence.pop_back();
				return;
			}
		}
		//well....... error..
		DERROR("given invalid observe ptr");
	}

	template<class T>
	inline void selfish_control_block<T>::remove(no_expire_obs_ptr<T>* ptr)
	{
		for (auto i = benevolence2.size(); i-- > 0; )
		{
			if (benevolence2[i] == ptr)
			{
				std::swap(benevolence2[i], benevolence2.back());
				benevolence2.pop_back();
				return;
			}
		}
		//well....... error..
		DERROR("given invalid no_expire ptr");
	}

	template<class T>
	inline selfish_control_block<T>::~selfish_control_block()
	{
		for (auto& e : benevolence)
		{
			e->ptr = nullptr;
			e->control_block = nullptr;
		}
		for (auto& e : benevolence2)
		{
			e->control_block = nullptr;
		}
		for (auto& e : uncertain_benevolence)
		{
			e->Reset();
		}
	}
}

#undef constyboi
#undef notconstyboi