#include "gtest/gtest.h"
#include "midori/regex_ast.h"

TEST(RegexTest, Group) {
	std::unique_ptr<RegexASTGroup::RangeList> r1(new RegexASTGroup::RangeList);
	r1->range.first = 100;
	r1->range.second = 116;

	std::unique_ptr<RegexASTGroup> g1(new RegexASTGroup(false, std::move(r1)));
	std::vector<RegexASTGroup::Range> l1;
	g1->flatten(&l1);
	ASSERT_EQ(l1.size(), 1);
	ASSERT_EQ(l1.at(0).first, 100);
	ASSERT_EQ(l1.at(0).second, 116);

	std::unique_ptr<RegexASTGroup::RangeList> r2(new RegexASTGroup::RangeList);
	r2->range.first = 0;
	r2->range.second = 16;
	r2->next = std::move(g1->span);

	std::unique_ptr<RegexASTGroup> g2(new RegexASTGroup(false, std::move(r2)));
	std::vector<RegexASTGroup::Range> l2;
	g2->flatten(&l2);
	ASSERT_EQ(l2.size(), 2);
	ASSERT_EQ(l2.at(0).first, 0);
	ASSERT_EQ(l2.at(0).second, 16);
	ASSERT_EQ(l2.at(1).first, 100);
	ASSERT_EQ(l2.at(1).second, 116);

	std::unique_ptr<RegexASTGroup::RangeList> r3(new RegexASTGroup::RangeList);
	r3->range.first = 200;
	r3->range.second = RegexASTGroup::UNICODE_MAX;
	r3->next = std::move(g2->span);

	std::unique_ptr<RegexASTGroup> g3(new RegexASTGroup(false, std::move(r3)));
	std::vector<RegexASTGroup::Range> l3;
	g3->flatten(&l3);
	ASSERT_EQ(l3.size(), 3);
	ASSERT_EQ(l3.at(0).first, 0);
	ASSERT_EQ(l3.at(0).second, 16);
	ASSERT_EQ(l3.at(1).first, 100);
	ASSERT_EQ(l3.at(1).second, 116);
	ASSERT_EQ(l3.at(2).first, 200);
	ASSERT_EQ(l3.at(2).second, RegexASTGroup::UNICODE_MAX);
}

TEST(RegexTest, GroupMerge) {
	std::vector<std::pair<UInt, UInt>> ranges = {
		std::make_pair(0, 16),
		std::make_pair(0, 4),
		std::make_pair(8, 12),
		std::make_pair(116, 132),
		std::make_pair(100, 124),
		std::make_pair(200, 216),
		std::make_pair(217, 232),
	};
	std::unique_ptr<RegexASTGroup::RangeList> span;
	for (std::pair<UInt, UInt> const& r : ranges) {
		std::unique_ptr<RegexASTGroup::RangeList> s(new RegexASTGroup::RangeList);
		s->range.first = r.first;
		s->range.second = r.second;
		s->next = std::move(span);
		span = std::move(s);
	}

	std::unique_ptr<RegexASTGroup> g(new RegexASTGroup(false, std::move(span)));
	std::vector<RegexASTGroup::Range> l;
	g->flatten(&l);
	ASSERT_EQ(l.size(), 3);
	ASSERT_EQ(l.at(0).first, 0);
	ASSERT_EQ(l.at(0).second, 16);
	ASSERT_EQ(l.at(1).first, 100);
	ASSERT_EQ(l.at(1).second, 132);
	ASSERT_EQ(l.at(2).first, 200);
	ASSERT_EQ(l.at(2).second, 232);
}

TEST(RegexTest, GroupNegate) {
	std::unique_ptr<RegexASTGroup::RangeList> r1(new RegexASTGroup::RangeList);
	r1->range.first = 100;
	r1->range.second = 116;

	std::unique_ptr<RegexASTGroup> g1(new RegexASTGroup(true, std::move(r1)));
	std::vector<RegexASTGroup::Range> l1;
	g1->flatten(&l1);
	ASSERT_EQ(l1.size(), 2);
	ASSERT_EQ(l1.at(0).first, 0);
	ASSERT_EQ(l1.at(0).second, 99);
	ASSERT_EQ(l1.at(1).first, 117);
	ASSERT_EQ(l1.at(1).second, RegexASTGroup::UNICODE_MAX);

	std::unique_ptr<RegexASTGroup::RangeList> r2(new RegexASTGroup::RangeList);
	r2->range.first = 0;
	r2->range.second = 16;
	r2->next = std::move(g1->span);

	std::unique_ptr<RegexASTGroup> g2(new RegexASTGroup(true, std::move(r2)));
	std::vector<RegexASTGroup::Range> l2;
	g2->flatten(&l2);
	ASSERT_EQ(l2.size(), 2);
	ASSERT_EQ(l2.at(0).first, 17);
	ASSERT_EQ(l2.at(0).second, 99);
	ASSERT_EQ(l2.at(1).first, 117);
	ASSERT_EQ(l2.at(1).second, RegexASTGroup::UNICODE_MAX);

	std::unique_ptr<RegexASTGroup::RangeList> r3(new RegexASTGroup::RangeList);
	r3->range.first = 200;
	r3->range.second = RegexASTGroup::UNICODE_MAX;
	r3->next = std::move(g2->span);

	std::unique_ptr<RegexASTGroup> g3(new RegexASTGroup(true, std::move(r3)));
	std::vector<RegexASTGroup::Range> l3;
	g3->flatten(&l3);
	ASSERT_EQ(l3.size(), 2);
	ASSERT_EQ(l3.at(0).first, 17);
	ASSERT_EQ(l3.at(0).second, 99);
	ASSERT_EQ(l3.at(1).first, 117);
	ASSERT_EQ(l3.at(1).second, 199);
}
