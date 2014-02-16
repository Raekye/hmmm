#ifndef __NODE_H_
#define __NODE_H_

#include <llvm/IR/Value.h>
#include <vector>
#include <string>

class CodeGen;
class NExpression;

typedef enum tagEBinaryOperationType {
	eMULTIPLY,
	eADD,
	eSUBTRACT,
	eDIVIDE,
	eRAISE,
} EBinaryOperationType;

typedef union tagNumberValue {
	int8_t b;
	uint8_t ub;
	int16_t is;
	uint16_t us;
	int32_t i;
	uint32_t ui;
	int64_t l;
	uint64_t ul;
	float f;
	double d;
} UNumberValue;

typedef enum tagNumberType {
	eBYTE,
	eUBYTE,
	eSHORT,
	eUSHORT,
	eINT,
	eUINT,
	eLONG,
	eULONG,
	eFLOAT,
	eDOUBLE,
} ENumberType;

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
	NPrimitiveNumber(UNumberValue, ENumberType);
	UNumberValue val;
	ENumberType type;
	virtual llvm::Value* gen_code(CodeGen*);
	virtual ~NPrimitiveNumber();

	static NPrimitiveNumber* parse(std::string);
};

class NBinaryOperator : public NExpression {
public:
	EBinaryOperationType op;
	NExpression* lhs;
	NExpression* rhs;
	NBinaryOperator(EBinaryOperationType, NExpression*, NExpression*);
	virtual llvm::Value* gen_code(CodeGen*);
	virtual ~NBinaryOperator();
};

class NFunction : public NExpression {
public:
	NExpression* body;
	NFunction(NExpression*);
	virtual llvm::Value* gen_code(CodeGen*);
	virtual ~NFunction();
};

class NIdentifier : public NExpression {
public:
	std::string name;
	NIdentifier(std::string);
	virtual llvm::Value* gen_code(CodeGen*);
};

class NAssignment : public NExpression {
public:
	NIdentifier* lhs;
	NExpression* rhs;
	NAssignment(NIdentifier*, NExpression*);
	~NAssignment();
	virtual llvm::Value* gen_code(CodeGen*);
};

class NVariableDeclaration : public NExpression {
public:
	NIdentifier* type;
	NIdentifier* var_name;
	NVariableDeclaration(NIdentifier*, NIdentifier*);
	~NVariableDeclaration();
	virtual llvm::Value* gen_code(CodeGen*);
};

class NBlock : public NExpression {
public:
	std::vector<NExpression*> statements;
	NBlock();
	virtual llvm::Value* gen_code(CodeGen*);
};

extern "C" {
	NPrimitiveNumber* NPrimitiveNumber_construct(UNumberValue, ENumberType);
	void NPrimitiveNumber_destruct(NPrimitiveNumber*);
}

#endif // __NODE_H_
