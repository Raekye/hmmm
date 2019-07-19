#ifndef MIDORI_UTF8_H_INCLUDED
#define MIDORI_UTF8_H_INCLUDED

#include <string>
#include "global.h"

class utf8 {
public:
	static std::string from_codepoint(UInt x) {
		char str[5] = { 0 };
		if (x <= 0x7f) {
			str[0] = x;
		} else if (x <= 0x7ff) {
			str[0] = 0xc0 | ((x >> 6) & 0x1f);
			str[1] = 0x80 | (x & 0x3f);
		} else if (x <= 0xffff) {
			str[0] = 0xe0 | ((x >> 12) & 0x0f);
			str[1] = 0x80 | ((x >> 6) & 0x3f);
			str[2] = 0x80 | (x & 0x3f);
		} else {
			str[0] = 0xf0 | ((x >> 18) & 0x07);
			str[1] = 0x80 | ((x >> 12) & 0x3f);
			str[2] = 0x80 | ((x >> 6) & 0x3f);
			str[3] = 0x80 | (x & 0x3f);
		}
		return std::string(str);
	}
};

#endif /* MIDORI_UTF8_H_INCLUDED */
