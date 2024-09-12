#pragma once

//Base class for objects in TLS that support auto cleanup
//polymorphically deletes registered instances on thread exiit
class TlsAutoCleanup
{
public:
	//virtual destructor
	virtual ~TlsAutoCleanup() {}

	//register this instance for cleanup
	void Register();

};


//wrapper for values to be stored in TLS that support auto-cleanup
template<class T>
class TlsAutoCleanupValue : public TlsAutoCleanup
{
public:
	//Constructor
	TlsAutoCleanupValue(const T& InValue) : Value(InValue) {}

	//gets the value
	T Get() const { return Value; }

	//sets the value
	void Set(const T& InValue) { Value = InValue; }

private:
	//the value
	T Value;
};