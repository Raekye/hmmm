#include "gtest/gtest.h"
#include "midori/interval_tree.h"

class IntervalTreeTest : public ::testing::Test {
public:
	typedef IntervalTree<Int, Int> IT;
};

TEST_F(IntervalTreeTest, IsBalanced) {
	IT a;
	for (Int i = 0; i < 1000; i++) {
		a.insert(IT::Interval(i, i + 16), i);
	}
	for (Int i = 0; i < 1000; i++) {
		Int j = 1000 - i;
		a.insert(IT::Interval(j, j + 16), j);
	}
	a.invariants();
}

TEST_F(IntervalTreeTest, Find) {
	IT a;
	for (Int i = 0; i < 1000; i++) {
		a.insert(IT::Interval(i, i + 2), i);
	}
	ASSERT_EQ(a.find(IT::Interval(700, 716))->size(), 19);
	ASSERT_EQ(a.all()->size(), 1000);
}

TEST_F(IntervalTreeTest, Pop) {
	IT a;
	for (Int i = 0; i < 1000; i++) {
		a.insert(IT::Interval(i, i + 16), i);
	}
	ASSERT_EQ(a.pop(IT::Interval(700, 702))->size(), 19);
	ASSERT_EQ(a.all()->size(), 1000 - 19);

	IT b;
	for (Int i = 0; i < 1000; i++) {
		b.insert(IT::Interval(i, i), i);
	}
	ASSERT_EQ(b.pop(IT::Interval(700, 716))->size(), 17);
	ASSERT_EQ(b.all()->size(), 1000 - 17);
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
