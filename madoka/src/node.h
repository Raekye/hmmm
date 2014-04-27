#ifndef __NODE_H_
#define __NODE_H_

#include <llvm/IR/Value.h>
#include <vector>
#include <string>
#include <map>
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

class NType {
public:
	NType(std::string, std::vector<NType*>, std::vector<NType*>, llvm::Type*);
	std::string name;
	std::vector<NType*> extends;
	std::vector<NType*> implements;
	llvm::Type* llvm_type;

	static NType* get(std::string);

	static NType* byte_ty();
	static NType* ubyte_ty();
	static NType* short_ty();
	static NType* ushort_ty();
	static NType* int_ty();
	static NType* uint_ty();
	static NType* long_ty();
	static NType* ulong_ty();
	static NType* float_ty();
	static NType* double_ty();

private:
	static std::map<std::string, NType*> types;
	static int __STATIC_INIT;
	static int __STATIC_INITIALIZER();
};

class NExpression : public Node {
public:
	NType* type;
	llvm::Value* value;
};

class NNumber : public NExpression {
};

class NPrimitiveNumber : public NNumber {
public:
	NPrimitiveNumber(std::string);
	UNumberValue val;
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
	NType* return_type;
	NFunction(NExpression*, NType*);
	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~NFunction();
};

class NIdentifier : public NExpression {
public:
	std::string name;
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
	NIdentifier* var_name;
	NIdentifier* type_name;
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
