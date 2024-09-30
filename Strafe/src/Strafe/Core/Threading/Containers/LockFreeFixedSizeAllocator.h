#pragma once

#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include "Strafe/Core/Threading/Containers/LockFreeList.h"
#include "Strafe/Core/Threading/Windows/WindowsPlatformTLS.h"

/**
* Thread safe, lock free pooling allocator of fixed size blocks that
* never returns free space, even at shutdown
* alignment isn't handled, assumes malloc will work /  if used :/
*/

//if there is a leak we are fucked but it shouldnt happen
//if there is a leak we implement tracking to identify, hopefully we wont need to


#define FORCEINLINE  __forceinline

/** Fake thread safe counter, used to avoid cluttering code with ifdefs when counters are only used for debugging. */
class NoopCounter
{
public:

	NoopCounter() { }
	NoopCounter(const NoopCounter& Other) { }
	NoopCounter(int32 Value) { }

	int32 Increment()
	{
		return 0;
	}

	int32_t Add(int32 Amount)
	{
		return 0;
	}

	int32 Decrement()
	{
		return 0;
	}

	int32 Subtract(int32 Amount)
	{
		return 0;
	}

	int32 Set(int32 Value)
	{
		return 0;
	}

	int32 Reset()
	{
		return 0;
	}

	int32 GetValue() const
	{
		return 0;
	}
};


template<int32 TSize, typename BundleRecycler, typename TrackingCounter = NoopCounter, bool AllowDisablingOfTrim = false>
class LockFreeFixedSizeAllocator_TLSCacheBase
{
	enum
	{
		SIZE_PER_BUNDLE = 65536,
		NUM_PER_BUNDLE = SIZE_PER_BUNDLE / SIZE
	};

	public:
		LockFreeFixedSizeAllocator_TLSCacheBase()
		{
			static_assert(SIZE >= sizeof(void*) && SIZE % sizeof(void*) == 0, "Blocks in TLockFreeFixedSizeAllocator must be at least the size of a pointer.");
			TlsSlot = WindowsPlatformTLS::AllocTlsSlot();
		}

		/** Destructor,  **/
		~LockFreeFixedSizeAllocator_TLSCacheBase()
		{

			//this is alot of overhead but sure
			// Clean up the GlobalFreeListBundles
			while (!GlobalFreeListBundles.IsEmpty())
			{
				void* Bundle = GlobalFreeListBundles.Pop(); // or another method to retrieve a bundle
				delete[] static_cast<void**>(Bundle); // Assuming the bundles were allocated with new[]
			}

			WindowsPlatformTLS::FreeTlsSlot(TlsSlot);
			TlsSlot = 0;
		}

		//allocates a memory block of size TSize
		FORCEINLINE void* Allocate()
		{

			//return malloc(TSize);
			////hide this bundle recycling shit as we find a way to free memory properly
			//@ UPDATE : TLS is freed but the thing is it may still cause leaks. BUT, this is an intentional leak and not technically a leak during runtime because all allocations are always accounted for.
			//this is for performance sake as the allocation on every tls awakening is too taxing so we recycle = blazing fast
			//the bundles will be available for the entire application lifecycle until shutdown where we leak everything COOL ACCEPTABLE
			//The cost of explicitly freeing each allocated block (especially if the application is allocating and deallocating frequently) may outweigh the benefits.
			ThreadLocalCache& TLS = GetTLS();

			if (!TLS.PartialBundle)
			{
				if (TLS.FullBundle)
				{
					TLS.PartialBundle = TLS.FullBundle;
					TLS.FullBundle = nullptr;
				}
				else
				{
					TLS.PartialBundle = GlobalFreeListBundles.Pop();
					if (!TLS.PartialBundle)
					{
						TLS.PartialBundle = new void* [NUM_PER_BUNDLE];
						void** Next = TLS.PartialBundle;
						for (int32 Index = 0; Index < NUM_PER_BUNDLE - 1; Index++)
						{
							void* NextNext = (void*)(((uint8*)Next) + SIZE);
							*Next = NextNext;
							Next = (void**)NextNext;
						}
						*Next = nullptr;
						NumFree.Add(NUM_PER_BUNDLE);
					}
				}
				TLS.NumPartial = NUM_PER_BUNDLE;
			}
			NumUsed.Increment();
			NumFree.Decrement();
			void* Result = (void*)TLS.PartialBundle;
			TLS.PartialBundle = (void**)*TLS.PartialBundle;
			TLS.NumPartial--;
			return Result;
		}

		//puts a memory block obtained from allocate back on the free llist for future use
		FORCEINLINE void Free(void* Item)
		{
			//free(Item);
			NumUsed.Decrement();
			NumFree.Increment();
			FThreadLocalCache& TLS = GetTLS();
			if (TLS.NumPartial >= NUM_PER_BUNDLE)
			{
				if (TLS.FullBundle)
				{
					GlobalFreeListBundles.Push(TLS.FullBundle);
					//TLS.FullBundle = nullptr;
				}
				TLS.FullBundle = TLS.PartialBundle;
				TLS.PartialBundle = nullptr;
				TLS.NumPartial = 0;
			}
			*(void**)Item = (void*)TLS.PartialBundle;
			TLS.PartialBundle = (void**)Item;
			TLS.NumPartial++;
		}

	private:

	//struct for the tls cache
		struct ThreadLocalCache
		{
			void** FullBundle;
			void** PartialBundle;
			int32 NumPartial;

			ThreadLocalCache()
				: FullBundle(nullptr)
				, PartialBundle(nullptr)
				, NumPartial(0)
			{
			}
		};

		ThreadLocalCache& GetTLS()
		{
				
			ThreadLocalCache* TLS = (ThreadLocalCache*)WindowsPlatformTLS::GetTlsValue(TlsSlot);
			if (!TLS)
			{
				TLS = new ThreadLocalCache();
				WindowsPlatformTLS::SetTlsValue(TlsSlot, TLS);
			}
			return *TLS;
		}

		/** Slot for TLS struct. */
		uint32 TlsSlot;

		/** Lock free list of free memory blocks, these are all linked into a bundle of NUM_PER_BUNDLE. */
		BundleRecycler GlobalFreeListBundles;

		/** Total number of blocks outstanding and not in the free list. */
		TrackingCounter NumUsed;

		/** Total number of blocks in the free list. */
		TrackingCounter NumFree;
};