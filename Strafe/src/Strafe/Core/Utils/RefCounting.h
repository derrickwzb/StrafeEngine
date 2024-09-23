#pragma once
#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformAtomics.h"
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
		return uint32(WindowsPlatformAtomics::InterlockedIncrement(&NumRefs));
	}
	inline uint32 Release() const
	{
		const int32 Refs = WindowsPlatformAtomics::InterlockedDecrement(&NumRefs);
		if (Refs == 0)
		{
			delete this;
		}

		return uint32(Refs);
	}

	uint32 GetRefCount() const
	{
		return uint32(NumRefs);
	}
private:
	mutable int32 NumRefs = 0;
};

/**
 * The base class of reference counted objects.
 *
 * This class should not be used for new code as it does not use atomic operations to update
 * the reference count.
 *
 */
class RefCountedObject
{
public:
	RefCountedObject() : NumRefs(0) {}
	virtual ~RefCountedObject() { /*@todo check if (!NumRefs);*/ }
	RefCountedObject(const RefCountedObject& Rhs) = delete;
	RefCountedObject& operator=(const RefCountedObject& Rhs) = delete;
	uint32 AddRef() const
	{
		return uint32(++NumRefs);
	}
	uint32 Release() const
	{
		uint32 Refs = uint32(--NumRefs);
		if (Refs == 0)
		{
			delete this;
		}
		return Refs;
	}
	uint32 GetRefCount() const
	{
		return uint32(NumRefs);
	}
private:
	mutable int32 NumRefs;
};


/**
 * Like FRefCountedObject, but internal ref count is thread safe
 */
class ThreadSafeRefCountedObject
{
public:
	ThreadSafeRefCountedObject() : NumRefs(0) {}
	virtual ~ThreadSafeRefCountedObject() { /*@todo check if (NumRefs.GetValue() == 0*/ }
	ThreadSafeRefCountedObject(const ThreadSafeRefCountedObject& Rhs) = delete;
	ThreadSafeRefCountedObject& operator=(const ThreadSafeRefCountedObject& Rhs) = delete;
	uint32 AddRef() const
	{
		return uint32(WindowsPlatformAtomics::InterlockedIncrement(&NumRefs)) // Increment and fetch the new value);
	}
	uint32 Release() const
	{
		uint32 Refs = uint32(WindowsPlatformAtomics::InterlockedDecrement(&NumRefs));  // Decrement and fetch the new value);
		if (Refs == 0)
		{
			delete this;
		}
		return Refs;
	}
	uint32 GetRefCount() const
	{
		return uint32(WindowsPlatformAtomics::AtomicRead(&NumRefs));
	}
private:
	/** Thread-safe counter */
	mutable volatile int32 NumRefs;
};

/**
 * RefCountingMode is used select between either 'fast' or 'thread safe' ref-counting types.
 * This is only used by templates at compile time to generate one code path or another.
 */
enum class RefCountingModeEnum : uint8
{
	/** Forced to be not thread-safe. */
	NotThreadSafe = 0,

	/** Thread-safe, never spin locks, but slower */
	ThreadSafe = 1
};