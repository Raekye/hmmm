#ifndef PRIMED_FINITE_AUTOMATA_H_INCLUDED
#define PRIMED_FINITE_AUTOMATA_H_INCLUDED

#include <map>
#include <vector>
#include "types.h"

template <typename T> class DFAState {
public:
	std::map<UInt, DFAState<T>*> next_states;
	UInt id;
	T data;
	bool terminal;

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
};

template <typename T> class DFA {
public:
	DFAState<T>* root;
	std::vector<DFAState<T>*> states;

	DFA();
	~DFA();
	DFAState<T>* new_state();
};

template <typename T> class NFA {
public:
	NFAState<T>* root;
	std::vector<NFAState<T>*> states;

	NFA();
	~NFA();
	NFAState<T>* new_state();
	void to_dfa(DFA<T>*);
private:
	void generating_dfa_visit_state(NFAState<T>*, DFAState<T>*, DFA<T>*, std::map<UInt, DFAState<T>*>*);
};

#endif /* PRIMED_FINITE_AUTOMATA_H_INCLUDED */
