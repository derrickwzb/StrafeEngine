#include "strafepch.h"
#include "WindowsGenericThread.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformProcess.h"


int WindowsGenericThread::TranslateThreadPriority(ThreadPriority priority)
{
	static_assert(ThreadPri_Num == 7, "Need to add another enum for new ThreadPri");

		switch(priority)
		{
		case ThreadPri_AboveNormal:
			return THREAD_PRIORITY_ABOVE_NORMAL;
		case ThreadPri_BelowNormal:
			return THREAD_PRIORITY_BELOW_NORMAL;
		case ThreadPri_Highest:
			return THREAD_PRIORITY_HIGHEST;
		case ThreadPri_Lowest:
			return THREAD_PRIORITY_LOWEST;
		case ThreadPri_Normal:
			return THREAD_PRIORITY_NORMAL;
		case ThreadPri_TimeCritical:
			return THREAD_PRIORITY_HIGHEST;

		default:
			return THREAD_PRIORITY_NORMAL; //todo: log error
		}

}

unsigned int WindowsGenericThread::GuardedRun()
{
	unsigned int ExitCode = 0;

	if (m_ThreadAffinityMask != WindowsPlatformAffinity::GetNoAffinityMask())
	{
		::SetThreadAffinityMask(::GetCurrentThread(), (DWORD_PTR)m_ThreadAffinityMask);
	}
	//might need to do exception handling but see how it goes
	return ExitCode = Run();
}