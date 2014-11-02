#include "ast_type.h"
#include <algorithm>

ASTType::ASTType(std::string name, std::vector<ASTType*> extends = std::vector<ASTType*>(), std::vector<ASTType*> implements = std::vector<ASTType*>(), llvm::Type* llvm_type = NULL) {
	if (ASTType::get(name) != NULL) {
		// TODO: badness
	}
	this->name = name;
	this->extends = extends;
	this->implements = implements;
	ASTType::types[this->name] = this;
}

bool ASTType::is_primitive() {
	static std::vector<ASTType*> primitives { ASTType::byte_ty(), ASTType::ubyte_ty(), ASTType::short_ty(), ASTType::ushort_ty(), ASTType::int_ty(), ASTType::uint_ty(), ASTType::long_ty(), ASTType::ulong_ty(), ASTType::float_ty(), ASTType::double_ty() };
	return std::find(primitives.begin(), primitives.end(), this) != primitives.end();
}

bool ASTType::is_signed() {
	if (!this->is_primitive()) {
		throw std::logic_error("Member function is_signed called on non primitive type");
	}
	return this->name[0] != 'U';
}

bool ASTType::is_integral() {
	if (!this->is_primitive()) {
		throw std::logic_error("Member function is_integral called on non primitive type");
	}
	return !this->is_floating();
}

bool ASTType::is_floating() {
	if (!this->is_primitive()) {
		throw std::logic_error("Member function is_floating called on non primitive type");
	}
	return this == ASTType::float_ty() || this == ASTType::double_ty();
}

ASTType* ASTType::byte_ty() {
	static ASTType instance("Byte");
	instance.llvm_type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
	return &instance;
}

ASTType* ASTType::ubyte_ty() {
	static ASTType instance("UByte");
	instance.llvm_type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
	return &instance;
}

ASTType* ASTType::short_ty() {
	static ASTType instance("Short");
	instance.llvm_type = llvm::Type::getInt16Ty(llvm::getGlobalContext());
	return &instance;
}

ASTType* ASTType::ushort_ty() {
	static ASTType instance("UShort");
	instance.llvm_type = llvm::Type::getInt16Ty(llvm::getGlobalContext());
	return &instance;
}

ASTType* ASTType::int_ty() {
	static ASTType instance("Int");
	instance.llvm_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
	return &instance;
}

ASTType* ASTType::uint_ty() {
	static ASTType instance("UInt");
	instance.llvm_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
	return &instance;
}

ASTType* ASTType::long_ty() {
	static ASTType instance("Long");
	instance.llvm_type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
	return &instance;
}

ASTType* ASTType::ulong_ty() {
	static ASTType instance("ULong");
	instance.llvm_type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
	return &instance;
}

ASTType* ASTType::float_ty() {
	static ASTType instance("Float");
	instance.llvm_type = llvm::Type::getFloatTy(llvm::getGlobalContext());
	return &instance;
}

ASTType* ASTType::double_ty() {
	static ASTType instance("Double");
	instance.llvm_type = llvm::Type::getDoubleTy(llvm::getGlobalContext());
	return &instance;
}

ASTType* ASTType::get(std::string name) {
	std::map<std::string, ASTType*>::iterator it = ASTType::types.find(name);
	if (it != ASTType::types.end()) {
		return it->second;
	}
	return NULL;
}

int ASTType::__STATIC_INITIALIZER() {
	ASTType::byte_ty();
	ASTType::ubyte_ty();
	ASTType::short_ty();
	ASTType::ushort_ty();
	ASTType::int_ty();
	ASTType::uint_ty();
	ASTType::long_ty();
	ASTType::ulong_ty();
	ASTType::float_ty();
	ASTType::double_ty();
	return 0;
}

std::map<std::string, ASTType*> ASTType::types;
int ASTType::__STATIC_INIT = ASTType::__STATIC_INITIALIZER();
