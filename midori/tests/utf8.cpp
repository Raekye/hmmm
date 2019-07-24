#include "gtest/gtest.h"
#include "midori/utf8.h"
#include "midori/lexer.h"

TEST(Utf8Test, Main) {
	std::vector<UInt> chars = { 0x0f, 0xff, 0xffff, 0x0fffff };
	for (UInt const ch : chars) {
		std::string str = utf8::string_from_codepoint(ch);
		ASSERT_EQ(utf8::codepoint_from_string(str, 0, nullptr), ch);
		std::stringstream ss;
		ss << str;
		FileInputStream fis(&ss);
		ASSERT_EQ(fis.get(), ch);
	}
}
