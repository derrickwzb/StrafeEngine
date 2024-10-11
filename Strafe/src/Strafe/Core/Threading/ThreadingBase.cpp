#include "strafepch.h"
#include "ThreadingBase.h"
#include "Strafe/Core/Utils/Windows/WindowsEvent.h"
#include "Strafe/Core/Utils/LazySingleton.h"
#include"Strafe/Core/Utils/Windows/WindowsEventPool.h"
#include "Strafe/Core/Threading/Windows/WindowsPlatformTLS.h"
#include "Strafe/Core/Threading/ThreadSingletonInitializer.h"
#include "Strafe/Core/Threading/TlsAutoCleanup.h"
#include "Strafe/Core/Utils/ScopedEvent.h"



/*-----------------------------------------------------------------------------
	GenericThread
-----------------------------------------------------------------------------*/

unsigned int GenericThread::m_TlsSlot = GenericThread::GetTlsSlot();

unsigned int GenericThread::GetTlsSlot()
{
	//implement 

	//check if is in game thread
	unsigned int TlsSlot = WindowsPlatformTLS::AllocTlsSlot();
	if (TlsSlot)
		return TlsSlot;
}

GenericThread::GenericThread()
	:m_Runnable(nullptr)
	,/*threadsyncinitevent nullptr */ m_ThreadAffinityMask(WindowsPlatformAffinity::GetNoAffinityMask())
	, m_ThreadPriority(ThreadPriority::ThreadPri_Normal)
	,m_ThreadId(0)

{
}

GenericThread::~GenericThread()
{
	//kill the thread
	//if engine exit is requested, use threadmanager to get and remove this thread.
}

GenericThread* GenericThread::Create(
Runnable* InRunnable
, const TCHAR* ThreadName
, unsigned int InStackSize
, ThreadPriority InThreadPri
, unsigned long long InThreadAffinityMask
, ThreadCreateFlags InCreateFlags)
{
	GenericThread* NewThread = nullptr;

	NewThread = new WindowsGenericThread();

	if (NewThread)
	{
		//setupcreated thread
		SetupThread(NewThread, InRunnable, ThreadName, InStackSize, InThreadPri, InThreadAffinityMask, InCreateFlags);
		
	}

	return NewThread;
}

void GenericThread::SetupThread(GenericThread*& NewThread, class Runnable* InRunnable, const TCHAR* ThreadName, uint32 InStackSize, ThreadPriority InThreadPri, uint64 InThreadAffinityMask, ThreadCreateFlags InCreateFlags)
{
	// Call the thread's create method
	bool bIsValid = NewThread->CreateInternal(InRunnable, ThreadName, InStackSize, InThreadPri, InThreadAffinityMask, InCreateFlags);

	if (bIsValid)
	{
		NewThread->PostCreate(InThreadPri);
	}
	else
	{
		delete NewThread;
		NewThread = nullptr;
	}
}

void GenericThread::PostCreate(ThreadPriority InThreadPri)
{
	//log that it is created
}

void GenericThread::SetTls()
{
	// Make sure it's called from the owning thread.
	/*check(ThreadID == FPlatformTLS::GetCurrentThreadId());
	check(FPlatformTLS::IsValidTlsSlot(RunnableTlsSlot));*/
	//set the tls slot for this thread
	WindowsPlatformTLS::SetTlsValue(m_TlsSlot, this);

	//todo set task tag might be useful 
}

void GenericThread::ClearTls()
{
	// Make sure it's called from the owning thread.
	/*check(ThreadID == FPlatformTLS::GetCurrentThreadId());
	check(FPlatformTLS::IsValidTlsSlot(RunnableTlsSlot));*/
	WindowsPlatformTLS::SetTlsValue(m_TlsSlot, nullptr);
}

//just assume this works
//apparently to count the array size in compile time
template <typename T, unsigned int N>
char(&ArrayCountHelper(const T(&)[N]))[N + 1];

/*-----------------------------------------------------------------------------
	ThreadManager
-----------------------------------------------------------------------------*/


bool ThreadManager::IsThreadListSafeToContinue()
{
	if (m_IsThreadListDirty)
	{
		
		//log this bitch/*"ThreadManager::Threads was modified during unsafe iteration.Iteration will be aborted."*/
		return false;
	}

	return true;
}

void ThreadManager::OnThreadListModification()
{
	m_IsThreadListDirty = true;
}
void ThreadManager::AddThread(unsigned int threadid, GenericThread* thread)
{
	//add thread to the map
	// Convert the thread's priority into an ordered value that is suitable
	// for sorting. Note we're using higher values so as to not collide with
	// existing trace data that's using TPri directly, and leaving gaps so
	// values can be added in between should need be
	//int8
	signed char PriorityRemap[][2] = {
		{ ThreadPri_TimeCritical,		0x10 },
		{ ThreadPri_Highest,				0x20 },
		{ ThreadPri_AboveNormal,			0x30 },
		{ ThreadPri_Normal,				0x40 },
		{ ThreadPri_SlightlyBelowNormal,	0x50 },
		{ ThreadPri_BelowNormal,			0x60 },
		{ ThreadPri_Lowest,				0x70 },
	};

	//update priority remap array when adding or removing thread priorites
	//count array size in compile time
	unsigned int SortHint = sizeof(ArrayCountHelper(PriorityRemap)) - 1;
	for (auto Candidate : PriorityRemap)
	{
		if (Candidate[0] == thread->GetThreadPriority())
		{
			SortHint = Candidate[1];
			break;
		}
	}

	//fake thread stuff here it might be needed :: todo implement fake threads for single threaded ;/ we never know what will happen


	ScopeLock ThreadLock(&m_ThreadListCritical);

	if (!m_Threads.contains(threadid))
	{
		m_Threads.emplace(threadid,thread);
	}
}

const unsigned int* FindKeyThreadMap(const std::map<unsigned int, GenericThread*>& map,GenericThread* thread)
{
	const unsigned int* ThreadId = nullptr;
	for (auto pair : map)
	{
		if (pair.second == thread)
		{
			ThreadId = &pair.first;
			return ThreadId;
		}
	}
	return nullptr;
}

void ThreadManager::RemoveThread(GenericThread* Thread)
{
	ScopeLock ThreadsLock(&m_ThreadListCritical);
	const unsigned int* ThreadId = FindKeyThreadMap(m_Threads,Thread);
	if (ThreadId)
	{
		m_Threads.erase(*ThreadId);
		OnThreadListModification();
	}
}

//void ThreadManager::Tick()
//{
//	// there is no need to tick manually right now as we are multithreading by default
//}


const std::string& ThreadManager::GetThreadNameInternal(unsigned int ThreadId)
{
	static std::string NoThreadName;
	ScopeLock ThreadsLock(&m_ThreadListCritical);
	GenericThread** Thread = &(m_Threads[ThreadId]);
	if (Thread)
	{
		return (*Thread)->getThreadName();
	}
	return NoThreadName;
}

void ThreadManager::ForEachThread(const std::function<void(unsigned int threadid, GenericThread* pthread)>& func)
{
	ScopeLock Lock(&m_ThreadListCritical);
	// threads can be added or removed while iterating over them, thus invalidating the iterator, so we iterate over the copy of threads collection
	Threads ThreadsCopy = m_Threads;

	for (const std::pair<unsigned int, GenericThread*>& Pair : ThreadsCopy)
	{
		func(Pair.first, Pair.second);
	}
}

/*-----------------------------------------------------------------------------
	ScopedEvent
-----------------------------------------------------------------------------*/

ScopedEvent::ScopedEvent()
	: Event(TLazySingleton<TEventPool<EventMode::AutoReset>>::Get().GetRawEvent())
{ }

bool ScopedEvent::IsReady()
{
	if (Event && Event->Wait(1))
	{
		TLazySingleton<TEventPool<EventMode::AutoReset>>::Get().ReturnRawEvent(Event);
		Event = nullptr;
		return true;
	}
	return Event == nullptr;
}

ScopedEvent::~ScopedEvent()
{
	if (Event)
	{
		Event->Wait();
		TLazySingleton<TEventPool<EventMode::AutoReset>>::Get().ReturnRawEvent(Event);
	}
}

/*-----------------------------------------------------------------------------
	EventRef, SharedEventRef
-----------------------------------------------------------------------------*/

EventRef::EventRef(EventMode Mode)
{
	if (Mode == EventMode::AutoReset)
	{
		Event = TLazySingleton<TEventPool<EventMode::AutoReset>>::Get().GetRawEvent();
	}
	else
	{
		Event = TLazySingleton<TEventPool<EventMode::ManualReset>>::Get().GetRawEvent();
	}
}


EventRef::~EventRef()
{
	if (Event->IsManualReset())
	{
		TLazySingleton<TEventPool<EventMode::ManualReset>>::Get().ReturnRawEvent(Event);
	}
	else
	{
		TLazySingleton<TEventPool<EventMode::AutoReset>>::Get().ReturnRawEvent(Event);
	}
}

SharedEventRef::SharedEventRef(EventMode Mode  /* = EEventMode::AutoReset */)
	: Ptr(TLazySingleton<TEventPool<EventMode::AutoReset>>::Get().GetRawEvent(),
		[](GenericEvent* Event) { TLazySingleton<TEventPool<EventMode::AutoReset>>::Get().ReturnRawEvent(Event); })
{
}

/*-----------------------------------------------------------------------------
	FThreadSingletonInitializer
-----------------------------------------------------------------------------*/
#define AUTORTFM_OPEN(...) do { __VA_ARGS__ } while (false)
int32 CustomInterlockedCompareExchange(volatile int32* Dest, int32 Exchange, int32 Comparand)
{
	return (int32)::_InterlockedCompareExchange((long*)Dest, (long)Exchange, (long)Comparand);
}

TlsAutoCleanup* ThreadSingletonInitializer::Get(TlsAutoCleanup* (*CreateInstance)(), uint32& TlsSlot)
{
	uint32 tlsslot;
	AUTORTFM_OPEN({

		tlsslot = (uint32)((volatile const int32*)&TlsSlot);
		if (tlsslot == WindowsPlatformTLS::InvalidTlsSlot)
		{
			const uint32 ThisTlsSlot = WindowsPlatformTLS::AllocTlsSlot();
			//check(FPlatformTLS::IsValidTlsSlot(ThisTlsSlot));
			const uint32 PrevTlsSlot = (int32)CustomInterlockedCompareExchange((int32*)&TlsSlot, (int32)ThisTlsSlot, WindowsPlatformTLS::InvalidTlsSlot);
			if (PrevTlsSlot != WindowsPlatformTLS::InvalidTlsSlot)
			{
				WindowsPlatformTLS::FreeTlsSlot(ThisTlsSlot);
				TlsSlot = PrevTlsSlot;
			}
			else
			{
				TlsSlot = ThisTlsSlot;
			}
		}

	});
	TlsAutoCleanup* ThreadSingleton = nullptr;
	AUTORTFM_OPEN(
		{
			ThreadSingleton = (TlsAutoCleanup*)WindowsPlatformTLS::GetTlsValue(TlsSlot);
			if (!ThreadSingleton)
			{
				// these are generally left open and only get cleaned up on thread exit so avoiding dealing with an OPENABORT here to clean this up
				ThreadSingleton = CreateInstance();
				ThreadSingleton->Register();
				WindowsPlatformTLS::SetTlsValue(TlsSlot, ThreadSingleton);
			}
		});
	return ThreadSingleton;
}

TlsAutoCleanup* ThreadSingletonInitializer::TryGet(uint32& TlsSlot)
{
	if (TlsSlot == WindowsPlatformTLS::InvalidTlsSlot)
	{
		return nullptr;
	}

	TlsAutoCleanup* ThreadSingleton = (TlsAutoCleanup*)WindowsPlatformTLS::GetTlsValue(TlsSlot);
	return ThreadSingleton;
}

TlsAutoCleanup* ThreadSingletonInitializer::Inject(TlsAutoCleanup* Instance, uint32& TlsSlot)
{
	if (TlsSlot == WindowsPlatformTLS::InvalidTlsSlot)
	{
		const uint32 ThisTlsSlot = WindowsPlatformTLS::AllocTlsSlot();
		//check(FPlatformTLS::IsValidTlsSlot(ThisTlsSlot));
		const uint32 PrevTlsSlot = CustomInterlockedCompareExchange((int32*)&TlsSlot, (int32)ThisTlsSlot, WindowsPlatformTLS::InvalidTlsSlot);
		if (PrevTlsSlot != WindowsPlatformTLS::InvalidTlsSlot)
		{
			WindowsPlatformTLS::FreeTlsSlot(ThisTlsSlot);
		}
	}

	TlsAutoCleanup* ThreadSingleton = (TlsAutoCleanup*)WindowsPlatformTLS::GetTlsValue(TlsSlot);
	WindowsPlatformTLS::SetTlsValue(TlsSlot, Instance);
	return ThreadSingleton;
}

/*-----------------------------------------------------------------------------
	TlsAutoCleanup
-----------------------------------------------------------------------------*/
void TlsAutoCleanup::Register()
{
	static thread_local std::vector<std::unique_ptr<TlsAutoCleanup>> TlsInstances;
	TlsInstances.push_back(std::unique_ptr<TlsAutoCleanup>(this));
}

