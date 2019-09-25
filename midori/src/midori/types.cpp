#include "types.h"
#include <tuple>
#include <utility>

Type::Type(std::string name, llvm::Type* type, TypeManager* m) : name(name), llvm_type(type), manager(m), _pointer_ty(nullptr), _array_ty(nullptr) {
	return;
}

Type::~Type() {
	return;
}

bool Type::is_primitive() {
	return this->llvm_type->isIntegerTy() || this->llvm_type->isFloatingPointTy();
}

PointerType* Type::pointer_ty() {
	if (this->_pointer_ty == nullptr) {
		this->_pointer_ty = this->manager->make_pointer(this);
	}
	return this->_pointer_ty;
}

ArrayType* Type::array_ty() {
	if (this->_array_ty == nullptr) {
		this->_array_ty = this->manager->make_array(this);
	}
	return this->_array_ty;
}

PrimitiveType::PrimitiveType(std::string name, llvm::Type* type, TypeManager* m) : Type(name, type, m) {
	return;
}

StructType::StructType(std::string name, llvm::Type* type, TypeManager* m) : Type(name, type, m) {
	return;
}

void StructType::set_fields(std::vector<Field> v) {
	std::vector<llvm::Type*> llvm_fields;
	Int i = 0;
	for (Field const& f : v) {
		llvm_fields.push_back(f.type->llvm_type);
		std::map<std::string, Field>::iterator it;
		bool inserted;
		std::tie(it, inserted) = this->fields_map.emplace(std::piecewise_construct, std::forward_as_tuple(f.name), std::forward_as_tuple(f));
		assert(inserted);
		it->second.index = i;
		i++;
	}
	llvm::dyn_cast<llvm::StructType>(this->llvm_type)->setBody(llvm_fields);
	this->fields = v;
}

StructType::Field StructType::field(std::string s) {
	std::map<std::string, Field>::iterator it = this->fields_map.find(s);
	if (it == this->fields_map.end()) {
		return Field(s, nullptr);
	}
	return it->second;
}

PointerType::PointerType(Type* t) : Type(t->name + "*", llvm::PointerType::get(t->llvm_type, 0), t->manager), base(t) {
	return;
}

ArrayType::ArrayType(Type* t) : Type(t->name + "[]", llvm::PointerType::get(t->llvm_type, 0), t->manager), base(t) {
	return;
}

// TODO: why this works? isn't the types map uninitialized
TypeManager::TypeManager(llvm::LLVMContext* c) : context(c) {
	this->_void_type = this->make_primitive("Void", llvm::Type::getVoidTy(*c));
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

		this->make_primitive(kv.first, kv.second);
	}
}

Type* TypeManager::get(std::string name) {
	std::map<std::string, std::unique_ptr<Type>>::iterator it = this->types.find(name);
	return (it == this->types.end()) ? nullptr : it->second.get();
}

StructType* TypeManager::make_struct(std::string name) {
	llvm::StructType* llvm_type = llvm::StructType::create(*(this->context), name);
	std::unique_ptr<StructType> t(new StructType(name, llvm_type, this));
	StructType* ret = t.get();
	this->register_type(std::move(t));
	return ret;
}

PointerType* TypeManager::make_pointer(Type* type) {
	std::unique_ptr<PointerType> t(new PointerType(type));
	PointerType* ret = t.get();
	this->register_type(std::move(t));
	return ret;
}

ArrayType* TypeManager::make_array(Type* type) {
	std::unique_ptr<ArrayType> t(new ArrayType(type));
	ArrayType* ret = t.get();
	this->register_type(std::move(t));
	return ret;
}

PrimitiveType* TypeManager::make_primitive(std::string name, llvm::Type* type) {
	std::unique_ptr<PrimitiveType> t(new PrimitiveType(name, type, this));
	PrimitiveType* ret = t.get();
	this->register_type(std::move(t));
	return ret;
}

void TypeManager::register_type(std::unique_ptr<Type> t) {
	std::map<std::string, std::unique_ptr<Type>>::iterator it = this->types.find(t->name);
	assert(it == this->types.end());
	this->types[t->name] = std::move(t);
}
