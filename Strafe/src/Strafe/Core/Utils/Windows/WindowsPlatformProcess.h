#pragma once
#include "windows.h"
#include "tlhelp32.h"

#define TCHAR_TO_WCHAR(str) (wchar_t*)(str)
#define WCHAR_TO_TCHAR(str) (TCHAR*)(str)
#define TCHAR_TO_ANSI(str)  (char*)StringCast<char>(static_cast<const TCHAR*>(str)).Get()
#define ANSI_TO_TCHAR(str)  (TCHAR*)StringCast<TCHAR>(static_cast<const ANSICHAR*>(str)).Get()
#define TCHAR_TO_UTF8(str)  (char*)FTCHARToUTF8((const TCHAR*)str).Get()
#define UTF8_TO_TCHAR(str)  (TCHAR*)FUTF8ToTCHAR((const ANSICHAR*)str).Get()
#define STRTOTCHAR(x)  L"x"

//windows specific implementation of the process os functions
//implement more generic to support other platforms
struct WindowsPlatformProcess
{
	//windows representation of a interprocess semaphore
	struct WindowsSemaphore
	{
		virtual void Lock();
		virtual bool TryLock(unsigned long long nanosecondstowait);
		virtual void Unlock();

		//returns the os handle
		void* GetSemaphore() { return m_SemaphoreHandle; }

		//constructor
		WindowsSemaphore(const TCHAR* name, HANDLE semaphore);

		//allocation free constructor
		WindowsSemaphore(const wchar_t& name, HANDLE semaphore);

		//destructor
		virtual ~WindowsSemaphore();

		const TCHAR* GetName() const
		{
			return Name;
		}

	protected:

		enum Limits
		{
			MaxSemaphoreName = 128
		};

		//os handle
		HANDLE m_SemaphoreHandle;
		TCHAR Name[MaxSemaphoreName];
	};

	struct ProcEnumInfo;

	//process enumerator
	class ProcEnumerator
	{
	public:
		//constructor
		ProcEnumerator();
		ProcEnumerator(const ProcEnumerator& ) = delete;
		ProcEnumerator& operator=(const ProcEnumerator&) = delete;

		//Destructor
		~ProcEnumerator();
		// get current info

		ProcEnumInfo GetCurrent() const;
		//moves current to the next process
		bool MoveNext();
	private:
		tagPROCESSENTRY32W* CurrentEntry;

		void* SnapshotHandle;
	};

	//process enumeration info struct
	struct ProcEnumInfo
	{
		friend ProcEnumInfo ProcEnumerator::GetCurrent() const;
	public:
		//destructor
		~ProcEnumInfo();
		
		//get the process id
		unsigned int GetPID() const;
		
		//get parent process id
		unsigned int GetParentPID() const;

		//gets process name
		std::string GetProcessName() const;

		//get process full image path
		std::string GetProcessPath() const;
	private:
		ProcEnumInfo(const tagPROCESSENTRY32W* info);

		tagPROCESSENTRY32W* Info;
	};

public:
	static WindowsSemaphore* NewInterprocessSynchObject(const TCHAR* Name, bool bCreate, unsigned int MaxLocks = 1);

	static  void SetThreadName(const TCHAR* ThreadName);
	static  void SetThreadAffinityMask(unsigned int AffinityMask);

};


struct ProcessorGroupDesc
{
	static constexpr unsigned short int MaxNumProcessorGroups = 16;
	unsigned long long ThreadAffinities[MaxNumProcessorGroups] = {};
	unsigned short int NumProcessorGroups = 0;
};