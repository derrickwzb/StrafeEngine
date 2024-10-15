#include "strafepch.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include "Strafe/Core/TaskGraph/TaskGraphInterface.h"
#include "Strafe/Core/Threading/Runnable.h"
#include "Strafe/Core/Utils/Windows/WindowsCriticalSection.h"
#include "Strafe/Core/Utils/ScopeLock.h"
#include "Strafe/Core/Threading/GenericThread.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformMisc.h"
#include "Strafe/Core/Utils/ScopedEvent.h"
#include "Strafe/Core/Utils/ReverseIterate.h"
#define STRTOTCHAR(x)  L"x"
static int32 GNumWorkerThreadsToIgnore = 0;

namespace NamedThreadsEnum
{
	std::atomic<Type> RenderThreadStatics::RenderThread(NamedThreadsEnum::GameThread);
	std::atomic<Type> RenderThreadStatics::RenderThread_Local(NamedThreadsEnum::GameThread_Local);
	int32 bHasBackgroundThreads = 1;
	int32 bHasHighPriorityThreads = 1;
}

//rendering thread.cpp sets these values if needed
bool RenderThreadPollingOn = false; // Access/Modify on GT only. This value is set on the GT before actual state is changed on the RT.
int32 RenderThreadPollPeriodMs = -1; // Access/Modify on RT only.

//Configures the number of foreground worker threads. Requires the scheduler to be restarted to have an affect
//TODO cvar or serialization or engine settings.
int32 GUseNewTaskBackend = 1;
int32 GNumForegroundWorkers = 2;

/**
 *	Pointer to the task graph implementation singleton.
 *	Because of the multithreaded nature of this system an ordinary singleton cannot be used.
 *	FTaskGraphImplementation::Startup() creates the singleton and the constructor actually sets this value.
 * @TODO can be changed to dependency injection format but leaving it as is for now
**/
class TaskGraphImplementation;
struct WorkerThread;

static TaskGraphInterface* TaskGraphImplementationSingleton = NULL;

//TaskThreadBase
//base class for a thread that executes tasks
//this calss implements the runnable api , but external threads dont use that because those threads are created elesewhere
class TaskThreadBase : public Runnable
{
	//calls meant to be called from a main 
	public:

	//constructor, initializes everything to unusable values. meant to be called from a main thread
	TaskThreadBase()
		:ThreadId(NamedThreadsEnum::AnyThread)
		,PerTheadIDTLSSlot(WindowsPlatformTLS::InvalidTlsSlot)
		,OwnerWorker(nullptr)
	{
		NewTasks.clear();
		NewTasks.reserve(128);
	}

	//sets up some basic information for a thread. meant to be called from a main thread. also creates stall event
	void Setup(NamedThreadsEnum::Type InThreadId, uint32 InPerTheadIDTLSSlot, WorkerThread* InOwnerWorker)
	{
		ThreadId = InThreadId;
		PerTheadIDTLSSlot = InPerTheadIDTLSSlot;
		OwnerWorker = InOwnerWorker;
	}

	//calls meant to be called from this thread

	//a one time call to set the tls entry for this thread
	void InitializeForCurrentThread()
	{
		WindowsPlatformTLS::SetTlsValue(PerTheadIDTLSSlot, OwnerWorker);
	}

	//returns the index of this thread
	NamedThreadsEnum::Type GetThreadId() const
	{
		return ThreadId;
	}

	//used for named threads to start processing tasks until the thread is idle and request quit has been called
	virtual void ProcessTasksUntilQuit(int32 QueueIndex) = 0;

	//used for named threads to start processing tasks until the thread is idle and request quit has been called
	virtual uint64 ProcessTasksUntilIdle(int32 QueueIndex)
	{
		return 0;
	}

	//queue a task, assuming that this thread is the same as the current thread
	virtual void EnqueueFromThisThread(int32 QueueIndex, BaseGraphTask* Task)
	{

	}

	//calls meant to be called from any thread.

	//will cause the thread to return to the caller when it becomes idle. used to return from ProcessTasksUntilQuit for named threads or to shut down unnamed threads.
	//caution this will not work under random circumstances. for example u should not attempt to stop unnamed threads unless they are known to be idle.
	virtual void RequestQuit(int32 QueueIndex) = 0;

	//queue a task, assuming that this thread is not the same as the current thread
	virtual bool EnqueueFromOtherThread(int32 QueueIndex, BaseGraphTask* Task)
	{
		return false;
	}

	virtual void WakeUp(int32 QueueIndex = 0) = 0;
	
	//return true if this thread is processing tasks, this is only a guess if you ask for a thread other than yourself because that can change before the function returns
	virtual bool IsProcessingTasks(int32 QueueIndex) = 0;

	//runnable api

	//allows per runnable obj initialization. this is called in the context fo the thread object that aggregates this, not the thread that passes this runnable to a new thread.
	virtual bool Init() override
	{
		InitializeForCurrentThread();
		return true;
	}

	//this is where all per object thread work is done. this is only called if the initialization was successful
	virtual uint32 Run() override
	{
		ProcessTasksUntilQuit(0);
		WindowsPlatformTLS::SetTlsValue(PerTheadIDTLSSlot, nullptr);
		return 0;
	}

	/**
	 * This is called if a thread is requested to terminate early
	 */
	virtual void Stop() override
	{
		RequestQuit(-1);
	}

	/**
	 * Called in the context of the aggregating thread to perform any cleanup.
	 */
	virtual void Exit() override
	{
	}

protected:
	//index of this thread
	NamedThreadsEnum::Type ThreadId;

	//tls slot that we store the taskthread* this pointer in
	uint32 PerTheadIDTLSSlot;
	//used to signal stalling. not safe 
	std::atomic<int32> isStalled;
	//array of tasks for this task threead
	std::vector<BaseGraphTask*> NewTasks;
	//back ptr to the owning worker thread
	WorkerThread* OwnerWorker;
};


//namedtaskthread
//a class for managing a named thread
class NamedTaskThread : public TaskThreadBase
{
	virtual void ProcessTasksUntilQuit(int32 QueueIndex) override
	{
		//check(Queue(QueueIndex).StallRestartEvent); // make sure we are started up

		Queue(QueueIndex).QuitForReturn = false;
		//check(++Queue(QueueIndex).RecursionGuard == 1);
		do
		{
			const bool bAllowStall = true;
			ProcessTasksNamedThread(QueueIndex, bAllowStall);
		} while (!Queue(QueueIndex).QuitForReturn && !Queue(QueueIndex).QuitForShutdown); 
		//check (!--Queue(QueueIndex).RecursionGuard);
	}

	virtual uint64 ProcessTasksUntilIdle(int32 QueueIndex) override
	{
		//check(Queue(QueueIndex).StallRestartEvent); // make sure we are started up

		Queue(QueueIndex).QuitForReturn = false;
		//check(++Queue(QueueIndex).RecursionGuard == 1);
		uint64 ProcessedTasks = ProcessTasksNamedThread(QueueIndex, false);
		//check (!--Queue(QueueIndex).RecursionGuard);
		return ProcessedTasks;
	}

	uint64 ProcessTasksNamedThread(int32 QueueIndex, bool AllowStall)
	{
		uint64 ProcessedTasks = 0;
		bool CountAsStall = false;
		const bool IsRenderThreadMainQueue = (NamedThreadsEnum::GetThreadIndex(ThreadId) == NamedThreadsEnum::ActualRenderingThread) && (QueueIndex == 0);
		while (!Queue(QueueIndex).QuitForReturn)
		{
			//for use of rendering thread
			const bool IsRenderThreadAndPolling = IsRenderThreadMainQueue && (RenderThreadPollPeriodMs >= 0);
			const bool StallQueueAllowStall = AllowStall && !IsRenderThreadAndPolling;
			BaseGraphTask* Task = Queue(QueueIndex).StallQueue.Pop(0, StallQueueAllowStall);
			if (!Task)
			{
				if (AllowStall)
				{
					Queue(QueueIndex).StallRestartEvent->Wait(IsRenderThreadAndPolling ? RenderThreadPollPeriodMs : ((uint32)0xffffffff), CountAsStall);
					if (Queue(QueueIndex).QuitForShutdown)
					{
						return ProcessedTasks;
					}
					continue;
				}
				else
				{
					break;
					//we were asked to quit
				}
			}
			else
			{
				Task->Execute(NewTasks, NamedThreadsEnum::Type(ThreadId | (QueueIndex << NamedThreadsEnum::QueueIndexShift)), true);
				//Queue(QueueIndex).QuitForReturn = true;
				ProcessedTasks++;
			}
		}
		return ProcessedTasks;
	}

	virtual void EnqueueFromThisThread(int32 QueueIndex, BaseGraphTask* Task) override
	{
		//TODO  check Task && Queue(QueueIndex).StallRestartEvent // make sure we are started up
		uint32 PriIndex = NamedThreadsEnum::GetTaskPriority(Task->GetThreadToExecuteOn()) ? 0 : 1;
		int32 ThreadToStart = Queue(QueueIndex).StallQueue.Push(Task, PriIndex);
		//check(ThreadToStart < 0); // if I am stalled, then how can I be queueing a task?
	}

	virtual void RequestQuit(int32 QueueIndex) override
	{
		//this will not work on certain circumstances, you should not attempt to stop  threads unless they are known to be idle
		if (!Queue(0).StallRestartEvent)
		{
			return;
		}
		if (QueueIndex == -1)
		{
			//we are shutting down
			//check queue 0 and 1 for stall restart event @TODO
			Queue(0).QuitForShutdown = true;
			Queue(1).QuitForShutdown = true;
			Queue(0).StallRestartEvent->Trigger();
			Queue(1).StallRestartEvent->Trigger();
		}
		else
		{
			//@TODO check for current queue index stall restart event
			Queue(QueueIndex).QuitForReturn = true;
		}
	}

	virtual bool EnqueueFromOtherThread(int32 QueueIndex, BaseGraphTask* Task) override
	{
	std::cout<< "Enqueueing from other thread"<<std::endl;
		//check if task exists and queue(index).slstall restart event exists
		uint32 PrimaryIndex = NamedThreadsEnum::GetTaskPriority(Task->GetThreadToExecuteOn())? 0:1;
		int32 ThreadToStart = Queue(QueueIndex).StallQueue.Push(Task, PrimaryIndex);

		if (ThreadToStart >= 0)
		{
			//check thread to start  ==0
			Queue(QueueIndex).StallRestartEvent->Trigger();
			return true;
		}
		return false;
	}

	virtual bool IsProcessingTasks(int32 QueueIndex)  override
	{
		return !!Queue(QueueIndex).RecursionGuard;
	}

	virtual void WakeUp(int32 QueueIndex ) override
	{
		Queue(QueueIndex).StallRestartEvent->Trigger();
	}
private:

	//grouping of the data for an individual queue
	struct ThreadTaskQueue
	{
		StallingTaskQueue<BaseGraphTask, PLATFORM_CACHE_LINE_SIZE, 2> StallQueue;

		//we need to disallow reentry of the processing loop
		uint32 RecursionGuard;

		//Indicates we executed a return task, so break out of the processing loop
		bool QuitForReturn;

		//indicates we executed a return task, so break out of the processing loop
		bool QuitForShutdown;

		//Event that this thread blocks on when it runs out of work
		GenericEvent* StallRestartEvent;

		ThreadTaskQueue()
			: RecursionGuard(0)
			, QuitForReturn(false)
			, QuitForShutdown(false)
			, StallRestartEvent(WindowsPlatformProcess::GetSynchEventFromPool(false))
		{
		}

		~ThreadTaskQueue()
		{
			WindowsPlatformProcess::ReturnSynchEventToPool(StallRestartEvent);
			StallRestartEvent = nullptr;
		}
	};


	FORCEINLINE ThreadTaskQueue& Queue(int32 QueueIndex)
	{
		return Queues[QueueIndex];
	}

	FORCEINLINE const ThreadTaskQueue& Queue(int32 QueueIndex) const
	{
		return Queues[QueueIndex];
	}

	ThreadTaskQueue Queues[NamedThreadsEnum::NumQueues];


};

//TaskThreadAnyThread
//a aclass for managign worker threads
class TaskThreadAnyThread : public TaskThreadBase
{
	public:
	TaskThreadAnyThread(int32 PriorityIndex)
		:PriorityIndex(PriorityIndex)
	{
	}

	virtual void ProcessTasksUntilQuit(int32 QueueIndex) override
	{
		if (PriorityIndex != (NamedThreadsEnum::BackgroundThreadPriority >> NamedThreadsEnum::ThreadPriorityShift))
		{
			if (!WindowsPlatformTLS::IsValidTlsSlot(PerTheadIDTLSSlot))
			{
				PerTheadIDTLSSlot = WindowsPlatformTLS::AllocTlsSlot();
			}
			//allocate new tls slot but then there is no memory allocator to cache it so leave it for now
			//does nothing
		}
		//TODO maybe cache? leave it for now got no memory allocator
		/*if (!WindowsPlatformTLS::IsValidTlsSlot(PerTheadIDTLSSlot))
		{
			PerTheadIDTLSSlot = WindowsPlatformTLS::AllocTlsSlot();
		}*/
		//void* ThreadSingleton = WindowsPlatformTLS::GetTlsValue(PerTheadIDTLSSlot);

		//check if not queue index
		do
		{
			ProcessTasks();
		} while (!Queue.QuitForShutdown); // @Hack - quit now when running with only one thread.
	}

	virtual uint64 ProcessTasksUntilIdle(int32 QueueIndex) override
	{
		//todo nothign for now
		return 0;
	}

	//calls meant to be called from any thread.

	//will cause the thread to return to the caller when it becomes idle. used to return from processtaskuntilquit for named threads or to shut down unnamed threads.
	//we should not attempt to stop unnamed threads unless they are actually idle
	virtual void RequestQuit(int32 QueueIndex) override
	{
		//check if queue index <1

		// this will not work under arbitrary circumstances. For example you should not attempt to stop threads unless they are known to be idle.
		//check if Queue.StallRestartEvent; // make sure we are started up
		Queue.QuitForShutdown = true;
		Queue.StallRestartEvent->Trigger();
	}
	
	virtual void WakeUp(int32 QueueIndex = 0) final override
	{
		Queue.StallRestartEvent->Trigger();
	}

	void StallForTuning(bool Stall)
	{
		if (Stall)
		{
			Queue.StallForTuning.Lock();
			Queue.bStallForTuning = true;
		}
		else
		{
			Queue.bStallForTuning = false;
			Queue.StallForTuning.Unlock();
		}
	}
	//return true if processing tasks. only a guess because it can change before the function returns
	virtual bool IsProcessingTasks(int32 QueueIndex) override
	{
		//todo check validity of queueindex
		return !!Queue.RecursionGuard;
	}
	//for profiling use
	virtual uint32 Run() override
	{
		return TaskThreadBase::Run();
	}

	private:


		//for profiling use
		static inline const TCHAR* ThreadPriorityToName(int32 PriorityIdx)
		{
			PriorityIdx <<= NamedThreadsEnum::ThreadPriorityShift;
			if (PriorityIdx == NamedThreadsEnum::HighThreadPriority)
			{
				return TEXT("Task Thread HP");
			}
			else if (PriorityIdx == NamedThreadsEnum::NormalThreadPriority)
			{
				return TEXT("Task Thread NP");
			}
			else if (PriorityIdx == NamedThreadsEnum::BackgroundThreadPriority)
			{
				return TEXT("Task Thread BP");
			}
			else
			{
				return TEXT("Task Thread Unknown Priority");
			}
		}

		//process tasks until idle. may block if allow stall is true
		uint64 ProcessTasks()
		{
			bool CountAsStall = true;
			uint64 ProcessedTasks = 0;

			//@TODO verify ++Queue.RecursionGuard == 1
			bool DidStall = false;
			while (1)
			{
				BaseGraphTask* Task = FindWork();
				if (!Task)
				{
					Queue.StallRestartEvent->Wait(((uint32)0xffffffff), CountAsStall);
					DidStall = true;
					if (Queue.QuitForShutdown)
					{
						break;
					}
					continue;
				}
				
				// the Win scheduler is ill behaved and will sometimes let BG tasks run even when other tasks are ready....kick the scheduler between tasks
				if (!DidStall && PriorityIndex == (NamedThreadsEnum::BackgroundThreadPriority >> NamedThreadsEnum::ThreadPriorityShift))
				{
					WindowsPlatformProcess::Sleep(0);
				}
				DidStall = false;
				Task->Execute(NewTasks, NamedThreadsEnum::Type(ThreadId), true);
				ProcessedTasks++;
				if (Queue.bStallForTuning)
				{
					{
						ScopeLock Lock(&Queue.StallForTuning);
					}
				}
				//verify !--Queue.RecursionGuard // just ignore this for now 
				return ProcessedTasks;
			}
		}

		struct FThreadTaskQueue
		{
			/** Event that this thread blocks on when it runs out of work. **/
			GenericEvent* StallRestartEvent;
			/** We need to disallow reentry of the processing loop **/
			uint32 RecursionGuard;
			/** Indicates we executed a return task, so break out of the processing loop. **/
			bool QuitForShutdown;
			/** Should we stall for tuning? **/
			bool bStallForTuning;
			WindowsCriticalSection StallForTuning;

			FThreadTaskQueue()
				: StallRestartEvent(WindowsPlatformProcess::GetSynchEventFromPool(false))
				, RecursionGuard(0)
				, QuitForShutdown(false)
				, bStallForTuning(false)
			{

			}
			~FThreadTaskQueue()
			{
				WindowsPlatformProcess::ReturnSynchEventToPool(StallRestartEvent);
				StallRestartEvent = nullptr;
			}
		};

		/**
		*	Internal function to call the system looking for work. Called from this thread.
		*	@return New task to process.
		*/
		BaseGraphTask* FindWork();

		/** Array of queues, only the first one is used for unnamed threads. **/
		FThreadTaskQueue Queue;

		int32 PriorityIndex;

};

/**
	*	WorkerThread
	*	Helper structure to aggregate a few items related to the individual threads.
**/
struct WorkerThread
{
	/** The actual FTaskThread that manager this task **/
	TaskThreadBase* TaskGraphWorker;
	/** For internal threads, the is non-NULL and holds the information about the runable thread that was created. **/
	GenericThread* RunnableThread;
	/** For external threads, this determines if they have been "attached" yet. Attachment is mostly setting up TLS for this individual thread. **/
	bool				bAttached;

	/** Constructor to set reasonable defaults. **/
	WorkerThread()
		: TaskGraphWorker(nullptr)
		, RunnableThread(nullptr)
		, bAttached(false)
	{
	}
};


class FMyWorker : public Runnable
{
public:

	// The boolean that acts as the main switch
// When this is false, inputs and outputs can be safely read from game thread
	bool bInputReady = false;


	// Declare the variables that are the inputs and outputs here.
	// You can have as many as you want. Remember not to use pointers, this must be
	// plain old data
	// For example:
	int ExampleIntInput = 0;
	float ExampleFloatOutput = 0.0f;
	// Constructor, create the thread by calling this
	FMyWorker()
	{
		// Constructs the actual thread object. It will begin execution immediately
		// If you've passed in any inputs, set them up before calling this.
		Thread = GenericThread::Create(this, TEXT("Give your thread a good name"), 0U, ThreadPri_Highest);
	}

	// Destructor
	virtual ~FMyWorker() override
	{
		if (Thread)
		{
			// Kill() is a blocking call, it waits for the thread to finish.
			// Hopefully that doesn't take too long
			Thread->Kill();
			delete Thread;
		}
	}


	// Overriden from FRunnable
	// Do not call these functions youself, that will happen automatically
	bool Init() override
	{
		std::cout << "My custom thread has been initialized" << std::endl;

		// Return false if you want to abort the thread
		return true;
	}// Do your setup here, allocate memory, ect.
	uint32 Run() override
	{
		while (1)
		{
			Sleep(5000);
		}
		// Peform your processor intensive task here. In this example, a neverending
		// task is created, which will only end when Stop is called.


		return 0;
	}// Main data processing happens here
	void Stop() override
	{
		// Clean up memory usage here, and make sure the Run() function stops soon
		// The main thread will be stopped until this finishes!

		// For this example, we just need to terminate the while loop
		// It will finish in <= 1 sec, due to the Sleep()
		bRunThread = false;
	}// Clean up any memory you allocated here


private:

	// Thread handle. Control the thread using this, with operators like Kill and Suspend
	GenericThread* Thread;

	// Used to know when the thread should exit, changed in Stop(), read in Run()
	bool bRunThread;
};

//TaskGraphImplementation
//Implementation of the centralized task graph system
//these parts of the system have no knowledge of the dependency graph, they exclusively work on tasks
class TaskGraphImplementation final : public TaskGraphInterface
{
public:

	//api related to lifecycle of the system and singletons

	//singleton returning the one and only taskgrpah implementation
	//note that a manual call to startup is required before it will return a valid ref
	static TaskGraphImplementation& Get()
	{
		if(TaskGraphImplementationSingleton)
			return  *static_cast<TaskGraphImplementation*>(TaskGraphImplementationSingleton);
	}

	//constructor 
	//initializes the data structure, sets the singleton pter and create internal threads
	TaskGraphImplementation(int32)
	{

		CreatedHiPriorityThreads = !!NamedThreadsEnum::bHasHighPriorityThreads;
		CreatedBackgroundPriorityThreads = !!NamedThreadsEnum::bHasBackgroundThreads;

		int32 MaxTaskThreads = MAX_THREADS;
		int32 NumTaskThreads = WindowsPlatformMisc::NumberOfWorkerThreadsToSpawn();

		LastExternalThread = NamedThreadsEnum::ActualRenderingThread;

		NumNamedThreads = LastExternalThread + 1;

		NumTaskThreadSets = 1 + CreatedHiPriorityThreads + CreatedBackgroundPriorityThreads;
		// if we don't have enough threads to allow all of the sets asked for, then we can't create what was asked for.
		//@TODO check if (NumTaskThreadSets == 1 || Min<int32>(NumTaskThreads * NumTaskThreadSets + NumNamedThreads, MAX_THREADS) == NumTaskThreads * NumTaskThreadSets + NumNamedThreads);
		NumThreads = Max<int32>(Min<int32>(NumTaskThreads * NumTaskThreadSets + NumNamedThreads, MAX_THREADS), NumNamedThreads + 1);

		// Cap number of extra threads to the platform worker thread count
		// if we don't have enough threads to allow all of the sets asked for, then we can't create what was asked for.
		//@TODO check if (NumTaskThreadSets == 1 || FMath::Min(NumThreads, NumNamedThreads + NumTaskThreads * NumTaskThreadSets) == NumThreads);
		NumThreads = Min(NumThreads, NumNamedThreads + NumTaskThreads * NumTaskThreadSets);

		NumTaskThreadsPerSet = (NumThreads - NumNamedThreads) / NumTaskThreadSets;
		//TODO check if ((NumThreads - NumNamedThreads) % NumTaskThreadSets == 0); // should be equal numbers of threads per priority set

		//log here that taskgraph is started with numnamedthreads, numthreads and numtaskthreadsets
		//TODO check if (NumThreads - NumNamedThreads >= 1);  // need at least one pure worker thread
		//TODO check if (NumThreads <= MAX_THREADS);
		//TODO check if (!ReentrancyCheck.GetValue()); ensure no reentrancy
		ReentrancyCheck.fetch_add(1); // just checking for reentrancy
		PerThreadIDTLSSlot = WindowsPlatformTLS::AllocTlsSlot();

		for (int32 ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++)
		{
			//TODO check(!WorkerThreads[ThreadIndex].bAttached); // reentrant?
			bool bAnyTaskThread = ThreadIndex >= NumNamedThreads;
			if (bAnyTaskThread)
			{
				WorkerThreads[ThreadIndex].TaskGraphWorker = new TaskThreadAnyThread(ThreadIndexToPriorityIndex(ThreadIndex));
			}
			else
			{
				WorkerThreads[ThreadIndex].TaskGraphWorker = new NamedTaskThread;
			}
			WorkerThreads[ThreadIndex].TaskGraphWorker->Setup(NamedThreadsEnum::Type(ThreadIndex), PerThreadIDTLSSlot, &WorkerThreads[ThreadIndex]);

		}
		TaskGraphImplementationSingleton = this; // now reentrancy is ok

		const TCHAR* PrevGroupName = nullptr;
		for (int32 ThreadIndex = LastExternalThread + 1; ThreadIndex < NumThreads; ThreadIndex++)
		{
			std::string Name;
			const TCHAR* GroupName = TEXT("TaskGraphNormal");
			int32 Priority = ThreadIndexToPriorityIndex(ThreadIndex);
			// These are below normal threads so that they sleep when the named threads are active
			ThreadPriority ThreadPri;
			uint64 Affinity = WindowsPlatformAffinity::GetTaskGraphThreadMask();
			if (Priority == 1)
			{
				Name = (TEXT("TaskGraphThreadHP %d"), ThreadIndex - (LastExternalThread + 1));
				GroupName = TEXT("TaskGraphHigh");
				ThreadPri = ThreadPri_SlightlyBelowNormal; // we want even hi priority tasks below the normal threads

				// If the platform defines FPlatformAffinity::GetTaskGraphHighPriorityTaskMask then use it
				if (WindowsPlatformAffinity::GetTaskGraphHighPriorityTaskMask() != 0xFFFFFFFFFFFFFFFF)
				{
					Affinity = WindowsPlatformAffinity::GetTaskGraphHighPriorityTaskMask();
				}
			}
			else if (Priority == 2)
			{
				Name = (TEXT("TaskGraphThreadBP %d"), ThreadIndex - (LastExternalThread + 1));
				GroupName = TEXT("TaskGraphLow");
				ThreadPri = ThreadPri_Lowest;
				// If the platform defines FPlatformAffinity::GetTaskGraphBackgroundTaskMask then use it
				if (WindowsPlatformAffinity::GetTaskGraphBackgroundTaskMask() != 0xFFFFFFFFFFFFFFFF)
				{
					Affinity = WindowsPlatformAffinity::GetTaskGraphBackgroundTaskMask();
				}
			}
			else
			{
				Name = (TEXT("TaskGraphThreadNP %d"), ThreadIndex - (LastExternalThread + 1));
				ThreadPri = ThreadPri_BelowNormal; // we want normal tasks below normal threads like the game thread
			}

			int32 StackSize;
			StackSize = 1024 * 1024;


			if (GroupName != PrevGroupName)
			{
				//group names can be used for profiling
				PrevGroupName = GroupName;
			}
			const TCHAR* nametchar = STRTOTCHAR("TaskGraphThread");
			WorkerThreads[ThreadIndex].RunnableThread = GenericThread::Create(&Thread(ThreadIndex), TEXT("Taskgraph"), StackSize, ThreadPri, Affinity);
			
			
			WorkerThreads[ThreadIndex].bAttached = true;
		}
	}

	//destructor - probably only works reliably when the system is completely idle. the system has no idea if it is idle or not
	virtual ~TaskGraphImplementation()
	{
		for (auto& Callback : ShutdownCallbacks)
		{
			Callback();
		}
		ShutdownCallbacks.clear();
		for (int32 ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++)
		{
			Thread(ThreadIndex).RequestQuit(-1);
		}
		for (int32 ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++)
		{
			if (ThreadIndex > LastExternalThread)
			{
				WorkerThreads[ThreadIndex].RunnableThread->WaitForCompletion();
				delete WorkerThreads[ThreadIndex].RunnableThread;
				WorkerThreads[ThreadIndex].RunnableThread = NULL;
			}
			WorkerThreads[ThreadIndex].bAttached = false;
		}
		TaskGraphImplementationSingleton = NULL;
		NumTaskThreadsPerSet = 0;
		WindowsPlatformTLS::FreeTlsSlot(PerThreadIDTLSSlot);
	}

	// API inherited from FTaskGraphInterface

	//function to queue a task, called from basegraphtask
	virtual void QueueTask(BaseGraphTask* Task, bool bWakeUpWorker, NamedThreadsEnum::Type ThreadToExecuteOn, NamedThreadsEnum::Type InCurrentThreadIfKnown = NamedThreadsEnum::AnyThread) final override
	{
		//check(Task);
		std::cout << "Trying to queue task" << std::endl;

		if (NamedThreadsEnum::GetThreadIndex(ThreadToExecuteOn) == NamedThreadsEnum::AnyThread)
		{
			
				uint32 TaskPriority = NamedThreadsEnum::GetTaskPriority(Task->GetThreadToExecuteOn());
				int32 Priority = NamedThreadsEnum::GetThreadPriorityIndex(Task->GetThreadToExecuteOn());
				if (Priority == (NamedThreadsEnum::BackgroundThreadPriority >> NamedThreadsEnum::ThreadPriorityShift) && (!CreatedBackgroundPriorityThreads || !NamedThreadsEnum::bHasBackgroundThreads))
				{
					Priority = NamedThreadsEnum::NormalThreadPriority >> NamedThreadsEnum::ThreadPriorityShift; // we don't have background threads, promote to normal
					TaskPriority = NamedThreadsEnum::NormalTaskPriority >> NamedThreadsEnum::TaskPriorityShift; // demote to normal task pri
				}
				else if (Priority == (NamedThreadsEnum::HighThreadPriority >> NamedThreadsEnum::ThreadPriorityShift) && (!CreatedHiPriorityThreads || !NamedThreadsEnum::bHasHighPriorityThreads))
				{
					Priority = NamedThreadsEnum::NormalThreadPriority >> NamedThreadsEnum::ThreadPriorityShift; // we don't have hi priority threads, demote to normal
					TaskPriority = NamedThreadsEnum::HighTaskPriority >> NamedThreadsEnum::TaskPriorityShift; // promote to hi task pri
				}
				uint32 PriIndex = TaskPriority ? 0 : 1;
				//TODO check if(Priority >= 0 && Priority < MAX_THREAD_PRIORITIES);
				{
					int32 IndexToStart = IncomingAnyThreadTasks[Priority].Push(Task, PriIndex);
					if (IndexToStart >= 0)
					{
						StartTaskThread(Priority, IndexToStart);
					}
				}
				return;
			
		}
		NamedThreadsEnum::Type CurrentThreadIfKnown;
		if (NamedThreadsEnum::GetThreadIndex(InCurrentThreadIfKnown) == NamedThreadsEnum::AnyThread)
		{
			CurrentThreadIfKnown = GetCurrentThread();
		}
		else
		{
			CurrentThreadIfKnown = NamedThreadsEnum::GetThreadIndex(InCurrentThreadIfKnown);
			//check if(CurrentThreadIfKnown == ENamedThreads::GetThreadIndex(GetCurrentThread()));
		}
		{
			int32 QueueToExecuteOn = NamedThreadsEnum::GetQueueIndex(ThreadToExecuteOn);
			ThreadToExecuteOn = NamedThreadsEnum::GetThreadIndex(ThreadToExecuteOn);
			TaskThreadBase* Target = &Thread(ThreadToExecuteOn);
			if (ThreadToExecuteOn == NamedThreadsEnum::GetThreadIndex(CurrentThreadIfKnown))
			{
				Target->EnqueueFromThisThread(QueueToExecuteOn, Task);
			}
			else
			{
				Target->EnqueueFromOtherThread(QueueToExecuteOn, Task);
			}
		}
	}

	virtual int32 GetNumWorkerThreads() final override
	{
		int32 Result = (NumThreads - NumNamedThreads) / NumTaskThreadSets - GNumWorkerThreadsToIgnore;
		//TODO check if(Result > 0); // can't tune it to zero task threads
		return Result;
	}

	virtual int32 GetNumForegroundThreads() final override
	{
		return CreatedHiPriorityThreads ? NumTaskThreadsPerSet : 0;
	}

	virtual int32 GetNumBackgroundThreads() final override
	{
		return CreatedBackgroundPriorityThreads ? NumTaskThreadsPerSet : 0;
	}

	virtual bool IsCurrentThreadKnown() final override
	{
		return WindowsPlatformTLS::GetTlsValue(PerThreadIDTLSSlot) != nullptr;
	}

	virtual NamedThreadsEnum::Type GetCurrentThreadIfKnown(bool bLocalQueue) final override
	{
		NamedThreadsEnum::Type Result = GetCurrentThread();
		if (bLocalQueue && NamedThreadsEnum::GetThreadIndex(Result) >= 0 && NamedThreadsEnum::GetThreadIndex(Result) < NumNamedThreads)
		{
			Result = NamedThreadsEnum::Type(int32(Result) | int32(NamedThreadsEnum::LocalQueue));
		}
		return Result;
	}

	virtual bool IsThreadProcessingTasks(NamedThreadsEnum::Type ThreadToCheck) final override
	{
		int32 QueueIndex = NamedThreadsEnum::GetQueueIndex(ThreadToCheck);
		ThreadToCheck = NamedThreadsEnum::GetThreadIndex(ThreadToCheck);
		//TODO check if(ThreadToCheck >= 0 && ThreadToCheck < NumNamedThreads);
		return Thread(ThreadToCheck).IsProcessingTasks(QueueIndex);
	}

	//External Thread API

	virtual void AttachToThread(NamedThreadsEnum::Type CurrentThread) final override
	{
		CurrentThread = NamedThreadsEnum::GetThreadIndex(CurrentThread);
		//TODO check(NumTaskThreadsPerSet);
		//TODO check(CurrentThread >= 0 && CurrentThread < NumNamedThreads);
		//TODO check(!WorkerThreads[CurrentThread].bAttached);
		Thread(CurrentThread).InitializeForCurrentThread();
	}

	virtual uint64 ProcessThreadUntilIdle(NamedThreadsEnum::Type CurrentThread) final override
	{
		int32 QueueIndex = NamedThreadsEnum::GetQueueIndex(CurrentThread);
		CurrentThread = NamedThreadsEnum::GetThreadIndex(CurrentThread);
		//check(CurrentThread >= 0 && CurrentThread < NumNamedThreads);
		//check(CurrentThread == GetCurrentThread());
		return Thread(CurrentThread).ProcessTasksUntilIdle(QueueIndex);
	}

	virtual void ProcessThreadUntilRequestReturn(NamedThreadsEnum::Type CurrentThread) final override
	{
		int32 QueueIndex = NamedThreadsEnum::GetQueueIndex(CurrentThread);
		CurrentThread = NamedThreadsEnum::GetThreadIndex(CurrentThread);
		//TODO check(CurrentThread >= 0 && CurrentThread < NumNamedThreads);
		//TODO check(CurrentThread == GetCurrentThread());
		Thread(CurrentThread).ProcessTasksUntilQuit(QueueIndex);
	}

	virtual void RequestReturn(NamedThreadsEnum::Type CurrentThread) final override
	{
		int32 QueueIndex = NamedThreadsEnum::GetQueueIndex(CurrentThread);
		CurrentThread = NamedThreadsEnum::GetThreadIndex(CurrentThread);
		//TODO check(CurrentThread != NamedThreadsEnum::AnyThread);
		Thread(CurrentThread).RequestQuit(QueueIndex);
	}

	virtual void WaitUntilTasksComplete(const GraphEventArray& Tasks, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread) final override
	{
		

		NamedThreadsEnum::Type CurrentThread = CurrentThreadIfKnown;
		if (NamedThreadsEnum::GetThreadIndex(CurrentThreadIfKnown) == NamedThreadsEnum::AnyThread)
		{
			bool bIsHiPri = !!NamedThreadsEnum::GetTaskPriority(CurrentThreadIfKnown);
			int32 Priority = NamedThreadsEnum::GetThreadPriorityIndex(CurrentThreadIfKnown);
			//TODO check(!NamedThreadsEnum::GetQueueIndex(CurrentThreadIfKnown));
			CurrentThreadIfKnown = NamedThreadsEnum::GetThreadIndex(GetCurrentThread());
			CurrentThread = NamedThreadsEnum::SetPriorities(CurrentThreadIfKnown, Priority, bIsHiPri);
		}
		else
		{
			CurrentThreadIfKnown = NamedThreadsEnum::GetThreadIndex(CurrentThreadIfKnown);
			//TODO check(CurrentThreadIfKnown == NamedThreadsEnum::GetThreadIndex(GetCurrentThread()));
			// we don't modify CurrentThread here because it might be a local queue
		}

		if (CurrentThreadIfKnown != NamedThreadsEnum::AnyThread && CurrentThreadIfKnown < NumNamedThreads && !IsThreadProcessingTasks(CurrentThread))
		{
			if (Tasks.size() < 8) // don't bother to check for completion if there are lots of prereqs...too expensive to check
			{
				bool bAnyPending = false;
				for (int32 Index = 0; Index < Tasks.size(); Index++)
				{
					GraphEvent* Task = Tasks[Index].GetReference();
					if (Task && !Task->IsComplete())
					{
						bAnyPending = true;
						break;
					}
				}
				if (!bAnyPending)
				{
					return;
				}
			}

			// named thread process tasks while we wait
			TGraphTask<ReturnGraphTask>::CreateTask(&Tasks, CurrentThread).ConstructAndDispatchWhenReady(CurrentThread);

			ProcessThreadUntilRequestReturn(CurrentThread);
		}
		else
		{

			// We will just stall this thread on an event while we wait
			ScopedEvent Event;
			TriggerEventWhenTasksComplete(Event.Get(), Tasks, CurrentThreadIfKnown);
		}
	}

	virtual void TriggerEventWhenTasksComplete(GenericEvent* InEvent, const GraphEventArray& Tasks, NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread, NamedThreadsEnum::Type TriggerThread = NamedThreadsEnum::AnyHiPriThreadHiPriTask) final override
	{
		//TODO check if(InEvent);
		bool bAnyPending = true;
		if (Tasks.size() < 8) // don't bother to check for completion if there are lots of prereqs...too expensive to check
		{
			bAnyPending = false;
			for (int32 Index = 0; Index < Tasks.size(); Index++)
			{
				GraphEvent* Task = Tasks[Index].GetReference();
				if (Task && !Task->IsComplete())
				{
					bAnyPending = true;
					break;
				}
			}
		}
		if (!bAnyPending)
		{
			InEvent->Trigger();
			return;
		}
		TGraphTask<TriggerEventGraphTask>::CreateTask(&Tasks, CurrentThreadIfKnown).ConstructAndDispatchWhenReady(InEvent, TriggerThread);
	}

	virtual void AddShutdownCallback(std::function<void()>& Callback) override
	{
		ShutdownCallbacks.emplace_back(Callback);
	}

	virtual void WakeNamedThread(NamedThreadsEnum::Type ThreadToWake) override
	{
		const NamedThreadsEnum::Type ThreadIndex = NamedThreadsEnum::GetThreadIndex(ThreadToWake);
		if (ThreadIndex < NumNamedThreads)
		{
			Thread(ThreadIndex).WakeUp(NamedThreadsEnum::GetQueueIndex(ThreadToWake));
		}
	}

	// Scheduling utilities

	void StartTaskThread(int32 Priority, int32 IndexToStart)
	{
		NamedThreadsEnum::Type ThreadToWake = NamedThreadsEnum::Type(IndexToStart + Priority * NumTaskThreadsPerSet + NumNamedThreads);
		((TaskThreadAnyThread&)Thread(ThreadToWake)).WakeUp();
	}
	void StartAllTaskThreads(bool bDoBackgroundThreads)
	{
		for (int32 Index = 0; Index < GetNumWorkerThreads(); Index++)
		{
			for (int32 Priority = 0; Priority < NamedThreadsEnum::NumThreadPriorities; Priority++)
			{
				if (Priority == (NamedThreadsEnum::NormalThreadPriority >> NamedThreadsEnum::ThreadPriorityShift) ||
					(Priority == (NamedThreadsEnum::HighThreadPriority >> NamedThreadsEnum::ThreadPriorityShift) && CreatedHiPriorityThreads) ||
					(Priority == (NamedThreadsEnum::BackgroundThreadPriority >> NamedThreadsEnum::ThreadPriorityShift) && CreatedBackgroundPriorityThreads && bDoBackgroundThreads)
					)
				{
					StartTaskThread(Priority, Index);
				}
			}
		}
	}

	BaseGraphTask* FindWork(NamedThreadsEnum::Type ThreadInNeed) override
	{
		int32 LocalNumWorkingThread = GetNumWorkerThreads() + GNumWorkerThreadsToIgnore;
		int32 MyIndex = int32((uint32(ThreadInNeed) - NumNamedThreads) % NumTaskThreadsPerSet);
		int32 Priority = int32((uint32(ThreadInNeed) - NumNamedThreads) / NumTaskThreadsPerSet);
		//TODO check if (MyIndex >= 0 && MyIndex < LocalNumWorkingThread && Priority >= 0 && Priority < NamedThreadsEnum::NumThreadPriorities);

		return IncomingAnyThreadTasks[Priority].Pop(MyIndex, true);
	}

	void StallForTuning(int32 Index, bool Stall) override
	{
		for (int32 Priority = 0; Priority < NamedThreadsEnum::NumThreadPriorities; Priority++)
		{
			NamedThreadsEnum::Type ThreadToWake = NamedThreadsEnum::Type(Index + Priority * NumTaskThreadsPerSet + NumNamedThreads);
			((TaskThreadAnyThread&)Thread(ThreadToWake)).StallForTuning(Stall);
		}
	}

	void SetTaskThreadPriorities(ThreadPriority Pri)
	{
		//TODO check if (NumTaskThreadSets == 1); // otherwise tuning this doesn't make a lot of sense
		for (int32 ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++)
		{
			if (ThreadIndex > LastExternalThread)
			{
				WorkerThreads[ThreadIndex].RunnableThread->SetThreadPriority(Pri);
			}
		}
	}

private:
	//internals
	
	//internal function to verify an index and return the taskthread
	TaskThreadBase& Thread(int32 Index)
	{
		//check if (Index >= 0 && Index < NumThreads);
		//check if (WorkerThreads[Index].TaskGraphWorker->GetThreadId() == Index);
		return *WorkerThreads[Index].TaskGraphWorker;
	}

	//examines thre tls to determine the identity of the current thread
	NamedThreadsEnum::Type GetCurrentThread()
	{
		NamedThreadsEnum::Type CurrentThreadIfKnown = NamedThreadsEnum::AnyThread;
		WorkerThread* TLSPointer = (WorkerThread*)WindowsPlatformTLS::GetTlsValue(PerThreadIDTLSSlot);
		if (TLSPointer)
		{
			//check if TLSPointer - WorkerThreads >= 0 && TLSPointer - WorkerThreads < NumThreads
			int32 ThreadIndex = static_cast<int32>(TLSPointer - WorkerThreads);
			//check if (Thread(ThreadIndex).GetThreadId() == ThreadIndex)
			if (ThreadIndex < NumNamedThreads)
			{
				CurrentThreadIfKnown = NamedThreadsEnum::Type(ThreadIndex);
			}
			else
			{
				int32 Priority = (ThreadIndex - NumNamedThreads) / NumTaskThreadsPerSet;
				CurrentThreadIfKnown = NamedThreadsEnum::SetPriorities(NamedThreadsEnum::Type(ThreadIndex), Priority, false);
			}
		}
		return CurrentThreadIfKnown;
	}

	int32 ThreadIndexToPriorityIndex(int32 ThreadIndex)
	{
		//@TODO check that ThreadIndex >= NumNamedThreads && ThreadIndex < NumThreads
		int32 Result = (ThreadIndex - NumNamedThreads) / NumTaskThreadsPerSet;
		//@TODO check if Result >= 0 && Result < NumTaskThreadSets
		return Result;
	}



	enum
	{
		// Compile time maximum number of threads.
		MAX_THREADS = 0xFFFF,
		MAX_THREAD_PRIORITIES = 3
	};

	//per thread data
	WorkerThread		WorkerThreads[MAX_THREADS];
	//number of threads actually in use
	int32				NumThreads;
	
	int32				NumNamedThreads;
	//number of tasks threads set s for priority
	int32				NumTaskThreadSets;
	//number of tasks threads per priority set
	int32				NumTaskThreadsPerSet;
	bool				CreatedHiPriorityThreads;
	bool				CreatedBackgroundPriorityThreads;

	//external threads are not created, the thread is created elsewhere and makes an explicit call to run
	//here all of the named threads are external. all unnamed threads must be internal
	NamedThreadsEnum::Type LastExternalThread;
	std::atomic<int32>	ReentrancyCheck;
	//tls slot
	uint32				PerThreadIDTLSSlot;

	//array of callbacks before shutdown
	std::vector<std::function<void()> > ShutdownCallbacks;

	StallingTaskQueue<BaseGraphTask, PLATFORM_CACHE_LINE_SIZE, 2>	IncomingAnyThreadTasks[MAX_THREAD_PRIORITIES];
};

// Implementations of FTaskThread function that require knowledge of FTaskGraphImplementation

BaseGraphTask* TaskThreadAnyThread::FindWork()
{
	/*if (TaskGraphImplementationSingleton)
	{
		return TaskGraphImplementationSingleton->FindWork(ThreadId);
	}
	return nullptr;*/
	return TaskGraphImplementationSingleton->FindWork(ThreadId);
}

// Statics in FTaskGraphInterface

void TaskGraphInterface::Startup(int32 NumThreads)
{
	new TaskGraphImplementation(NumThreads);
}

void TaskGraphInterface::Shutdown()
{
	delete TaskGraphImplementationSingleton;
	TaskGraphImplementationSingleton = nullptr;
}

TaskGraphInterface& TaskGraphInterface::Get()
{
	//check if(TaskGraphImplementationSingleton);
	return *TaskGraphImplementationSingleton;
}

static LockFreeClassAllocator_TLSCache<GraphEvent, PLATFORM_CACHE_LINE_SIZE>& GetGraphEventAllocator()
{
	static LockFreeClassAllocator_TLSCache<GraphEvent, PLATFORM_CACHE_LINE_SIZE> Allocator;
	return Allocator;
}

GraphEventRef GraphEvent::CreateGraphEvent()
{
	GraphEvent* Instance = new(GetGraphEventAllocator().Allocate()) GraphEvent{};

	bool test = Instance;

	return Instance;
}

void GraphEvent::Recycle(GraphEvent* ToRecycle)
{
	GetGraphEventAllocator().Free(ToRecycle);
}

void GraphEvent::DispatchSubsequents(NamedThreadsEnum::Type CurrentThreadIfKnown)
{
	std::vector<BaseGraphTask*> NewTasks;
	DispatchSubsequents(NewTasks, CurrentThreadIfKnown);
}

/**
 * Swap two values.  Assumes the types are trivially relocatable.
 */
template <typename T>
inline void Swap(T& A, T& B)
{
	
		T Temp = std::move(A);
		A = std::move(B);
		B = std::move(Temp);
	
}

void GraphEvent::DispatchSubsequents(std::vector<BaseGraphTask*>& NewTasks, NamedThreadsEnum::Type CurrentThreadIfKnown, bool bInternal/* = false */)
{
	if (EventsToWaitFor.size())
	{
		// need to save this first and empty the actual tail of the task might be recycled faster than it is cleared.
		GraphEventArray TempEventsToWaitFor;
		Swap(EventsToWaitFor, TempEventsToWaitFor);

		bool bSpawnGatherTask = true;


		bSpawnGatherTask = false;
		for (GraphEventRef& Item : TempEventsToWaitFor)
		{
			if (!Item->IsComplete())
			{
				bSpawnGatherTask = true;
				break;
			}
		}


		if (bSpawnGatherTask)
		{
			// create the Gather...this uses a special version of private CreateTask that "assumes" the subsequent list (which other threads might still be adding too).


			NamedThreadsEnum::Type LocalThreadToDoGatherOn = NamedThreadsEnum::AnyHiPriThreadHiPriTask;

			
			//LocalThreadToDoGatherOn = ThreadToDoGatherOn;
			NamedThreadsEnum::Type CurrentThreadIndex = NamedThreadsEnum::GetThreadIndex(CurrentThreadIfKnown);
			if (CurrentThreadIndex <= NamedThreadsEnum::ActualRenderingThread)
			{
				LocalThreadToDoGatherOn = CurrentThreadIndex;
			}

			TGraphTask<NullGraphTask>::CreateTask(GraphEventRef(this), &TempEventsToWaitFor, CurrentThreadIfKnown).ConstructAndDispatchWhenReady(LocalThreadToDoGatherOn);
			return;
			
		}

		bool bWakeUpWorker = false;
		std::vector<BaseGraphTask*> PoppedTasks;
		SubsequentList.PopAllAndClose(PoppedTasks);
		for (BaseGraphTask* NewTask : ReverseIterate(PoppedTasks)) // reverse the order since PopAll is implicitly backwards
		{
			//check if(NewTask);
			NewTask->ConditionalQueueTask(CurrentThreadIfKnown, bWakeUpWorker);
		}

	}
}

GraphEvent::~GraphEvent()
{

	CheckDontCompleteUntilIsEmpty(); // We should not have any wait untils outstanding

}
