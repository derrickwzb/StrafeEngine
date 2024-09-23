#pragma once

#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
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