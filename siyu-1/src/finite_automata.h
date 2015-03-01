#ifndef SIYU_FINITE_AUTOMATA_H_INCLUDED
#define SIYU_FINITE_AUTOMATA_H_INCLUDED

#include <map>
#include <vector>
#include <memory>
#include "global.h"

class DFAState {
public:
	std::map<UInt, DFAState*> next_states;
	bool terminal;
	DFAState();
};

class DFA {
	std::map<DFAState*, UInt> id_from_state;
public:
	DFAState* root;
	std::vector<std::unique_ptr<DFAState>> states;

	DFA();
	DFAState* new_state();
	UInt id(DFAState*);
};

#endif /* SIYU_FINITE_AUTOMATA_H_INCLUDED */
