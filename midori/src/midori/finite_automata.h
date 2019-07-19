#ifndef MIDORI_FINITE_AUTOMATA_H_INCLUDED
#define MIDORI_FINITE_AUTOMATA_H_INCLUDED

#include <map>
#include <vector>
#include <set>
#include <memory>
#include "global.h"
#include "interval_tree.h"

class RegexDFAState {
public:
	static UInt const OPTIMIZED_CHARS = 128;

	typedef IntervalTree<UInt, RegexDFAState*> Tree;

	UInt const id;
	std::vector<std::string> terminals;
	RegexDFAState* _transitions[OPTIMIZED_CHARS];
	Tree transitions;

	RegexDFAState(UInt);
};

class RegexNFAState {
public:
	typedef std::vector<RegexNFAState*> StateList;
	typedef IntervalTree<UInt, StateList> UnicodeIntervalTree;
	typedef UnicodeIntervalTree::Interval Interval;

	UInt const id;
	bool terminal;
	std::string data;
	StateList _transitions[RegexDFAState::OPTIMIZED_CHARS];
	UnicodeIntervalTree transitions;
	std::vector<RegexNFAState*> epsilon;

	RegexNFAState(UInt);
	void add(Interval, RegexNFAState*);
};

class RegexDFA {
public:
	std::vector<std::unique_ptr<RegexDFAState>> states;

	RegexDFAState* root();
	RegexDFAState* new_state();
};

class RegexNFA {
public:
	std::vector<std::unique_ptr<RegexNFAState>> states;
	RegexNFAState* root;

	RegexNFA();
	void reset();
	RegexNFAState* new_state();
	std::unique_ptr<RegexDFA> to_dfa();

private:
	typedef std::set<RegexNFAState*> GroupedNFAState;

	void generate_star(RegexNFAState*, GroupedNFAState&);
	void register_state(GroupedNFAState*);
};

#endif /* MIDORI_FINITE_AUTOMATA_H_INCLUDED */
