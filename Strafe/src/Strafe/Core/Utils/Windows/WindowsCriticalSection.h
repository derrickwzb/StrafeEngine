#pragma once

#include "WindowsApiAbstractions.h"
#include <string>


// Critical section interface for windows platform for locking purposes
class WindowsCriticalSection
{
public:
	WindowsCriticalSection(const WindowsCriticalSection&) = delete;
	WindowsCriticalSection& operator=(const WindowsCriticalSection&) = delete;

	//Constructor to initialize windows critical section
	FORCEINLINE WindowsCriticalSection()
	{
		__pragma(warning(suppress:28125));
		InitializeCriticalSection(&m_CriticalSection);
		SetCriticalSectionSpinCount(&m_CriticalSection, 4000);
	}

	//Destructor
	FORCEINLINE ~WindowsCriticalSection()
	{
		DeleteCriticalSection(&m_CriticalSection);
	}

	//Lock the critical section
	FORCEINLINE void Lock()
	{
		EnterCriticalSection(&m_CriticalSection);
	}

	//Attempt to take a locka and return true if successful
	FORCEINLINE bool TryLock()
	{
		return TryEnterCriticalSection(&m_CriticalSection);
	}

	//Unlock the critical section
	FORCEINLINE void Unlock()
	{
		LeaveCriticalSection(&m_CriticalSection);
	}

private:
// windows specific critical section
	CRITICAL_SECTION m_CriticalSection;
};

//Windows Read Write Lock/Mutex
//provides non recursive read write lock or shared exclusive access
//windows specific lock structures /calls from msdn documentation
class WindowsReadWriteLock
{
public:
	WindowsReadWriteLock(const WindowsReadWriteLock&) = delete;
	WindowsReadWriteLock& operator=(const WindowsReadWriteLock&) = delete;

	FORCEINLINE WindowsReadWriteLock(unsigned int Level = 0)
	{
		InitializeSRWLock(&m_Mutex);
	}

	~WindowsReadWriteLock()
	{
		//check if islocked , and log error if destroying a lock that is still locked
	}

	FORCEINLINE void ReadLock()
	{
		AcquireSRWLockShared(&m_Mutex);
	}

	FORCEINLINE void WriteLock()
	{
		AcquireSRWLockExclusive(&m_Mutex);
	}

	FORCEINLINE void TryReadLock()
	{
		TryAcquireSRWLockShared(&m_Mutex);
	}

	FORCEINLINE bool TryWriteLock()
	{
		return TryAcquireSRWLockExclusive(&m_Mutex);
	}

	FORCEINLINE void ReadUnlock()
	{
		ReleaseSRWLockShared(&m_Mutex);
	}
	
	FORCEINLINE void WriteUnlock()
	{
		ReleaseSRWLockExclusive(&m_Mutex);
	}

private:
	bool isLocked()
	{
		if (TryAcquireSRWLockExclusive(&m_Mutex))
		{
			ReleaseSRWLockExclusive(&m_Mutex);
			return false;
		}
		else
		{
			return true;
		}
	}


private:
	SRWLOCK m_Mutex;

};

typedef WindowsReadWriteLock RWLock;
typedef WindowsCriticalSection CriticalSection;


// implement if needed // needs timespan which is annoying
//class WindowsSystemWideCriticalSection
//{
//public:
//	explicit WindowsSystemWideCriticalSection(const std::string& Name, );
//
//private:
//	WindowsSystemWideCriticalSection(const WindowsSystemWideCriticalSection&);
//	WindowsSystemWideCriticalSection& operator=(const WindowsSystemWideCriticalSection&);
//private:
//	void* m_mutex;
//};

