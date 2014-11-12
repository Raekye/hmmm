#ifndef __AST_TYPES_RESOLVER_H_
#define __AST_TYPES_RESOLVER_H_

#include <map>
#include "ast_type.h"

class ASTTypesResolver {
public:
	llvm::LLVMContext& llvm_context;

	ASTTypesResolver(llvm::LLVMContext& llvm_context);
	ASTType* get(std::string);

	ASTType* bit_ty(bool = false);
	ASTType* byte_ty(bool = false);
	ASTType* ubyte_ty(bool = false);
	ASTType* short_ty(bool = false);
	ASTType* ushort_ty(bool = false);
	ASTType* int_ty(bool = false);
	ASTType* uint_ty(bool = false);
	ASTType* long_ty(bool = false);
	ASTType* ulong_ty(bool = false);
	ASTType* float_ty(bool = false);
	ASTType* double_ty(bool = false);

	~ASTTypesResolver();
private:
	std::map<std::string, ASTType*> types_map;

	void put(ASTType*);
};

#endif /* __AST_TYPES_RESOLVER_H_ */
