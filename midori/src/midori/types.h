#ifndef MIDORI_TYPES_H_INCLUDED
#define MIDORI_TYPES_H_INCLUDED

#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include <string>
#include <map>

class TypeManager;

class Type {
public:
	std::string const name;
	llvm::Type* const llvm_type;

	Type(std::string, llvm::Type*, TypeManager*);
	bool is_primitive();

private:
	TypeManager* manager;
};

class TypeManager {
public:
	TypeManager(llvm::LLVMContext*);
	Type* register_type(std::string, llvm::Type*);
	Type* get(std::string);
	bool is_primitive(std::string);
	Type* void_type() {
		return this->_void_type;
	}

private:
	Type* _void_type;
	std::map<std::string, std::unique_ptr<Type>> types;
	std::map<std::string, Type*> primitives;
};

#endif /* MIDORI_TYPES_H_INCLUDED */
