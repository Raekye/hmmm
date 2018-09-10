#ifndef TK_GLOBAL_H_INCLUDED
#define TK_GLOBAL_H_INCLUDED

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
typedef std::size_t SizeT;

static_assert(sizeof(Byte) == 1, "sanity");
static_assert(sizeof(Short) == 2, "sanity");
static_assert(sizeof(Int) == 4, "sanity");
static_assert(sizeof(Long) == 8, "sanity");
static_assert(sizeof(SizeT) == 8, "sanity");
static_assert(std::is_same<ULong, SizeT>::value, "sanity");
static_assert(!std::is_same<Long, SizeT>::value, "sanity");
static_assert(std::is_same<int, Int>::value, "sanity");

#endif /* TK_GLOBAL_H_INCLUDED */
