#ifndef MIDORI_LANG_H_INCLUDED
#define MIDORI_LANG_H_INCLUDED

#include "parser.h"
#include "types.h"
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
public:
	Type* type;

	LangASTExpression() : type(nullptr) {
		return;
	}
	virtual ~LangASTExpression() = 0;
};

class LangASTVoid : public LangAST {
public:
	virtual ~LangASTVoid() = 0;
};

class LangASTBlock : public LangASTVoid {
public:
	std::vector<std::unique_ptr<LangAST>> lines;

	LangASTBlock(std::vector<std::unique_ptr<LangAST>> v) : lines(std::move(v)) {
		return;
	}
	virtual void accept(ILangASTVisitor*);
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

class LangASTUnOp : public LangASTExpression {
public:
	enum Op {
		MINUS,
		NOT,
	};
	Op op;
	std::unique_ptr<LangASTExpression> expr;

	LangASTUnOp(Op op, std::unique_ptr<LangASTExpression> e) : op(op), expr(std::move(e)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTBinOp : public LangASTExpression {
public:
	enum Op {
		PLUS,
		MINUS,
		STAR,
		SLASH,
		ASSIGN,
		EQ,
		NE,
		LT,
		GT,
		LE,
		GE,
	};
	Op op;
	std::unique_ptr<LangASTExpression> left;
	std::unique_ptr<LangASTExpression> right;

	LangASTBinOp(Op op, std::unique_ptr<LangASTExpression> l, std::unique_ptr<LangASTExpression> r) : op(op), left(std::move(l)), right(std::move(r)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTDecl : public LangASTExpression {
public:
	std::string name;
	std::string type_name;

	LangASTDecl(std::string n, std::string t) : name(n), type_name(t) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTIf : public LangASTVoid {
public:
	std::unique_ptr<LangASTExpression> predicate;
	std::unique_ptr<LangASTBlock> block_if;
	std::unique_ptr<LangASTBlock> block_else;

	LangASTIf(std::unique_ptr<LangASTExpression> p
			, std::unique_ptr<LangASTBlock> b1
			, std::unique_ptr<LangASTBlock> b2)
		: predicate(std::move(p))
		, block_if(std::move(b1))
		, block_else(std::move(b2)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTWhile : public LangASTVoid {
public:
	std::unique_ptr<LangASTExpression> predicate;
	std::unique_ptr<LangASTBlock> block;

	LangASTWhile(std::unique_ptr<LangASTExpression> p, std::unique_ptr<LangASTBlock> b) : predicate(std::move(p)), block(std::move(b)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTPrototype : public LangASTVoid {
public:
	std::string name;
	std::string return_type;
	std::vector<std::unique_ptr<LangASTDecl>> args;

	LangASTPrototype(std::string n, std::string r, std::vector<std::unique_ptr<LangASTDecl>> a) : name(n), return_type(r), args(std::move(a)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTFunction : public LangASTVoid {
public:
	std::unique_ptr<LangASTPrototype> proto;
	std::unique_ptr<LangASTBlock> body;

	LangASTFunction(std::unique_ptr<LangASTPrototype> p, std::unique_ptr<LangASTBlock> b) : proto(std::move(p)), body(std::move(b)) {
		return;
	}
	virtual void accept(ILangASTVisitor*) override;
};

class LangASTReturn : public LangASTVoid {
public:
	std::unique_ptr<LangASTExpression> val;
	LangASTReturn(std::unique_ptr<LangASTExpression> v) : val(std::move(v)) {
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
	virtual void visit(LangASTBlock*) = 0;
	virtual void visit(LangASTIdent*) = 0;
	virtual void visit(LangASTDecl*) = 0;
	virtual void visit(LangASTUnOp*) = 0;
	virtual void visit(LangASTBinOp*) = 0;
	virtual void visit(LangASTInt*) = 0;
	virtual void visit(LangASTDouble*) = 0;
	virtual void visit(LangASTIf*) = 0;
	virtual void visit(LangASTWhile*) = 0;
	virtual void visit(LangASTPrototype*) = 0;
	virtual void visit(LangASTFunction*) = 0;
	virtual void visit(LangASTReturn*) = 0;
	virtual void visit(LangASTCall*) = 0;
};

class LangASTPrinter : public ILangASTVisitor {
public:
	LangASTPrinter();
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
	virtual void visit(LangASTReturn*) override;
	virtual void visit(LangASTCall*) override;

private:
	Int indents;
	void f();
};

class Lang {
public:
	Lang();
	std::unique_ptr<LangASTBlock> parse(IInputStream*);

private:
	Parser parser;

	void generate();
};

#endif /* MIDORI_LANG_H_INCLUDED */
