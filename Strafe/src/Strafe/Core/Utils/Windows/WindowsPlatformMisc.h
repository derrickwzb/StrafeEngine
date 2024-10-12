#pragma once
#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include <cstdlib> 
#include "windows.h"

//todo move this to appropriate place
static constexpr  unsigned int CountBits(unsigned long long Bits)
{
    // https://en.wikipedia.org/wiki/Hamming_weight
    Bits -= (Bits >> 1) & 0x5555555555555555ull;
    Bits = (Bits & 0x3333333333333333ull) + ((Bits >> 2) & 0x3333333333333333ull);
    Bits = (Bits + (Bits >> 4)) & 0x0f0f0f0f0f0f0f0full;
    return (Bits * 0x0101010101010101) >> 56;
}

/** Returns higher value in a generic way */
template< class T >
static constexpr  T Max(const T A, const T B)
{
    return (B < A) ? A : B;
}

/** Returns lower value in a generic way */
template< class T >
static constexpr  T Min(const T A, const T B)
{
    return (A < B) ? A : B;
}

struct ProcessorGroupDesc
{
    static constexpr unsigned short int MaxNumProcessorGroups = 16;
    unsigned long long ThreadAffinities[MaxNumProcessorGroups] = {};
    unsigned short int NumProcessorGroups = 0;
};

class WindowsPlatformMisc
{
public:

    static  const ProcessorGroupDesc& GetProcessorGroupDesc();

    static void GetConfiguredCoreLimits(int32 PlatformNumPhysicalCores, int32 PlatformNumLogicalCores,
        bool& bOutFullyInitialized, int32& OutPhysicalCoreLimit, int32& OutLogicalCoreLimit,
        bool& bOutSetPhysicalCountToLogicalCount);

    static int32 NumberOfCoresIncludingHyperthreads();

    static int32 NumberOfCores();


    static int32 NumberOfWorkerThreadsToSpawn();

};
