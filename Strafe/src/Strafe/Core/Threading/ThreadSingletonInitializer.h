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

template <class T>
class ThreadSingleton : public TlsAutoCleanup
{
	static  uint32& GetTlsSlot()
	{
		static uint32 TlsSlot = WindowsPlatformTLS::InvalidTlsSlot;
		return TlsSlot;
	}

protected:
	//default constructor 
	ThreadSingleton()
		: m_ThreadId(WindowsPlatformTLS::GetCurrentThreadId())
	{}

	//returns a new instance of the thread singleton
	static TlsAutoCleanup* CreateInstance)()
	{
		return new T();
	}
	//thread id of this thread singleton
	const uint32 m_ThreadId;
public:
	//returns an instane of a singleton for the current thread
	FORCEINLINE  static T& Get()
	{
		return *(T*)ThreadSingletonInitializer::Get([]() { return (TlsAutoCleanup*)new T(); }, T::GetTlsSlot()); //-V572
	}

	//overloaded : create instance function to call when a new instance must be created
	FORCEINLINE static T& Get(TlsAutoCleanup* (*CreateInstance) ())
	{
		return *(T*)ThreadSingletonInitializer::Get(CreateInstance, T::GetTlsSlot()); //-V572
	}

	//return pointer to an instance of a singleton for the current thread. may be nullptr, prefer to access by ref
	FORCEINLINE static T* TryGet()
	{
		return (T*)ThreadSingletonInitializer::TryGet(T::GetTlsSlot());
	}

	//returns sets the tls store to the instance and returns the previous instance
	FORCEINLINE static T* Inject(T* Instance)
	{
		return (T*)ThreadSingletonInitializer::Inject(Instance, T::GetTlsSlot());
	}
};