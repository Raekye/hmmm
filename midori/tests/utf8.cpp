#include "gtest/gtest.h"
#include "midori/utf8.h"

class Utf8Test : public ::testing::Test {
};

TEST_F(Utf8Test, Main) {
	std::vector<UInt> chars = { 0x0f, 0xff, 0xffff, 0x0fffff };
	for (UInt const ch : chars) {
		ASSERT_EQ(utf8::codepoint_from_string(utf8::string_from_codepoint(ch), 0, nullptr), ch);
	}
}
