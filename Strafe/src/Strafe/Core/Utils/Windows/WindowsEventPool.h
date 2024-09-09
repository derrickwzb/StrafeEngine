#pragma once
#include "Strafe/Core/Utils/Windows/WindowsEvent.h"
#include "Strafe/Core/Threading/Containers/LockFreeList.h"

#define PRAGMA_DISABLE_DEPRECATION_WARNINGS \
			__pragma (warning(push)) \
			__pragma (warning(disable: 4995)) /* 'function': name was marked as #pragma deprecated */ \
			__pragma (warning(disable: 4996)) /* The compiler encountered a deprecated declaration. */

#define PRAGMA_ENABLE_DEPRECATION_WARNINGS \
			__pragma (warning(pop))
class FSafeRecyclableEvent  final : public GenericEvent
{
public:
	GenericEvent* InnerEvent;

	FSafeRecyclableEvent(GenericEvent* InInnerEvent)
		: InnerEvent(InInnerEvent)
	{
	}

	~FSafeRecyclableEvent()
	{
		InnerEvent = nullptr;
	}

	virtual bool Create(bool bIsManualReset = false)
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
			return InnerEvent->Create(bIsManualReset);
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}

	virtual bool IsManualReset()
	{
		return InnerEvent->IsManualReset();
	}

	virtual void Trigger()
	{
		InnerEvent->Trigger();
	}

	virtual void Reset()
	{
		InnerEvent->Reset();
	}

	virtual bool Wait(uint32 WaitTime, const bool bIgnoreThreadIdleStats = false)
	{
		return InnerEvent->Wait(WaitTime, bIgnoreThreadIdleStats);
	}

};


/**
 * Template class for event pools.
 *
 * Events are expensive to create on most platforms. This pool allows for efficient
 * recycling of event instances that are no longer used. Events can have their signaled
 * state reset automatically or manually. The PoolType template parameter specifies
 * which type of events the pool managers.
 *
 * @param PoolType Specifies the type of pool.
 * @see FEvent
 */
template<EventMode PoolType>
class TEventPool
{
public:
	~TEventPool()
	{
		EmptyPool();
	}


	/**
	 * Gets an event from the pool or creates one if necessary.
	 *
	 * @return The event.
	 * @see ReturnToPool
	 */
	GenericEvent* GetEventFromPool()
	{
		return new FSafeRecyclableEvent(GetRawEvent());
	}

	/**
	 * Returns an event to the pool.
	 *
	 * @param Event The event to return.
	 * @see GetEventFromPool
	 */
	void ReturnToPool(GenericEvent* Event)
	{
		//check(Event);
		//check(Event->IsManualReset() == (PoolType == EEventMode::ManualReset));

		FSafeRecyclableEvent* SafeEvent = (FSafeRecyclableEvent*)Event;
		ReturnRawEvent(SafeEvent->InnerEvent);
		delete SafeEvent;
	}

	void EmptyPool()
	{

		while (GenericEvent* Event = Pool.Pop())
		{
			delete Event;
		}

	}

	/**
	* Gets a "raw" event (as opposite to `FSafeRecyclableEvent` handle returned by `GetEventFromPool`) from the pool or
	* creates one if necessary.
	* @see ReturnRaw
	*/
	GenericEvent* GetRawEvent()
	{
		GenericEvent* Event =

			Pool.Pop();


		if (Event == nullptr)
		{
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
				Event = WindowsPlatformProcess::CreateSynchEvent(PoolType == EventMode::ManualReset);
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
		}

		//check(Event);

		//Event->AdvanceStats();

		return Event;
	}

	/**
	 * Returns a "raw" event to the pool.
	 * @see GetRaw
	 */
	void ReturnRawEvent(GenericEvent* Event)
	{


		Event->Reset();
		Pool.Push(Event);

		//delete Event;

	}

private:

	/** Holds the collection of recycled events. */
	LockFreePointerListUnordered<GenericEvent, PLATFORM_CACHE_LINE_SIZE> Pool;

};
