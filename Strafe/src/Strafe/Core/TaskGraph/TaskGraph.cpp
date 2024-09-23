#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include "Strafe/Core/TaskGraph/TaskGraphInterface.h"

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
class FTaskGraphImplementation;
struct FWorkerThread;

//static FTaskGraphInterface* TaskGraphImplementationSingleton = NULL;