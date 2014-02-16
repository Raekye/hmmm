#include "codescope.h"
#include <iostream>

// TODO: freeing stl

CodeScope::CodeScope() {
}

CodeScope::~CodeScope() {
	// while (this->stacks.size() > 0) {
	// 	this->pop();
	// }
}

void CodeScope::put(std::string key, llvm::Value* val) {
	(*this->stacks.back())[key] = val;
}

llvm::Value* CodeScope::get(std::string key) {
	for (std::deque<std::map<std::string, llvm::Value*>*>::reverse_iterator it = this->stacks.rbegin(); it != this->stacks.rend(); it++) {
		std::map<std::string, llvm::Value*>::iterator found = (*it)->find(key);
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
	std::map<std::string, llvm::Value*>* scope = new std::map<std::string, llvm::Value*>();
	this->stacks.push_back(scope);
}

void CodeScope::pop() {
	for (std::map<std::string, llvm::Value*>::iterator it = this->stacks.back()->begin(); it != this->stacks.back()->end(); it++) {
		std::cout << "Deleting " << it->second << std::endl;
		//delete it->second;
	}
	this->stacks.pop_back();
}