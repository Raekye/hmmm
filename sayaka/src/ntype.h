#ifndef __NTYPE_H_
#define __NTYPE_H_

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

#endif // __NTYPE_H_
