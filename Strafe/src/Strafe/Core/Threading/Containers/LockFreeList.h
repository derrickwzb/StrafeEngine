#pragma once
#pragma warning(disable : 4706)

#include "Strafe/Core/Utils/Windows/WindowsPlatformProcess.h"
#include <intrin.h>

#define FORCEINLINE __forceinline	
#define TSAN_SAFE
#define TSAN_BEFORE(Addr)
#define TSAN_AFTER(Addr)
#define TSAN_ATOMIC(Type) Type

typedef unsigned int uint32;
typedef unsigned __int64	SIZE_T;
typedef signed long long int64;
typedef unsigned long long uint64;
typedef signed int int32;

//all these not a problem just sleep or smth
void LockFreeTagCounterHasOverflowed()
{
	//todo log this shit
	std::cout << "sleep" << std::endl;
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

#define MAX_LOCK_FREE_LINKS_AS_BITS (26)
#define MAX_LOCK_FREE_LINKS (1 << 26)
#define PLATFORM_CACHE_LINE_SIZE	64


#define checkLockFreePointerList(x) ((x)||((*(char*)3) = 0))

/**
 * Checks if a pointer is aligned to the specified alignment.
 *
 * @param  Val        The value to align.
 * @param  Alignment  The alignment value, must be a power of two.
 *
 * @return true if the pointer is aligned to the specified alignment, false otherwise.
 */
template <typename T>
FORCEINLINE constexpr bool IsAligned(T Val, uint64 Alignment)
{
	/*static_assert(TIsIntegral<T>::Value || TIsPointer<T>::Value, "IsAligned expects an integer or pointer type");*/

	return !((uint64)Val & (Alignment - 1));
}

template<class T, unsigned int MaxTotalItems, unsigned int ItemsPerPage>
class LockFreeAllocOnceIndexedAllocator
{
	enum
	{
		MaxBlocks = (MaxTotalItems + ItemsPerPage - 1) / ItemsPerPage
	};
public:

	LockFreeAllocOnceIndexedAllocator()
	{
		NextIndex.Increment(); // skip the null ptr
		for (uint32 Index = 0; Index < MaxBlocks; Index++)
		{
			Pages[Index] = nullptr;
		}
	}

	FORCEINLINE uint32 Alloc(uint32 Count = 1)
	{
		uint32 FirstItem = NextIndex.Add(Count);
		if (FirstItem + Count > MaxTotalItems)
		{
			LockFreeLinksExhausted(MaxTotalItems);
		}
		for (uint32 CurrentItem = FirstItem; CurrentItem < FirstItem + Count; CurrentItem++)
		{
			new (GetRawItem(CurrentItem)) T();
		}
		return FirstItem;
	}
	FORCEINLINE T* GetItem(uint32 Index)
	{
		if (!Index)
		{
			return nullptr;
		}
		uint32 BlockIndex = Index / ItemsPerPage;
		uint32 SubIndex = Index % ItemsPerPage;

		checkLockFreePointerList(Index < (uint32)NextIndex.load() && Index < MaxTotalItems && BlockIndex < MaxBlocks && Pages[BlockIndex]);
		return Pages[BlockIndex] + SubIndex;
	}

private:
	void* GetRawItem(uint32 Index)
	{
		uint32 BlockIndex = Index / ItemsPerPage;
		uint32 SubIndex = Index % ItemsPerPage;
		checkLockFreePointerList(Index && Index < (uint32)NextIndex.load() && Index < MaxTotalItems && BlockIndex < MaxBlocks);
		if (!Pages[BlockIndex])
		{
			T* NewBlock = (T*)LockFreeAllocLinks(ItemsPerPage * sizeof(T));
			checkLockFreePointerList(IsAligned(NewBlock, alignof(T)));

			//might need to check atomic operation
			if (::_InterlockedCompareExchangePointer((void**)&Pages[BlockIndex], NewBlock, nullptr) != nullptr)
			{
				// we lost discard block
				checkLockFreePointerList(Pages[BlockIndex] && Pages[BlockIndex] != NewBlock);
				LockFreeFreeLinks(ItemsPerPage * sizeof(T), NewBlock);
			}
			else
			{
				checkLockFreePointerList(Pages[BlockIndex]);
			}
		}
		return (void*)(Pages[BlockIndex] + SubIndex);
	}

	//check load function	
	alignas(PLATFORM_CACHE_LINE_SIZE) std::atomic<signed int> NextIndex;
	alignas(PLATFORM_CACHE_LINE_SIZE) T* Pages[MaxBlocks];
};


#define MAX_TagBitsValue (uint64(1) << (64 - MAX_LOCK_FREE_LINKS_AS_BITS))
struct IndexedLockFreeLink;

struct IndexedPointer
{
	// no constructor, intentionally. We need to keep the ABA double counter in tact

	// This should only be used for FIndexedPointer's with no outstanding concurrency.
	// Not recycled links, for example.
	void Init()
	{
		static_assert(((MAX_LOCK_FREE_LINKS - 1) & MAX_LOCK_FREE_LINKS) == 0, "MAX_LOCK_FREE_LINKS must be a power of two");
		Ptrs = 0;
	}
	FORCEINLINE void SetAll(uint32 Ptr, uint64 CounterAndState)
	{
		checkLockFreePointerList(Ptr < MAX_LOCK_FREE_LINKS && CounterAndState < (uint64(1) << (64 - MAX_LOCK_FREE_LINKS_AS_BITS)));
		Ptrs = (uint64(Ptr) | (CounterAndState << MAX_LOCK_FREE_LINKS_AS_BITS));
	}

	FORCEINLINE uint32 GetPtr() const
	{
		return uint32(Ptrs & (MAX_LOCK_FREE_LINKS - 1));
	}

	FORCEINLINE void SetPtr(uint32 To)
	{
		SetAll(To, GetCounterAndState());
	}

	FORCEINLINE uint64 GetCounterAndState() const
	{
		return (Ptrs >> MAX_LOCK_FREE_LINKS_AS_BITS);
	}

	FORCEINLINE void SetCounterAndState(uint64 To)
	{
		SetAll(GetPtr(), To);
	}

	FORCEINLINE void AdvanceCounterAndState(const IndexedPointer& From, uint64 ABAInc)
	{
		SetCounterAndState(From.GetCounterAndState() + ABAInc);
		if (!!(GetCounterAndState() < From.GetCounterAndState()))
		{
			// this is not expected to be a problem and it is not expected to happen very often. When it does happen, we will sleep as an extra precaution.
			LockFreeTagCounterHasOverflowed();
		}
	}

	template<uint64 ABAInc>
	FORCEINLINE uint64 GetState() const
	{
		return GetCounterAndState() & (ABAInc - 1);
	}

	template<uint64 ABAInc>
	FORCEINLINE void SetState(uint64 Value)
	{
		checkLockFreePointerList(Value < ABAInc);
		SetCounterAndState((GetCounterAndState() & ~(ABAInc - 1)) | Value);
	}

	FORCEINLINE void AtomicRead(const IndexedPointer& Other)
	{
		checkLockFreePointerList(IsAligned(&Ptrs, 8) && IsAligned(&Other.Ptrs, 8));
		Ptrs = uint64((int64)::_InterlockedCompareExchange64((int64*)((volatile const int64*)&Other.Ptrs), 0, 0));
	}

	FORCEINLINE bool IPInterlockedCompareExchange(const IndexedPointer& Exchange, const IndexedPointer& Comparand)
	{
		return uint64(::_InterlockedCompareExchange64((volatile int64*)&Ptrs, Exchange.Ptrs, Comparand.Ptrs)) == Comparand.Ptrs;
	}

	FORCEINLINE bool operator==(const IndexedPointer& Other) const
	{
		return Ptrs == Other.Ptrs;
	}
	FORCEINLINE bool operator!=(const IndexedPointer& Other) const
	{
		return Ptrs != Other.Ptrs;
	}

private:
	uint64 Ptrs;

};

struct IndexedLockFreeLink
{
	IndexedPointer DoubleNext;
	void* Payload;
	uint32 SingleNext;
};

struct LockFreeLinkPolicy
{
	enum
	{
		MAX_BITS_IN_TLinkPtr = MAX_LOCK_FREE_LINKS_AS_BITS
	};
	typedef IndexedPointer TDoublePtr;
	typedef IndexedLockFreeLink TLink;
	typedef uint32 TLinkPtr;
	typedef LockFreeAllocOnceIndexedAllocator<IndexedLockFreeLink, MAX_LOCK_FREE_LINKS, 16384> Allocator;

	static FORCEINLINE IndexedLockFreeLink* DerefLink(uint32 Ptr)
	{
		return LinkAllocator.GetItem(Ptr);
	}
	static FORCEINLINE IndexedLockFreeLink* IndexToLink(uint32 Index)
	{
		return LinkAllocator.GetItem(Index);
	}
	static FORCEINLINE uint32 IndexToPtr(uint32 Index)
	{
		return Index;
	}

	static uint32 AllocLockFreeLink();
	static void FreeLockFreeLink(uint32 Item);
	static Allocator LinkAllocator;
};

template<int PaddingForCacheContention, uint64 TABAInc = 1>
class LockFreePointerListLIFORoot
{
	//UE_NONCOPYABLE(FLockFreePointerListLIFORoot)

	typedef LockFreeLinkPolicy::TDoublePtr TDoublePtr;
	typedef LockFreeLinkPolicy::TLink TLink;
	typedef LockFreeLinkPolicy::TLinkPtr TLinkPtr;

public:
	FORCEINLINE LockFreePointerListLIFORoot()
	{
		// We want to make sure we have quite a lot of extra counter values to avoid the ABA problem. This could probably be relaxed, but eventually it will be dangerous. 
		// The question is "how many queue operations can a thread starve for".
		static_assert(MAX_TagBitsValue / TABAInc >= (1 << 23), "risk of ABA problem");
		static_assert((TABAInc & (TABAInc - 1)) == 0, "must be power of two");
		Reset();
	}

	void Reset()
	{
		Head.Init();
	}

	void Push(TLinkPtr Item) TSAN_SAFE
	{
		while (true)
		{
			TDoublePtr LocalHead;
			LocalHead.AtomicRead(Head);
			TDoublePtr NewHead;
			NewHead.AdvanceCounterAndState(LocalHead, TABAInc);
			NewHead.SetPtr(Item);
			LockFreeLinkPolicy::DerefLink(Item)->SingleNext = LocalHead.GetPtr();
			if (Head.IPInterlockedCompareExchange(NewHead, LocalHead))
			{
				break;
			}
		}
	}

	bool PushIf(TLinkPtr(*AllocateIfOkToPush)(uint64)) TSAN_SAFE
	{
		static_assert(TABAInc > 1, "method should not be used for lists without state");
		while (true)
		{
			TDoublePtr LocalHead;
			LocalHead.AtomicRead(Head);
			uint64 LocalState = LocalHead.GetState<TABAInc>();
			TLinkPtr Item = AllocateIfOkToPush(LocalState);
			if (!Item)
			{
				return false;
			}

			TDoublePtr NewHead;
			NewHead.AdvanceCounterAndState(LocalHead, TABAInc);
			LockFreeLinkPolicy::DerefLink(Item)->SingleNext = LocalHead.GetPtr();
			NewHead.SetPtr(Item);
			if (Head.IPInterlockedCompareExchange(NewHead, LocalHead))
			{
				break;
			}
		}
		return true;
	}


	TLinkPtr Pop() TSAN_SAFE
	{
		TLinkPtr Item = 0;
		while (true)
		{
			TDoublePtr LocalHead;
			LocalHead.AtomicRead(Head);
			Item = LocalHead.GetPtr();
			if (!Item)
			{
				break;
			}
			TDoublePtr NewHead;
			NewHead.AdvanceCounterAndState(LocalHead, TABAInc);
			TLink* ItemP = LockFreeLinkPolicy::DerefLink(Item);
			NewHead.SetPtr(ItemP->SingleNext);
			if (Head.IPInterlockedCompareExchange(NewHead, LocalHead))
			{
				ItemP->SingleNext = 0;
				break;
			}
		}
		return Item;
	}

	TLinkPtr PopAll() TSAN_SAFE
	{
		TLinkPtr Item = 0;
		while (true)
		{
			TDoublePtr LocalHead;
			LocalHead.AtomicRead(Head);
			Item = LocalHead.GetPtr();
			if (!Item)
			{
				break;
			}
			TDoublePtr NewHead;
			NewHead.AdvanceCounterAndState(LocalHead, TABAInc);
			NewHead.SetPtr(0);
			if (Head.IPInterlockedCompareExchange(NewHead, LocalHead))
			{
				break;
			}
		}
		return Item;
	}

	TLinkPtr PopAllAndChangeState(uint64(*StateChange)(uint64)) TSAN_SAFE
	{
		static_assert(TABAInc > 1, "method should not be used for lists without state");
		TLinkPtr Item = 0;
		while (true)
		{
			TDoublePtr LocalHead;
			LocalHead.AtomicRead(Head);
			Item = LocalHead.GetPtr();
			TDoublePtr NewHead;
			NewHead.AdvanceCounterAndState(LocalHead, TABAInc);
			NewHead.SetState<TABAInc>(StateChange(LocalHead.GetState<TABAInc>()));
			NewHead.SetPtr(0);
			if (Head.IPInterlockedCompareExchange(NewHead, LocalHead))
			{
				break;
			}
		}
		return Item;
	}

	FORCEINLINE bool IsEmpty() const
	{
		return !Head.GetPtr();
	}

	FORCEINLINE uint64 GetState() const
	{
		TDoublePtr LocalHead;
		LocalHead.AtomicRead(Head);
		return LocalHead.GetState<TABAInc>();
	}

private:
	alignas(PaddingForCacheContention) TDoublePtr Head;
};

template<class T, int PaddingForCacheContention, uint64 TABAInc = 1>
class LockFreePointerListLIFOBase
{

	typedef LockFreeLinkPolicy::TDoublePtr TDoublePtr;
	typedef LockFreeLinkPolicy::TLink TLink;
	typedef LockFreeLinkPolicy::TLinkPtr TLinkPtr;

public:
	LockFreePointerListLIFOBase() = default;

	~LockFreePointerListLIFOBase()
	{
		while (Pop()) {};
	}

	void Reset()
	{
		while (Pop()) {};
		RootList.Reset();
	}

	void Push(T* InPayload) TSAN_SAFE
	{
		TLinkPtr Item = LockFreeLinkPolicy::AllocLockFreeLink();
		LockFreeLinkPolicy::DerefLink(Item)->Payload = InPayload;
		RootList.Push(Item);
	}

	bool PushIf(T* InPayload, bool(*OkToPush)(uint64)) TSAN_SAFE
	{
		TLinkPtr Item = 0;

		auto AllocateIfOkToPush = [&OkToPush, InPayload, &Item](uint64 State)->TLinkPtr
			{
				if (OkToPush(State))
				{
					if (!Item)
					{
						Item = LockFreeLinkPolicy::AllocLockFreeLink();
						LockFreeLinkPolicy::DerefLink(Item)->Payload = InPayload;
					}
					return Item;
				}
				return 0;
			};
		if (!RootList.PushIf(AllocateIfOkToPush))
		{
			if (Item)
			{
				// we allocated the link, but it turned out that the list was closed
				LockFreeLinkPolicy::FreeLockFreeLink(Item);
			}
			return false;
		}
		return true;
	}


	T* Pop() TSAN_SAFE
	{
		TLinkPtr Item = RootList.Pop();
		T* Result = nullptr;
		if (Item)
		{
			Result = (T*)LockFreeLinkPolicy::DerefLink(Item)->Payload;
			LockFreeLinkPolicy::FreeLockFreeLink(Item);
		}
		return Result;
	}

	void PopAll(std::vector<T*>& OutArray) TSAN_SAFE
	{
		TLinkPtr Links = RootList.PopAll();
		while (Links)
		{
			TLink* LinksP = LockFreeLinkPolicy::DerefLink(Links);
			OutArray.emplace((T*)LinksP->Payload);
			TLinkPtr Del = Links;
			Links = LinksP->SingleNext;
			LockFreeLinkPolicy::FreeLockFreeLink(Del);
		}
	}

	void PopAllAndChangeState(std::vector<T*>& OutArray, uint64(*StateChange)(uint64)) TSAN_SAFE
	{
		TLinkPtr Links = RootList.PopAllAndChangeState(StateChange);
		while (Links)
		{
			TLink* LinksP = LockFreeLinkPolicy::DerefLink(Links);
			OutArray.emplace((T*)LinksP->Payload);
			TLinkPtr Del = Links;
			Links = LinksP->SingleNext;
			LockFreeLinkPolicy::FreeLockFreeLink(Del);
		}
	}

	FORCEINLINE bool IsEmpty() const
	{
		return RootList.IsEmpty();
	}

	FORCEINLINE uint64 GetState() const
	{
		return RootList.GetState();
	}

private:

	LockFreePointerListLIFORoot<PaddingForCacheContention, TABAInc> RootList;
};

template<class T, int PaddingForCacheContention, uint64 TABAInc = 1>
class LockFreePointerFIFOBase
{

	typedef LockFreeLinkPolicy::TDoublePtr TDoublePtr;
	typedef LockFreeLinkPolicy::TLink TLink;
	typedef LockFreeLinkPolicy::TLinkPtr TLinkPtr;
public:

	FORCEINLINE LockFreePointerFIFOBase()
	{
		// We want to make sure we have quite a lot of extra counter values to avoid the ABA problem. This could probably be relaxed, but eventually it will be dangerous. 
		// The question is "how many queue operations can a thread starve for".
		static_assert(TABAInc <= 65536, "risk of ABA problem");
		static_assert((TABAInc & (TABAInc - 1)) == 0, "must be power of two");

		Head.Init();
		Tail.Init();
		TLinkPtr Stub = LockFreeLinkPolicy::AllocLockFreeLink();
		Head.SetPtr(Stub);
		Tail.SetPtr(Stub);
	}

	~LockFreePointerFIFOBase()
	{
		while (Pop()) {};
		LockFreeLinkPolicy::FreeLockFreeLink(Head.GetPtr());
	}

	void Push(T* InPayload) TSAN_SAFE
	{
		TLinkPtr Item = LockFreeLinkPolicy::AllocLockFreeLink();
		LockFreeLinkPolicy::DerefLink(Item)->Payload = InPayload;
		TDoublePtr LocalTail;
		while (true)
		{
			LocalTail.AtomicRead(Tail);
			TLink* LocalTailP = LockFreeLinkPolicy::DerefLink(LocalTail.GetPtr());
			TDoublePtr LocalNext;
			LocalNext.AtomicRead(LocalTailP->DoubleNext);
			TDoublePtr TestLocalTail;
			TestLocalTail.AtomicRead(Tail);
			if (TestLocalTail == LocalTail)
			{
				if (LocalNext.GetPtr())
				{
					TDoublePtr NewTail;
					NewTail.AdvanceCounterAndState(LocalTail, TABAInc);
					NewTail.SetPtr(LocalNext.GetPtr());
					Tail.IPInterlockedCompareExchange(NewTail, LocalTail);
				}
				else
				{
					TDoublePtr NewNext;
					NewNext.AdvanceCounterAndState(LocalNext, TABAInc);
					NewNext.SetPtr(Item);
					if (LocalTailP->DoubleNext.IPInterlockedCompareExchange(NewNext, LocalNext))
					{
						break;
					}
				}
			}
		}
		{
			TDoublePtr NewTail;
			NewTail.AdvanceCounterAndState(LocalTail, TABAInc);
			NewTail.SetPtr(Item);
			Tail.IPInterlockedCompareExchange(NewTail, LocalTail);
		}
	}

	T* Pop() TSAN_SAFE
	{
		T* Result = nullptr;
		TDoublePtr LocalHead;
		while (true)
		{
			LocalHead.AtomicRead(Head);
			TDoublePtr LocalTail;
			LocalTail.AtomicRead(Tail);
			TDoublePtr LocalNext;
			LocalNext.AtomicRead(LockFreeLinkPolicy::DerefLink(LocalHead.GetPtr())->DoubleNext);
			TDoublePtr LocalHeadTest;
			LocalHeadTest.AtomicRead(Head);
			if (LocalHead == LocalHeadTest)
			{
				if (LocalHead.GetPtr() == LocalTail.GetPtr())
				{
					if (!LocalNext.GetPtr())
					{
						return nullptr;
					}
					TDoublePtr NewTail;
					NewTail.AdvanceCounterAndState(LocalTail, TABAInc);
					NewTail.SetPtr(LocalNext.GetPtr());
					Tail.IPInterlockedCompareExchange(NewTail, LocalTail);
				}
				else
				{
					Result = (T*)LockFreeLinkPolicy::DerefLink(LocalNext.GetPtr())->Payload;
					TDoublePtr NewHead;
					NewHead.AdvanceCounterAndState(LocalHead, TABAInc);
					NewHead.SetPtr(LocalNext.GetPtr());
					if (Head.IPInterlockedCompareExchange(NewHead, LocalHead))
					{
						break;
					}
				}
			}
		}
		LockFreeLinkPolicy::FreeLockFreeLink(LocalHead.GetPtr());
		return Result;
	}

	void PopAll(std::vector<T*>& OutArray)
	{
		while (T* Item = Pop())
		{
			OutArray.Add(Item);
		}
	}


	FORCEINLINE bool IsEmpty() const
	{
		TDoublePtr LocalHead;
		LocalHead.AtomicRead(Head);
		TDoublePtr LocalNext;
		LocalNext.AtomicRead(LockFreeLinkPolicy::DerefLink(LocalHead.GetPtr())->DoubleNext);
		return !LocalNext.GetPtr();
	}

private:
	alignas(PaddingForCacheContention) TDoublePtr Head;
	alignas(PaddingForCacheContention) TDoublePtr Tail;
};

template<class T, int PaddingForCacheContention, int NumPriorities>
class StallingTaskQueue
{

	typedef LockFreeLinkPolicy::TDoublePtr TDoublePtr;
	typedef LockFreeLinkPolicy::TLink TLink;
	typedef LockFreeLinkPolicy::TLinkPtr TLinkPtr;
public:
	StallingTaskQueue()
	{
		MasterState.Init();
	}
	int32 Push(T* InPayload, uint32 Priority)
	{
		checkLockFreePointerList(Priority < NumPriorities);
		TDoublePtr LocalMasterState;
		LocalMasterState.AtomicRead(MasterState);
		PriorityQueues[Priority].Push(InPayload);
		TDoublePtr NewMasterState;
		NewMasterState.AdvanceCounterAndState(LocalMasterState, 1);
		int32 ThreadToWake = FindThreadToWake(LocalMasterState.GetPtr());
		if (ThreadToWake >= 0)
		{
			NewMasterState.SetPtr(TurnOffBit(LocalMasterState.GetPtr(), ThreadToWake));
		}
		else
		{
			NewMasterState.SetPtr(LocalMasterState.GetPtr());
		}
		while (!MasterState.IPInterlockedCompareExchange(NewMasterState, LocalMasterState))
		{
			LocalMasterState.AtomicRead(MasterState);
			NewMasterState.AdvanceCounterAndState(LocalMasterState, 1);
			ThreadToWake = FindThreadToWake(LocalMasterState.GetPtr());

			if (ThreadToWake >= 0)
			{
				NewMasterState.SetPtr(TurnOffBit(LocalMasterState.GetPtr(), ThreadToWake));
			}
			else
			{
				NewMasterState.SetPtr(LocalMasterState.GetPtr());
			}
		}
		return ThreadToWake;
	}

	T* Pop(int32 MyThread, bool bAllowStall)
	{
		//check(MyThread >= 0 && MyThread < LockFreeLinkPolicy::MAX_BITS_IN_TLinkPtr);

		while (true)
		{
			TDoublePtr LocalMasterState;
			LocalMasterState.AtomicRead(MasterState);
			//checkLockFreePointerList(!TestBit(LocalMasterState.GetPtr(), MyThread) || !FPlatformProcess::SupportsMultithreading()); // you should not be stalled if you are asking for a task
			for (int32 Index = 0; Index < NumPriorities; Index++)
			{
				T* Result = PriorityQueues[Index].Pop();
				if (Result)
				{
					while (true)
					{
						TDoublePtr NewMasterState;
						NewMasterState.AdvanceCounterAndState(LocalMasterState, 1);
						NewMasterState.SetPtr(LocalMasterState.GetPtr());
						if (MasterState.IPInterlockedCompareExchange(NewMasterState, LocalMasterState))
						{
							return Result;
						}
						LocalMasterState.AtomicRead(MasterState);
						//checkLockFreePointerList(!TestBit(LocalMasterState.GetPtr(), MyThread) || !FPlatformProcess::SupportsMultithreading()); // you should not be stalled if you are asking for a task
					}
				}
			}
			if (!bAllowStall)
			{
				break; // if we aren't stalling, we are done, the queues are empty
			}
			{
				TDoublePtr NewMasterState;
				NewMasterState.AdvanceCounterAndState(LocalMasterState, 1);
				NewMasterState.SetPtr(TurnOnBit(LocalMasterState.GetPtr(), MyThread));
				if (MasterState.IPInterlockedCompareExchange(NewMasterState, LocalMasterState))
				{
					break;
				}
			}
		}
		return nullptr;
	}

private:

	static int32 FindThreadToWake(TLinkPtr Ptr)
	{
		int32 Result = -1;
		uint64 Test = uint64(Ptr);
		if (Test)
		{
			Result = 0;
			while (!(Test & 1))
			{
				Test >>= 1;
				Result++;
			}
		}
		return Result;
	}

	static TLinkPtr TurnOffBit(TLinkPtr Ptr, int32 BitToTurnOff)
	{
		return (TLinkPtr)(uint64(Ptr) & ~(uint64(1) << BitToTurnOff));
	}

	static TLinkPtr TurnOnBit(TLinkPtr Ptr, int32 BitToTurnOn)
	{
		return (TLinkPtr)(uint64(Ptr) | (uint64(1) << BitToTurnOn));
	}

	static bool TestBit(TLinkPtr Ptr, int32 BitToTest)
	{
		return !!(uint64(Ptr) & (uint64(1) << BitToTest));
	}

	LockFreePointerFIFOBase<T, PaddingForCacheContention> PriorityQueues[NumPriorities];
	// not a pointer to anything, rather tracks the stall state of all threads servicing this queue.
	alignas(PaddingForCacheContention) TDoublePtr MasterState;
};

template<class T, int PaddingForCacheContention>
class LockFreePointerListLIFOPad : private LockFreePointerListLIFOBase<T, PaddingForCacheContention>
{
public:

	/**
	*	Push an item onto the head of the list.
	*
	*	@param NewItem, the new item to push on the list, cannot be NULL.
	*/
	void Push(T* NewItem)
	{
		LockFreePointerListLIFOBase<T, PaddingForCacheContention>::Push(NewItem);
	}

	/**
	*	Pop an item from the list or return NULL if the list is empty.
	*	@return The popped item, if any.
	*/
	T* Pop()
	{
		return LockFreePointerListLIFOBase<T, PaddingForCacheContention>::Pop();
	}

	/**
	*	Pop all items from the list.
	*
	*	@param Output The array to hold the returned items. Must be empty.
	*/
	void PopAll(std::vector<T*>& Output)
	{
		LockFreePointerListLIFOBase<T, PaddingForCacheContention>::PopAll(Output);
	}

	/**
	*	Check if the list is empty.
	*
	*	@return true if the list is empty.
	*	CAUTION: This methods safety depends on external assumptions. For example, if another thread could add to the list at any time, the return value is no better than a best guess.
	*	As typically used, the list is not being access concurrently when this is called.
	*/
	FORCEINLINE bool IsEmpty() const
	{
		return LockFreePointerListLIFOBase<T, PaddingForCacheContention>::IsEmpty();
	}
};

template<class T>
class LockFreePointerListLIFO : public LockFreePointerListLIFOPad<T, 0>
{

};

template<class T, int PaddingForCacheContention>
class LockFreePointerListUnordered : public LockFreePointerListLIFOPad<T, PaddingForCacheContention>
{

};

template<class T, int PaddingForCacheContention>
class LockFreePointerListFIFO : private LockFreePointerFIFOBase<T, PaddingForCacheContention>
{
public:

	/**
	*	Push an item onto the head of the list.
	*
	*	@param NewItem, the new item to push on the list, cannot be NULL.
	*/
	void Push(T* NewItem)
	{
		LockFreePointerFIFOBase<T, PaddingForCacheContention>::Push(NewItem);
	}

	/**
	*	Pop an item from the list or return NULL if the list is empty.
	*	@return The popped item, if any.
	*/
	T* Pop()
	{
		return LockFreePointerFIFOBase<T, PaddingForCacheContention>::Pop();
	}

	/**
	*	Pop all items from the list.
	*
	*	@param Output The array to hold the returned items. Must be empty.
	*/
	void PopAll(std::vector<T*>& Output)
	{
		LockFreePointerFIFOBase<T, PaddingForCacheContention>::PopAll(Output);
	}

	/**
	*	Check if the list is empty.
	*
	*	@return true if the list is empty.
	*	CAUTION: This methods safety depends on external assumptions. For example, if another thread could add to the list at any time, the return value is no better than a best guess.
	*	As typically used, the list is not being access concurrently when this is called.
	*/
	FORCEINLINE bool IsEmpty() const
	{
		return LockFreePointerFIFOBase<T, PaddingForCacheContention>::IsEmpty();
	}
};

template<class T, int PaddingForCacheContention>
class ClosableLockFreePointerListUnorderedSingleConsumer : private LockFreePointerListLIFOBase<T, PaddingForCacheContention, 2>
{
public:

	/**
	*	Reset the list to the initial state. Not thread safe, but used for recycling when we know all users are gone.
	*/
	void Reset()
	{
		LockFreePointerListLIFOBase<T, PaddingForCacheContention, 2>::Reset();
	}

	/**
	*	Push an item onto the head of the list, unless the list is closed
	*
	*	@param NewItem, the new item to push on the list, cannot be NULL
	*	@return true if the item was pushed on the list, false if the list was closed.
	*/
	bool PushIfNotClosed(T* NewItem)
	{
		return LockFreePointerListLIFOBase<T, PaddingForCacheContention, 2>::PushIf(NewItem, [](uint64 State)->bool {return !(State & 1); });
	}

	/**
	*	Pop all items from the list and atomically close it.
	*
	*	@param Output The array to hold the returned items. Must be empty.
	*/
	void PopAllAndClose(std::vector<T*>& Output)
	{
		auto CheckOpenAndClose = [](uint64 State) -> uint64
			{
				checkLockFreePointerList(!(State & 1));
				return State | 1;
			};
		LockFreePointerListLIFOBase<T, PaddingForCacheContention, 2>::PopAllAndChangeState(Output, CheckOpenAndClose);
	}

	/**
	*	Check if the list is closed
	*
	*	@return true if the list is closed.
	*/
	bool IsClosed() const
	{
		return !!(LockFreePointerListLIFOBase<T, PaddingForCacheContention, 2>::GetState() & 1);
	}

};