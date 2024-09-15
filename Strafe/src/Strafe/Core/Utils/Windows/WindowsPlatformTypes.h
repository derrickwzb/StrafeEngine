#pragma once

typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef unsigned short int uint16;
typedef unsigned char uint8;
typedef signed long long int64;
typedef signed int int32;
typedef signed short int16;
typedef signed char int8;
typedef unsigned char byte;

#define STRTOTCHAR(x)  L"x"
#include <tchar.h>
#include <string>
#include <locale>
#include <codecvt>

static std::string TCharToStdString(const TCHAR* tcharStr) {

	// TCHAR is wchar_t, convert to std::string using wide-to-narrow conversion
	std::wstring wstr(tcharStr);
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}