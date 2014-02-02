#include "node.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;

/* Compile the AST into a module */
void CodeGenContext::generateCode(NExpression* root)
{
	return;
	// std::cout << "Generating code...\n";
	
	// /* Create the top level interpreter function to call as entry */
	// vector<const Type*> argTypes;
	// FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
	// mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", module);
	// BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);
	
	// /* Push a new variable/block context */
	// pushBlock(bblock);
	// root.codeGen(*this); /* emit bytecode for the toplevel block */
	// ReturnInst::Create(getGlobalContext(), bblock);
	// popBlock();
	
	// /* Print the bytecode in a human-readable format 
	//    to see if our program compiled properly
	//  */
	// std::cout << "Code is generated.\n";
	// PassManager pm;
	// pm.add(createPrintModulePass(&outs()));
	// pm.run(*module);
}

/* Executes the AST by running the main function */
void CodeGenContext::runCode() {
	// std::cout << "Running code...\n";
	// ExistingModuleProvider *mp = new ExistingModuleProvider(module);
	// ExecutionEngine *ee = ExecutionEngine::create(mp, false);
	// vector<GenericValue> noargs;
	// GenericValue v = ee->runFunction(mainFunction, noargs);
	// std::cout << "Code was run.\n";
	// return v;
}
/* -- Code Generation -- */

// Value* NInteger::codeGen(CodeGenContext& context)
// {
// 	std::cout << "Creating integer: " << value << std::endl;
// 	return ConstantInt::get(Type::getInt64Ty(getGlobalContext()), value, true);
// }

// Value* NDouble::codeGen(CodeGenContext& context)
// {
// 	std::cout << "Creating double: " << value << std::endl;
// 	return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
// }

Value* NPrimitiveNumber::gen_code(CodeGenContext& context) {
	// TODO: hmmmmsauce
	return NULL;
}

Value* NBinaryOperator::gen_code(CodeGenContext& context)
{
	return NULL;
	// std::cout << "Creating binary operation " << op << std::endl;
	// Instruction::BinaryOps instr;
	// switch (op) {
	// 	case TPLUS: instr = Instruction::Add; goto math;
	// 	case TMINUS: instr = Instruction::Sub; goto math;
	// 	case TMUL: instr = Instruction::Mul; goto math;
	// 	case TDIV: instr = Instruction::SDiv; goto math;
				
	// 	/* TODO comparison */
	// }

	// math:
	// return BinaryOperator::Create(instr, lhs.code_gen(context), rhs.code_gen(context), "", context.currentBlock());
}