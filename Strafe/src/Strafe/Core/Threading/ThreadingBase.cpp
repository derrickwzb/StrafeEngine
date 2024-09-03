#include "strafepch.h"
#include "ThreadingBase.h"



/*-----------------------------------------------------------------------------
	GenericThread
-----------------------------------------------------------------------------*/

unsigned int GenericThread::m_TlsSlot = GenericThread::GetTlsSlot();

unsigned int GenericThread::GetTlsSlot()
{
	//implement 
	return 0;
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


//todo still have alot of stuff
