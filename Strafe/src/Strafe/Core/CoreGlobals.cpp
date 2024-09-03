#include"strafepch.h"
#include "CoreGlobals.h"

unsigned int GameThreadID = 0;
unsigned int RenderThreadID = 0;
unsigned int AudioThreadID = 0;
unsigned int PhysXThreadID = 0;
unsigned int UILoadingThreadID = 0;

bool IsGameThreadIdInitialized = false;