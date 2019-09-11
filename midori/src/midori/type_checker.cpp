#include "type_checker.h"

TypeChecker::TypeChecker(TypeManager* tm) : type_manager(tm) {
	return;
}

void TypeChecker::visit(LangASTIdent* v) {
	this->ret = this->named_value_type(v->name);
}

void TypeChecker::visit(LangASTDecl* v) {
	Type* t = this->type_manager->get(v->type);
	this->frames.front()[v->name] = t;
	this->ret = t;
}

void TypeChecker::visit(LangASTBinOp* v) {
	v->left->accept(this);
	Type* lhs = this->ret;
	v->right->accept(this);
	Type* rhs = this->ret;
	if (lhs != rhs) {
		this->ret = nullptr;
	}
}

void TypeChecker::visit(LangASTInt* v) {
	(void) v;
}

void TypeChecker::visit(LangASTDouble* v) {
	(void) v;
}

void TypeChecker::visit(LangASTIf* v) {
	this->push_scope();
	v->predicate->accept(this);
	for (std::unique_ptr<LangAST> const& l : v->block) {
		l->accept(this);
	}
	this->pop_scope();
	this->ret = this->type_manager->void_type();
}

void TypeChecker::visit(LangASTWhile* v) {
	this->push_scope();
	v->predicate->accept(this);
	for (std::unique_ptr<LangAST> const& l : v->block) {
		l->accept(this);
	}
	this->pop_scope();
	this->ret = this->type_manager->void_type();
}

void TypeChecker::visit(LangASTPrototype* v) {
	(void) v;
	this->ret = this->type_manager->void_type();
}

void TypeChecker::visit(LangASTFunction* v) {
	this->push_scope();
	for (std::unique_ptr<LangAST> const& l : v->body) {
		l->accept(this);
	}
	this->pop_scope();
	this->ret = this->type_manager->void_type();
}

void TypeChecker::visit(LangASTCall* v) {
	for (std::unique_ptr<LangASTExpression> const& a : v->args) {
		a->accept(this);
	}
	//this->ret = this->type_manager->get(v->fun
}

void TypeChecker::push_scope() {
	this->frames.emplace_front();
	this->function_frames.emplace_front();
}

void TypeChecker::pop_scope() {
	this->frames.pop_front();
	this->function_frames.pop_front();
}

Type* TypeChecker::named_value_type(std::string name) {
	for (std::map<std::string, Type*> const& f : this->frames) {
		std::map<std::string, Type*>::const_iterator it = f.find(name);
		if (it != f.end()) {
			return it->second;
		}
	}
	return nullptr;
}
