#include "type_checker.h"

TypeChecker::TypeChecker(TypeManager* tm) : type_manager(tm) {
	this->push_scope();
}

void TypeChecker::visit(LangASTBasicType* v) {
	this->ret(this->type_manager->get(v->name), nullptr);
}

void TypeChecker::visit(LangASTPointerType* v) {
	v->base->accept(this);
	this->ret(this->ret()->pointer_ty(), nullptr);
}

void TypeChecker::visit(LangASTArrayType* v) {
	v->base->accept(this);
	this->ret(this->ret()->array_ty(), nullptr);
}

void TypeChecker::visit(LangASTBlock* v) {
	this->push_scope();
	for (std::unique_ptr<LangAST> const& l : v->lines) {
		l->accept(this);
	}
	this->pop_scope();
	this->ret(this->type_manager->void_type(), nullptr);
}

void TypeChecker::visit(LangASTIdent* v) {
	this->ret(this->named_value_type(v->name), v);
}

void TypeChecker::visit(LangASTDecl* v) {
	v->decl_type->accept(this);
	Type* t = this->ret();
	this->frames.front()[v->name] = t;
	this->ret(t, v);
}

void TypeChecker::visit(LangASTUnOp* v) {
	v->expr->accept(this);
	this->ret(this->ret(), v);
}

void TypeChecker::visit(LangASTBinOp* v) {
	v->left->accept(this);
	Type* lhs = this->ret();
	this->want(lhs);
	v->right->accept(this);
	Type* rhs = this->ret();
	if (lhs != rhs) {
		this->ret(nullptr, v);
		return;
	}
	this->ret(lhs, v);
}

void TypeChecker::visit(LangASTInt* v) {
	Type* t = this->want();
	if (t == nullptr) {
		t = this->type_manager->get("Long");
	}
	this->ret(t, v);
}

void TypeChecker::visit(LangASTDouble* v) {
	Type* t = this->want();
	if (t == nullptr) {
		t = this->type_manager->get("Double");
	}
	this->ret(t, v);
}

void TypeChecker::visit(LangASTIf* v) {
	this->push_scope();
	v->predicate->accept(this);
	v->block_if->accept(this);
	this->pop_scope();
	if (v->block_else != nullptr) {
		this->push_scope();
		v->block_else->accept(this);
		this->pop_scope();
	}
	this->ret(this->type_manager->void_type(), nullptr);
}

void TypeChecker::visit(LangASTWhile* v) {
	this->push_scope();
	v->predicate->accept(this);
	v->block->accept(this);
	this->pop_scope();
	this->ret(this->type_manager->void_type(), nullptr);
}

void TypeChecker::visit(LangASTPrototype* v) {
	for (std::unique_ptr<LangASTDecl> const& a : v->args) {
		a->accept(this);
	}
	this->function_frames.front()[v->name] = v;
	this->ret(this->type_manager->void_type(), nullptr);
}

void TypeChecker::visit(LangASTFunction* v) {
	v->proto->accept(this);
	this->push_scope();
	v->body->accept(this);
	this->pop_scope();
	this->ret(this->type_manager->void_type(), nullptr);
}

void TypeChecker::visit(LangASTReturn* v) {
	if (v->val != nullptr) {
		v->val->accept(this);
	}
}

void TypeChecker::visit(LangASTCall* v) {
	LangASTPrototype* f = this->find_function(v->function);
	Int i = 0;
	for (std::unique_ptr<LangASTExpression> const& a : v->args) {
		Type* arg_type = f->args.at(i)->type;
		this->want(arg_type);
		a->accept(this);
		if (arg_type != this->ret()) {
			this->ret(nullptr, v);
			return;
		}
		i++;
	}
	f->return_type->accept(this);
	this->ret(this->ret(), v);
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

LangASTPrototype* TypeChecker::find_function(std::string name) {
	for (std::map<std::string, LangASTPrototype*> const& f : this->function_frames) {
		std::map<std::string, LangASTPrototype*>::const_iterator it = f.find(name);
		if (it != f.end()) {
			return it->second;
		}
	}
	return nullptr;
}
