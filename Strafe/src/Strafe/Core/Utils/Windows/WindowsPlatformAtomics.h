#pragma once

#define FORCEINLINE __forceinline	

#include "Strafe/Core/Utils/Windows/WindowsPlatformTypes.h"
#include <intrin.h>

/**
 * Windows implementation of the Atomics OS functions
 */
struct WindowsPlatformAtomics
{
	static_assert(sizeof(int8) == sizeof(char) && alignof(int8) == alignof(char), "int8 must be compatible with char");
	static_assert(sizeof(int16) == sizeof(short) && alignof(int16) == alignof(short), "int16 must be compatible with short");
	static_assert(sizeof(int32) == sizeof(long) && alignof(int32) == alignof(long), "int32 must be compatible with long");
	static_assert(sizeof(int64) == sizeof(long long) && alignof(int64) == alignof(long long), "int64 must be compatible with long long");

	static FORCEINLINE int8 cInterlockedIncrement(volatile int8* Value)
	{
		return (int8)::_InterlockedExchangeAdd8((char*)Value, 1) + 1;
	}

	static FORCEINLINE int16 cInterlockedIncrement(volatile int16* Value)
	{
		return (int16)::_InterlockedIncrement16((short*)Value);
	}

	static FORCEINLINE int32 cInterlockedIncrement(volatile int32* Value)
	{
		return (int32)::_InterlockedIncrement((long*)Value);
	}

	static FORCEINLINE int64 cInterlockedIncrement(volatile int64* Value)
	{

		return (int64)::_InterlockedIncrement64((long long*)Value);

	}

	static FORCEINLINE int8 cInterlockedDecrement(volatile int8* Value)
	{
		return (int8)::_InterlockedExchangeAdd8((char*)Value, -1) - 1;
	}

	static FORCEINLINE int16 cInterlockedDecrement(volatile int16* Value)
	{
		return (int16)::_InterlockedDecrement16((short*)Value);
	}

	static FORCEINLINE int32 cInterlockedDecrement(volatile int32* Value)
	{
		return (int32)::_InterlockedDecrement((long*)Value);
	}

	static FORCEINLINE int64 cInterlockedDecrement(volatile int64* Value)
	{

		return (int64)::_InterlockedDecrement64((long long*)Value);

	}

	static FORCEINLINE int8 cInterlockedAdd(volatile int8* Value, int8 Amount)
	{
		return (int8)::_InterlockedExchangeAdd8((char*)Value, (char)Amount);
	}

	static FORCEINLINE int16 cInterlockedAdd(volatile int16* Value, int16 Amount)
	{
		return (int16)::_InterlockedExchangeAdd16((short*)Value, (short)Amount);
	}

	static FORCEINLINE int32 cInterlockedAdd(volatile int32* Value, int32 Amount)
	{
		return (int32)::_InterlockedExchangeAdd((long*)Value, (long)Amount);
	}

	static FORCEINLINE int64 cInterlockedAdd(volatile int64* Value, int64 Amount)
	{

		return (int64)::_InterlockedExchangeAdd64((int64*)Value, (int64)Amount);

	}

	static FORCEINLINE int8 cInterlockedExchange(volatile int8* Value, int8 Exchange)
	{
		return (int8)::_InterlockedExchange8((char*)Value, (char)Exchange);
	}

	static FORCEINLINE int16 cInterlockedExchange(volatile int16* Value, int16 Exchange)
	{
		return (int16)::_InterlockedExchange16((short*)Value, (short)Exchange);
	}

	static FORCEINLINE int32 cInterlockedExchange(volatile int32* Value, int32 Exchange)
	{
		return (int32)::_InterlockedExchange((long*)Value, (long)Exchange);
	}

	static FORCEINLINE int64 cInterlockedExchange(volatile int64* Value, int64 Exchange)
	{

		return (int64)::_InterlockedExchange64((long long*)Value, (long long)Exchange);

	}

	static FORCEINLINE void* cInterlockedExchangePtr(void* volatile* Dest, void* Exchange)
	{


		return ::_InterlockedExchangePointer(Dest, Exchange);
	}

	static FORCEINLINE int8 cInterlockedCompareExchange(volatile int8* Dest, int8 Exchange, int8 Comparand)
	{
		return (int8)::_InterlockedCompareExchange8((char*)Dest, (char)Exchange, (char)Comparand);
	}

	static FORCEINLINE int16 cInterlockedCompareExchange(volatile int16* Dest, int16 Exchange, int16 Comparand)
	{
		return (int16)::_InterlockedCompareExchange16((short*)Dest, (short)Exchange, (short)Comparand);
	}

	static FORCEINLINE int32 cInterlockedCompareExchange(volatile int32* Dest, int32 Exchange, int32 Comparand)
	{
		return (int32)::_InterlockedCompareExchange((long*)Dest, (long)Exchange, (long)Comparand);
	}

	static FORCEINLINE int64 cInterlockedCompareExchange(volatile int64* Dest, int64 Exchange, int64 Comparand)
	{


		return (int64)::_InterlockedCompareExchange64(Dest, Exchange, Comparand);
	}

	static FORCEINLINE int8 cInterlockedAnd(volatile int8* Value, const int8 AndValue)
	{
		return (int8)::_InterlockedAnd8((volatile char*)Value, (char)AndValue);
	}

	static FORCEINLINE int16 cInterlockedAnd(volatile int16* Value, const int16 AndValue)
	{
		return (int16)::_InterlockedAnd16((volatile short*)Value, (short)AndValue);
	}

	static FORCEINLINE int32 cInterlockedAnd(volatile int32* Value, const int32 AndValue)
	{
		return (int32)::_InterlockedAnd((volatile long*)Value, (long)AndValue);
	}

	static FORCEINLINE int64 cInterlockedAnd(volatile int64* Value, const int64 AndValue)
	{

		return (int64)::_InterlockedAnd64((volatile long long*)Value, (long long)AndValue);

	}

	static FORCEINLINE int8 cInterlockedOr(volatile int8* Value, const int8 OrValue)
	{
		return (int8)::_InterlockedOr8((volatile char*)Value, (char)OrValue);
	}

	static FORCEINLINE int16 cInterlockedOr(volatile int16* Value, const int16 OrValue)
	{
		return (int16)::_InterlockedOr16((volatile short*)Value, (short)OrValue);
	}

	static FORCEINLINE int32 cInterlockedOr(volatile int32* Value, const int32 OrValue)
	{
		return (int32)::_InterlockedOr((volatile long*)Value, (long)OrValue);
	}

	static FORCEINLINE int64 cInterlockedOr(volatile int64* Value, const int64 OrValue)
	{

		return (int64)::_InterlockedOr64((volatile long long*)Value, (long long)OrValue);

	}

	static FORCEINLINE int8 cInterlockedXor(volatile int8* Value, const int8 XorValue)
	{
		return (int8)::_InterlockedXor8((volatile char*)Value, (char)XorValue);
	}

	static FORCEINLINE int16 cInterlockedXor(volatile int16* Value, const int16 XorValue)
	{
		return (int16)::_InterlockedXor16((volatile short*)Value, (short)XorValue);
	}

	static FORCEINLINE int32 cInterlockedXor(volatile int32* Value, const int32 XorValue)
	{
		return (int32)::_InterlockedXor((volatile long*)Value, (int32)XorValue);
	}

	static FORCEINLINE int64 cInterlockedXor(volatile int64* Value, const int64 XorValue)
	{

		return (int64)::_InterlockedXor64((volatile long long*)Value, (long long)XorValue);

	}

	static FORCEINLINE int8 AtomicRead(volatile const int8* Src)
	{
		return cInterlockedCompareExchange((int8*)Src, 0, 0);
	}

	static FORCEINLINE int16 AtomicRead(volatile const int16* Src)
	{
		return cInterlockedCompareExchange((int16*)Src, 0, 0);
	}

	static FORCEINLINE int32 AtomicRead(volatile const int32* Src)
	{
		return cInterlockedCompareExchange((int32*)Src, 0, 0);
	}

	static FORCEINLINE int64 AtomicRead(volatile const int64* Src)
	{
		return cInterlockedCompareExchange((int64*)Src, 0, 0);
	}

	static FORCEINLINE int8 AtomicRead_Relaxed(volatile const int8* Src)
	{
		return *Src;
	}

	static FORCEINLINE int16 AtomicRead_Relaxed(volatile const int16* Src)
	{
		return *Src;
	}

	static FORCEINLINE int32 AtomicRead_Relaxed(volatile const int32* Src)
	{
		return *Src;
	}

	static FORCEINLINE int64 AtomicRead_Relaxed(volatile const int64* Src)
	{

		return *Src;

	}

	static FORCEINLINE void AtomicStore(volatile int8* Src, int8 Val)
	{
		cInterlockedExchange(Src, Val);
	}

	static FORCEINLINE void AtomicStore(volatile int16* Src, int16 Val)
	{
		cInterlockedExchange(Src, Val);
	}

	static FORCEINLINE void AtomicStore(volatile int32* Src, int32 Val)
	{
		cInterlockedExchange(Src, Val);
	}

	static FORCEINLINE void AtomicStore(volatile int64* Src, int64 Val)
	{
		cInterlockedExchange(Src, Val);
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int8* Src, int8 Val)
	{
		*Src = Val;
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int16* Src, int16 Val)
	{
		*Src = Val;
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int32* Src, int32 Val)
	{
		*Src = Val;
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int64* Src, int64 Val)
	{

		* Src = Val;

	}

	


	static FORCEINLINE void* cInterlockedCompareExchangePointer(void* volatile* Dest, void* Exchange, void* Comparand)
	{
		return ::_InterlockedCompareExchangePointer(Dest, Exchange, Comparand);
	}



};




