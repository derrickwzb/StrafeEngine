#pragma once

#include "WindowsApiAbstractions.h"


// Critical section interface for windows platform for locking purposes
class WindowsCriticalSection
{
public:
	WindowsCriticalSection(const WindowsCriticalSection&) = delete;
	WindowsCriticalSection& operator=(const WindowsCriticalSection&) = delete;

	void Lock();
	void Unlock();
private:
// windows specific critical section
	CRITICAL_SECTION m_CriticalSection;
};

