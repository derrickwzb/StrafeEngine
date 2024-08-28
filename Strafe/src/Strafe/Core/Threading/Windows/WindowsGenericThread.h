#pragma once

#include "Windows.h"
#include "Strafe/Core/Threading/Platform/WindowsPlatformAffinity.h"
class WindowsGenericThread
{
	//THE THREAD HANDLE FOR THE THREAD
	HANDLE m_ThreadHandle = 0;

	static ::DWORD WINAPI ThreadProc(::LPVOID lpParameter)
	{
		auto* thread = (WindowsGenericThread*)lpParameter;
		/*return thread->m_ThreadFunction();*/
	}

	unsigned int GuardedRun();


	unsigned int Run();

	public:
		~WindowsGenericThread()
		{
			if (m_ThreadHandle)
			{
			//kill
			}
		}

		/*virtual void SetThreadPriority(ThreadPriority priority)override
		{
			threadPriority = priority;
		}*/
	
};

