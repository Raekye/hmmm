#ifndef __AST_TYPE_H_
#define __AST_TYPE_H_

#include <string>
#include <vector>
#include <map>
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>

class ASTType {
public:
	std::string name;
	llvm::Type* llvm_type;
	bool primitive;

	ASTType(std::string, llvm::Type* = NULL, bool = false);

	bool is_primitive();
	bool is_signed();
	bool is_integral();
	bool is_floating();
};

#endif // __NTYPE_H_
