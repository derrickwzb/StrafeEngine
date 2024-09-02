#pragma once

#include "WindowsApiAbstractions.h"

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

