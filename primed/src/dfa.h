#ifndef PRIMED_DFA_H_INCLUDED
#define PRIMED_DFA_H_INCLUDED

#include <map>

template <typename T> class DFAState {
public:
	std::map<T, DFAState<T>*>* link;
	bool terminal;
};

#endif /* PRIMED_DFA_H_INCLUDED */
