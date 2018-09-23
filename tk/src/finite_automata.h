#ifndef TK_FINITE_AUTOMATA_H_INCLUDED
#define TK_FINITE_AUTOMATA_H_INCLUDED

#include <map>
#include <vector>
#include <set>
#include <memory>
#include "global.h"

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

#endif /* TK_FINITE_AUTOMATA_H_INCLUDED */
