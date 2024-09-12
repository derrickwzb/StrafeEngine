#include "strafepch.h"
#include "ThreadingBase.h"
#include "Strafe/Core/Utils/Windows/WindowsEvent.h"
#include "Strafe/Core/Utils/LazySingleton.h"
#include"Strafe/Core/Utils/Windows/WindowsEventPool.h"
#include "Strafe/Core/Threading/Windows/WindowsPlatformTLS.h"



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
, const std::string* ThreadName
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
	}

	return NewThread;
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

//todo still have alot of stuff

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
