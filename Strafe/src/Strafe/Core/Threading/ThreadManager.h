#pragma once

#include "Strafe/Core/Utils/Windows/WindowsCriticalSection.h"
#include <map>
#include <functional>
#include "Strafe/Core/CoreGlobals.h"

class GenericThread;
// still needs implementation
//manages threads and runnable objects
class ThreadManager
{
	//critical section for thread list
	CriticalSection m_ThreadListCritical;

	using Threads = std::map<unsigned int, GenericThread*>;

	//list of thread objects to be ticked
	Threads m_Threads;

	//helper variable for catching unexpected modification on the thread map/list
	bool m_IsThreadListDirty = false;

	bool IsThreadListSafeToContinue();
	void OnThreadListModification();

public:
	//add new thread object to the list
	void AddThread(unsigned int ThreadId , GenericThread* InThread);

	//remove thread object from the list
	void RemoveThread(GenericThread* Thread);

	//get the number of registered threadas
	unsigned int GetThreadCount() const { return m_Threads.size(); }

	//if fake thread is implemented, implement this
	//void Tick();

	//return the name of a thread given its id
	inline static const TCHAR* GetThreadName(unsigned int ThreadId)
	{
		static const TCHAR* GameThreadName = TEXT("GameThread");
		static const TCHAR* RenderThreadName = TEXT("RenderThread");

		if (ThreadId == GameThreadID)
		{
			return GameThreadName;
		}
		else if (ThreadId == RenderThreadID)
		{
			return RenderThreadName;
		}
		return Get().GetThreadNameInternal(ThreadId);
	}

	//might want to implement thread stack backtraces 

	//iterate each thread
	void ForEachThread(const std::function<void(unsigned int threadid , GenericThread* pthread)>& func);

	//access to the singleton thread manager object
	static ThreadManager& Get()
	{
		static ThreadManager Instance;
		return Instance;
	}

private:
	
	//might want to implmement this 
	//fork process helper is a helper class for processes that fork in order to share memory pages
	// When a process gets forked, any existing threads will not exist on the new forked process.
	// To solve this we use forkable threads that are notified when the fork occurs and will automatically convert themselves into real runnable threads.
	//friend class forkprocesshelper
	// will need to implement forkable threads too


	const TCHAR* GetThreadNameInternal(unsigned int ThreadId);

};