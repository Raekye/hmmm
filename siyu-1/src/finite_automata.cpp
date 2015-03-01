#include "finite_automata.h"

#include <cstdio>

DFAState::DFAState() : terminal(false) {
	return;
}

DFA::DFA() {
	this->root = this->new_state();
}

DFAState* DFA::new_state() {
	std::unique_ptr<DFAState> state(new DFAState());
	DFAState* ptr = state.get();
	this->id_from_state[ptr] = this->states.size();
	this->states.push_back(std::move(state));
	return ptr;
}

UInt DFA::id(DFAState* state) {
	return this->id_from_state.at(state);
}
