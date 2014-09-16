#ifndef __NODE_H_
#define __NODE_H_

#include <llvm/IR/Value.h>
#include <vector>
#include <string>
#include <map>
#include <cstdint>

class CodeGen;
class NExpression;

typedef union tagNumberValue {
	int8_t b;
	uint8_t ub;
	int16_t s;
	uint16_t us;
	int32_t i;
	uint32_t ui;
	int64_t l;
	uint64_t ul;
	float f;
	double d;
} UNumberValue;

typedef enum tagNumberType {
	eBYTE = 0,
	eUBYTE = 1,
	eSHORT = (1 << 1),
	eUSHORT = (1 << 1) | 1,
	eINT = (1 << 2),
	eUINT = (1 << 2) | 1,
	eLONG = (1 << 3),
	eULONG = (1 << 3) | 1,
	eFLOAT = (1 << 4),
	eDOUBLE = (1 << 5),
} ENumberType;

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

	bool is_primitive();
	bool is_signed();
	bool is_integral();
	bool is_floating();

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
	NVariableDeclaration(NType*, NIdentifier*);
	~NVariableDeclaration();
	virtual llvm::Value* gen_code(CodeGen*) override;
};

class NCast : public NExpression {
public:
	NType* target_type;
	NExpression* val;
	NCast(NExpression*, NType*);
	~NCast();
	virtual llvm::Value* gen_code(CodeGen*) override;
};

class NBlock : public NExpression {
public:
	std::vector<NExpression*> statements;
	NBlock();
	virtual llvm::Value* gen_code(CodeGen*) override;
	void push(NExpression*);
};

#endif // __NODE_H_
