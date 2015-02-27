#ifndef PRIMED_FINITE_AUTOMATA_H_INCLUDED
#define PRIMED_FINITE_AUTOMATA_H_INCLUDED

#include <map>
#include <vector>
#include "types.h"

#include <iostream>

template <typename T> class DFAState {
public:
	std::map<UInt, DFAState<T>*> next_states;
	UInt id;
	T data;
	bool terminal;

	DFAState(UInt id) : id(id), terminal(false) {
		return;
	}
};

template <typename T> class NFAState {
public:
	UInt id;
	bool terminal;
	std::map<UInt, std::vector<NFAState<T>*>> next_states;
	NFAState<T>* epsilon;
	T data;

	NFAState(UInt id) : id(id), terminal(false), epsilon(NULL) {
		return;
	}
};

template <typename T> class DFA {
public:
	DFAState<T>* root;
	std::vector<DFAState<T>*> states;

	DFA() {
		this->root = this->new_state();
	}
	~DFA() {
		for (typename std::vector<DFAState<T>*>::iterator it = this->states.begin(); it != this->states.end(); it++) {
			delete *it;
		}
	}
	DFAState<T>* new_state() {
		DFAState<T>* s = new DFAState<T>(this->states.size());
		this->states.push_back(s);
		return s;
	}
};

template <typename T> class NFA {
public:
	NFAState<T>* root;
	std::vector<NFAState<T>*> states;

	NFA() {
		this->root = this->new_state();
	}
	~NFA() {
		for (typename std::vector<NFAState<T>*>::iterator it = this->states.begin(); it != this->states.end(); it++) {
			delete *it;
		}
	}
	NFAState<T>* new_state() {
		NFAState<T>* s = new NFAState<T>(this->states.size());
		this->states.push_back(s);
		return s;
	}
};

#endif /* PRIMED_FINITE_AUTOMATA_H_INCLUDED */
