#ifndef MIDORI_INTERVAL_TREE_H_INCLUDED
#define MIDORI_INTERVAL_TREE_H_INCLUDED

#include <memory>
#include <utility>
#include <list>
#include <algorithm>
#include "global.h"

template <typename K, typename V> class IntervalTree {
public:
	typedef std::pair<K, K> Interval;
	typedef std::pair<Interval, V> SearchResult;
	typedef std::list<SearchResult> SearchList;

	void insert(Interval, V);
	std::unique_ptr<SearchList> pop(Interval);
	std::unique_ptr<SearchList> find(Interval);
	std::unique_ptr<SearchList> all();

	void invariants();

private:
	class Node {
	public:
		Interval key;
		V value;
		Int height;
		K lower;
		K upper;
		std::unique_ptr<Node> left;
		std::unique_ptr<Node> right;

		static Int _height(Node* n);
		static std::unique_ptr<Node> balance(std::unique_ptr<Node>);
		static std::unique_ptr<Node> rotate(std::unique_ptr<Node>, Int);
		static std::unique_ptr<Node> rotate_cw(std::unique_ptr<Node>);
		static std::unique_ptr<Node> rotate_ccw(std::unique_ptr<Node>);
		static std::unique_ptr<Node> insert(std::unique_ptr<Node>, Interval, V);
		static std::unique_ptr<Node> pop_min(std::unique_ptr<Node>, std::unique_ptr<Node>*);
		static std::unique_ptr<Node> pop(std::unique_ptr<Node>, Interval, std::unique_ptr<Node>*);
		static void find(Node*, Interval, SearchList*);
		static void all(Node*, SearchList*);

		Int balance_factor();
		void update();
		bool contains(K);
		bool contains_recursive(K);
		bool overlaps(Interval);
		bool overlaps_recursive(Interval);

		static void _invariants(Node*);
		void invariants();
	};

	std::unique_ptr<Node> root;
};

#include "interval_tree.impl.h"

#endif /* MIDORI_INTERVAL_TREE_H_INCLUDED */
