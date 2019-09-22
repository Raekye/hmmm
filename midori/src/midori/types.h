#ifndef MIDORI_TYPES_H_INCLUDED
#define MIDORI_TYPES_H_INCLUDED

#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include <string>
#include <map>

class TypeManager;
class PointerType;
class ArrayType;

class Type {
public:
	std::string const name;
	llvm::Type* const llvm_type;
	TypeManager* const manager;

	Type(std::string, llvm::Type*, TypeManager*);
	virtual ~Type() = 0;
	bool is_primitive();
	PointerType* pointer_ty();
	ArrayType* array_ty();

private:
	PointerType* _pointer_ty;
	ArrayType* _array_ty;
};

class PrimitiveType : public Type {
public:
	PrimitiveType(std::string, llvm::Type*, TypeManager*);
};

class StructType : public Type {
public:
	StructType(std::string, llvm::Type*, TypeManager*);
};

class PointerType : public Type {
public:
	Type* const base;

	PointerType(Type*);
};

class ArrayType : public Type {
public:
	Type* const base;

	ArrayType(Type*);
};

class TypeManager {
public:
	TypeManager(llvm::LLVMContext*);
	Type* get(std::string);
	Type* void_type() {
		return this->_void_type;
	}
	StructType* make_struct(std::string);
	PointerType* make_pointer(Type*);
	ArrayType* make_array(Type*);

private:
	llvm::LLVMContext* context;
	Type* _void_type;
	std::map<std::string, std::unique_ptr<Type>> types;

	PrimitiveType* make_primitive(std::string, llvm::Type*);
	void register_type(std::unique_ptr<Type>);
};

#endif /* MIDORI_TYPES_H_INCLUDED */
