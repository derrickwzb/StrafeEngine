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

//todo still have alot of stuff
