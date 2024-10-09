#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include "Strafe/Core/TaskGraph/TaskGraphInterface.h"
#include "Strafe/Core/Threading/Runnable.h"

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