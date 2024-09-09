#pragma once
#include "Windows.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"

#define WINDOWS_MAX_NUM_TLS_SLOTS 2048
#define WINDOWS_MAX_NUM_THREADS_WITH_TLS_SLOTS 512

// Custom cross-platform dynamic TLS implementation because we hit Windows TLS slot limit (1088). compile-time defined limits for the maximum
// number of TLS slots and the maximum number of threads that use TLS. No dynamic memory allocation. Lock-free.
// Getting and setting a slot value is fast but has an additional indirection compared with OS TLS.
// Memory footprint is (roughly): MaxSlots * 4B + MaxThreads * 4B + MaxSlots * MaxThreads * 8B

namespace WindowsPlatformTLS_Private
{
	constexpr uint32 MaxSlots = WINDOWS_MAX_NUM_TLS_SLOTS; // how many slots are available
	constexpr uint32 MaxThreads = WINDOWS_MAX_NUM_THREADS_WITH_TLS_SLOTS; // how many threads can use TLS

	// a single OS TLS slot that is used to store the thread storage
	 extern uint32 PrimarySlot;

	 void** AllocThreadStorage();
}

/**
 * Windows implementation of the TLS OS functions.
 */
struct WindowsPlatformTLS
{
	static const uint32 InvalidTlsSlot = 0xFFFFFFFF;

	/**
	 * Return false if this is an invalid TLS slot
	 * @param SlotIndex the TLS index to check
	 * @return true if this looks like a valid slot
	 */
	static FORCEINLINE bool IsValidTlsSlot(uint32 SlotIndex)
	{
		return SlotIndex != InvalidTlsSlot;
	}

	/**
	 * Returns the currently executing thread's identifier.
	 *
	 * @return The thread identifier.
	 */
	static FORCEINLINE uint32 GetCurrentThreadId(void)
	{
		return ::GetCurrentThreadId();
	}

	/**
	 * Allocates a thread local store slot.
	 *
	 * @return The index of the allocated slot.
	 */

	FORCEINLINE static uint32 AllocTlsSlot()
	{
		return ::TlsAlloc();
	}

	/**
	 * Frees a previously allocated TLS slot
	 *
	 * @param SlotIndex the TLS index to store it in
	 */

	FORCEINLINE static void FreeTlsSlot(uint32 SlotIndex)
	{
		::TlsFree(SlotIndex);
	}

	/**
	 * Sets a value in the specified TLS slot.
	 *
	 * @param SlotIndex the TLS index to store it in.
	 * @param Value the value to store in the slot.
	 */
	static FORCEINLINE void SetTlsValue(uint32 SlotIndex, void* Value)
	{

		::TlsSetValue(SlotIndex, Value);

	}

	/**
	 * Reads the value stored at the specified TLS slot.
	 *
	 * @param SlotIndex The index of the slot to read.
	 * @return The value stored in the slot.
	 */
	static FORCEINLINE void* GetTlsValue(uint32 SlotIndex)
	{
		return ::TlsGetValue(SlotIndex);
	}
};

