#include "finite_automata.h"

#include <iostream>
#include <cstdio>

template <typename T> DFAState<T>::DFAState(UInt id) : id(id), terminal(false) {
	return;
}

template <typename T> NFAState<T>::NFAState(UInt id): id(id), terminal(false), epsilon(NULL) {
	return;
}

template <typename T> DFA<T>::DFA() {
	this->root = this->new_state();
}

template <typename T> DFA<T>::~DFA() {
	for (typename std::vector<DFAState<T>*>::iterator it = this->states.begin(); it != this->states.end(); it++) {
		delete *it;
	}
}

template <typename T> DFAState<T>* DFA<T>::new_state() {
	DFAState<T>* s = new DFAState<T>(this->states.size());
	this->states.push_back(s);
	return s;
}

template <typename T> NFA<T>::NFA() {
	this->root = this->new_state();
}

template <typename T> NFA<T>::~NFA() {
	for (typename std::vector<NFAState<T>*>::iterator it = this->states.begin(); it != this->states.end(); it++) {
		delete *it;
	}
}

template <typename T> NFAState<T>* NFA<T>::new_state() {
	NFAState<T>* s = new NFAState<T>(this->states.size());
	this->states.push_back(s);
	return s;
}

template <typename T> void NFA<T>::to_dfa(DFA<T>* dfa) {
	std::map<UInt, DFAState<T>*> dfa_state_from_nfa_state;
	dfa_state_from_nfa_state[this->root->id] = dfa->root;
	generating_dfa_visit_state(this->root, dfa->root, dfa, &dfa_state_from_nfa_state);
}

template <typename T> void NFA<T>::generating_dfa_visit_state(NFAState<T>* nfa_state, DFAState<T>* dfa_state, DFA<T>* dfa, std::map<UInt, DFAState<T>*>* dfa_state_from_nfa_state) {
	printf("NFA state %u\n", nfa_state->id);
	dfa_state->terminal = nfa_state->terminal;
	for (auto &kv : nfa_state->next_states) {
		DFAState<T>* next_dfa = NULL;
		typename std::map<UInt, DFAState<T>*>::iterator it = dfa_state->next_states.find(kv.first);
		if (it == dfa_state->next_states.end()) {
			for (auto &link : kv.second) {
				typename std::map<UInt, DFAState<T>*>::iterator it = dfa_state_from_nfa_state->find(link->id);
				if (it != dfa_state_from_nfa_state->end()) {
					if (next_dfa == NULL) {
						next_dfa = it->second;
					} else {
						if (next_dfa != it->second) {
							printf("Found differing state %u from first %u\n", it->second->id, next_dfa->id);
						}
					}
				}
			}
			if (next_dfa == NULL) {
				next_dfa = dfa->new_state();
			}
			dfa_state->next_states[kv.first] = next_dfa;
		} else {
			next_dfa = it->second;
		}
		printf("\tFor char %c, use state %u\n", (char) kv.first, next_dfa->id);
		for (auto &link : kv.second) {
			printf("\t\tVisiting NFA state %u\n", link->id);
			if (dfa_state_from_nfa_state->count(link->id) == 0) {
				(*dfa_state_from_nfa_state)[link->id] = next_dfa;
				generating_dfa_visit_state(link, next_dfa, dfa, dfa_state_from_nfa_state);
			}
		}
	}
	if (nfa_state->epsilon != NULL) {
		generating_dfa_visit_state(nfa_state->epsilon, dfa_state, dfa, dfa_state_from_nfa_state);
	}
}

template class DFAState<std::string>;
template class NFAState<std::string>;
template class DFA<std::string>;
template class NFA<std::string>;
