#include "finite_automata.h"
#include <list>
#include <cassert>

#include <iostream>
#include <cstdio>


#pragma mark - Local types
template <typename K, typename T> using NFAGroupedState = std::set<NFAState<K, T>*>;

#pragma mark - DFAState
template <typename K, typename T> DFAState<K, T>::DFAState(UInt id) : id(id), terminal(false) {
	return;
}

#pragma mark - NFAState
template <typename K, typename T> NFAState<K, T>::NFAState(UInt id): id(id), terminal(false), epsilon(nullptr) {
	return;
}

template <typename K, typename T> void NFAState<K, T>::generate_epsilon_star_into(std::set<NFAState<K, T>*>& states) {
	NFAState<K, T>* curr = this;
	while (curr->epsilon != nullptr && states.insert(curr->epsilon).second) {
		curr = curr->epsilon;
	}
}

#pragma mark - DFA
template <typename K, typename T> DFA<K, T>::DFA() {
	this->root = this->new_state();
}

template <typename K, typename T> DFAState<K, T>* DFA<K, T>::new_state() {
	std::unique_ptr<DFAState<K, T>> ptr(new DFAState<K, T>(this->states.size()));
	DFAState<K, T>* state = ptr.get();
	this->states.push_back(std::move(ptr));
	return state;
}

#pragma mark - NFA
template <typename K, typename T> NFA<K, T>::NFA() {
	this->root = this->new_state();
}

template <typename K, typename T> void NFA<K, T>::reset() {
	typename std::vector<std::unique_ptr<NFAState<K, T>>>::iterator it = this->states.begin();
	std::advance(it, 1);
	this->states.erase(it, this->states.end());
}

template <typename K, typename T> NFAState<K, T>* NFA<K, T>::new_state() {
	std::unique_ptr<NFAState<K, T>> ptr(new NFAState<K, T>(this->states.size()));
	NFAState<K, T>* state = ptr.get();
	this->states.push_back(std::move(ptr));
	return state;
}

template <typename K, typename T> std::unique_ptr<DFA<K, T>> NFA<K, T>::to_dfa() {
	std::unique_ptr<DFA<K, T>> dfa(new DFA<K, T>());

	std::map<NFAGroupedState<K, T>, DFAState<K, T>*> generated_states;
	std::list<NFAGroupedState<K, T>> unmarked_grouped_states;

	auto register_state = [&generated_states, &unmarked_grouped_states](NFAGroupedState<K, T>& nfa_grouped_state, DFAState<K, T>* dfa_state) -> void {
		for (auto& nfa_state : nfa_grouped_state) {
			if (nfa_state->terminal) {
				if (dfa_state->terminal && dfa_state->data != nfa_state->data) {
					fprintf(stderr, "Warn: ambiguous NFA terminal at state %u conflicts with %u.\n", dfa_state->id, nfa_state->id);
				} else {
					dfa_state->terminal = true;
					dfa_state->data = nfa_state->data;
				}
			}
		}
		generated_states[nfa_grouped_state] = dfa_state;
		unmarked_grouped_states.push_back(nfa_grouped_state);
	};

	NFAGroupedState<K, T> start;
	start.insert(this->root);
	this->root->generate_epsilon_star_into(start);

	register_state(start, dfa->root);

	while (unmarked_grouped_states.size() > 0) {
		NFAGroupedState<K, T> curr = unmarked_grouped_states.front();
		DFAState<K, T>* curr_dfa_state = generated_states[curr];
		unmarked_grouped_states.pop_front();

		std::map<K, NFAGroupedState<K, T>> next_grouped_states;
		for (auto& nfa_state : curr) {
			for (auto& kv : nfa_state->next_states) {
				for (auto& next_state : kv.second) {
					next_grouped_states[kv.first].insert(next_state);
					next_state->generate_epsilon_star_into(next_grouped_states[kv.first]);
				}
			}
		}
		for (auto& kv : next_grouped_states) {
			DFAState<K, T>* next_dfa_state = nullptr;
			// TODO: check unmarked_grouped_states?
			typename std::map<NFAGroupedState<K, T>, DFAState<K, T>*>::iterator it = generated_states.find(kv.second);
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
template class DFAState<UInt, std::string>;
template class NFAState<UInt, std::string>;
template class DFA<UInt, std::string>;
template class NFA<UInt, std::string>;
