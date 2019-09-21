#include "types.h"

Type::Type(std::string name, llvm::Type* type, TypeManager* m) : name(name), llvm_type(type), manager(m) {
	return;
}

bool Type::is_primitive() {
	return this->manager->is_primitive(this->name);
}

// TODO: why this works? isn't the types map uninitialized
TypeManager::TypeManager(llvm::LLVMContext* c) {
	this->_void_type = this->register_type("Void", llvm::Type::getVoidTy(*c));
	llvm::IntegerType* llvm_int1 = llvm::Type::getInt1Ty(*c);
	llvm::IntegerType* llvm_int8 = llvm::Type::getInt8Ty(*c);
	llvm::IntegerType* llvm_int16 = llvm::Type::getInt16Ty(*c);
	llvm::IntegerType* llvm_int32 = llvm::Type::getInt32Ty(*c);
	llvm::IntegerType* llvm_int64 = llvm::Type::getInt64Ty(*c);
	llvm::Type* llvm_float = llvm::Type::getFloatTy(*c);
	llvm::Type* llvm_double = llvm::Type::getDoubleTy(*c);
	std::map<std::string, llvm::Type*> m = {
		{ "Bool", llvm_int1 },
		{ "Byte", llvm_int8 },
		{ "UByte", llvm_int8 },
		{ "Short", llvm_int16 },
		{ "UShort", llvm_int16 },
		{ "Int", llvm_int32 },
		{ "UInt", llvm_int32 },
		{ "Long", llvm_int64 },
		{ "ULong", llvm_int64 },
		{ "Float", llvm_float },
		{ "Double", llvm_double },
	};
	for (std::map<std::string, llvm::Type*>::value_type const& kv : m) {
		this->primitives[kv.first] = this->register_type(kv.first, kv.second);
	}
}

Type* TypeManager::register_type(std::string name, llvm::Type* llvm_type) {
	if (this->types.find(name) != this->types.end()) {
		return nullptr;
	}
	std::unique_ptr<Type> t(new Type(name, llvm_type, this));
	Type* ptr = t.get();
	this->types[name] = std::move(t);
	return ptr;
}

Type* TypeManager::get(std::string name) {
	std::map<std::string, std::unique_ptr<Type>>::iterator it = this->types.find(name);
	return (it == this->types.end()) ? nullptr : it->second.get();
}

bool TypeManager::is_primitive(std::string name) {
	return this->primitives.find(name) != this->primitives.end();
}
