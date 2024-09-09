#pragma once

#include "Strafe/Core/Utils/Timespan.h"
#include <algorithm>
#include <atomic>
#include <memory>

//interface for waitable events
//platform specific implementation that are used to wait for another thread to signal that it is ready 
//for the waiting thread to do work. can also be used to tell groups of threads to exit

//use eventref as a safer and more convenient alternative

class GenericEvent
{
public:
	//creates the event
	//manually reset events stay triggered until reset.
	//named events share the same underlying event.
	//try not to use
	//direct creation of events is not recommended for performance reasons. use GenericPlatformProcess::GetSynchEventFromPool/ReturnSynchEventToPool instead
	virtual bool Create(bool IsManualReset = false) = 0;


	//whether the signaled state of this event needs to be reset manually
	virtual bool IsManualReset() = 0;

	//Triggers the event so any waiting threads are activated
	virtual void Trigger() = 0;

	//Resets the event to an untriggered (waitable) state
	virtual void Reset() = 0;

	//Waits for the event to be triggered
	//a wait time of max_uint32 will wait indefinitely
	virtual bool Wait(unsigned int WaitTime, const bool IgnoreThreadIdleStats = false) = 0;

	//waits an infinite amount of time for the event to be triggered
	bool Wait()
	{
		return Wait(((unsigned int)0xffffffff));
	}

	//wait the specific amount of time for the event to be triggered
	bool Wait(const Timespan& WaitTime, const bool bIgnoreThreadIdleStats = false)
	{
		//check(WaitTime.GetTicks() >= 0);
		return Wait((unsigned int)std::clamp((unsigned int)(WaitTime.GetTicks() / TimespanConstants::TicksPerMillisecond), (unsigned int)0, ((unsigned int)0xffffffff)), bIgnoreThreadIdleStats);
	}

	/** Default constructor. */
	GenericEvent()
		: EventId(0)
		, EventStartCycles(0)
	{}
	/** Virtual destructor. */
	virtual ~GenericEvent()
	{}

protected:
	static std::atomic<unsigned int> EventUniqueId;
	unsigned int EventId;
	std::atomic<unsigned int> EventStartCycles;

};

enum class EventMode{AutoReset, ManualReset};


/**
 * RAII-style pooled `FEvent`
 *
 * non-copyable, non-movable
 */
class EventRef final
{
public:
	 explicit EventRef(EventMode Mode = EventMode::AutoReset);

	 ~EventRef();

	 EventRef(const EventRef&) = delete;
	 EventRef& operator=(const EventRef&) = delete;
	 EventRef(EventRef&& Other) = delete;
	 EventRef& operator=(EventRef&& Other) = delete;

	GenericEvent* operator->() const
	{
		return Event;
	}

	GenericEvent* Get()
	{
		return Event;
	}

private:
	GenericEvent* Event;
};

/**
 * RAII-style shared and pooled `FEvent`
 */
class SharedEventRef final
{
public:
	 explicit SharedEventRef(EventMode Mode = EventMode::AutoReset);

	GenericEvent* operator->() const
	{
		return Ptr.get();
	}

private:
	std::shared_ptr<GenericEvent> Ptr;
};


/**
 * Implements the Windows version of the FEvent interface.
 */
class EventWin
	: public GenericEvent
{
public:

	/** Default constructor. */
	EventWin()
		: Event(nullptr)
	{ }

	/** Virtual destructor. */
	virtual ~EventWin()
	{
		if (Event != nullptr)
		{
			CloseHandle(Event);
		}
	}

	// FEvent interface

	virtual bool Create(bool bIsManualReset = false) override
	{
		// Create the event and default it to non-signaled
		Event = CreateEvent(nullptr, bIsManualReset, 0, nullptr);
		ManualReset = bIsManualReset;

		return Event != nullptr;
	}

	virtual bool IsManualReset() override
	{
		return ManualReset;
	}

	virtual void Trigger() override;
	virtual void Reset() override;
	virtual bool Wait(uint32 WaitTime, const bool bIgnoreThreadIdleStats = false) override;

private:
	/** Holds the handle to the event. */
	HANDLE Event;

	/** Whether the signaled state of the event needs to be reset manually. */
	bool ManualReset;
};