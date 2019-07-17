#include "finite_automata.h"
#include <list>
#include <algorithm>

#include <iostream>
#include <cstdio>

UInt const RegexDFAState::OPTIMIZED_CHARS;

RegexDFAState::RegexDFAState(UInt id) : id(id) {
	return;
}

RegexNFAState::RegexNFAState(UInt id) : id(id) {
	return;
}

void RegexNFAState::add(Interval i, RegexNFAState* s) {
	UInt lower = i.first;
	if (i.first < RegexDFAState::OPTIMIZED_CHARS) {
		for (UInt j = 0; j < std::min(i.second, RegexDFAState::OPTIMIZED_CHARS); j++) {
			this->_transitions[j].push_back(s);
		}
		if (i.second >= RegexDFAState::OPTIMIZED_CHARS) {
			lower = RegexDFAState::OPTIMIZED_CHARS;
		}
	}
	std::unique_ptr<UnicodeIntervalTree::SearchList> overlaps = this->transitions.pop(Interval(lower, i.second));
	for (UnicodeIntervalTree::SearchList::iterator it = overlaps->begin(); it != overlaps->end(); it++) {
		UInt a = (*it).first.first;
		UInt b = (*it).first.second;
		UInt c = std::max(a, lower);
		UInt d = std::min(b, i.second);
		StateList cd = (*it).second;
		cd.push_back(s);
		this->transitions.insert(Interval(c, d), cd);
		if (a < lower) {
			this->transitions.insert(Interval(a, lower - 1), (*it).second);
		} else if (a > lower) {
			this->transitions.insert(Interval(lower, a - 1), { s });
		}
		if (b < i.second) {
			this->transitions.insert(Interval(b + 1, i.second), { s });
		} else if (b > i.second) {
			this->transitions.insert(Interval(i.second + 1, b), (*it).second);
		}
	}
}

RegexNFA::RegexNFA() {
	this->root = this->new_state();
}

RegexNFAState* RegexNFA::new_state() {
	std::unique_ptr<RegexNFAState> ptr(new RegexNFAState(this->states.size()));
	RegexNFAState* state = ptr.get();
	this->states.push_back(std::move(ptr));
	return state;
}

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
					// TODO better handle this
					fprintf(stderr, "[warn]: ambiguous NFA terminal at state %u conflicts with %u.\n", dfa_state->id, nfa_state->id);
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
					std::set<NFAState<K, T>*>& states = next_grouped_states[kv.first];
					next_state->generate_epsilon_star_into(states);
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
