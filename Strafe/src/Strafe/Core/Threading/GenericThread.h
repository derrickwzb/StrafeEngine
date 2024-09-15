#pragma once

#include "Strafe/Core/Threading/Platform/WindowsPlatformAffinity.h"
#include "Strafe/Core/Utils/Windows/WindowsEvent.h"
#include "Strafe/Core/Threading/TlsAutoCleanup.h"
#include "Strafe/Core/Threading/ThreadSingletonInitializer.h"
#include "Strafe/Core/Threading/ThreadManager.h"

class Runnable;

class GenericThread
{
	//no forkable threads because we are not doing any process forking
	//we aint a server
	friend class ThreadSingletonInitializer;
	friend class TlsAutoCleanup;
	friend class ThreadManager;
	

	//index of tls slot for thread pointer
	static unsigned int m_TlsSlot;

public:
	//get a new tls slot for storing the thread pointer
	static unsigned int GetTlsSlot();
	
	//factory method to create a thread with specific stack size and thread priority
	static GenericThread* Create(class Runnable* InRunnable,
		const TCHAR* ThreadName,
		unsigned int InStackSize = 0,
		ThreadPriority InThreadPri = ThreadPri_Normal,
		unsigned long long InThreadAffinityMask = WindowsPlatformAffinity::GetNoAffinityMask(),
		ThreadCreateFlags InCreateFlags = ThreadCreateFlags::None);


	//changes the thread priority of the current running thread
	virtual void SetThreadPriority(ThreadPriority priority) = 0;

	//changes the thread affinity of the current running thread
	virtual bool SetThreadAffinityMask(const ThreadAffinity& Affinity) { return false;  };

	//tell the thread to pause execution or resume depending on the value
	virtual void Suspend(bool ShouldPause = true) = 0;

	//tells the thread to exit. if the caller needs to know when the thread has exited, it should use the shouldwait value. 
	//it is highly recommended not to kill the thread without waiting for it to exit. Having a thread forcibly killed can cause leaks and deadlocks
	virtual bool Kill(bool ShouldWait = true) = 0;

	//block the caller until this thread has completed its work.
	virtual void WaitForCompletion() = 0;

	enum class ThreadType
	{
		Real,
		Fake,
		Forkable
	};

	//Return the type of thread this is
	virtual GenericThread::ThreadType GetThreadType()const 
	{
			return ThreadType::Real;
	};

	//thread id for this thread
	const unsigned int GetThreadId() const { return m_ThreadId; }

	//get the name of the thread
	const std::string & getThreadName() const { return m_ThreadName; }

	//return the thread priority
	ThreadPriority GetThreadPriority() const { return m_ThreadPriority; }

	//default ctor
	GenericThread();

	virtual ~GenericThread();

	/**
	 * @return a runnable thread that is executing this runnable, if return value is nullptr, it means the running thread can be game thread or a thread created outside the runnable interface
	 */
	static GenericThread* GetGenericThread()
	{
		GenericThread* RunnableThread = (GenericThread*)WindowsPlatformTLS::GetTlsValue(m_TlsSlot);
		return RunnableThread;
	}

protected:

	//create the thread with the specified stack size and thread priority
	virtual bool CreateInternal(class Runnable* InRunnable,
		const TCHAR* ThreadName,
		unsigned int InStackSize = 0,
		ThreadPriority InThreadPri = ThreadPri_Normal,
		unsigned long long InThreadAffinityMask = 0 ,
		ThreadCreateFlags InCreateFlags = ThreadCreateFlags::None) = 0;

	//store this instance in the generic thread tls slot
	void SetTls();

	//delete all tls auto cleanup objects created for this thread
	void ClearTls();


	//thread name
	std::string m_ThreadName;
	//runnable object to execute on this thread
	Runnable* m_Runnable;

	GenericEvent* ThreadInitSyncEvent;

	// the affinity to run the thread with
	unsigned long long m_ThreadAffinityMask;
	// the priority to run the thread with
	ThreadPriority m_ThreadPriority;
	//id set during thread creation
	unsigned int m_ThreadId;

private:
	
	//called to setup a newly created GenericThread
	static void SetupThread(GenericThread*& thread, class Runnable* InRunnable, const TCHAR* InThreadName, uint32 InStackSize, ThreadPriority InThreadPri, uint64 InThreadAffinityMask, ThreadCreateFlags InCreateFlags);

	//used by the thread manager to tick threads in single threaded mode
	virtual void Tick() {};

	//called on the forked process when the forkable thread can create a real thread
	/*virtual void OnPostFork()
	{
		checkf(false, TEXT("Only forkable threads should receive OnPostFork."));
	}*/

	//called after internal thread is created so it can register debug info
	void PostCreate(ThreadPriority threadpri);
};