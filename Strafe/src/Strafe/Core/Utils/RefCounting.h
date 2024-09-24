#pragma once
#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include "Strafe/Core/Utils/Windows/WindowsPlatformAtomics.h"
#include <type_traits>
#include <atomic>
#include <utility>

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

/**
 * A smart pointer to an object which implements AddRef/Release.
 */
template<typename ReferencedType>
class RefCountPtr
{
	typedef ReferencedType* ReferenceType;

public:

	FORCEINLINE RefCountPtr() :
		Reference(nullptr)
	{ }

	RefCountPtr(ReferencedType* InReference, bool bAddRef = true)
	{
		Reference = InReference;
		if (Reference && bAddRef)
		{
			Reference->AddRef();
		}
	}

	RefCountPtr(const RefCountPtr& Copy)
	{
		Reference = Copy.Reference;
		if (Reference)
		{
			Reference->AddRef();
		}
	}

	template<typename CopyReferencedType>
	explicit RefCountPtr(const RefCountPtr<CopyReferencedType>& Copy)
	{
		Reference = static_cast<ReferencedType*>(Copy.GetReference());
		if (Reference)
		{
			Reference->AddRef();
		}
	}

	FORCEINLINE RefCountPtr(TRefCountPtr&& Move)
	{
		Reference = Move.Reference;
		Move.Reference = nullptr;
	}

	template<typename MoveReferencedType>
	explicit RefCountPtr(RefCountPtr<MoveReferencedType>&& Move)
	{
		Reference = static_cast<ReferencedType*>(Move.GetReference());
		Move.Reference = nullptr;
	}

	~RefCountPtr()
	{
		if (Reference)
		{
			Reference->Release();
		}
	}

	RefCountPtr& operator=(ReferencedType* InReference)
	{
		if (Reference != InReference)
		{
			// Call AddRef before Release, in case the new reference is the same as the old reference.
			ReferencedType* OldReference = Reference;
			Reference = InReference;
			if (Reference)
			{
				Reference->AddRef();
			}
			if (OldReference)
			{
				OldReference->Release();
			}
		}
		return *this;
	}

	FORCEINLINE RefCountPtr& operator=(const RefCountPtr& InPtr)
	{
		return *this = InPtr.Reference;
	}

	template<typename CopyReferencedType>
	FORCEINLINE RefCountPtr& operator=(const RefCountPtr<CopyReferencedType>& InPtr)
	{
		return *this = InPtr.GetReference();
	}

	RefCountPtr& operator=(RefCountPtr&& InPtr)
	{
		if (this != &InPtr)
		{
			ReferencedType* OldReference = Reference;
			Reference = InPtr.Reference;
			InPtr.Reference = nullptr;
			if (OldReference)
			{
				OldReference->Release();
			}
		}
		return *this;
	}

	template<typename MoveReferencedType>
	RefCountPtr& operator=(RefCountPtr<MoveReferencedType>&& InPtr)
	{
		// InPtr is a different type (or we would have called the other operator), so we need not test &InPtr != this
		ReferencedType* OldReference = Reference;
		Reference = InPtr.Reference;
		InPtr.Reference = nullptr;
		if (OldReference)
		{
			OldReference->Release();
		}
		return *this;
	}

	FORCEINLINE ReferencedType* operator->() const
	{
		return Reference;
	}

	FORCEINLINE operator ReferenceType() const
	{
		return Reference;
	}

	FORCEINLINE ReferencedType** GetInitReference()
	{
		*this = nullptr;
		return &Reference;
	}

	FORCEINLINE ReferencedType* GetReference() const
	{
		return Reference;
	}

	FORCEINLINE friend bool IsValidRef(const RefCountPtr& InReference)
	{
		return InReference.Reference != nullptr;
	}

	FORCEINLINE bool IsValid() const
	{
		return Reference != nullptr;
	}

	FORCEINLINE void SafeRelease()
	{
		*this = nullptr;
	}

	uint32 GetRefCount()
	{
		uint32 Result = 0;
		if (Reference)
		{
			Result = Reference->GetRefCount();
			check(Result > 0); // you should never have a zero ref count if there is a live ref counted pointer (*this is live)
		}
		return Result;
	}

	FORCEINLINE void Swap(RefCountPtr& InPtr) // this does not change the reference count, and so is faster
	{
		ReferencedType* OldReference = Reference;
		Reference = InPtr.Reference;
		InPtr.Reference = OldReference;
	}

	void Serialize(FArchive& Ar)
	{
		ReferenceType PtrReference = Reference;
		Ar << PtrReference;
		if (Ar.IsLoading())
		{
			*this = PtrReference;
		}
	}

private:

	ReferencedType* Reference;

	template <typename OtherType>
	friend class RefCountPtr;

public:
	FORCEINLINE bool operator==(const RefCountPtr& B) const
	{
		return GetReference() == B.GetReference();
	}

	FORCEINLINE bool operator==(ReferencedType* B) const
	{
		return GetReference() == B;
	}
};

//This defines a C++20 concept named BoolIdentityConcept. A concept is a way of specifying constraints that a template parameter must satisfy. Here, BoolIdentityConcept ensures that a boolean constant B is true.
template <bool B>
concept BoolIdentityConcept = B;
//The requires keyword in C++20 specifies constraints in template functions.
//!!(__VA_ARGS__)is a way to cast any condition to a boolean value(true or false).
//BoolIdentityConcept<true> ensures the concept always resolves to true (probably to enforce compile - time checks).
//MakeRefCount is only instantiated if the condition provided in the macro is satisfied.

//if T is not an array , then the concept is satisfied.
//then it will compile
#define REQUIRED(...) > requires (!!(__VA_ARGS__)) && BoolIdentityConcept<true
template <
	typename T,
	typename... TArgs
	REQUIRED(!std::is_array_v<T>)
>


FORCEINLINE RefCountPtr<T> MakeRefCount(TArgs&&... Args)
{
	return RefCountPtr<T>(new T(Forward<TArgs>(Args)...));
}