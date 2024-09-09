#include "strafepch.h"
#include "WindowsPlatformTLS.h"
#include "Strafe/Core/Utils/AtomicQueue.h"

// Custom  dynamic TLS implementation for OS TLS slot limit 
// Compile-time defined num slot and num thread limits.
// Allocating and freeing slots can be contended, getting and setting a slot value is fast but has an additional indirection compared with OS TLS.

namespace WindowsPlatformTLS_Private
{
	static const uint32 InvalidTlsSlot = 0xFFFFFFFF;

	// lock-free container for indices in the range [0..Size)
	template<uint32 Size>
	class LockFreeIndices
	{
	public:
		LockFreeIndices()
		{
			// initially all indices are available
			for (uint32 i = 0; i != Size; ++i)
			{
				// `0` is a special value for AtomicQueue, shift away from it
				Queue.push(i + 1);
			}
		}

		uint32 Alloc()
		{
			return Queue.pop() - 1;
		}

		void Free(uint32 Value)
		{
			Queue.push(Value + 1);
		}

	private:
		atomic_queue::AtomicQueue<uint32, Size> Queue;
	};

	// TLS storage for all threads and slots
	class TLSStorage
	{
	public:
		void** GetThreadStorage(uint32 ThreadIndex)
		{
			return Buffer[ThreadIndex];
		}

		uint32 GetThreadIndex(void** ThreadStorage)
		{
			uint32 ThreadIndex = uint32((ThreadStorage - *Buffer) / MaxSlots);
			//check(ThreadIndex < MaxThreads);
			return ThreadIndex;
		}

		void ResetSlot(uint32 SlotIndex)
		{
			for (uint32 ThreadIndex = 0; ThreadIndex != MaxThreads; ++ThreadIndex)
			{
				Buffer[ThreadIndex][SlotIndex] = nullptr;
			}
		}

		void ResetThread(uint32 ThreadIndex)
		{
			memset(Buffer[ThreadIndex],0, MaxSlots * sizeof(void*));
		}

	private:
		void* Buffer[MaxThreads][MaxSlots]; // compile-time zero initialized
	};

	TLSStorage Storage;

	void OnThreadExit(void* TlsValue);

	// uses local static var to not depend on static initialization order
	class FSingleton
	{
	private:
		FSingleton()
		{
			PrimarySlot = ::FlsAlloc(&OnThreadExit);
		}

	public:
		static FSingleton& Get()
		{
			static FSingleton Singleton;
			return Singleton;
		}

		LockFreeIndices<MaxSlots> Slots;
		LockFreeIndices<MaxThreads> Threads;
	};

	// called on thread exit if the thread has ever set TLS value
	// TlsValue: a value set to PrimarySlot
	void OnThreadExit(void* TlsValue)
	{
		/*check(PrimarySlot < MaxSlots);*/

		::FlsSetValue(PrimarySlot, nullptr); // clear the value otherwise this function will be called again

		void** ThreadStorage = (void**)TlsValue;
		uint32 ThreadIndex = Storage.GetThreadIndex(ThreadStorage);
		Storage.ResetThread(ThreadIndex);
		FSingleton::Get().Threads.Free(ThreadIndex);
	}

	void** AllocThreadStorage()
	{
		uint32 ThreadIndex = FSingleton::Get().Threads.Alloc();
		void** ThreadStorage = Storage.GetThreadStorage(ThreadIndex);
		::FlsSetValue(PrimarySlot, ThreadStorage);
		return ThreadStorage;
	}
}
uint32 WindowsPlatformTLS::AllocTlsSlot()
{
	return WindowsPlatformTLS_Private::FSingleton::Get().Slots.Alloc();
}

void WindowsPlatformTLS::FreeTlsSlot(uint32 SlotIndex)
{
	using namespace WindowsPlatformTLS_Private;

	//checkf(SlotIndex < MaxSlots, TEXT("Invalid TLS slot index %u"), SlotIndex);

	Storage.ResetSlot(SlotIndex);
	FSingleton::Get().Slots.Free(SlotIndex);
}
