#include "ast_types_resolver.h"

ASTTypesResolver::ASTTypesResolver(llvm::LLVMContext& llvm_context) : llvm_context(llvm_context) {
	this->put(this->bit_ty(true));
	this->put(this->byte_ty(true));
	this->put(this->short_ty(true));
	this->put(this->int_ty(true));
	this->put(this->long_ty(true));
	this->put(this->ubyte_ty(true));
	this->put(this->ushort_ty(true));
	this->put(this->uint_ty(true));
	this->put(this->ulong_ty(true));
	this->put(this->float_ty(true));
	this->put(this->double_ty(true));
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

ASTType* ASTTypesResolver::bit_ty(bool create) {
	if (!create) {
		return this->get("Bit");
	}
	ASTType* instance = new ASTType("Bit");
	instance->llvm_type = llvm::Type::getInt1Ty(this->llvm_context);
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::byte_ty(bool create) {
	if (!create) {
		return this->get("Byte");
	}
	ASTType* instance = new ASTType("Byte");
	instance->llvm_type = llvm::Type::getInt8Ty(this->llvm_context);
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::ubyte_ty(bool create) {
	if (!create) {
		return this->get("UByte");
	}
	ASTType* instance = new ASTType("UByte");
	instance->llvm_type = llvm::Type::getInt8Ty(this->llvm_context);
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::short_ty(bool create) {
	if (!create) {
		return this->get("Short");
	}
	ASTType* instance = new ASTType("Short");
	instance->llvm_type = llvm::Type::getInt16Ty(this->llvm_context);
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::ushort_ty(bool create) {
	if (!create) {
		return this->get("UShort");
	}
	ASTType* instance = new ASTType("UShort");
	instance->llvm_type = llvm::Type::getInt16Ty(this->llvm_context);
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::int_ty(bool create) {
	if (!create) {
		return this->get("Int");
	}
	ASTType* instance = new ASTType("Int");
	instance->llvm_type = llvm::Type::getInt32Ty(this->llvm_context);
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::uint_ty(bool create) {
	if (!create) {
		return this->get("UInt");
	}
	ASTType* instance = new ASTType("UInt");
	instance->llvm_type = llvm::Type::getInt32Ty(this->llvm_context);
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::long_ty(bool create) {
	if (!create) {
		return this->get("Long");
	}
	ASTType* instance = new ASTType("Long");
	instance->llvm_type = llvm::Type::getInt64Ty(this->llvm_context);
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::ulong_ty(bool create) {
	if (!create) {
		return this->get("ULong");
	}
	ASTType* instance = new ASTType("ULong");
	instance->llvm_type = llvm::Type::getInt64Ty(this->llvm_context);
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::float_ty(bool create) {
	if (!create) {
		return this->get("Float");
	}
	ASTType* instance = new ASTType("Float");
	instance->llvm_type = llvm::Type::getFloatTy(this->llvm_context);
	instance->primitive = true;
	return instance;
}

ASTType* ASTTypesResolver::double_ty(bool create) {
	if (!create) {
		return this->get("Double");
	}
	ASTType* instance = new ASTType("Double");
	instance->llvm_type = llvm::Type::getDoubleTy(this->llvm_context);
	instance->primitive = true;
	return instance;
}
