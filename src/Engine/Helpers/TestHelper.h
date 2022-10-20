#pragma once

#include <functional>

namespace TmpName
{
	namespace Testing
	{
		bool ApproximatelyEquals(float a, float b, float epsilon = 0.001f);
		bool ApproximatelyEquals(double a, double b, double epsilon = 0.001);
		
		// Asserts that the condition is true, otherwise throws a runtime_error with the given message
		void Assert(bool condition, const char* message);

		// Asserts that calling a function throws an exception
		// The function must be a lambda, function pointer or an std::bind result
		// Will throw an exception with message if the function does not throw
		void AssertThrows(std::function<void()> func, const char* message);
		
		// Aseerts that calling a function throws an exception
		// This overload accepts 1 argument
		// For more arguments use std::bind with the overload above
		// Will throw an exception with message if the function does not throw
		template<class TArg>
		void AssertThrows(std::function<void(TArg)> func, TArg arg, const char* message);

		// Asserts that every element in the given container returns true from the given predicate
		// Throws an runtime_error with the message if any element returns false
		// Will not throw for an empty container
		template<class TContainer>
		void AssertAllOf(const TContainer& container, std::function<bool(typename TContainer::value_type)> predicate, const char* message);

		// Asserts that any element in the given container returns true from the given predicate
		// Throws an runtime_error with the message if no element returns true
		// This includes if there are no elements
		template<class TContainer>
		void AssertAnyOf(const TContainer& container, std::function<bool(typename TContainer::value_type)> predicate, const char* message);

		// Asserts that no element in the given container returns true from the given predicate
		// Throws an runtime_error with the message if any element returns true
		// Will not throw for an empty container
		template<class TContainer>
		void AssertNoneOf(const TContainer& container, std::function<bool(typename TContainer::value_type)> predicate, const char* message);

		// Asserts that the given container contains the given element
		// Throws an runtime_error with the message if the element is not found
		template<class TContainer>
		void AssertContains(const TContainer& container, typename TContainer::value_type element, const char* message);



		template<class TArg>
		void AssertThrows(std::function<void(TArg)> func, TArg arg, const char* message)
		{
			return AssertThrows(std::bind(func, arg), message);
		}

		template<class TContainer>
		void AssertAllOf(const TContainer& container, std::function<bool(typename TContainer::value_type)> predicate, const char* message)
		{
			for (auto& element : container)
			{
				if (!predicate(element))
				{
					throw std::runtime_error(message);
				}
			}
		}

		template<class TContainer>
		void AssertAnyOf(const TContainer& container, std::function<bool(typename TContainer::value_type)> predicate, const char* message)
		{
			for (auto& element : container)
			{
				if (predicate(element))
				{
					return;
				}
			}

			throw std::runtime_error(message);
		}

		template<class TContainer>
		void AssertNoneOf(const TContainer& container, std::function<bool(typename TContainer::value_type)> predicate, const char* message)
		{
			for (auto& element : container)
			{
				if (predicate(element))
				{
					throw std::runtime_error(message);
				}
			}
		}

		template<class TContainer>
		void AssertContains(const TContainer& container, typename TContainer::value_type element, const char* message)
		{
			for (auto& containerElement : container)
			{
				if (containerElement == element)
				{
					return;
				}
			}

			throw std::runtime_error(message);
		}
	}
}