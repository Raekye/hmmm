#ifndef MIDORI_LANG_H_INCLUDED
#define MIDORI_LANG_H_INCLUDED

#include "parser.h"
#include "types.h"
#include <memory>

template <typename T> class LangASTLiteral;
typedef LangASTLiteral<Long> LangASTInt;
typedef LangASTLiteral<double> LangASTDouble;

class LangASTBasicType;
class LangASTPointerType;
class LangASTArrayType;
class LangASTBlock;
class LangASTLIdent;
class LangASTRIdent;
class LangASTAssignment;
class LangASTDecl;
class LangASTUnOp;
class LangASTBinOp;
class LangASTIf;
class LangASTWhile;
class LangASTPrototype;
class LangASTFunction;
class LangASTReturn;
class LangASTCall;
class LangASTClassDef;

class ILangASTVisitor {
public:
	virtual ~ILangASTVisitor() = 0;
	virtual void visit(LangASTBasicType*) = 0;
	virtual void visit(LangASTPointerType*) = 0;
	virtual void visit(LangASTArrayType*) = 0;
	virtual void visit(LangASTBlock*) = 0;
	virtual void visit(LangASTLIdent*) = 0;
	virtual void visit(LangASTRIdent*) = 0;
	virtual void visit(LangASTAssignment*) = 0;
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
	virtual void visit(LangASTClassDef*) = 0;
};

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

class LangASTLValue : public LangAST {
public:
	Type* type;

	LangASTLValue() : type(nullptr) {
		return;
	}
	virtual ~LangASTLValue() = 0;
};

class LangASTVoid : public LangAST {
public:
	virtual ~LangASTVoid() = 0;
};

class LangASTType : public LangAST {
public:
	std::string name;

	LangASTType(std::string s) : name(s) {
		return;
	}
	virtual ~LangASTType() = 0;
};

class LangASTBasicType : public LangASTType {
public:
	LangASTBasicType(std::string s) : LangASTType(s) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) {
		v->visit(this);
	}
};

class LangASTPointerType : public LangASTType {
public:
	std::unique_ptr<LangASTType> base;

	LangASTPointerType(std::unique_ptr<LangASTType> t) : LangASTType(t->name + "*"), base(std::move(t)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTArrayType : public LangASTType {
public:
	std::unique_ptr<LangASTType> base;

	LangASTArrayType(std::unique_ptr<LangASTType> t) : LangASTType(t->name + "*"), base(std::move(t)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTBlock : public LangASTVoid {
public:
	std::vector<std::unique_ptr<LangAST>> lines;

	LangASTBlock(std::vector<std::unique_ptr<LangAST>> v) : lines(std::move(v)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

template <typename T> class LangASTLiteral : public LangASTExpression {
public:
	T value;

	LangASTLiteral(T v) : value(v) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTLIdent : public LangASTLValue {
public:
	struct NameOrIndex {
		std::string name;
		std::unique_ptr<LangASTExpression> index;

		NameOrIndex(std::string s, std::unique_ptr<LangASTExpression> i) : name(s), index(std::move(i)) {
			return;
		}
	};

	std::vector<NameOrIndex> parts;

	LangASTLIdent(std::string s) {
		this->parts.emplace_back(s, nullptr);
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTRIdent : public LangASTExpression {
public:
	std::unique_ptr<LangASTLValue> ident;

	LangASTRIdent(std::unique_ptr<LangASTLIdent> li) : ident(std::move(li)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTAssignment : public LangASTExpression {
public:
	std::unique_ptr<LangASTLValue> left;
	std::unique_ptr<LangASTExpression> right;

	LangASTAssignment(std::unique_ptr<LangASTLValue> l, std::unique_ptr<LangASTExpression> r) : left(std::move(l)), right(std::move(r)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
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
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTBinOp : public LangASTExpression {
public:
	enum Op {
		PLUS,
		MINUS,
		STAR,
		SLASH,
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
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTDecl : public LangASTLValue {
public:
	std::string name;
	std::unique_ptr<LangASTType> decl_type;

	LangASTDecl(std::string n, std::unique_ptr<LangASTType> t) : name(n), decl_type(std::move(t)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
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
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTWhile : public LangASTVoid {
public:
	std::unique_ptr<LangASTExpression> predicate;
	std::unique_ptr<LangASTBlock> block;

	LangASTWhile(std::unique_ptr<LangASTExpression> p, std::unique_ptr<LangASTBlock> b) : predicate(std::move(p)), block(std::move(b)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTPrototype : public LangASTVoid {
public:
	std::string name;
	std::unique_ptr<LangASTType> return_type;
	std::vector<std::unique_ptr<LangASTDecl>> args;

	LangASTPrototype(std::string n, std::unique_ptr<LangASTType> r, std::vector<std::unique_ptr<LangASTDecl>> a) : name(n), return_type(std::move(r)), args(std::move(a)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTFunction : public LangASTVoid {
public:
	std::unique_ptr<LangASTPrototype> proto;
	std::unique_ptr<LangASTBlock> body;

	LangASTFunction(std::unique_ptr<LangASTPrototype> p, std::unique_ptr<LangASTBlock> b) : proto(std::move(p)), body(std::move(b)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTReturn : public LangASTVoid {
public:
	std::unique_ptr<LangASTExpression> val;
	LangASTReturn(std::unique_ptr<LangASTExpression> v) : val(std::move(v)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTCall : public LangASTExpression {
public:
	std::string function;
	std::vector<std::unique_ptr<LangASTExpression>> args;

	LangASTCall(std::string f, std::vector<std::unique_ptr<LangASTExpression>> a) : function(f), args(std::move(a)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTClassDef : public LangASTVoid {
public:
	std::string name;
	std::vector<std::unique_ptr<LangASTDecl>> fields;

	LangASTClassDef(std::string s, std::vector<std::unique_ptr<LangASTDecl>> v) : name(s), fields(std::move(v)) {
		return;
	}
	virtual void accept(ILangASTVisitor* v) override {
		v->visit(this);
	}
};

class LangASTPrinter : public ILangASTVisitor {
public:
	LangASTPrinter();
	virtual void visit(LangASTBasicType*) override;
	virtual void visit(LangASTPointerType*) override;
	virtual void visit(LangASTArrayType*) override;
	virtual void visit(LangASTBlock*) override;
	virtual void visit(LangASTLIdent*) override;
	virtual void visit(LangASTRIdent*) override;
	virtual void visit(LangASTAssignment*) override;
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
	virtual void visit(LangASTClassDef*) override;

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
