#pragma once
#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include <type_traits>
#include <atomic>

//a virtual interface for ref counted objects to implement
class RefCountedObject
{
public:
	virtual ~RefCountedObject() {};
	virtual uint32 AddRef() const = 0;
	virtual uint32 Release() const = 0;
	virtual uint32 GetRefCount() const = 0;
};

//base class implementing thread safe ref counting
class RefCountBase
{
public:
	RefCountBase() = default;
	virtual ~RefCountBase() = default;

	RefCountBase(const RefCountBase& Rhs) = delete;
	RefCountBase& operator=(const RefCountBase& Rhs) = delete;

	inline uint32 AddRef() const
	{
	}
	
};
