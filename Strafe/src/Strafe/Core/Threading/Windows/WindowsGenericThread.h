#pragma once

#include "Windows.h"
#include "Strafe/Core/Threading/Platform/WindowsPlatformAffinity.h"
#include "Strafe/Core/Threading/GenericThread.h"
#include "Strafe/Core/Threading/Runnable.h"

class Runnable;
class WindowsGenericThread : public GenericThread
{
	//THE THREAD HANDLE FOR THE THREAD
	HANDLE m_ThreadHandle = 0;

	static ::DWORD WINAPI _ThreadProc(::LPVOID lpParameter)
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

		virtual void SetThreadPriority(ThreadPriority priority) override
		{
			m_ThreadPriority = priority;
			::SetThreadPriority(m_ThreadHandle, TranslateThreadPriority(priority));
		}

		virtual void Suspend(bool ShouldPause = true) override
		{
			if (ShouldPause == true)
			{
				SuspendThread(m_ThreadHandle);
			}
			else
			{
				ResumeThread(m_ThreadHandle);
			}
		}

		virtual bool Kill(bool bShouldWait = false) override
		{
			//todo check if thread is valid
			bool bDidExitOK = true;

			// Let the runnable have a chance to stop without brute force killing
			if (m_Runnable)
			{
				m_Runnable->Stop();
			}

			if (bShouldWait == true)
			{
				// Wait indefinitely for the thread to finish.  IMPORTANT:  It's not safe to just go and
				// kill the thread with TerminateThread() as it could have a mutex lock that's shared
				// with a thread that's continuing to run, which would cause that other thread to
				// dead-lock.  
				//
				// This can manifest itself in code as simple as the synchronization
				// object that is used by our logging output classes

				WaitForSingleObject(m_ThreadHandle, INFINITE);
			}

			CloseHandle(m_ThreadHandle);
			m_ThreadHandle = NULL;

			return bDidExitOK;
		}


		virtual void WaitForCompletion() override
		{
			WaitForSingleObject(m_ThreadHandle, INFINITE);
		}

		virtual bool SetThreadAffinityMask(const ThreadAffinity& Affinity)override;

		static int TranslateThreadPriority(ThreadPriority priority);

	protected:
		virtual bool CreateInternal(Runnable* InRunnable
			, const std::string* ThreadName
			, unsigned int InStackSize = 0
			, ThreadPriority InThreadPri = ThreadPri_Normal
			, unsigned long long InThreadAffinityMask = 0
			, ThreadCreateFlags InCreateFlags = ThreadCreateFlags::None) override
		{
			static bool RunOnce = false;

			if (!RunOnce)
			{
				RunOnce = true;
				::SetThreadPriority(::GetCurrentThread(), TranslateThreadPriority(ThreadPri_Normal));
			}

			m_Runnable = InRunnable;
			m_ThreadAffinityMask = InThreadAffinityMask;

			//todo create sync event to guarantee the init function is called first
			//ThreadInitSyncEvent = FPlatformProcess::GetSynchEventFromPool(true);

			m_ThreadName = ThreadName->c_str() ? ThreadName->c_str() : "UnnamedThread";
			m_ThreadPriority = InThreadPri;

			//create new thread here
			{
				m_ThreadHandle = CreateThread(NULL, InStackSize, _ThreadProc, this, STACK_SIZE_PARAM_IS_A_RESERVATION | CREATE_SUSPENDED, (::DWORD*)&m_ThreadId);
			}

			// if it fails, clean up the corpse
			if (m_ThreadHandle == NULL)
			{
				m_Runnable = nullptr;
			}
			else
			{
				ResumeThread(m_ThreadHandle);

				//startup thread
				//todo threadsyncinitevent
				//ThreadInitSyncEvent->Wait(INFINITE);

				m_ThreadPriority = ThreadPri_Normal;
				SetThreadPriority(InThreadPri);
			}


			//// Cleanup the sync event
			//FPlatformProcess::ReturnSynchEventToPool(ThreadInitSyncEvent);
			//ThreadInitSyncEvent = nullptr;
			return m_ThreadHandle != NULL;

		}



		//virtual void SetThreadPriority(ThreadPriority priority)override
		//{
		//	threadPriority = priority;
		//}
	
};

