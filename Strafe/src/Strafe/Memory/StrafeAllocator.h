
#pragma once
#include "MemoryManager.h"

namespace strafe
{
	template<typename T>
	struct StrafeAllocator
	{
		typedef T value_type;

		StrafeAllocator() noexcept = default;
		template <class U> constexpr StrafeAllocator(const StrafeAllocator <U>&) noexcept {}

		[[nodiscard]] T* allocate(size_t n)
		{
			if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
				throw std::bad_array_new_length();
			if (T* p = static_cast<T*>(MemoryManager::GetInstance().Allocate<T>(n))) {
				return p;
			}

			throw std::bad_alloc();
		}
		void deallocate(T* p, size_t n) const
		{
			UNREFERENCED_PARAMETER(n);
			MemoryManager::GetInstance().Deallocate<T>(p);
		}
	};
}
