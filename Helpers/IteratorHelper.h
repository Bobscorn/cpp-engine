#pragma once

#include <functional>
#include <iterator>

namespace Iteration
{
	namespace Recursion
	{
		// Not flexible :/
		template<class T>
		void BasicRecurse(T* thing, std::function<void(T*)> funcToCall)
		{
			funcToCall(thing);
			for (auto& child : (*thing)->Children)
				BasicRecurse<T>(&child, funcToCall);
		}
	}
	
	template <typename T>
	struct reversion_wrapper { T& iterable; };

	template <typename T>
	auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

	template <typename T>
	auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }

	template <typename T>
	reversion_wrapper<T> reverse(T&& iterable) { return { iterable }; }


	template<typename T>
	struct hierachy_find_iter { T& iter; };

	template<typename T>
	hierachy_find_iter<T> hierachy_find_children(T& of) {  }
	
	template<class T>
	struct hierachy_children
	{
		template<class Iterable>
		Iterable operator() (T& in)
		{
			return in.children;
		}
	};

	// Used by HierachyFind
	template<class T, class Key, class Equ_, class children>
	bool HierachyBase(Key key, T& in, Equ_& equal, T** out, children& child)
	{
		if (equal(key, in))
		{
			(*out) = &in;
			return true;
		}
		else
			for (auto& c : child(in))
				if (HierachyBase<T, Key, Equ_, children>(key, c, equal, out, child))
					return true;

		return false;
	}

	template<class T, class Key = T, class Equ_ = std::equal_to<T>, class children = hierachy_children<T>>
	bool HierachyFind(Key key, T& in, T** out)
	{
		Equ_ equal;
		children child;
		if (equal(key, in))
		{
			*out = &in;
			return true;
		}
		else
			for (auto& child : child(in))
				if (HierachyBase<T, Key, Equ_, children>(key, child, equal, out, child))
					return true;

		return false;
	}
}