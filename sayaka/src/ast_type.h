#ifndef __AST_TYPE_H_
#define __AST_TYPE_H_

#include <string>
#include <vector>
#include <map>
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>

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

typedef enum tagEBinaryOperationType {
	eMULTIPLY,
	eADD,
	eSUBTRACT,
	eDIVIDE,
	ePOW,
} EBinaryOperationType;

class ASTType {
public:
	ASTType(std::string, std::vector<ASTType*>, std::vector<ASTType*>, llvm::Type*);
	std::string name;
	std::vector<ASTType*> extends;
	std::vector<ASTType*> implements;
	llvm::Type* llvm_type;

	bool is_primitive();
	bool is_signed();
	bool is_integral();
	bool is_floating();

	static ASTType* get(std::string);

	static ASTType* byte_ty();
	static ASTType* ubyte_ty();
	static ASTType* short_ty();
	static ASTType* ushort_ty();
	static ASTType* int_ty();
	static ASTType* uint_ty();
	static ASTType* long_ty();
	static ASTType* ulong_ty();
	static ASTType* float_ty();
	static ASTType* double_ty();

private:
	static std::map<std::string, ASTType*> types;
	static int __STATIC_INIT;
	static int __STATIC_INITIALIZER();
};

#endif // __NTYPE_H_
