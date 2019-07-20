#ifndef MIDORI_UTF8_H_INCLUDED
#define MIDORI_UTF8_H_INCLUDED

#include <string>
#include "global.h"

class utf8 {
public:
	static std::string string_from_codepoint(UInt x) {
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

	static Long codepoint_from_string(std::string str, UInt pos, UInt* end) {
		if (pos >= str.length()) {
			return -1;
		}
		char a = str.at(pos);
		if ((a & 0x80) == 0) {
			if (end != nullptr) {
				*end = pos + 1;
			}
			return a;
		}
		if (pos + 1 >= str.length()) {
			return -1;
		}
		char b = str.at(pos + 1);
		if ((a & 0xe0) == 0xc0) {
			if (end != nullptr) {
				*end = pos + 2;
			}
			return ((a & 0x1f) << 6) | (b & 0x3f);
		}
		if (pos + 2 >= str.length()) {
			return -1;
		}
		char c = str.at(pos + 2);
		if ((a & 0xf0) == 0xe0) {
			if (end != nullptr) {
				*end = pos + 3;
			}
			return ((a & 0x0f) << 12) | ((b & 0x3f) << 6) | (c & 0x3f);
		}
		if (pos + 3 >= str.length()) {
			return -1;
		}
		char d = str.at(pos + 3);
		if (end != nullptr) {
			*end = pos + 4;
		}
		return ((a & 0x07) << 18) | ((b & 0x3f) << 12) | ((c & 0x3f) << 6) | (d & 0x3f);
	}
};

#endif /* MIDORI_UTF8_H_INCLUDED */
