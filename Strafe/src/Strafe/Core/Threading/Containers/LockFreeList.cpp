#include "strafepch.h"
#include "LockFreeList.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformProcess.h"
#include "Strafe/Core/Threading/Windows/WindowsPlatformTLS.h"
#include <iostream>

void LockFreeTagCounterHasOverflowed()
{
	//todo log this shit
	WindowsPlatformProcess::Sleep(.001f);
}

void LockFreeLinksExhausted(uint32 TotalNum)
{
	//todo log this shit 
	std::cout << TotalNum << std::endl;
}

void* LockFreeAllocLinks(SIZE_T AllocSize)
{
	return malloc(AllocSize);
}
void LockFreeFreeLinks(SIZE_T AllocSize, void* Ptr)
{
	free(Ptr);
}

/**
 * utility template for a class that should not be copyable.
 * Derive from this class to make your class non-copyable
 */
class Noncopyable
{
protected:
	// ensure the class cannot be constructed directly
	Noncopyable() {}
	// the class should not be used polymorphically
	~Noncopyable() {}
private:
	Noncopyable(const Noncopyable&);
	Noncopyable& operator=(const Noncopyable&);
};
class LockFreeLinkAllocator_TLSCache : public Noncopyable
{
	enum
	{
		NUM_PER_BUNDLE = 64,
	};

	typedef LockFreeLinkPolicy::TLink TLink;
	typedef LockFreeLinkPolicy::TLinkPtr TLinkPtr;

public:

	LockFreeLinkAllocator_TLSCache()
	{
		//check if is in game thread implmeent later dhasuhaj
		TlsSlot = WindowsPlatformTLS::AllocTlsSlot();
		//check(FPlatformTLS::IsValidTlsSlot(TlsSlot));
	}
	/** Destructor, leaks all of the memory **/
	~LockFreeLinkAllocator_TLSCache()
	{
		WindowsPlatformTLS::FreeTlsSlot(TlsSlot);
		TlsSlot = WindowsPlatformTLS::InvalidTlsSlot;
	}

	/**
	* Allocates a memory block of size SIZE.
	*
	* @return Pointer to the allocated memory.
	* @see Free
	*/
	TLinkPtr Pop() TSAN_SAFE
	{
		FThreadLocalCache& TLS = GetTLS();

		if (!TLS.PartialBundle)
		{
			if (TLS.FullBundle)
			{
				TLS.PartialBundle = TLS.FullBundle;
				TLS.FullBundle = 0;
			}
			else
			{
				TLS.PartialBundle = GlobalFreeListBundles.Pop();
				if (!TLS.PartialBundle)
				{
					int32 FirstIndex = LockFreeLinkPolicy::LinkAllocator.Alloc(NUM_PER_BUNDLE);
					for (int32 Index = 0; Index < NUM_PER_BUNDLE; Index++)
					{
						TLink* Event = LockFreeLinkPolicy::IndexToLink(FirstIndex + Index);
						Event->DoubleNext.Init();
						Event->SingleNext = 0;
						Event->Payload = (void*)uint64(TLS.PartialBundle);
						TLS.PartialBundle = LockFreeLinkPolicy::IndexToPtr(FirstIndex + Index);
					}
				}
			}
			TLS.NumPartial = NUM_PER_BUNDLE;
		}
		TLinkPtr Result = TLS.PartialBundle;
		TLink* ResultP = LockFreeLinkPolicy::DerefLink(TLS.PartialBundle);
		TLS.PartialBundle = TLinkPtr(uint64(ResultP->Payload));
		TLS.NumPartial--;
		//checkLockFreePointerList(TLS.NumPartial >= 0 && ((!!TLS.NumPartial) == (!!TLS.PartialBundle)));
		ResultP->Payload = nullptr;
		checkLockFreePointerList(!ResultP->DoubleNext.GetPtr() && !ResultP->SingleNext);
		return Result;
	}

	/**
	* Puts a memory block previously obtained from Allocate() back on the free list for future use.
	*
	* @param Item The item to free.
	* @see Allocate
	*/
	void Push(TLinkPtr Item) TSAN_SAFE
	{
		FThreadLocalCache& TLS = GetTLS();
		if (TLS.NumPartial >= NUM_PER_BUNDLE)
		{
			if (TLS.FullBundle)
			{
				GlobalFreeListBundles.Push(TLS.FullBundle);
				//TLS.FullBundle = nullptr;
			}
			TLS.FullBundle = TLS.PartialBundle;
			TLS.PartialBundle = 0;
			TLS.NumPartial = 0;
		}
		TLink* ItemP = LockFreeLinkPolicy::DerefLink(Item);
		ItemP->DoubleNext.SetPtr(0);
		ItemP->SingleNext = 0;
		ItemP->Payload = (void*)uint64(TLS.PartialBundle);
		TLS.PartialBundle = Item;
		TLS.NumPartial++;
	}

private:

	/** struct for the TLS cache. */
	struct FThreadLocalCache
	{
		TLinkPtr FullBundle;
		TLinkPtr PartialBundle;
		int32 NumPartial;

		FThreadLocalCache()
			: FullBundle(0)
			, PartialBundle(0)
			, NumPartial(0)
		{
		}
	};

	FThreadLocalCache& GetTLS()
	{
		//checkSlow(WindowsPlatformTLS::IsValidTlsSlot(TlsSlot));
		FThreadLocalCache* TLS = (FThreadLocalCache*)WindowsPlatformTLS::GetTlsValue(TlsSlot);
		if (!TLS)
		{
			TLS = new FThreadLocalCache();
			WindowsPlatformTLS::SetTlsValue(TlsSlot, TLS);
		}
		return *TLS;
	}

	/** Slot for TLS struct. */
	uint32 TlsSlot;

	/** Lock free list of free memory blocks, these are all linked into a bundle of NUM_PER_BUNDLE. */
	LockFreePointerListLIFORoot<PLATFORM_CACHE_LINE_SIZE> GlobalFreeListBundles;
};


static LockFreeLinkAllocator_TLSCache& GetLockFreeAllocator()
{
	// make memory that will not go away, a replacement for TLazySingleton, which will still get destructed
	alignas(LockFreeLinkAllocator_TLSCache) static unsigned char Data[sizeof(LockFreeLinkAllocator_TLSCache)];
	static bool bIsInitialized = false;
	if (!bIsInitialized)
	{
		new(Data)LockFreeLinkAllocator_TLSCache();
		bIsInitialized = true;
	}
	return *(LockFreeLinkAllocator_TLSCache*)Data;
}

void LockFreeLinkPolicy::FreeLockFreeLink(LockFreeLinkPolicy::TLinkPtr Item)
{
	GetLockFreeAllocator().Push(Item);
}

LockFreeLinkPolicy::TLinkPtr LockFreeLinkPolicy::AllocLockFreeLink() TSAN_SAFE
{
	LockFreeLinkPolicy::TLinkPtr Result = GetLockFreeAllocator().Pop();
	// this can only really be a mem stomp
	checkLockFreePointerList(Result && !LockFreeLinkPolicy::DerefLink(Result)->DoubleNext.GetPtr() && !LockFreeLinkPolicy::DerefLink(Result)->Payload && !LockFreeLinkPolicy::DerefLink(Result)->SingleNext);
	return Result;
}

LockFreeLinkPolicy::Allocator LockFreeLinkPolicy::LinkAllocator;
