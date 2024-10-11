#pragma once


#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include "Strafe/Core/Utils/Windows/WindowsEvent.h"

//this class allows a simple one shot scoped event
//usage
// ScopedEvent Event;
//SendReferenceOrPointerToSomeOtherThread(&MyEvent); // Other thread calls MyEvent->Trigger();
//Event destructor is here we wait here

class ScopedEvent
{
public:
	ScopedEvent();
	~ScopedEvent();

	void Trigger()
	{
		Event->Trigger();
	}

	//check if the event has been triggered 
	//if this return true once it will return true forever
	bool IsReady();

	GenericEvent* Get() { return Event; }

private:
	GenericEvent* Event;
};