#pragma once

#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include "Strafe/Core/Utils/Windows/WindowsEvent.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformAtomics.h"
#include <vector>

#define FORCEINLINE __forceinline	

namespace NamedThreadsEnum
{
	enum Type : int32
	{
		UnusedAnchor = -1,
		/** The always-present, named threads are listed next **/
		RHIThread,
		GameThread,
		// The render thread is sometimes the game thread and is sometimes the actual rendering thread
		ActualRenderingThread = GameThread + 1,
		// CAUTION ThreadedRenderingThread must be the last named thread, insert new named threads before it

		/** not actually a thread index. Means "Unknown Thread" or "Any Unnamed Thread" **/
		AnyThread = 0xff,

		/** High bits are used for a queue index and priority**/

		MainQueue = 0x000,
		LocalQueue = 0x100,

		NumQueues = 2,
		ThreadIndexMask = 0xff,
		QueueIndexMask = 0x100,
		QueueIndexShift = 8,

		/** High bits are used for a queue index task priority and thread priority**/

		NormalTaskPriority = 0x000,
		HighTaskPriority = 0x200,

		NumTaskPriorities = 2,
		TaskPriorityMask = 0x200,
		TaskPriorityShift = 9,

		NormalThreadPriority = 0x000,
		HighThreadPriority = 0x400,
		BackgroundThreadPriority = 0x800,

		NumThreadPriorities = 3,
		ThreadPriorityMask = 0xC00,
		ThreadPriorityShift = 10,

		/** Combinations **/
		GameThread_Local = GameThread | LocalQueue,
		ActualRenderingThread_Local = ActualRenderingThread | LocalQueue,

		AnyHiPriThreadNormalTask = AnyThread | HighThreadPriority | NormalTaskPriority,
		AnyHiPriThreadHiPriTask = AnyThread | HighThreadPriority | HighTaskPriority,

		AnyNormalThreadNormalTask = AnyThread | NormalThreadPriority | NormalTaskPriority,
		AnyNormalThreadHiPriTask = AnyThread | NormalThreadPriority | HighTaskPriority,

		AnyBackgroundThreadNormalTask = AnyThread | BackgroundThreadPriority | NormalTaskPriority,
		AnyBackgroundHiPriTask = AnyThread | BackgroundThreadPriority | HighTaskPriority,
	};

	struct RenderThreadStatics
	{
	private:
		// These are private to prevent direct access by anything except the friend functions below
		static  std::atomic<Type> RenderThread;
		static std::atomic<Type> RenderThread_Local;

		friend Type GetRenderThread();
		friend Type GetRenderThread_Local();
		friend void SetRenderThread(Type Thread);
		friend void SetRenderThread_Local(Type Thread);
	};

	FORCEINLINE Type GetRenderThread()
	{
		return RenderThreadStatics::RenderThread.load(std::memory_order_relaxed);
	}

	FORCEINLINE Type GetRenderThread_Local()
	{
		return RenderThreadStatics::RenderThread_Local.load(std::memory_order_relaxed);
	}

	FORCEINLINE void SetRenderThread(Type Thread)
	{
		RenderThreadStatics::RenderThread.store(Thread, std::memory_order_relaxed);
	}

	FORCEINLINE void SetRenderThread_Local(Type Thread)
	{
		RenderThreadStatics::RenderThread_Local.store(Thread, std::memory_order_relaxed);
	}

	// these allow external things to make custom decisions based on what sorts of task threads we are running now.
	// this are bools to allow runtime tuning.
	extern  int32 bHasBackgroundThreads;
	extern  int32 bHasHighPriorityThreads;

	FORCEINLINE Type GetThreadIndex(Type ThreadAndIndex)
	{
		return ((ThreadAndIndex & ThreadIndexMask) == AnyThread) ? AnyThread : Type(ThreadAndIndex & ThreadIndexMask);
	}

	FORCEINLINE int32 GetQueueIndex(Type ThreadAndIndex)
	{
		return (ThreadAndIndex & QueueIndexMask) >> QueueIndexShift;
	}

	FORCEINLINE int32 GetTaskPriority(Type ThreadAndIndex)
	{
		return (ThreadAndIndex & TaskPriorityMask) >> TaskPriorityShift;
	}

	FORCEINLINE int32 GetThreadPriorityIndex(Type ThreadAndIndex)
	{
		int32 Result = (ThreadAndIndex & ThreadPriorityMask) >> ThreadPriorityShift;
		//assert(Result >= 0 && Result < NumThreadPriorities);
		return Result;
	}

	FORCEINLINE Type SetPriorities(Type ThreadAndIndex, Type ThreadPriority, Type TaskPriority)
	{
		//assert(
		//	!(ThreadAndIndex & ~ThreadIndexMask) &&  // not a thread index
		//	!(ThreadPriority & ~ThreadPriorityMask) && // not a thread priority
		//	(ThreadPriority & ThreadPriorityMask) != ThreadPriorityMask && // not a valid thread priority
		//	!(TaskPriority & ~TaskPriorityMask) // not a task priority
		//);
		return Type(ThreadAndIndex | ThreadPriority | TaskPriority);
	}

	FORCEINLINE Type SetPriorities(Type ThreadAndIndex, int32 PriorityIndex, bool bHiPri)
	{
		//assert(
		//	!(ThreadAndIndex & ~ThreadIndexMask) && // not a thread index
		//	PriorityIndex >= 0 && PriorityIndex < NumThreadPriorities // not a valid thread priority
		//);
		return Type(ThreadAndIndex | (PriorityIndex << ThreadPriorityShift) | (bHiPri ? HighTaskPriority : NormalTaskPriority));
	}
	FORCEINLINE Type SetThreadPriority(Type ThreadAndIndex, Type ThreadPriority)
	{
		//assert(
		//	!(ThreadAndIndex & ~ThreadIndexMask) &&  // not a thread index
		//	!(ThreadPriority & ~ThreadPriorityMask) && // not a thread priority
		//	(ThreadPriority & ThreadPriorityMask) != ThreadPriorityMask // not a valid thread priority
		//);
		return Type(ThreadAndIndex | ThreadPriority);
	}
	FORCEINLINE Type SetTaskPriority(Type ThreadAndIndex, Type TaskPriority)
	{
		//assert(
		//	!(ThreadAndIndex & ~ThreadIndexMask) &&  // not a thread index
		//	!(TaskPriority & ~TaskPriorityMask) // not a task priority
		//);
		return Type(ThreadAndIndex | TaskPriority);
	}

};

namespace SubsequentsModeEnum
{
	enum Type
	{
		/** Necessary when another task will depend on this task. */
		TrackSubsequents,
		/** Can be used to save task graph overhead when firing off a task that will not be a dependency of other tasks. */
		FireAndForget
	};
}

//convenience typedef for an array of graph events
//typedef std::vector<FGraphEventRef> FGraphEventArray;

//interface to the task graph system
class TaskGraphInterface
{
	friend class BaseGraphTask;


	//internal function to queue a task 
	//either a named thread for a threadlocked task or anythread for a task that is to run on a worker thread
	virtual void QueueTask(class BaseGraphTask* Task, bool WakeUpWorker, NamedThreadsEnum::Type ThreadToExecuteOn, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread) = 0;

public:
	virtual ~TaskGraphInterface() {}	

	//startup , shut down and singleton api
	//@todo : change to dependency injection later on, should not be a big deal

	//explicit start call for the system. The ordinary singleton pattern is not used because internal threads start asking for the singleton before the constructor has returned.
	static void Startup(int32 NumThreads);

	//explicit start call to shutdown the system. this is unlikely to work unless the system is idle
	static void Shutdown();

	//check to see if the system is running
	static bool IsRunning();

	//get the singleton instance
	static TaskGraphInterface& Get();

	//return the current thread type if known
	virtual NamedThreadsEnum::Type GetCurrentThreadIfKnown(bool LocalQueue = false) = 0;

	//return true if the current thread is known
	virtual bool IsCurrentThreadKnown() = 0;

	//Return the number of worker (non named) threads per priority set
	//this is useful to determine how many tasks to split a job into
	virtual int32 GetNumWorkerThreads() = 0;

	//return the number of foreground worker threads. 
	//return the number of high priority worker threads if any
	virtual int32 GetNumForegroundThreads() = 0;

	//retun the number of background worker threads
	virtual int32 GetNumBackgroundThreads() = 0;

	//return true if the given named thread is processing tasks, this is only a "guess" if you ask for a thread other than yourself because that can change before the function changes.
	virtual bool IsThreadProcessingTasks(NamedThreadsEnum::Type ThreadToCheck) = 0;

	//External ThreadAPI

	//one time call that introduces and external thread to the task graph system. it just sets up the tls info
	virtual void AttachToThread(NamedThreadsEnum::Type CurrentThread) = 0;

	//Requests that a named thread, which must be this thread, run until idle, then return
	virtual uint64 ProcessThreadUntilIdle(NamedThreadsEnum::Type CurrentThread) = 0;

	//Requests that a named thread which must be this thread , run until an explicit return is received, then return
	virtual void ProcessThreadUntilRequestReturn(NamedThreadsEnum::Type CurrentThread) = 0;

	//request that the given thread stop when it is idle
	virtual void RequestReturn(NamedThreadsEnum::Type CurrentThread) = 0;



	//Todo
	//requests that a named thread, which must be this thread, run until a list of tasks is complete.
	//virtual void WaitUntilTasksComplete(const GraphEventArray& Tasks, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread) = 0;

	//when a set of tasks complete, fire a scoped event
	//virtual void TriggerEventWhenTasksComplete(GenericEvent* Event,const GraphEventArray& Tasks, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread, NamedThreadsEnum::Type TriggerThread = NamedThreadsEnum::AnyHiPriThreadHiPriTask) = 0;
	
	//requests that a named thread, which must be this thread, run until a task is complete
	//void WaitUntilTaskCompletes(const GraphEventRef& Task, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread)
	//{
	//	WaitUntilTasksComplete({ Task }, CurrentThreadIfKnown);
	//}

	//void WaitUntilTaskCompletes(GraphEventRef&& Task, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread)
	//{
	//	WaitUntilTasksComplete({ MoveTemp(Task) }, CurrentThreadIfKnown);
	//}

	////when a task completes, fire a scoped event
	//void TriggerEventWhenTaskCompletes(GenericEvent* Event, const GraphEventRef& Task, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread, NamedThreadsEnum::Type TriggerThread = NamedThreadsEnum::AnyHiPriThreadHiPriTask)
	//{
	//	GraphEventArray Prerequistes;
	//	Prerequistes.Add(Task);
	//	TriggerEventWhenTasksComplete(InEvent, Prerequistes, CurrentThreadIfKnown, TriggerThread);
	//}

	//virtual BaseGraphTask* FindWork(NamedThreadsEnum::Type ThreadInNeed) = 0;

	//virtual void StallForTuning(int32 Index, bool Stall) = 0;

	////Delegates for shutdown
	//virtual void AddShutdownCallback(std::function<void()> Callback) = 0;

	//virtual void WakeNamedThread(NamedThreadsEnum::Type ThreadToWake) = 0;
	//

};

//we do not need to worry about memory allocation now, only if extra performance boost then i will do a custom memory allocator for linear allocation

//base class for all tasks
//tasks go through a very specific life stage progression
class BaseGraphTask
{
protected:
	//constructor
	BaseGraphTask(int32 pNumberOfPrerequisitesOutstanding)
		: ThreadToExecuteOn(NamedThreadsEnum::AnyThread)
		, NumberOfPrerequisitesOutstanding(pNumberOfPrerequisitesOutstanding + 1) // +1 is not a prerequisite, it isa  lock to prevent it from executing while it is getting prerequisites, once it is safe to execute, call PrerequisitesComplete
	{

	}

	//sets the desired execution thread,. this is not a part of the constructor because this information mnay not be known yet during construction
	void SetThreadToExecuteOn(NamedThreadsEnum::Type pThreadToExecuteOn)
	{
		ThreadToExecuteOn = pThreadToExecuteOn;
	}

	//indicates that the prerequisites are set up and the task can be executed aas soon as the prerequisites are complete
	void PrerequisitesComplete(NamedThreadsEnum::Type CurrentThread, int32 NumAlreadyFinishedPrequistes, bool bUnlock = true)
	{
		int32 NumToSub = NumAlreadyFinishedPrequistes + (bUnlock ? 1 : 0); // the +1 is for the lock we set up in the constructor
		if (WindowsPlatformAtomics::cInterlockedAdd(&NumberOfPrerequisitesOutstanding, -NumToSub) == NumToSub)
		{
			bool WakeUpWorker = true;
			QueueTask(CurrentThread, WakeUpWorker);
		}
	}

	//destructor 
	virtual ~BaseGraphTask() {}

	//an indication that a prerequisite has been complete. reduces the number of prerequisites by one and if no prerequisites are outstanding, it queues the task for execution.
	void ConditionalQueueTask(NamedThreadsEnum::Type CurrentThread, bool& bWakeUpWorker)
	{
		if (WindowsPlatformAtomics::cInterlockedDecrement(&NumberOfPrerequisitesOutstanding) == 0)
		{
			QueueTask(CurrentThread, bWakeUpWorker);
			bWakeUpWorker = true;
		}
	}

	NamedThreadsEnum::Type GetThreadToExecuteOn() const
	{
		return ThreadToExecuteOn;
	}

private:
	/*friend class FNamedTaskThread;
	friend class FTaskThreadBase;
	friend class FTaskThreadAnyThread;
	friend class FGraphEvent;
	friend class FTaskGraphImplementation;*/

	//subclass api
	//virtual call to actually execute the task. this will also call the destructor and free any memory if deleteoncompletion is set to true
	virtual void ExecuteTask(std::vector<BaseGraphTask*>& NewTasks, NamedThreadsEnum::Type CurrentThread, bool bDeleteOnCompletion) = 0;

	/**
	*	Virtual call to actually delete the task any memory.
	**/
	virtual void DeleteTask() = 0;

	// API called from other parts of the system

	//called by the system to execute this task after it has been removed from an internal queue
	//just pass off tot he virtual execute task method
	FORCEINLINE void Execute(std::vector<BaseGraphTask*>& NewTasks, NamedThreadsEnum::Type CurrentThread, bool bDeleteOnCompletion)
	{
			ExecuteTask(NewTasks, CurrentThread, bDeleteOnCompletion);
	}

	//internal use
	
	//queues the task for execution
	void QueueTask(NamedThreadsEnum::Type CurrentThread, bool WakeUpWorker)
	{
		TaskGraphInterface::Get().QueueTask(this, WakeUpWorker, ThreadToExecuteOn, CurrentThread);
	}


	/**	Thread to execute on, can be ENamedThreads::AnyThread to execute on any unnamed thread **/
	NamedThreadsEnum::Type			ThreadToExecuteOn;
	/**	Number of prerequisites outstanding. When this drops to zero, the thread is queued for execution.  **/
	volatile int32		NumberOfPrerequisitesOutstanding;

};