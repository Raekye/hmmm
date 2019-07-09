#ifndef MIDORI_GLOBAL_H_INCLUDED
#define MIDORI_GLOBAL_H_INCLUDED

#include <cstdint>
#include <cstddef>
#include <type_traits>

typedef int8_t Byte;
typedef uint8_t UByte;
typedef int16_t Short;
typedef uint16_t UShort;
typedef int32_t Int;
typedef uint32_t UInt;
typedef int64_t Long;
typedef uint64_t ULong;

static_assert(sizeof(Byte) == 1, "sanity");
static_assert(sizeof(Short) == 2, "sanity");
static_assert(sizeof(Int) == 4, "sanity");
static_assert(sizeof(Long) == 8, "sanity");
static_assert(sizeof(size_t) == 8, "sanity");
static_assert(std::is_same<Int, int>::value, "sanity");
static_assert(std::is_same<ULong, size_t>::value, "sanity");
static_assert(!std::is_same<Long, size_t>::value, "sanity");

#endif /* MIDORI_GLOBAL_H_INCLUDED */
