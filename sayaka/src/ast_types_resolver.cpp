#include "ast_types_resolver.h"

ASTTypesResolver::ASTTypesResolver() {
	this->put(this->bit_ty());
	this->put(this->byte_ty());
	this->put(this->short_ty());
	this->put(this->int_ty());
	this->put(this->long_ty());
	this->put(this->ubyte_ty());
	this->put(this->ushort_ty());
	this->put(this->uint_ty());
	this->put(this->ulong_ty());
	this->put(this->float_ty());
	this->put(this->double_ty());
}

ASTTypesResolver::~ASTTypesResolver() {
	for (std::map<std::string, ASTType*>::iterator it = this->types_map.begin(); it != this->types_map.end(); it++) {
		delete it->second;
	}
}

ASTType* ASTTypesResolver::get(std::string name) {
	return this->types_map[name];
}

void ASTTypesResolver::put(ASTType* type) {
	this->types_map[type->name] = type;
}

ASTType* ASTTypesResolver::bit_ty() {
	if (this->get("Bit") != NULL) {
		return this->get("Bit");
	}
	ASTType* instance = new ASTType("Bit");
	instance->llvm_type = llvm::Type::getInt1Ty(llvm::getGlobalContext());
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::byte_ty() {
	if (this->get("Byte") != NULL) {
		return this->get("Byte");
	}
	ASTType* instance = new ASTType("Byte");
	instance->llvm_type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::ubyte_ty() {
	if (this->get("UByte") != NULL) {
		return this->get("UByte");
	}
	ASTType* instance = new ASTType("UByte");
	instance->llvm_type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::short_ty() {
	if (this->get("Short") != NULL) {
		return this->get("Short");
	}
	ASTType* instance = new ASTType("Short");
	instance->llvm_type = llvm::Type::getInt16Ty(llvm::getGlobalContext());
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::ushort_ty() {
	if (this->get("UShort") != NULL) {
		return this->get("UShort");
	}
	ASTType* instance = new ASTType("UShort");
	instance->llvm_type = llvm::Type::getInt16Ty(llvm::getGlobalContext());
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::int_ty() {
	if (this->get("Int") != NULL) {
		return this->get("Int");
	}
	ASTType* instance = new ASTType("Int");
	instance->llvm_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::uint_ty() {
	if (this->get("UInt") != NULL) {
		return this->get("UInt");
	}
	ASTType* instance = new ASTType("UInt");
	instance->llvm_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::long_ty() {
	if (this->get("Long") != NULL) {
		return this->get("Long");
	}
	ASTType* instance = new ASTType("Long");
	instance->llvm_type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::ulong_ty() {
	if (this->get("ULong") != NULL) {
		return this->get("ULong");
	}
	ASTType* instance = new ASTType("ULong");
	instance->llvm_type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::float_ty() {
	if (this->get("Float") != NULL) {
		return this->get("Float");
	}
	ASTType* instance = new ASTType("Float");
	instance->llvm_type = llvm::Type::getFloatTy(llvm::getGlobalContext());
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::double_ty() {
	if (this->get("Double") != NULL) {
		return this->get("Double");
	}
	ASTType* instance = new ASTType("Double");
	instance->llvm_type = llvm::Type::getDoubleTy(llvm::getGlobalContext());
	instance->primitive = true;
	return instance;
}
