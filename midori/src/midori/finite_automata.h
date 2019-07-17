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

	UInt const id;
	bool terminal;
	std::string data;
	RegexDFAState* _transitions[OPTIMIZED_CHARS];
	IntervalTree<UInt, RegexDFAState*> transitions;

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
	RegexNFAState* epsilon;

	RegexNFAState(UInt);
	void add(Interval, RegexNFAState*);
};

class RegexDFA {
public:
	std::vector<std::unique_ptr<RegexDFAState>> states;
	RegexDFAState* root;

	RegexDFA();
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
};

template <typename K, typename T> class DFAState {
public:
	UInt id;
	bool terminal;
	std::map<K, DFAState<K, T>*> next_states;
	std::map<K, DFAState<K, T>*> not_states;
	T data;

	DFAState(UInt);
};

template <typename K, typename T> class NFAState {
public:
	UInt id;
	bool terminal;
	std::map<K, std::vector<NFAState<K, T>*>> next_states;
	std::map<K, std::vector<NFAState<K, T>*>> not_states;
	NFAState<K, T>* epsilon;
	T data;

	NFAState(UInt);
	void generate_epsilon_star_into(std::set<NFAState<K, T>*>&);
};

template <typename K, typename T> class DFA {
public:
	DFAState<K, T>* root;
	std::vector<std::unique_ptr<DFAState<K, T>>> states;

	DFA();
	DFAState<K, T>* new_state();
};

template <typename K, typename T> class NFA {
public:
	NFAState<K, T>* root;
	std::vector<std::unique_ptr<NFAState<K, T>>> states;

	NFA();
	void reset();
	NFAState<K, T>* new_state();
	std::unique_ptr<DFA<K, T>> to_dfa();
};

#endif /* MIDORI_FINITE_AUTOMATA_H_INCLUDED */
