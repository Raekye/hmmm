#ifndef __NODE_H_
#define __NODE_H_

#include <llvm/IR/Value.h>
#include <vector>
#include <string>
#include "number.h"

class CodeGen;
class NExpression;

// TODO: destructors

typedef enum tagEBinaryOperationType {
	eMULTIPLY,
	eADD,
	eSUBTRACT,
	eDIVIDE,
	ePOW,
} EBinaryOperationType;

class Node {
public:
	virtual ~Node();
	virtual llvm::Value* gen_code(CodeGen*) = 0;
};

class NExpression : public Node {
};

class NNumber : public NExpression {
};

class NPrimitiveNumber : public NNumber {
public:
	NPrimitiveNumber(std::string str);
	UNumberValue val;
	ENumberType type;
	std::string str;
	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~NPrimitiveNumber();
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
	NFunction(NExpression*);
	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~NFunction();
};

class NIdentifier : public NExpression {
public:
	std::string name;
	std::string type;
	NIdentifier(std::string);
	virtual llvm::Value* gen_code(CodeGen*) override;
};

class NAssignment : public NExpression {
public:
	NIdentifier* lhs;
	NExpression* rhs;
	NAssignment(NIdentifier*, NExpression*);
	~NAssignment();
	virtual llvm::Value* gen_code(CodeGen*) override;
};

class NVariableDeclaration : public NExpression {
public:
	NIdentifier* type;
	NIdentifier* var_name;
	NVariableDeclaration(NIdentifier*, NIdentifier*);
	~NVariableDeclaration();
	virtual llvm::Value* gen_code(CodeGen*) override;
};

class NBlock : public NExpression {
public:
	std::vector<NExpression*> statements;
	NBlock();
	virtual llvm::Value* gen_code(CodeGen*) override;
};

#endif // __NODE_H_
