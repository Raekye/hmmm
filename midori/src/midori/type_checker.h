#ifndef MIDORI_TYPE_CHECKER_H_INCLUDED
#define MIDORI_TYPE_CHECKER_H_INCLUDED

#include "lang.h"
#include "types.h"
#include <list>
#include <map>

class TypeChecker : public ILangASTVisitor {
public:
	TypeChecker(TypeManager*);
	virtual void visit(LangASTBlock*) override;
	virtual void visit(LangASTIdent*) override;
	virtual void visit(LangASTDecl*) override;
	virtual void visit(LangASTUnOp*) override;
	virtual void visit(LangASTBinOp*) override;
	virtual void visit(LangASTInt*) override;
	virtual void visit(LangASTDouble*) override;
	virtual void visit(LangASTIf*) override;
	virtual void visit(LangASTWhile*) override;
	virtual void visit(LangASTPrototype*) override;
	virtual void visit(LangASTFunction*) override;
	virtual void visit(LangASTCall*) override;

private:
	TypeManager* type_manager;
	Type* _ret;
	Type* _want;
	std::list<std::map<std::string, Type*>> frames;
	std::list<std::map<std::string, LangASTPrototype*>> function_frames;

	void ret(Type* t, LangASTExpression* e) {
		if (e != nullptr) {
			e->type = t;
		}
		this->_ret = t;
	}
	Type* ret() {
		Type* t = this->_ret;
		this->_ret = nullptr;
		return t;
	}
	void want(Type* t) {
		this->_want = t;
	}
	Type* want() {
		Type* t = this->_want;
		this->_want = nullptr;
		return t;
	}
	void push_scope();
	void pop_scope();
	Type* named_value_type(std::string);
	LangASTPrototype* find_function(std::string);
};

#endif /* MIDORI_TYPE_CHECKER_H_INCLUDED */
