#include "identifier_scope.h"
#include <iostream>

IdentifierScope::IdentifierScope() {
	return;
}

IdentifierScope::~IdentifierScope() {
	while (this->stacks.size() > 0) {
		this->pop();
	}
}

void IdentifierScope::put(std::string key, ASTNodeIdentifier* val) {
	(*this->stacks.back())[key] = val;
}

ASTNodeIdentifier* IdentifierScope::get(std::string key) {
	for (std::deque<std::map<std::string, ASTNodeIdentifier*>*>::reverse_iterator it = this->stacks.rbegin(); it != this->stacks.rend(); it++) {
		std::map<std::string, ASTNodeIdentifier*>::iterator found = (*it)->find(key);
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
	std::map<std::string, ASTNodeIdentifier*>* scope = new std::map<std::string, ASTNodeIdentifier*>();
	this->stacks.push_back(scope);
}

void IdentifierScope::pop() {
	delete this->stacks.back();
	this->stacks.pop_back();
}
