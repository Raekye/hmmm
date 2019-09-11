#ifndef MIDORI_LANG_H_INCLUDED
#define MIDORI_LANG_H_INCLUDED

#include "parser.h"
#include <memory>

class ILangASTVisitor;
template <typename T> class LangASTLiteral;
typedef LangASTLiteral<Long> LangASTInt;
typedef LangASTLiteral<double> LangASTDouble;

class LangAST {
public:
	virtual ~LangAST() = 0;
	virtual void accept(ILangASTVisitor*) = 0;
};

class LangASTExpression : public LangAST {
};

class LangASTBlock : public LangAST {
};

template <typename T> class LangASTLiteral : public LangASTExpression {
public:
	T value;

	LangASTLiteral(T v) : value(v) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTIdent : public LangASTExpression {
public:
	std::string name;

	LangASTIdent(std::string s) : name(s) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTBinOp : public LangASTExpression {
public:
	Int op;
	std::unique_ptr<LangASTExpression> left;
	std::unique_ptr<LangASTExpression> right;

	LangASTBinOp(Int op, std::unique_ptr<LangASTExpression> l, std::unique_ptr<LangASTExpression> r) : op(op), left(std::move(l)), right(std::move(r)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTDecl : public LangASTExpression {
public:
	std::string name;
	std::string type;

	LangASTDecl(std::string n, std::string t) : name(n), type(t) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTIf : public LangASTBlock {
public:
	std::unique_ptr<LangASTExpression> predicate;
	std::vector<std::unique_ptr<LangAST>> block;

	LangASTIf(std::unique_ptr<LangASTExpression> p, std::vector<std::unique_ptr<LangAST>> b) : predicate(std::move(p)), block(std::move(b)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTWhile : public LangASTBlock {
public:
	std::unique_ptr<LangASTExpression> predicate;
	std::vector<std::unique_ptr<LangAST>> block;

	LangASTWhile(std::unique_ptr<LangASTExpression> p, std::vector<std::unique_ptr<LangAST>> b) : predicate(std::move(p)), block(std::move(b)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTPrototype : public LangASTBlock {
public:
	std::string name;
	std::string return_type;
	std::vector<std::unique_ptr<LangASTDecl>> args;

	LangASTPrototype(std::string n, std::string r, std::vector<std::unique_ptr<LangASTDecl>> a) : name(n), return_type(r), args(std::move(a)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTFunction : public LangASTBlock {
public:
	std::unique_ptr<LangASTPrototype> proto;
	std::vector<std::unique_ptr<LangAST>> body;

	LangASTFunction(std::unique_ptr<LangASTPrototype> p, std::vector<std::unique_ptr<LangAST>> b) : proto(std::move(p)), body(std::move(b)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTCall : public LangASTExpression {
public:
	std::string function;
	std::vector<std::unique_ptr<LangASTExpression>> args;

	LangASTCall(std::string f, std::vector<std::unique_ptr<LangASTExpression>> a) : function(f), args(std::move(a)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class ILangASTVisitor {
public:
	virtual ~ILangASTVisitor() = 0;
	virtual void visit(LangASTIdent*) = 0;
	virtual void visit(LangASTDecl*) = 0;
	virtual void visit(LangASTBinOp*) = 0;
	virtual void visit(LangASTInt*) = 0;
	virtual void visit(LangASTDouble*) = 0;
	virtual void visit(LangASTIf*) = 0;
	virtual void visit(LangASTWhile*) = 0;
	virtual void visit(LangASTPrototype*) = 0;
	virtual void visit(LangASTFunction*) = 0;
	virtual void visit(LangASTCall*) = 0;
};

class LangASTPrinter : public ILangASTVisitor {
public:
	LangASTPrinter();
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
	Int indents;
	void f();
};

class Lang {
public:
	Lang();
	std::vector<std::unique_ptr<LangAST>> parse(IInputStream*);

private:
	Parser parser;

	void generate();
};

#endif /* MIDORI_LANG_H_INCLUDED */
