#include "codegen.h"
#include "types.h"
#include "type_checker.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/MCJIT.h"
//#include "llvm/ExecutionEngine/Interpreter.h"
#include <tuple>
#include <utility>
#include <cassert>

CodeGen::CodeGen() : builder(this->context), module(new llvm::Module("midori", this->context)), type_manager(&(this->context)) {
	llvm::BasicBlock::Create(this->context, "main");
}

void CodeGen::process(LangAST* program) {
	TypeChecker tc(&(this->type_manager));
	program->accept(&tc);
	program->accept(this);
	llvm::verifyModule(*(this->module), &(llvm::errs()));
}

std::error_code CodeGen::dump(std::string path) {
	std::error_code ec;
	llvm::raw_fd_ostream o(path, ec, llvm::sys::fs::OpenFlags::F_None);
	llvm::WriteBitcodeToFile(this->module.get(), o);
	return ec;
}

void CodeGen::run() {
	llvm::Function* f = this->get_function("main");
	llvm::ExecutionEngine* ee = llvm::EngineBuilder(std::move(this->module)).create();
	ee->runFunction(f, {});
}

void CodeGen::visit(LangASTBasicType* v) {
	(void) v;
}

void CodeGen::visit(LangASTPointerType* v) {
	(void) v;
}

void CodeGen::visit(LangASTArrayType* v) {
	(void) v;
}

void CodeGen::visit(LangASTBlock* v) {
	this->push_scope();
	for (std::unique_ptr<LangAST> const& l : v->lines) {
		l->accept(this);
	}
	this->pop_scope();
	this->ret = nullptr;
}

void CodeGen::visit(LangASTLIdent* v) {
	LangASTLIdent::NameOrIndex& ni = v->parts.at(0);
	assert(ni.index == nullptr);
	assert(ni.name.length() > 0);
	std::string name = ni.name;
	Variable var = this->named_value(ni.name);
	for (size_t i = 1; i < v->parts.size(); i++) {
		LangASTLIdent::NameOrIndex& ni2 = v->parts.at(i);
		if (ni2.index == nullptr) {
			assert(ni2.name.length() > 0);
			name += "." + ni2.name;
			StructType* st = dynamic_cast<StructType*>(var.type);
			assert(st != nullptr);
			StructType::Field f = st->field(ni2.name);
			assert(f.type != nullptr);
			assert(f.index >= 0);
			var.type = f.type;
			var.value = this->builder.CreateStructGEP(st->llvm_type, var.value, f.index, name);
		} else {
			assert(ni2.name.length() == 0);
			name += "[]";
			ArrayType* at = dynamic_cast<ArrayType*>(var.type);
			assert(at != nullptr);
			ni2.index->accept(this);
			llvm::Value* index = this->ret;
			var.type = at->base;
			var.value = this->builder.CreateGEP(at->llvm_type, var.value, index, name);
		}
	}
	this->ret = var.value;
}

void CodeGen::visit(LangASTRIdent* v) {
	v->ident->accept(this);
	this->ret = this->builder.CreateLoad(this->ret, "load");
}

void CodeGen::visit(LangASTDecl* v) {
	llvm::Type* t = this->type_manager.get(v->decl_type->name)->llvm_type;
	llvm::Value* x = this->builder.CreateAlloca(t, nullptr, v->name);
	this->set_named_value(v->name, Variable(v->type, x));
	this->ret = x;
}

void CodeGen::visit(LangASTAssignment* v) {
	v->left->accept(this);
	llvm::Value* lhs = this->ret;
	v->right->accept(this);
	llvm::Value* rhs = this->ret;
	this->builder.CreateStore(rhs, lhs);
	// this->ret = rhs;
}

void CodeGen::visit(LangASTUnOp* v) {
	v->expr->accept(this);
	llvm::Value* expr = this->ret;
	switch (v->op) {
	case LangASTUnOp::Op::MINUS:
		this->ret = this->builder.CreateNeg(expr, "negtmp");
		break;
	case LangASTUnOp::Op::NOT:
		this->ret = this->builder.CreateNot(expr, "nottmp");
		break;
	default:
		this->ret = nullptr;
	}
}

void CodeGen::visit(LangASTBinOp* v) {
	v->left->accept(this);
	llvm::Value* lhs = this->ret;
	v->right->accept(this);
	llvm::Value* rhs = this->ret;
	llvm::Type* type = lhs->getType();
	this->ret = nullptr;
	switch (v->op) {
	case LangASTBinOp::Op::PLUS:
		if (type->isIntegerTy()) {
			this->ret = this->builder.CreateAdd(lhs, rhs, "addtmp");
		} else if (type->isFloatingPointTy()) {
			this->ret = this->builder.CreateFAdd(lhs, rhs, "addftmp");
		}
		break;
	case LangASTBinOp::Op::MINUS:
		if (type->isIntegerTy()) {
			this->ret = this->builder.CreateSub(lhs, rhs, "subtmp");
		} else if (type->isFloatingPointTy()) {
			this->ret = this->builder.CreateFSub(lhs, rhs, "subftmp");
		}
		break;
	case LangASTBinOp::Op::STAR:
		if (type->isIntegerTy()) {
			this->ret = this->builder.CreateMul(lhs, rhs, "multmp");
		} else if (type->isFloatingPointTy()) {
			this->ret = this->builder.CreateFMul(lhs, rhs, "mulftmp");
		}
		break;
	case LangASTBinOp::Op::SLASH:
		if (type->isIntegerTy()) {
			this->ret = this->builder.CreateSDiv(lhs, rhs, "divtmp");
		} else if (type->isFloatingPointTy()) {
			this->ret = this->builder.CreateFDiv(lhs, rhs, "divftmp");
		}
		break;
	case LangASTBinOp::Op::EQ:
		if (type->isIntegerTy()) {
			this->ret = this->builder.CreateICmpEQ(lhs, rhs, "eqtmp");
		} else if (type->isFloatingPointTy()) {
			this->ret = this->builder.CreateFCmpOEQ(lhs, rhs, "eqftmp");
		}
		break;
	case LangASTBinOp::Op::NE:
		if (type->isIntegerTy()) {
			this->ret = this->builder.CreateICmpNE(lhs, rhs, "netmp");
		} else if (type->isFloatingPointTy()) {
			this->ret = this->builder.CreateFCmpONE(lhs, rhs, "neftmp");
		}
		break;
	case LangASTBinOp::Op::LT:
		if (type->isIntegerTy()) {
			this->ret = this->builder.CreateICmpSLT(lhs, rhs, "lttmp");
		} else if (type->isFloatingPointTy()) {
			this->ret = this->builder.CreateFCmpOLT(lhs, rhs, "ltftmp");
		}
		break;
	case LangASTBinOp::Op::GT:
		if (type->isIntegerTy()) {
			this->ret = this->builder.CreateICmpSGT(lhs, rhs, "gttmp");
		} else if (type->isFloatingPointTy()) {
			this->ret = this->builder.CreateFCmpOGT(lhs, rhs, "gtftmp");
		}
		break;
	case LangASTBinOp::Op::LE:
		if (type->isIntegerTy()) {
			this->ret = this->builder.CreateICmpSGE(lhs, rhs, "getmp");
		} else if (type->isFloatingPointTy()) {
			this->ret = this->builder.CreateFCmpOGE(lhs, rhs, "geftmp");
		}
		break;
	case LangASTBinOp::Op::GE:
		if (type->isIntegerTy()) {
			this->ret = this->builder.CreateICmpSLE(lhs, rhs, "letmp");
		} else if (type->isFloatingPointTy()) {
			this->ret = this->builder.CreateFCmpOLE(lhs, rhs, "leftmp");
		}
		break;
	default:
		this->ret = nullptr;
	}
}

void CodeGen::visit(LangASTInt* v) {
	Type* t = v->type;
	if ((t == this->type_manager.get("Float")) || (t == this->type_manager.get("Double"))) {
		this->ret = llvm::ConstantFP::get(t->llvm_type, (double) v->value);
		return;
	}
	this->ret = llvm::ConstantInt::get(t->llvm_type, v->value, true);
}

void CodeGen::visit(LangASTDouble* v) {
	this->ret = llvm::ConstantFP::get(v->type->llvm_type, v->value);
}

void CodeGen::visit(LangASTIf* v) {
	this->push_scope();
	v->predicate->accept(this);
	llvm::Value* cond = this->ret;
	llvm::Function* f = this->builder.GetInsertBlock()->getParent();
	llvm::BasicBlock* then_bb = llvm::BasicBlock::Create(this->context, "then", f);
	llvm::BasicBlock* else_bb = llvm::BasicBlock::Create(this->context, "else");
	llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(this->context, "ifcont");
	int x = 0;

	this->builder.CreateCondBr(cond, then_bb, else_bb);

	this->builder.SetInsertPoint(then_bb);
	v->block_if->accept(this);

	if (this->builder.GetInsertBlock()->getTerminator() == nullptr) {
		this->builder.CreateBr(merge_bb);
		x++;
	}

	f->getBasicBlockList().push_back(else_bb);
	this->builder.SetInsertPoint(else_bb);

	if (v->block_else != nullptr) {
		v->block_else->accept(this);
	}

	if (this->builder.GetInsertBlock()->getTerminator() == nullptr) {
		this->builder.CreateBr(merge_bb);
		x++;
	}

	if (x > 0) {
		f->getBasicBlockList().push_back(merge_bb);
		this->builder.SetInsertPoint(merge_bb);
	}
	this->pop_scope();
	this->ret = nullptr;
}

void CodeGen::visit(LangASTWhile* v) {
	this->push_scope();
	llvm::Function* f = this->builder.GetInsertBlock()->getParent();
	llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(this->context, "cond", f);
	llvm::BasicBlock* loop_bb = llvm::BasicBlock::Create(this->context, "loop");
	llvm::BasicBlock* after_bb = llvm::BasicBlock::Create(this->context, "after");
	this->builder.CreateBr(cond_bb);
	this->builder.SetInsertPoint(cond_bb);
	v->predicate->accept(this);
	llvm::Value* cond = this->ret;
	this->builder.CreateCondBr(cond, loop_bb, after_bb);
	f->getBasicBlockList().push_back(loop_bb);
	this->builder.SetInsertPoint(loop_bb);
	v->block->accept(this);
	if (this->builder.GetInsertBlock()->getTerminator() == nullptr) {
		this->builder.CreateBr(cond_bb);
	}
	f->getBasicBlockList().push_back(after_bb);
	this->builder.SetInsertPoint(after_bb);
	this->pop_scope();
	this->ret = nullptr;
}

void CodeGen::visit(LangASTPrototype* v) {
	std::vector<llvm::Type*> arg_types;
	for (std::unique_ptr<LangASTDecl> const& a : v->args) {
		arg_types.push_back(a->type->llvm_type);
	}
	llvm::FunctionType* ft = llvm::FunctionType::get(this->type_manager.get(v->return_type->name)->llvm_type, arg_types, false);
	llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, v->name, this->module.get());
	Int i = 0;
	for (llvm::Argument& a : f->args()) {
		a.setName(v->args.at(i)->name);
		i++;
	}
	this->ret = f;
}

void CodeGen::visit(LangASTFunction* v) {
	this->push_scope();
	llvm::BasicBlock* old = this->builder.GetInsertBlock();
	v->proto->accept(this);
	llvm::Function* f = llvm::dyn_cast<llvm::Function>(this->ret);
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(this->context, "entry", f);
	this->builder.SetInsertPoint(bb);

	Int i = 0;
	for (llvm::Argument& a : f->args()) {
		v->proto->args.at(i)->accept(this);
		llvm::Value* alloc = this->ret;
		this->builder.CreateStore(&a, alloc);
		i++;
	}
	v->body->accept(this);
	if (this->builder.GetInsertBlock()->getTerminator() == nullptr) {
		if (v->proto->return_type->name == this->type_manager.void_type()->name) {
			this->builder.CreateRetVoid();
		}
	}

	f->print(llvm::errs());
	llvm::verifyFunction(*f, &(llvm::errs()));

	this->pop_scope();
	this->builder.SetInsertPoint(old);
	this->ret = f;
}

void CodeGen::visit(LangASTReturn* v) {
	if (v->val == nullptr) {
		this->ret = this->builder.CreateRetVoid();
		return;
	}
	v->val->accept(this);
	this->ret = this->builder.CreateRet(this->ret);
}

void CodeGen::visit(LangASTCall* v) {
	llvm::Function* f = this->get_function(v->function);
	if (f->arg_size() != v->args.size()) {
		this->ret = nullptr;
		return;
	}
	std::vector<llvm::Value*> args;
	for (std::unique_ptr<LangASTExpression> const& a : v->args) {
		a->accept(this);
		args.push_back(this->ret);
	}
	if (f->getReturnType() == this->type_manager.void_type()->llvm_type) {
		this->ret = this->builder.CreateCall(f, args);
		return;
	}
	this->ret = this->builder.CreateCall(f, args, "calltmp");
}

void CodeGen::visit(LangASTClassDef* v) {
	(void) v;
}

void CodeGen::push_scope() {
	this->frames.emplace_front();
}

void CodeGen::pop_scope() {
	this->frames.pop_front();
}

CodeGen::Variable CodeGen::named_value(std::string s) {
	for (std::map<std::string, Variable> const& f : this->frames) {
		std::map<std::string, Variable>::const_iterator it = f.find(s);
		if (it != f.end()) {
			return it->second;
		}
	}
	return Variable(nullptr, nullptr);
}

void CodeGen::set_named_value(std::string s, Variable v) {
	std::map<std::string, Variable>& m = this->frames.front();
	std::map<std::string, Variable>::iterator it;
	bool inserted;
	std::tie(it, inserted) = m.emplace(std::piecewise_construct, std::forward_as_tuple(s), std::forward_as_tuple(v));
	assert(inserted);
}

llvm::Function* CodeGen::get_function(std::string name) {
	if (llvm::Function* f = this->module->getFunction(name)) {
		return f;
	}
	return nullptr;
}
