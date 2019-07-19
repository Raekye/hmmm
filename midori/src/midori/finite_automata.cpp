#include "finite_automata.h"
#include <list>
#include <algorithm>
#include <cstring>

#include <iostream>
#include <cstdio>

UInt const RegexDFAState::OPTIMIZED_CHARS;

RegexDFAState::RegexDFAState(UInt id) : id(id) {
	std::memset(this->_transitions, 0, RegexDFAState::OPTIMIZED_CHARS * sizeof(RegexDFAState*));
}

RegexNFAState::RegexNFAState(UInt id) : id(id), terminal(false) {
	return;
}

void RegexNFAState::add(Interval i, RegexNFAState* s) {
	UInt last = i.first;
	if (i.first < RegexDFAState::OPTIMIZED_CHARS) {
		UInt bound = std::min(i.second, RegexDFAState::OPTIMIZED_CHARS - 1);
		for (UInt j = i.first; j <= bound; j++) {
			this->_transitions[j].push_back(s);
		}
		last = bound + 1;
	}
	if (last > i.second) {
		return;
	}
	std::unique_ptr<UnicodeIntervalTree::SearchList> overlaps = this->transitions.pop(Interval(last, i.second));
	size_t j = 0;
	for (UnicodeIntervalTree::SearchList::iterator it = overlaps->begin(); it != overlaps->end(); it++) {
		j++;
		UInt a = (*it).first.first;
		UInt b = (*it).first.second;
		UInt c = std::max(a, last);
		UInt d = std::min(b, i.second);
		StateList cd = (*it).second;
		cd.push_back(s);
		this->transitions.insert(Interval(c, d), cd);
		if (a < last) {
			this->transitions.insert(Interval(a, last - 1), (*it).second);
		} else if (a > last) {
			this->transitions.insert(Interval(last, a - 1), { s });
		}
		if (b > i.second) {
			assert(j == overlaps->size());
			this->transitions.insert(Interval(i.second + 1, b), (*it).second);
		}
		last = b + 1;
	}
	if (last != 0 && last <= i.second) {
		this->transitions.insert(Interval(last, i.second), { s });
	}
}

RegexDFAState* RegexDFA::root() {
	return this->states.at(0).get();
}

RegexDFAState* RegexDFA::new_state() {
	std::unique_ptr<RegexDFAState> ptr(new RegexDFAState(this->states.size()));
	RegexDFAState* state = ptr.get();
	this->states.push_back(std::move(ptr));
	return state;
}

RegexNFA::RegexNFA() {
	this->reset();
}

void RegexNFA::reset() {
	this->states.clear();
	this->root = this->new_state();
}

std::unique_ptr<RegexDFA> RegexNFA::to_dfa() {
	std::unique_ptr<RegexDFA> dfa(new RegexDFA);

	std::map<GroupedNFAState, RegexDFAState*> generated_states;
	std::list<GroupedNFAState> unmarked_grouped_states;

	auto register_state = [ &dfa, &generated_states, &unmarked_grouped_states ](GroupedNFAState const& group) -> RegexDFAState* {
		RegexDFAState* dfa_state = dfa->new_state();
		for (RegexNFAState* const s : group) {
			if (s->terminal) {
				dfa_state->terminals.push_back(s->data);
			}
		}
		assert(generated_states.find(group) == generated_states.end());
		generated_states[group] = dfa_state;
		unmarked_grouped_states.push_back(group);
		return dfa_state;
	};

	GroupedNFAState start;
	this->generate_star(this->root, start);
	register_state(start);

	while (!unmarked_grouped_states.empty()) {
		GroupedNFAState curr = unmarked_grouped_states.front();
		unmarked_grouped_states.pop_front();
		RegexDFAState* curr_dfa_state = generated_states.at(curr);

		std::map<UInt, GroupedNFAState> _transitions;
		std::map<RegexNFAState::UnicodeIntervalTree::Interval, GroupedNFAState> transitions;
		for (RegexNFAState* const s : curr) {
			for (UInt i = 0; i < RegexDFAState::OPTIMIZED_CHARS; i++ ){
				for (RegexNFAState* s2 : s->_transitions[i]) {
					GroupedNFAState& next = _transitions[i];
					this->generate_star(s2, next);
				}
			}
			std::unique_ptr<RegexNFAState::UnicodeIntervalTree::SearchList> l = s->transitions.all();
			for (RegexNFAState::UnicodeIntervalTree::SearchList::iterator it = l->begin(); it != l->end(); it++) {
				for (RegexNFAState* const s2 : (*it).second) {
					GroupedNFAState& next = transitions[(*it).first];
					this->generate_star(s2, next);
				}
			}
		}

		for (auto const& kv : _transitions) {
			RegexDFAState* next_dfa_state = nullptr;
			std::map<GroupedNFAState, RegexDFAState*>::iterator it = generated_states.find(kv.second);
			if (it == generated_states.end()) {
				next_dfa_state = register_state(kv.second);
			} else {
				next_dfa_state = it->second;
			}
			assert(curr_dfa_state->_transitions[kv.first] == nullptr);
			curr_dfa_state->_transitions[kv.first] = next_dfa_state;
		}
		for (auto const& kv : transitions) {
			RegexDFAState* next_dfa_state = nullptr;
			std::map<GroupedNFAState, RegexDFAState*>::iterator it = generated_states.find(kv.second);
			if (it == generated_states.end()) {
				next_dfa_state = register_state(kv.second);
			} else {
				next_dfa_state = it->second;
			}
			assert(curr_dfa_state->transitions.find(kv.first)->empty());
			curr_dfa_state->transitions.insert(kv.first, next_dfa_state);
		}
	}

	return dfa;
}

RegexNFAState* RegexNFA::new_state() {
	std::unique_ptr<RegexNFAState> ptr(new RegexNFAState(this->states.size()));
	RegexNFAState* state = ptr.get();
	this->states.push_back(std::move(ptr));
	return state;
}

void RegexNFA::generate_star(RegexNFAState* state, GroupedNFAState& group) {
	std::list<RegexNFAState*> l;
	l.push_back(state);
	while (l.size() > 0) {
		RegexNFAState* s = l.front();
		l.pop_front();
		if (group.insert(s).second) {
			for (RegexNFAState* const s2 : s->epsilon) {
				l.push_back(s2);
			}
		}
	}
}
