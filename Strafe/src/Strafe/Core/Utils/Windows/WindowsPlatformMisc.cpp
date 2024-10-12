#include "strafepch.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformMisc.h"


void QueryCpuInformation(
    ProcessorGroupDesc& OutGroupDesc,
    uint32& OutNumaNodeCount,
    uint32& OutCoreCount,
    uint32& OutLogicalProcessorCount,
    bool bForceSingleNumaNode = false)
{
    GROUP_AFFINITY FilterGroupAffinity = {};
    if (bForceSingleNumaNode)
    {
        PROCESSOR_NUMBER ProcessorNumber = {};
        USHORT NodeNumber = 0;

        GetThreadIdealProcessorEx(GetCurrentThread(), &ProcessorNumber);
        GetNumaProcessorNodeEx(&ProcessorNumber, &NodeNumber);
        GetNumaNodeProcessorMaskEx(NodeNumber, &FilterGroupAffinity);
    }

    OutNumaNodeCount = OutCoreCount = OutLogicalProcessorCount = 0;
    uint8* BufferPtr = nullptr;
    DWORD BufferBytes = 0;

    // Initial call to get the buffer size
    if (false == GetLogicalProcessorInformationEx(RelationAll, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)BufferPtr, &BufferBytes))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            BufferPtr = reinterpret_cast<uint8*>(malloc(BufferBytes));

            // Retrieve the processor information
            if (GetLogicalProcessorInformationEx(RelationAll, reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(BufferPtr), &BufferBytes))
            {
                uint8* InfoPtr = BufferPtr;

                while (InfoPtr < BufferPtr + BufferBytes)
                {
                    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ProcessorInfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)InfoPtr;

                    if (nullptr == ProcessorInfo)
                    {
                        break;
                    }

                    if (ProcessorInfo->Relationship == RelationProcessorCore)
                    {
                        if (bForceSingleNumaNode)
                        {
                            for (int GroupIdx = 0; GroupIdx < ProcessorInfo->Processor.GroupCount; ++GroupIdx)
                            {
                                if (FilterGroupAffinity.Group == ProcessorInfo->Processor.GroupMask[GroupIdx].Group)
                                {
                                    KAFFINITY Intersection = FilterGroupAffinity.Mask & ProcessorInfo->Processor.GroupMask[GroupIdx].Mask;

                                    if (Intersection > 0)
                                    {
                                        OutCoreCount++;
                                        OutLogicalProcessorCount += CountBits(Intersection);
                                    }
                                }
                            }
                        }
                        else
                        {
                            OutCoreCount++;

                            for (int GroupIdx = 0; GroupIdx < ProcessorInfo->Processor.GroupCount; ++GroupIdx)
                            {
                                OutLogicalProcessorCount += CountBits(ProcessorInfo->Processor.GroupMask[GroupIdx].Mask);
                            }
                        }
                    }
                    if (ProcessorInfo->Relationship == RelationNumaNode)
                    {
                        OutNumaNodeCount++;
                    }
                    if (ProcessorInfo->Relationship == RelationGroup)
                    {
                        OutGroupDesc.NumProcessorGroups = Min<uint16>(ProcessorGroupDesc::MaxNumProcessorGroups, ProcessorInfo->Group.ActiveGroupCount);
                        for (int GroupIndex = 0; GroupIndex < OutGroupDesc.NumProcessorGroups; GroupIndex++)
                        {
                            OutGroupDesc.ThreadAffinities[GroupIndex] = ProcessorInfo->Group.GroupInfo[GroupIndex].ActiveProcessorMask;
                        }
                    }

                    InfoPtr += ProcessorInfo->Size;
                }
            }

            free(BufferPtr);
        }
    }
}



ProcessorGroupDesc NumberOfProcessorGroupsInternal()
{
    ProcessorGroupDesc GroupDesc;
    unsigned int NumaNodeCount = 0;
    unsigned int NumCores = 0;
    unsigned int LogicalProcessorCount = 0;
    QueryCpuInformation(GroupDesc, NumaNodeCount, NumCores, LogicalProcessorCount);
    return GroupDesc;
}

const ProcessorGroupDesc& WindowsPlatformMisc::GetProcessorGroupDesc()
{
    static ProcessorGroupDesc GroupDesc(NumberOfProcessorGroupsInternal());
    return GroupDesc;
}

 void WindowsPlatformMisc::GetConfiguredCoreLimits(int32 PlatformNumPhysicalCores, int32 PlatformNumLogicalCores,
    bool& bOutFullyInitialized, int32& OutPhysicalCoreLimit, int32& OutLogicalCoreLimit,
    bool& bOutSetPhysicalCountToLogicalCount)
{
    //for now it defaults everything to 0 but it can be changed according to user needs or system requirements
    int32 PhysicalCoreLimit = 0;
    int32 LogicalCoreLimit = 0;
    int32 LegacyCoreLimit = 0;
    bool bSetPhysicalCountToLogicalCount = false;

    if (bSetPhysicalCountToLogicalCount)
    {
        LogicalCoreLimit = PhysicalCoreLimit;
    }
    else
    {
        LogicalCoreLimit = PlatformNumPhysicalCores > 0 ?
            (PhysicalCoreLimit * PlatformNumLogicalCores) / PlatformNumPhysicalCores :
            PhysicalCoreLimit;
    }
    if (LegacyCoreLimit > 0)
    {
        PhysicalCoreLimit = PhysicalCoreLimit == 0 ? LegacyCoreLimit : Min(PhysicalCoreLimit, LegacyCoreLimit);
        LogicalCoreLimit = LogicalCoreLimit == 0 ? LegacyCoreLimit : Min(LogicalCoreLimit, LegacyCoreLimit);
    }

    bOutFullyInitialized = true;
    OutPhysicalCoreLimit = PhysicalCoreLimit;
    OutLogicalCoreLimit = LogicalCoreLimit;
    bOutSetPhysicalCountToLogicalCount = bSetPhysicalCountToLogicalCount;
}


int32 WindowsPlatformMisc::NumberOfCoresIncludingHyperthreads()
{
    static int32 CoreCount = 0;
    if (CoreCount > 0)
    {
        return CoreCount;
    }

    ProcessorGroupDesc GroupDesc;
    uint32 NumaNodeCount = 0;
    uint32 NumCores = 0;
    uint32 LogicalProcessorCount = 0;
    QueryCpuInformation(GroupDesc, NumaNodeCount, NumCores, LogicalProcessorCount);

    bool bLimitsInitialized;
    int32 PhysicalCoreLimit;
    int32 LogicalCoreLimit;
    bool bSetPhysicalCountToLogicalCount;
    GetConfiguredCoreLimits(NumCores, LogicalProcessorCount, bLimitsInitialized, PhysicalCoreLimit,
        LogicalCoreLimit, bSetPhysicalCountToLogicalCount);

    CoreCount = LogicalProcessorCount;

    // Optionally limit number of threads (we don't necessarily scale super well with very high core counts)
    if (LogicalCoreLimit > 0)
    {
        CoreCount = Min(CoreCount, LogicalCoreLimit);
    }

    return CoreCount;
}
int32 WindowsPlatformMisc::NumberOfCores()
{
    static int32 CoreCount = 0;
    if (CoreCount > 0)
    {
        return CoreCount;
    }

    ProcessorGroupDesc GroupDesc;
    uint32 NumaNodeCount = 0;
    uint32 NumCores = 0;
    uint32 LogicalProcessorCount = 0;
    QueryCpuInformation(GroupDesc, NumaNodeCount, NumCores, LogicalProcessorCount);

    bool bLimitsInitialized;
    int32 PhysicalCoreLimit;
    int32 LogicalCoreLimit;
    bool bSetPhysicalCountToLogicalCount;
    GetConfiguredCoreLimits(NumCores, LogicalProcessorCount, bLimitsInitialized, PhysicalCoreLimit,
        LogicalCoreLimit, bSetPhysicalCountToLogicalCount);

    CoreCount = bSetPhysicalCountToLogicalCount ? LogicalProcessorCount : NumCores;

    // Optionally limit number of threads (we don't necessarily scale super well with very high core counts)
    if (PhysicalCoreLimit > 0)
    {
        CoreCount = Min(CoreCount, PhysicalCoreLimit);
    }

    return CoreCount;
}

int32  WindowsPlatformMisc::NumberOfWorkerThreadsToSpawn()
{
    static int32 MaxGameThreads = 4;

    int32 MaxThreads = 16;

    int32 NumberOfCores = WindowsPlatformMisc::NumberOfCores();
    int32 MaxWorkerThreadsWanted = MaxThreads;
    // need to spawn at least two worker thread (see FTaskGraphImplementation)
    std::cout << Max(Min(NumberOfCores - 1, MaxWorkerThreadsWanted), 2) << std::endl;
    return Max(Min(NumberOfCores - 1, MaxWorkerThreadsWanted), 2);
}
