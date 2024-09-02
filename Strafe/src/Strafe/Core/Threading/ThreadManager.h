#pragma once

#include "Strafe/Core/Utils/Windows/WindowsCriticalSection.h"
#include <map>
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

	
};