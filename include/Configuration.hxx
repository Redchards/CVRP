#pragma once

#include <cstdint>

#include <Platform.hxx>

typedef uint64_t size_type;

typedef uint16_t flag_type;

#define stringify(x) stringify_(x)
#define stringify_(x) #x

enum class Endianness : flag_type
{
	little,
	big
};

enum class PageType : uint8_t
{
	ReadOnly,
	Writable
};