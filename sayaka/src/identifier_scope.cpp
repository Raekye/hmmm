#include "identifier_scope.h"

IdentifierScope::IdentifierScope() {
	return;
}

IdentifierScope::~IdentifierScope() {
	while (this->stacks.size() > 0) {
		this->pop();
	}
}

void IdentifierScope::put(std::string key, CodeGenVariable* var) {
	(*this->stacks.back())[key] = var;
}

CodeGenVariable* IdentifierScope::get(std::string key) {
	for (std::deque<std::map<std::string, CodeGenVariable*>*>::reverse_iterator it = this->stacks.rbegin(); it != this->stacks.rend(); it++) {
		std::map<std::string, CodeGenVariable*>::iterator found = (*it)->find(key);
		if (found != (*it)->end()) {
			return found->second;
		}
	}
	return NULL;
}

bool IdentifierScope::contains(std::string key) {
	return this->get(key) != NULL;
}

bool IdentifierScope::in_top(std::string key) {
	return (*this->stacks.back())[key];
}

void IdentifierScope::push() {
	this->stacks.push_back(new std::map<std::string, CodeGenVariable*>());
}

void IdentifierScope::pop() {
	for (std::map<std::string, CodeGenVariable*>::iterator it = this->stacks.back()->begin(); it != this->stacks.back()->end(); it++) {
		delete it->second;
	}
	delete this->stacks.back();
	this->stacks.pop_back();
}
