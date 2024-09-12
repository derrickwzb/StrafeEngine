#pragma once

#include "Strafe/Core/Threading/Windows/WindowsPlatformTLS.h"
#include "Strafe/Core/Threading/TlsAutoCleanup.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"


//thread singleton initializer
class ThreadSingletonInitializer
{
public:
	//return an instance of a singleton for the current thread
	static TlsAutoCleanup* Get(TlsAutoCleanup* (*CreateInstance)(), uint32& TlsSlot);

	//return an instance of the singleton if it exists on the current thread
	static TlsAutoCleanup* TryGet(uint32& TlsSlot);

	//return sets the TLS store to the instance and returns the previous instance
	static TlsAutoCleanup* Inject(TlsAutoCleanup* Instance, uint32& TlsSlot);
};


//this is a special version of singleton. it means that there is created only one instance for each thread
// calling get method is thread safe

