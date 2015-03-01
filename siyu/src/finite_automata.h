#ifndef SIYU_FINITE_AUTOMATA_H_INCLUDED
#define SIYU_FINITE_AUTOMATA_H_INCLUDED

#include <map>
#include <vector>
#include <set>
#include <memory>
#include "global.h"

template <typename T> class DFAState {
public:
	UInt id;
	bool terminal;
	std::map<UInt, DFAState<T>*> next_states;
	T data;

	DFAState(UInt);
};

template <typename T> class NFAState {
public:
	UInt id;
	bool terminal;
	std::map<UInt, std::vector<NFAState<T>*>> next_states;
	NFAState<T>* epsilon;
	T data;

	NFAState(UInt);
	void generate_epsilon_star_into(std::set<NFAState<T>*>&);
};

template <typename T> class DFA {
public:
	DFAState<T>* root;
	std::vector<std::unique_ptr<DFAState<T>>> states;

	DFA();
	DFAState<T>* new_state();
};

template <typename T> class NFA {
public:
	NFAState<T>* root;
	std::vector<std::unique_ptr<NFAState<T>>> states;

	NFA();
	NFAState<T>* new_state();
	std::unique_ptr<DFA<T>> to_dfa();
};

#endif /* SIYU_FINITE_AUTOMATA_H_INCLUDED */
