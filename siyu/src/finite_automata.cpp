#include "finite_automata.h"
#include <list>
#include <cassert>

#include <iostream>
#include <cstdio>


#pragma mark - Local types
template <typename T> using NFAGroupedState = std::set<NFAState<T>*>;

#pragma mark - DFAState
template <typename T> DFAState<T>::DFAState(UInt id) : id(id), terminal(false) {
	return;
}

#pragma mark - NFAState
template <typename T> NFAState<T>::NFAState(UInt id): id(id), terminal(false), epsilon(nullptr) {
	return;
}

template <typename T> void NFAState<T>::generate_epsilon_star_into(std::set<NFAState<T>*>& states) {
	NFAState<T>* curr = this;
	while (curr->epsilon != nullptr && states.insert(curr->epsilon).second) {
		curr = curr->epsilon;
	}
}

#pragma mark - DFA
template <typename T> DFA<T>::DFA() {
	this->root = this->new_state();
}

template <typename T> DFAState<T>* DFA<T>::new_state() {
	std::unique_ptr<DFAState<T>> ptr(new DFAState<T>(this->states.size()));
	DFAState<T>* state = ptr.get();
	this->states.push_back(std::move(ptr));
	return state;
}

#pragma mark - NFA
template <typename T> NFA<T>::NFA() {
	this->root = this->new_state();
}

template <typename T> NFAState<T>* NFA<T>::new_state() {
	std::unique_ptr<NFAState<T>> ptr(new NFAState<T>(this->states.size()));
	NFAState<T>* state = ptr.get();
	this->states.push_back(std::move(ptr));
	return state;
}

template <typename T> std::unique_ptr<DFA<T>> NFA<T>::to_dfa() {
	std::unique_ptr<DFA<T>> dfa(new DFA<T>());

	std::map<NFAGroupedState<T>, DFAState<T>*> generated_states;
	std::list<NFAGroupedState<T>> unmarked_grouped_states;

	auto register_state = [&generated_states, &unmarked_grouped_states](NFAGroupedState<T>& nfa_grouped_state, DFAState<T>* dfa_state) -> void {
		for (auto& nfa_state : nfa_grouped_state) {
			if (nfa_state->terminal) {
				if (dfa_state->terminal && dfa_state->data != nfa_state->data) {
					throw std::runtime_error("Ambiguous NFA");
				} else {
					dfa_state->terminal = true;
					dfa_state->data = nfa_state->data;
				}
			}
		}
		generated_states[nfa_grouped_state] = dfa_state;
		unmarked_grouped_states.push_back(nfa_grouped_state);
	};

	NFAGroupedState<T> start;
	start.insert(this->root);

	register_state(start, dfa->root);

	while (unmarked_grouped_states.size() > 0) {
		NFAGroupedState<T> curr = unmarked_grouped_states.front();
		DFAState<T>* curr_dfa_state = generated_states[curr];
		unmarked_grouped_states.pop_front();

		std::map<UInt, NFAGroupedState<T>> next_grouped_states;
		for (auto& nfa_state : curr) {
			for (auto& kv : nfa_state->next_states) {
				UInt ch = kv.first;
				for (auto& next_state : kv.second) {
					next_grouped_states[ch].insert(next_state);
					next_state->generate_epsilon_star_into(next_grouped_states[ch]);
				}
			}
		}
		for (auto& kv : next_grouped_states) {
			DFAState<T>* next_dfa_state = nullptr;
			typename std::map<NFAGroupedState<T>, DFAState<T>*>::iterator it = generated_states.find(kv.second);
			if (it == generated_states.end()) {
				next_dfa_state = dfa->new_state();
				register_state(kv.second, next_dfa_state);
			} else {
				next_dfa_state = it->second;
			}
			assert(curr_dfa_state->next_states.find(kv.first) == curr_dfa_state->next_states.end());
			curr_dfa_state->next_states[kv.first] = next_dfa_state;
		}
	}

	return dfa;
}

#pragma mark - Finite automata templates
template class DFAState<std::string>;
template class NFAState<std::string>;
template class DFA<std::string>;
template class NFA<std::string>;
