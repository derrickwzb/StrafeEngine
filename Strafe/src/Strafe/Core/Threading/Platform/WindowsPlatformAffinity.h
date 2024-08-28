#pragma once

#define uint64 unsigned long long
#define int8 signed char

#define		MAKEAFFINITYMASK1(x)							((1<<x))
#define		MAKEAFFINITYMASK2(x,y)							((1<<x)+(1<<y))
#define		MAKEAFFINITYMASK3(x,y,z)						((1<<x)+(1<<y)+(1<<z))
#define		MAKEAFFINITYMASK4(w,x,y,z)						((1<<w)+(1<<x)+(1<<y)+(1<<z))
#define		MAKEAFFINITYMASK5(v,w,x,y,z)					((1<<v)+(1<<w)+(1<<x)+(1<<y)+(1<<z))
#define		MAKEAFFINITYMASK6(u,v,w,x,y,z)					((1<<u)+(1<<v)+(1<<w)+(1<<x)+(1<<y)+(1<<z))
#define		MAKEAFFINITYMASK7(t,u,v,w,x,y,z)				((1<<t)+(1<<u)+(1<<v)+(1<<w)+(1<<x)+(1<<y)+(1<<z))
#define		MAKEAFFINITYMASK8(s,t,u,v,w,x,y,z)				((1<<s)+(1<<t)+(1<<u)+(1<<v)+(1<<w)+(1<<x)+(1<<y)+(1<<z))
#define		MAKEAFFINITYMASK9(r,s,t,u,v,w,x,y,z)			((1<<r)+(1<<s)+(1<<t)+(1<<u)+(1<<v)+(1<<w)+(1<<x)+(1<<y)+(1<<z))
#define		MAKEAFFINITYMASK10(q,r,s,t,u,v,w,x,y,z)			((1<<q)+(1<<r)+(1<<s)+(1<<t)+(1<<u)+(1<<v)+(1<<w)+(1<<x)+(1<<y)+(1<<z))
#define		MAKEAFFINITYMASK11(p,q,r,s,t,u,v,w,x,y,z)		((1<<p)+(1<<q)+(1<<r)+(1<<s)+(1<<t)+(1<<u)+(1<<v)+(1<<w)+(1<<x)+(1<<y)+(1<<z))
#define		MAKEAFFINITYMASK12(o,p,q,r,s,t,u,v,w,x,y,z)		((1<<o)+(1<<p)+(1<<q)+(1<<r)+(1<<s)+(1<<t)+(1<<u)+(1<<v)+(1<<w)+(1<<x)+(1<<y)+(1<<z))
#define		MAKEAFFINITYMASK13(n,o,p,q,r,s,t,u,v,w,x,y,z)	((1<<n)+(1<<o)+(1<<p)+(1<<q)+(1<<r)+(1<<s)+(1<<t)+(1<<u)+(1<<v)+(1<<w)+(1<<x)+(1<<y)+(1<<z))

enum ThreadPriority
{
	ThreadPri_Normal,
	ThreadPri_AboveNormal,
	ThreadPri_BelowNormal,
	ThreadPri_Highest,
	ThreadPri_Lowest,
	ThreadPri_SlightlyBelowNormal,
	ThreadPri_TimeCritical,
	ThreadPri_Num,
};
enum class ThreadCreateFlags : int8
{
	None = 0,
	SMTExclusive = (1 << 0),
};


class WindowsPlatformAffinity
{
	public:
		static const uint64 GetMainGameMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetRenderingThreadMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetRHIThreadMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetRHIFrameOffsetThreadMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetRTHeartBeatMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetPoolThreadMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetTaskGraphThreadMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetAudioRenderThreadMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetNoAffinityMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetTaskGraphBackgroundTaskMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetTaskGraphHighPriorityTaskMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetAsyncLoadingThreadMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetIoDispatcherThreadMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const uint64 GetTraceThreadMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		// @todo what do we think about having this as a function in this class? Should be make a whole new one? 
		// scrap it and force the priority like before?
		static ThreadPriority GetRenderingThreadPriority()
		{
			return ThreadPri_Normal;
		}

		static ThreadCreateFlags GetRenderingThreadFlags()
		{
			return ThreadCreateFlags::None;
		}

		static ThreadPriority GetRHIThreadPriority()
		{
			return ThreadPri_Normal;
		}

		static ThreadPriority GetGameThreadPriority()
		{
			return ThreadPri_Normal;
		}

		static ThreadCreateFlags GetRHIThreadFlags()
		{
			return ThreadCreateFlags::None;
		}

		static ThreadPriority GetTaskThreadPriority()
		{
			return ThreadPri_SlightlyBelowNormal;
		}

		static ThreadPriority GetTaskBPThreadPriority()
		{
			return ThreadPri_BelowNormal;
		}
};