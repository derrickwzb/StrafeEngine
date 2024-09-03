#pragma once


#include "Strafe/Core/Utils/Windows/WindowsCriticalSection.h"

/**
* Makes a type non-copyable and non-movable by deleting copy/move constructors and assignment/move operators.
* The macro should be placed in the public section of the type for better compiler diagnostic messages.
* Example usage:
*
*	class MyClassName
*	{
*	public:
*		NONCOPY(FMyClassName)
*		MyClassName() = default;
*	};
*/
#define NONCOPY(TypeName) \
	TypeName(TypeName&&) = delete; \
	TypeName(const TypeName&) = delete; \
	TypeName& operator=(const TypeName&) = delete; \
	TypeName& operator=(TypeName&&) = delete;

//implementation of a scope lock

//this is a utility class that handles scope level locking.
//useful to keep from causing deadlocks due to exceptions being caught and knowing about the number of locks a given thread has on a resource
/*
* example usage:
* <code>
 *	{
 *		// Synchronize thread access to the following data
 *		ScopeLock scopelock(SynchObject);
 *		// Access data that is shared among multiple threads
 *		...
 *		// When ScopeLock goes out of scope, other threads can access data
 *	}
 * </code>
*/

class ScopeLock
{
public:
	
	//constructor that performs the lock on the object
	[[nodiscard]] ScopeLock(CriticalSection* SynchObject)
	: m_SynchObject(SynchObject)
	{
		SynchObject->Lock();
	}

	//destructor
	~ScopeLock()
	{
		Unlock();
	}

	void Unlock()
	{
		if(m_SynchObject)
		{
			m_SynchObject->Unlock();
			m_SynchObject = nullptr;
		 }
	}

private:
//default constructor
	ScopeLock();

	//copy constructor
	ScopeLock(const ScopeLock& scopelock);

	//assignment operator
	ScopeLock& operator=(const ScopeLock& scopelock)
	{
		return *this;
	}
private:	
	//hold the synchronization object
	CriticalSection* m_SynchObject;
};


//implementation of a scope unlock

//this is a utility class that handles scope level unlocking.
//useful to to allow access to a protected object when you are sure it can happen.
/*
* example usage:
* <code>
 *	{
 *		//Access data that is shared among multiple threads.
 *		ScopeUnLock scopeunlock(SynchObject);
 *		// Access data that is shared among multiple threads
 *		...
 *		// When ScopeUnLock goes out of scope, other threads cannot access data
 *	}
 * </code>
*/

class ScopeUnlock
{
public:
	
	//constructor that performs the unlock on the object
	[[nodiscard]] ScopeUnlock(CriticalSection* SynchObject)
	: m_SynchObject(SynchObject)
	{
		if (m_SynchObject)
		{
			SynchObject->Unlock();
		}
	}

	//destructor
	~ScopeUnlock()
	{
		if (m_SynchObject)
		{
			m_SynchObject->Lock();
		}
	}
private:
	//default constructor
	ScopeUnlock();

	//copy constructor
	ScopeUnlock(const ScopeLock& scopeunlock);

	//assignment operator
	ScopeUnlock& operator=(const ScopeLock& scopeunlock)
	{
		return *this;
	}
private:
	//hold the synchronization object
	CriticalSection* m_SynchObject;

};


// RAII-style scope locking of a synchronisation primitive
	// `MutexType` is required to implement `Lock` and `Unlock` methods
	// Example:
	//	{
	//		TScopeLock<FCriticalSection> ScopeLock(CriticalSection);
	//		...
	//	}
template<typename MutexType>
class RAIIScopeLock
{
public:
	NONCOPY(RAIIScopeLock);

	[[nodiscard]] RAIIScopeLock(MutexType& InMutex)
		: Mutex(&InMutex)
	{
		//check if mutex : todo@ implement
		Mutex->Lock();
	}

	~RAIIScopeLock()
	{
		Unlock();
	}

	void Unlock()
	{
		if (Mutex)
		{
			Mutex->Unlock();
			Mutex = nullptr;
		}
	}

private:
	MutexType* Mutex;
};

// RAII-style scope unlocking of a synchronisation primitive
	// `MutexType` is required to implement `Lock` and `Unlock` methods
	// Example:
	//	{
	//		TScopeLock<FCriticalSection> ScopeLock(CriticalSection);
	//		for (FElementType& Element : ThreadUnsafeContainer)
	//		{
	//			TScopeUnlock<FCriticalSection> ScopeUnlock(CriticalSection);
	//			Process(Element);
	//		}
	//	}
template<typename MutexType>
class RAIIScopeUnlock
{
public:
	NONCOPY(RAIIScopeUnlock);

	[[nodiscard]] RAIIScopeUnlock(MutexType& InMutex)
		: Mutex(&InMutex)
	{
		//check if mutex : todo@ implement
		Mutex->Unlock();
	}

	~RAIIScopeUnlock()
	{
		Mutex->Lock();
	}

private:
	MutexType* Mutex;
};