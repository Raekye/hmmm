#ifndef MIDORI_TYPE_CHECKER_H_INCLUDED
#define MIDORI_TYPE_CHECKER_H_INCLUDED

#include "lang.h"
#include "types.h"
#include <list>
#include <map>

class TypeChecker : public ILangASTVisitor {
public:
	TypeChecker(TypeManager*);
	virtual void visit(LangASTIdent*) override;
	virtual void visit(LangASTDecl*) override;
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
	Type* ret;
	std::list<std::map<std::string, Type*>> frames;
	std::list<std::map<std::string, LangASTFunction*>> function_frames;

	void push_scope();
	void pop_scope();
	Type* named_value_type(std::string);
};

#endif /* MIDORI_TYPE_CHECKER_H_INCLUDED */
