#include "strafepch.h"
#include "Strafe/Core/Threading/Platform/WindowsPlatformAffinity.h"
#include  "Strafe/Core/Utils/LazySingleton.h"
#include "Strafe/Core/Utils/Windows/WindowsEvent.h"
#include "Strafe/Core/Utils/Windows/WindowsEventPool.h"
#include "WindowsPlatformProcess.h"
#include <shellapi.h>
#include <ShlObj.h>
#include <LM.h>
#include <Psapi.h>
#include <TlHelp32.h>
#pragma comment(lib, "psapi.lib")
__pragma (warning(push)) 
__pragma (warning(disable: 4244)) /* 'argument': conversion from 'type1' to 'type2', possible loss of data */	
__pragma (warning(disable: 4838)) /* 'argument': conversion from 'type1' to 'type2' requires a narrowing conversion */


WindowsPlatformProcess::WindowsSemaphore::WindowsSemaphore(const TCHAR* InName, HANDLE InSemaphore)
{
	wcsncpy(Name, InName, MaxSemaphoreName - 1);
	Name[MaxSemaphoreName - 1] = L'\0'; // Ensure null-termination

	m_SemaphoreHandle = InSemaphore;
}

WindowsPlatformProcess::WindowsSemaphore::~WindowsSemaphore()
{
	// actual cleanup should be done in DeleteInterprocessSynchObject() since it can return errors
}
void WindowsPlatformProcess::WindowsSemaphore::Lock()
{
	//check
	DWORD WaitResult = WaitForSingleObject(m_SemaphoreHandle, INFINITE);
	if (WaitResult != WAIT_OBJECT_0)
	{
		DWORD ErrNo = GetLastError();
		//log the errorno
	}
}

bool WindowsPlatformProcess::WindowsSemaphore::TryLock(unsigned long long NanosecondsToWait)
{
	
	DWORD MillisecondsToWait = NanosecondsToWait / 1000000ULL;
	DWORD WaitResult = WaitForSingleObject(m_SemaphoreHandle, MillisecondsToWait);
	if (WaitResult != WAIT_OBJECT_0 && WaitResult != WAIT_TIMEOUT)	// timeout is not a warning
	{
		DWORD ErrNo = GetLastError();
		//log name and errorno
	}

	return WaitResult == WAIT_OBJECT_0;
}

void WindowsPlatformProcess::WindowsSemaphore::Unlock()
{
	//check semaphores
	if (!ReleaseSemaphore(m_SemaphoreHandle, 1, NULL))
	{
		DWORD ErrNo = GetLastError();
		//log error no
	}
}



WindowsPlatformProcess::WindowsSemaphore* WindowsPlatformProcess::NewInterprocessSynchObject(const TCHAR* Name, bool bCreate, unsigned int MaxLocks)
{
	HANDLE Semaphore = NULL;

	if (bCreate)
	{
		Semaphore = CreateSemaphore(NULL, MaxLocks, MaxLocks, Name);
		if (NULL == Semaphore)
		{
			DWORD ErrNo = GetLastError();
			//todo log errorno and name
			return NULL;
		}
	}
	else
	{
		DWORD AccessRights = SYNCHRONIZE | SEMAPHORE_MODIFY_STATE;
		Semaphore = OpenSemaphore(AccessRights, false, Name);
		if (NULL == Semaphore)
		{
			DWORD ErrNo = GetLastError();
			//todo log errorno and name
			return NULL;
		}
	}
	//check semaphore

	return new WindowsSemaphore(Name, Semaphore);
}


//semaphore implementation until here only will continue later if needed coz idt i need it for now

void WindowsPlatformProcess::SetThreadAffinityMask(unsigned int AffinityMask)
{
	if (AffinityMask != WindowsPlatformAffinity::GetNoAffinityMask())
	{
		::SetThreadAffinityMask(::GetCurrentThread(), (DWORD_PTR)AffinityMask);
	}
}

namespace WindowsPlatformProcessImpl
{
	static void SetThreadName(LPCSTR ThreadName)
	{
		/**
		 * Code setting the thread name for use in the debugger.
		 *
		 * http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
		 */
		const unsigned int MS_VC_EXCEPTION = 0x406D1388;

		struct THREADNAME_INFO
		{
			unsigned int dwType;		// Must be 0x1000.
			LPCSTR szName;		// Pointer to name (in user addr space).
			unsigned int dwThreadID;	// Thread ID (-1=caller thread).
			unsigned int dwFlags;		// Reserved for future use, must be zero.
		};

		THREADNAME_INFO ThreadNameInfo;
		ThreadNameInfo.dwType = 0x1000;
		ThreadNameInfo.szName = ThreadName;
		ThreadNameInfo.dwThreadID = ::GetCurrentThreadId();
		ThreadNameInfo.dwFlags = 0;

		__try
		{
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(ThreadNameInfo) / sizeof(ULONG_PTR), (ULONG_PTR*)&ThreadNameInfo);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)

		{
		}
	}

	static void SetThreadDescription(PCWSTR lpThreadDescription)
	{
		// SetThreadDescription is only available from Windows 10 version 1607 / Windows Server 2016
		//
		// So in order to be compatible with older Windows versions we probe for the API at runtime
		// and call it only if available.

		typedef HRESULT(WINAPI* SetThreadDescriptionFnPtr)(HANDLE hThread, PCWSTR lpThreadDescription);

		#pragma warning( push )
		#pragma warning( disable: 4191 )	// unsafe conversion from 'type of expression' to 'type required'
		static SetThreadDescriptionFnPtr RealSetThreadDescription = (SetThreadDescriptionFnPtr)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "SetThreadDescription");
		#pragma warning( pop )

		if (RealSetThreadDescription)
		{
			RealSetThreadDescription(::GetCurrentThread(), lpThreadDescription);
		}
	}
}

std::string ConvertTCHARToLPCSTR(const TCHAR* tcharStr)
{

	// Calculate the size of the buffer needed
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, tcharStr, -1, nullptr, 0, nullptr, nullptr);

	// Allocate the buffer
	std::string str(bufferSize, 0);

	// Perform the conversion
	WideCharToMultiByte(CP_UTF8, 0, tcharStr, -1, &str[0], bufferSize, nullptr, nullptr);

	return str;

}

void WindowsPlatformProcess::SetThreadName(const TCHAR* ThreadName)
{
	// We try to use the SetThreadDescription API where possible since this
	// enables thread names in crashdumps and ETW traces
	WindowsPlatformProcessImpl::SetThreadDescription(TCHAR_TO_WCHAR(ThreadName));

	WindowsPlatformProcessImpl::SetThreadName(ConvertTCHARToLPCSTR(ThreadName).c_str());
	//check here if correct
	
}


void WindowsPlatformProcess::Sleep(float Seconds)
{
	unsigned int Milliseconds = (unsigned int)(Seconds * 1000.0);
	if (Milliseconds == 0)
	{
		::SwitchToThread();
	}
	else
	{
		::Sleep(Milliseconds);
	}
}

#include "Strafe/Core/Utils/Windows/WindowsEvent.h"

GenericEvent* WindowsPlatformProcess::CreateSynchEvent(bool bIsManualReset)
{
	//// While windows does not support forking we can still simulate the forking codeflow and test the singlethread to multithread switch on Win targets
	//const bool bIsMultithread = PlatformProcess::SupportsMultithreading() || FForkProcessHelper::SupportsMultithreadingPostFork();
	const bool bIsMultithread = true;
	// Allocate the new object
	GenericEvent* Event = NULL;
	if (bIsMultithread)
	{
		Event = new EventWin();
	}
	else
	{
		//// Fake event object.
		//Event = new FSingleThreadEvent();
	}
	// If the internal create fails, delete the instance and return NULL
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
		if (!Event->Create(bIsManualReset))
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
		{
			delete Event;
			Event = NULL;
		}
	return Event;
	/*return NULL;*/
}
GenericEvent* WindowsPlatformProcess::GetSynchEventFromPool(bool bIsManualReset)
{
	return bIsManualReset
		? TLazySingleton<TEventPool<EventMode::ManualReset>>::Get().GetEventFromPool()
		: TLazySingleton<TEventPool<EventMode::AutoReset>>::Get().GetEventFromPool();
}

void WindowsPlatformProcess::FlushPoolSyncEvents()
{
	TLazySingleton<TEventPool<EventMode::ManualReset>>::Get().EmptyPool();
	TLazySingleton<TEventPool<EventMode::AutoReset>>::Get().EmptyPool();
}
void WindowsPlatformProcess::ReturnSynchEventToPool(GenericEvent* Event)
{
	if (!Event)
	{
		return;
	}

	if (Event->IsManualReset())
	{
		TLazySingleton<TEventPool<EventMode::ManualReset>>::Get().ReturnToPool(Event);
	}
	else
	{
		TLazySingleton<TEventPool<EventMode::AutoReset>>::Get().ReturnToPool(Event);
	}
}


/////////////////////////windows event

bool EventWin::Wait(uint32 WaitTime, const bool bIgnoreThreadIdleStats /*= false*/)
{
	//WaitForStats();

	
	////check(Event);

	////FThreadIdleStats::ScopeIdle Scope(bIgnoreThreadIdleStats);
	return (WaitForSingleObject(Event, WaitTime) == WAIT_OBJECT_0);
}

void EventWin::Trigger()
{
	//TriggerForStats();
	//check(Event);
	SetEvent(Event);
}


void EventWin::Reset()
{
	//ResetForStats();
	//check(Event);
	ResetEvent(Event);
}



__pragma(warning(pop))