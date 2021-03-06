#include "codescope.h"
#include <iostream>

CodeScope::CodeScope() {
	this->push();
}

CodeScope::~CodeScope() {
	while (this->stacks.size() > 0) {
		this->pop();
	}
}

void CodeScope::put(std::string key, NExpression* val) {
	(*this->stacks.back())[key] = val;
}

NExpression* CodeScope::get(std::string key) {
	for (std::deque<std::map<std::string, NExpression*>*>::reverse_iterator it = this->stacks.rbegin(); it != this->stacks.rend(); it++) {
		std::map<std::string, NExpression*>::iterator found = (*it)->find(key);
		if (found != (*it)->end()) {
			return found->second;
		}
	}
	return NULL;
}

bool CodeScope::contains(std::string key) {
	return this->get(key) != NULL;
}

void CodeScope::push() {
	std::map<std::string, NExpression*>* scope = new std::map<std::string, NExpression*>();
	this->stacks.push_back(scope);
}

void CodeScope::pop() {
	this->stacks.pop_back();
}
