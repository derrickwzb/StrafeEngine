#pragma once


/*-----------------------------------------------------------------------------
	Global variables. (Threading)
-----------------------------------------------------------------------------*/

extern unsigned int GameThreadID;
extern unsigned int RenderThreadID;
extern unsigned int AudioThreadID;
extern unsigned int PhysXThreadID;
extern unsigned int UILoadingThreadID;

extern bool IsGameThreadIdInitialized;