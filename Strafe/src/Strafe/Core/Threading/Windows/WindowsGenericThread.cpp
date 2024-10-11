#include "strafepch.h"
#include "WindowsGenericThread.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformProcess.h"
#include <cstdlib>  // For malloc
#include <cstdint>  // For uint8_t
#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformMisc.h"


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

uint32 WindowsGenericThread::GuardedRun()
{
    uint32 ExitCode = 0;


    //equivalent to windowsplatform process set thread affinity mask function
	if (m_ThreadAffinityMask != WindowsPlatformAffinity::GetNoAffinityMask())
	{
		::SetThreadAffinityMask(::GetCurrentThread(), (DWORD_PTR)m_ThreadAffinityMask);
	}

	WindowsPlatformProcess::SetThreadName(STRTOTCHAR(m_ThreadName));
	//might need to do exception handling but see how it goes
	return ExitCode = Run();
}



uint32 WindowsGenericThread::Run()
{
	uint32 ExitCode = 1;
    //check runnable valid

    if (m_Runnable->Init() == true)
    {
        ThreadInitSyncEvent->Trigger();

		//setup tls for this thread . to be used by tls auto cleanup objects
        SetTls();

		ExitCode = m_Runnable->Run();

        //allow any allocated resources to be cleaned up
		m_Runnable->Exit();

        ClearTls();


        
    }
    else
    {
        //initialization has failed , release the sync event
		ThreadInitSyncEvent->Trigger();
    }
    return ExitCode;
}


////todo move this to appropriate place
//static constexpr  unsigned int CountBits(unsigned long long Bits)
//{
//    // https://en.wikipedia.org/wiki/Hamming_weight
//    Bits -= (Bits >> 1) & 0x5555555555555555ull;
//    Bits = (Bits & 0x3333333333333333ull) + ((Bits >> 2) & 0x3333333333333333ull);
//    Bits = (Bits + (Bits >> 4)) & 0x0f0f0f0f0f0f0f0full;
//    return (Bits * 0x0101010101010101) >> 56;
//}
//
///** Returns lower value in a generic way */
//template< class T >
//static constexpr  T Min(const T A, const T B)
//{
//    return (A < B) ? A : B;
//}
//
//void QueryCpuInformation(
//    ProcessorGroupDesc& OutGroupDesc,
//    uint32_t& OutNumaNodeCount,
//    uint32_t& OutCoreCount,
//    uint32_t& OutLogicalProcessorCount,
//    bool bForceSingleNumaNode = false)
//{
//    GROUP_AFFINITY FilterGroupAffinity = {};
//    if (bForceSingleNumaNode)
//    {
//        PROCESSOR_NUMBER ProcessorNumber = {};
//        USHORT NodeNumber = 0;
//
//        GetThreadIdealProcessorEx(GetCurrentThread(), &ProcessorNumber);
//        GetNumaProcessorNodeEx(&ProcessorNumber, &NodeNumber);
//        GetNumaNodeProcessorMaskEx(NodeNumber, &FilterGroupAffinity);
//    }
//
//    OutNumaNodeCount = OutCoreCount = OutLogicalProcessorCount = 0;
//    uint8_t* BufferPtr = nullptr;
//    DWORD BufferBytes = 0;
//
//    // Initial call to get the buffer size
//    if (false == GetLogicalProcessorInformationEx(RelationAll, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)BufferPtr, &BufferBytes))
//    {
//        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
//        {
//            BufferPtr = reinterpret_cast<uint8_t*>(malloc(BufferBytes));
//
//            // Retrieve the processor information
//            if (GetLogicalProcessorInformationEx(RelationAll, reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(BufferPtr), &BufferBytes))
//            {
//                uint8_t* InfoPtr = BufferPtr;
//
//                while (InfoPtr < BufferPtr + BufferBytes)
//                {
//                    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ProcessorInfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)InfoPtr;
//
//                    if (nullptr == ProcessorInfo)
//                    {
//                        break;
//                    }
//
//                    if (ProcessorInfo->Relationship == RelationProcessorCore)
//                    {
//                        if (bForceSingleNumaNode)
//                        {
//                            for (int GroupIdx = 0; GroupIdx < ProcessorInfo->Processor.GroupCount; ++GroupIdx)
//                            {
//                                if (FilterGroupAffinity.Group == ProcessorInfo->Processor.GroupMask[GroupIdx].Group)
//                                {
//                                    KAFFINITY Intersection = FilterGroupAffinity.Mask & ProcessorInfo->Processor.GroupMask[GroupIdx].Mask;
//
//                                    if (Intersection > 0)
//                                    {
//                                        OutCoreCount++;
//                                        OutLogicalProcessorCount += CountBits(Intersection);
//                                    }
//                                }
//                            }
//                        }
//                        else
//                        {
//                            OutCoreCount++;
//
//                            for (int GroupIdx = 0; GroupIdx < ProcessorInfo->Processor.GroupCount; ++GroupIdx)
//                            {
//                                OutLogicalProcessorCount += CountBits(ProcessorInfo->Processor.GroupMask[GroupIdx].Mask);
//                            }
//                        }
//                    }
//                    if (ProcessorInfo->Relationship == RelationNumaNode)
//                    {
//                        OutNumaNodeCount++;
//                    }
//                    if (ProcessorInfo->Relationship == RelationGroup)
//                    {
//                        OutGroupDesc.NumProcessorGroups = Min<uint16_t>(ProcessorGroupDesc::MaxNumProcessorGroups, ProcessorInfo->Group.ActiveGroupCount);
//                        for (int GroupIndex = 0; GroupIndex < OutGroupDesc.NumProcessorGroups; GroupIndex++)
//                        {
//                            OutGroupDesc.ThreadAffinities[GroupIndex] = ProcessorInfo->Group.GroupInfo[GroupIndex].ActiveProcessorMask;
//                        }
//                    }
//
//                    InfoPtr += ProcessorInfo->Size;
//                }
//            }
//
//            free(BufferPtr);
//        }
//    }
//}
//
//
//
//ProcessorGroupDesc NumberOfProcessorGroupsInternal()
//{
//	ProcessorGroupDesc GroupDesc;
//	unsigned int NumaNodeCount = 0;
//	unsigned int NumCores = 0;
//	unsigned int LogicalProcessorCount = 0;
//	QueryCpuInformation(GroupDesc, NumaNodeCount, NumCores, LogicalProcessorCount);
//	return GroupDesc;
//}
//
//const ProcessorGroupDesc& GetProcessorGroupDesc()
//{
//	static ProcessorGroupDesc GroupDesc(NumberOfProcessorGroupsInternal());
//	return GroupDesc;
//}

bool WindowsGenericThread::SetThreadAffinityMask(const ThreadAffinity& affinity)
{
    const ProcessorGroupDesc& ProcessorGroups = GetProcessorGroupDesc();
    uint32 CpuGroupCount = ProcessorGroups.NumProcessorGroups;
    //todo check that affinity processor group is lesser than cpu group count

    GROUP_AFFINITY GroupAffinity = {};
    GROUP_AFFINITY PreviousGroupAffinity = {};
    GroupAffinity.Mask = affinity.m_ThreadAffinityMask & ProcessorGroups.ThreadAffinities[affinity.m_ProcessorGroup];
    GroupAffinity.Group = affinity.m_ProcessorGroup;
    if (SetThreadGroupAffinity(m_ThreadHandle, &GroupAffinity, &PreviousGroupAffinity) == 0)
    {
        DWORD LastError = GetLastError();
        //todo log error
        return  false;
    }
    m_ThreadAffinityMask = affinity.m_ThreadAffinityMask;
    return PreviousGroupAffinity.Mask != GroupAffinity.Mask || PreviousGroupAffinity.Group != GroupAffinity.Group;
}