#pragma once

#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include "Strafe/Core/Utils/Windows/WindowsEvent.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformAtomics.h"
#include "Strafe/Core/TaskGraph/TaskGraphFwd.h"
#include "Strafe/Core/Threading/Containers/LockFreeFixedSizeAllocator.h"
#include <vector>
#include <functional>
#include <utility>  // For std::forward
#include <new>      // For placement new

#define FORCEINLINE __forceinline	

template<int32 Size, uint32 Alignment>
struct TAlignedBytes
{
	alignas(Alignment) uint8 Pad[Size];
};

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
typedef std::vector<GraphEventRef> GraphEventArray;

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
	virtual void WaitUntilTasksComplete(const GraphEventArray& Tasks, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread) = 0;

	//when a set of tasks complete, fire a scoped event
	virtual void TriggerEventWhenTasksComplete(GenericEvent* Event,const GraphEventArray& Tasks, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread, NamedThreadsEnum::Type TriggerThread = NamedThreadsEnum::AnyHiPriThreadHiPriTask) = 0;
	
	//requests that a named thread, which must be this thread, run until a task is complete
	void WaitUntilTaskCompletes(const GraphEventRef& Task, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread)
	{
		WaitUntilTasksComplete({ Task }, CurrentThreadIfKnown);
	}

	void WaitUntilTaskCompletes(GraphEventRef&& Task, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread)
	{
		WaitUntilTasksComplete({ std::move(Task) }, CurrentThreadIfKnown);
	}

	////when a task completes, fire a scoped event
	void TriggerEventWhenTaskCompletes(GenericEvent* Event, const GraphEventRef& Task, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread, NamedThreadsEnum::Type TriggerThread = NamedThreadsEnum::AnyHiPriThreadHiPriTask)
	{
		GraphEventArray Prerequistes;
		Prerequistes.emplace_back(Task);
		TriggerEventWhenTasksComplete(Event, Prerequistes, CurrentThreadIfKnown, TriggerThread);
	}

	virtual BaseGraphTask* FindWork(NamedThreadsEnum::Type ThreadInNeed) = 0;

	virtual void StallForTuning(int32 Index, bool Stall) = 0;

	//Delegates for shutdown
	virtual void AddShutdownCallback(std::function<void()> Callback) = 0;

	virtual void WakeNamedThread(NamedThreadsEnum::Type ThreadToWake) = 0;
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
		if (NumberOfPrerequisitesOutstanding.fetch_sub(NumToSub) == NumToSub) // check results @TODO
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
		if (NumberOfPrerequisitesOutstanding.fetch_sub(1) == 0)
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
	//friend class FNamedTaskThread;
	//friend class FTaskThreadBase;
	//friend class FTaskThreadAnyThread;
	friend class GraphEvent;
	//friend class FTaskGraphImplementation;

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
	//i think this is a better approach for cross platform atomic operations but for now i dont give a fuck let it be messy
	std::atomic<signed int>		NumberOfPrerequisitesOutstanding;
};

//a graphevent is a list of tasks wauting for something
//these tasks are called the subsequents
//a graph event is a prerequisite for each of its subsequents
//graph events have a lifetime managed by ref counting
class GraphEvent
{
public:
	//a factory method to create a graph event
	static GraphEventRef CreateGraphEvent();

	//Attempts to a new subsequent task. If this event has already fired, false is returned and action must be taken to ensure that the task will still fire even though this event cannot be a prerequisite (because it is already finished).
	bool AddSubsequent(class BaseGraphTask* Subsequent)
	{
		bool bSucceeded = SubsequentList.PushIfNotClosed(Subsequent);
		if (bSucceeded)
		{
			//log that it is added
		}
		return bSucceeded;
	}

	//verification function to ensure that nobody was tried to add wait until;s outside of the context of executrion
	void CheckDontCompleteUntilIsEmpty()
	{
		//break if !EventsToWaitFor.Num());
	}

	//delay the firing of this event until the given event fires.
	//caution:this is only legal while executing the task associated with this event.
	void DontCompleteUntil(GraphEventRef EventToWaitFor)
	{
		//break if !IsComplete()); // it is not legal to add a DontCompleteUntil after the event has been completed. Basically, this is only legal within a task function.
		EventsToWaitFor.emplace_back(EventToWaitFor);
		//log that event has been added
	}

	//"complete" the event. This grabs the list of subsequents and atomically closes it. then for each subsequent it reduces the number of prerequisites outstanding and if that 
	//drops to zero, the task is queued.
	void DispatchSubsequents(NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread);

	//"complete" the event. This grabs the list of subsequents and atomically closes it. then for each subsequent it reduces the number of prerequisites outstanding and if that 
	//drops to zero, the task is queued.

	void DispatchSubsequents(std::vector<BaseGraphTask*>& NewTasks, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread, bool bInternal = false);

	//determine if the event has been completed. this can be used to poll for completion
	//caution if this returns false, thevent could still end up completeing before this function even returns.
	//in other words, a false return does not mean that event is not yet completed
	bool IsComplete() const
	{
		return SubsequentList.IsClosed();
	}

	//a convenient short version of wait until task completes
	void Wait(NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread)
	{
		TaskGraphInterface::Get().WaitUntilTaskCompletes(*this, CurrentThreadIfKnown);
	}

private:
	friend class RefCountPtr<GraphEvent>;
	friend class LockFreeClassAllocator_TLSCache<GraphEvent, PLATFORM_CACHE_LINE_SIZE>;

	//internal function to call the destructor and recycle a graph event
	static void Recycle(GraphEvent* Event);
	

	//hidden constructor
	GraphEvent()
		: ThreadToDoGatherOn(NamedThreadsEnum::AnyHiPriThreadHiPriTask)
	{
	}

	//destructor. verifies wer arent destroyign it prematurwl
	~GraphEvent();

public:

	//increases the ref count
	uint32 AddRef()
	{
		int32 refcount = ReferenceCount.fetch_add(1);
		//check if ref count > 0
		return refcount;
	}

	uint32 Release()
	{
		int32 refcount = ReferenceCount.fetch_sub(1);
		//check if ref count >= 0
		if (refcount == 1)
		{
			//recycle the event
			Recycle(this);
		}
		return refcount;
	}

	uint32 GetRefCount() const
	{
		return ReferenceCount.load();
	}

private:

	/** Threadsafe list of subsequents for the event **/
	ClosableLockFreePointerListUnorderedSingleConsumer<BaseGraphTask, 0>	SubsequentList;
	/** List of events to wait for until firing. This is not thread safe as it is only legal to fill it in within the context of an executing task. **/
	GraphEventArray														EventsToWaitFor;
	/** Number of outstanding references to this graph event **/
	std::atomic<int32>														ReferenceCount;
	NamedThreadsEnum::Type														ThreadToDoGatherOn;

};




//tgraphtask
//embeds a user defined task, as exemplified above, for doing the work and provides the functionality for setting up and handling prerequisites and subsequents
template<typename TTask>
class TGraphTask final : public BaseGraphTask
{
public:
	//this is a helper class returned from the factory. it constructs the embedded task with a set of arguments and sets the task up and makes it ready to execute.
	//the task may complete before these routines even return.
	class FConstructor
	{
	public:
		/** Passthrough internal task constructor and dispatch. Note! Generally speaking references will not pass through; use pointers */
		template<typename...T>
		GraphEventRef ConstructAndDispatchWhenReady(T&&... Args)
		{
			new ((void*)&Owner->TaskStorage) TTask(std::forward<T>(Args)...);
			return Owner->Setup(Prerequisites, CurrentThreadIfKnown);
		}

		/** Passthrough internal task constructor and hold. */
		template<typename...T>
		TGraphTask* ConstructAndHold(T&&... Args)
		{
			new ((void*)&Owner->TaskStorage) TTask(std::forward<T>(Args)...);
			return Owner->Hold(Prerequisites, CurrentThreadIfKnown);
		}

	private:
		friend TGraphTask;

		/** The task that created me to assist with embeded task construction and preparation. **/
		TGraphTask* Owner;
		/** The list of prerequisites. **/
		const GraphEventArray* Prerequisites;
		/** If known, the current thread.  ENamedThreads::AnyThread is also fine, and if that is the value, we will determine the current thread, as needed, via TLS. **/
		NamedThreadsEnum::Type				CurrentThreadIfKnown;

		/** Constructor, simply saves off the arguments for later use after we actually construct the embeded task. **/
		FConstructor(TGraphTask* InOwner, const GraphEventArray* InPrerequisites, NamedThreadsEnum::Type InCurrentThreadIfKnown)
			: Owner(InOwner)
			, Prerequisites(InPrerequisites)
			, CurrentThreadIfKnown(InCurrentThreadIfKnown)
		{
		}
		/** Prohibited copy construction **/
		FConstructor(const FConstructor& Other)
		{
			check(0);
		}
		/** Prohibited copy **/
		void operator=(const FConstructor& Other)
		{
			check(0);
		}
	};

	//factory to create a task and returnt he helper object to construct the embedded task and set it up for execution
	static FConstructor CreateTask(const GraphEventArray* Prerequisites = NULL, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread)
	{
		GraphEventRef GraphEvent = TTask::GetSubsequentsMode() == SubsequentsModeEnum::FireAndForget ? NULL : GraphEvent::CreateGraphEvent();

		int32 NumPrereq = Prerequisites ? Prerequisites->Num() : 0;
		return FConstructor(new TGraphTask(std::move(GraphEvent), NumPrereq), Prerequisites, CurrentThreadIfKnown);
	}

	void Unlock(NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread)
	{
		/*TaskTrace::Launched(GetTraceId(), nullptr, Subsequents.IsValid(), ((TTask*)&TaskStorage)->GetDesiredThread(), sizeof(*this));*/

		bool bWakeUpWorker = true;
		ConditionalQueueTask(CurrentThreadIfKnown, bWakeUpWorker);
	}

	GraphEventRef GetCompletionEvent()
	{
		return Subsequents;
	}
private:
	friend class FConstructor;
	friend class GraphEvent;

	//api derived from basegraphtask 
	//virtual call to actually execute the task.
	//executes, destroy the embedded task. dispatch the subsequents, destroy myself
	void ExecuteTask(TArray<BaseGraphTask*>& NewTasks, NamedThreadsEnum::Type CurrentThread, bool bDeleteOnCompletion) override
	{

		// Fire and forget mode must not have subsequents
		// Track subsequents mode must have subsequents

		if (TTask::GetSubsequentsMode() == SubsequentsModeEnum::TrackSubsequents)
		{
			Subsequents->CheckDontCompleteUntilIsEmpty(); // we can only add wait for tasks while executing the task
		}

		TTask& Task = *(TTask*)&TaskStorage;
		{
			//TaskTrace::FTaskTimingEventScope TaskEventScope(GetTraceId());
			//FScopeCycleCounter Scope(Task.GetStatId(), true);
			Task.DoTask(CurrentThread, Subsequents);
			Task.~TTask();
			//checkThreadGraph(ENamedThreads::GetThreadIndex(CurrentThread) <= ENamedThreads::GetRenderThread() || FMemStack::Get().IsEmpty()); // you must mark and pop memstacks if you use them in tasks! Named threads are excepted.
		}

		TaskConstructed = false;

		if (TTask::GetSubsequentsMode() == SubsequentsModeEnum::TrackSubsequents)
		{
			_mm_sfence();
			Subsequents->DispatchSubsequents(NewTasks, CurrentThread, true);
		}
		else
		{
			// "fire and forget" tasks don't have an accompanying FGraphEvent that traces completion and destruction

		}

		if (bDeleteOnCompletion)
		{
			DeleteTask();
		}
	}

	void DeleteTask() final override
	{
		delete this;
	}

	//internals

	//private constructor , constructs the base class with the number of preq
	TGraphTask(GraphEventRef InSubsequents, int32 NumberOfPrerequistitesOutstanding)
		: BaseGraphTask(NumberOfPrerequistitesOutstanding)
		, TaskConstructed(false)
	{
		Subsequents.Swap(InSubsequents);
		//SetTraceId(Subsequents.IsValid() ? Subsequents->GetTraceId() : TaskTrace::GenerateTaskId());
	}
	//private destructor, just checks that the task appears to be completed
	~TGraphTask() override
	{
		//check if !TaskConstructed);
	}

	//call from FConstructor to complete the setup process
	//create the completed event
	//set the thread to execute on based on the embedded task
	//attemp to add myself as a subsequent to each prerequisite
	//tell the bease task that i am ready to start as soon as my prerequisites are ready.
	void SetupPrereqs(const GraphEventArray* Prerequisites, NamedThreadsEnum::Type CurrentThreadIfKnown, bool bUnlock)
	{
		//check if !TaskConstructed);
		TaskConstructed = true;
		TTask& Task = *(TTask*)&TaskStorage;
		SetThreadToExecuteOn(Task.GetDesiredThread());
		int32 AlreadyCompletedPrerequisites = 0;
		if (Prerequisites)
		{
			for (int32 Index = 0; Index < Prerequisites->Num(); Index++)
			{
				GraphEvent* Prerequisite = (*Prerequisites)[Index];
				if (Prerequisite == nullptr || !Prerequisite->AddSubsequent(this))
				{
					AlreadyCompletedPrerequisites++;
				}
			}
		}
		PrerequisitesComplete(CurrentThreadIfKnown, AlreadyCompletedPrerequisites, bUnlock);
	}


	//call from FConstructor to complete the setup process
	//create the completed event
	//set the thread to execute on based on the embedded task
	///attempt to add myself as a subsequent to each prerequisite
	//tell the base task that i am ready to start as soon as my prerequisites are ready
	GraphEventRef Setup(const GraphEventArray* Prerequisites = NULL, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread)
	{
		/*TaskTrace::Launched(GetTraceId(), nullptr, Subsequents.IsValid(), ((TTask*)&TaskStorage)->GetDesiredThread(), sizeof(*this));*/

		FraphEventRef ReturnedEventRef = Subsequents; // very important so that this doesn't get destroyed before we return
		SetupPrereqs(Prerequisites, CurrentThreadIfKnown, true);
		return ReturnedEventRef;
	}

	
	//Call from FConstructor to complete the setup process, but doesn't allow the task to dispatch yet
	//Create the completed event
	//Set the thread to execute on based on the embedded task
	//Attempt to add myself as a subsequent to each prerequisite
	//Tell the base task that I am ready to start as soon as my prerequisites are ready.
	 
	TGraphTask* Hold(const GraphEventArray* Prerequisites = NULL, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread)
	{
		//TaskTrace::Created(GetTraceId(), sizeof(*this));

		SetupPrereqs(Prerequisites, CurrentThreadIfKnown, false);
		return this;
	}

	//Factory to create a gather task which assumes the given subsequent list from some other tasks.
	static FConstructor CreateTask(GraphEventRef SubsequentsToAssume, const GraphEventArray* Prerequisites = NULL, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread)
	{
		return FConstructor(new TGraphTask(SubsequentsToAssume, Prerequisites ? Prerequisites->Num() : 0), Prerequisites, CurrentThreadIfKnown);
	}

	/** An aligned bit of storage to hold the embedded task **/
	TAlignedBytes<sizeof(TTask), alignof(TTask)> TaskStorage;

	/** Used to sanity check the state of the object **/
	bool						TaskConstructed;
	/** A reference counted pointer to the completion event which lists the tasks that have me as a prerequisite. **/
	GraphEventRef				Subsequents;
};


// Returns a graph event that gets completed as soon as any of the given tasks gets completed
GraphEventRef AnyTaskCompleted(const GraphEventArray& GraphEvents);
