#ifndef __AST_TYPES_RESOLVER_H_
#define __AST_TYPES_RESOLVER_H_

#include <map>
#include "ast_type.h"

class ASTTypesResolver {
public:
	ASTTypesResolver();
	ASTType* get(std::string);

	ASTType* bit_ty();
	ASTType* byte_ty();
	ASTType* ubyte_ty();
	ASTType* short_ty();
	ASTType* ushort_ty();
	ASTType* int_ty();
	ASTType* uint_ty();
	ASTType* long_ty();
	ASTType* ulong_ty();
	ASTType* float_ty();
	ASTType* double_ty();

	~ASTTypesResolver();
private:
	std::map<std::string, ASTType*> types_map;

	void put(ASTType*);
};

#endif /* __AST_TYPES_RESOLVER_H_ */
