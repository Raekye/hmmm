#ifndef __NODE_H_
#define __NODE_H_

#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <llvm/IR/Value.h>
#include "ntype.h"

class CodeGen;

class Node {
public:
	virtual ~Node() = 0;
	virtual llvm::Value* gen_code(CodeGen*) = 0;
};

class NExpression : public Node {
public:
	NType* type;
	llvm::Value* value;

	virtual ~NExpression() = 0;
	virtual llvm::Value* gen_code(CodeGen*) = 0;
};

class NPrimitive : public NExpression {
public:
	UNumberValue val;
	std::string str;

	NPrimitive(std::string);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~NPrimitive();
};

class NBinaryOperator : public NExpression {
public:
	EBinaryOperationType op;
	NExpression* lhs;
	NExpression* rhs;

	NBinaryOperator(EBinaryOperationType, NExpression*, NExpression*);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~NBinaryOperator();
};

class NFunction : public NExpression {
public:
	NExpression* body;
	NType* return_type;

	NFunction(NExpression*, NType*);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~NFunction();
};

class NIdentifier : public NExpression {
public:
	std::string name;

	NIdentifier(std::string, NType* type);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~NIdentifier();
};

class NAssignment : public NExpression {
public:
	NIdentifier* lhs;
	NExpression* rhs;

	NAssignment(NIdentifier*, NExpression*);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~NAssignment();
};

class NVariableDeclaration : public NExpression {
public:
	NIdentifier* var_name;

	NVariableDeclaration(NType*, NIdentifier*);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~NVariableDeclaration();
};

class NCast : public NExpression {
public:
	NType* target_type;
	NExpression* val;

	NCast(NExpression*, NType*);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~NCast();
};

class NBlock : public NExpression {
public:
	std::vector<NExpression*> statements;

	NBlock();
	void push(NExpression*);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~NBlock();
};

#endif // __NODE_H_
