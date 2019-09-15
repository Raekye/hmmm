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

CodeGen::CodeGen() : builder(this->context), module("midori", this->context), type_manager(&(this->context)) {
	llvm::BasicBlock::Create(this->context, "main");
}

void CodeGen::process(LangAST* program) {
	TypeChecker tc(&(this->type_manager));
	program->accept(&tc);
	program->accept(this);
}

std::error_code CodeGen::dump(std::string path) {
	std::error_code ec;
	llvm::raw_fd_ostream o(path, ec, llvm::sys::fs::OpenFlags::F_None);
	llvm::WriteBitcodeToFile(&(this->module), o);
	o.close();
	return ec;
}

void CodeGen::visit(LangASTBlock* v) {
	this->push_scope();
	for (std::unique_ptr<LangAST> const& l : v->lines) {
		l->accept(this);
	}
	this->pop_scope();
	this->ret = nullptr;
}

void CodeGen::visit(LangASTIdent* v) {
	this->ret = this->builder.CreateLoad(this->named_value(v->name), v->name);
}

void CodeGen::visit(LangASTDecl* v) {
	llvm::Type* t = this->type_manager.get(v->type_name)->llvm_type;
	llvm::Value* x = this->builder.CreateAlloca(t, nullptr, v->name);
	this->frames.front()[v->name] = x;
	this->ret = x;
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
	v->right->accept(this);
	llvm::Value* rhs = this->ret;
	if (v->op == LangASTBinOp::Op::ASSIGN) {
		std::string name;
		if (LangASTIdent* i = dynamic_cast<LangASTIdent*>(v->left.get())) {
			name = i->name;
		} else if (LangASTDecl* d = dynamic_cast<LangASTDecl*>(v->left.get())) {
			this->visit(d);
			name = d->name;
		} else {
			this->ret = nullptr;
			return;
		}
		llvm::Value* lhs = this->named_value(name);
		this->ret = this->builder.CreateStore(rhs, lhs);
		return;
	}
	v->left->accept(this);
	llvm::Value* lhs = this->ret;
	switch (v->op) {
	case LangASTBinOp::Op::PLUS:
		this->ret = this->builder.CreateAdd(lhs, rhs, "addtmp");
		break;
	case LangASTBinOp::Op::MINUS:
		this->ret = this->builder.CreateSub(lhs, rhs, "subtmp");
		break;
	case LangASTBinOp::Op::STAR:
		this->ret = this->builder.CreateMul(lhs, rhs, "multmp");
		break;
	case LangASTBinOp::Op::SLASH:
		this->ret = this->builder.CreateSDiv(lhs, rhs, "divtmp");
		break;
	case LangASTBinOp::Op::EQ:
		this->ret = this->builder.CreateICmpEQ(lhs, rhs, "eqtmp");
		break;
	case LangASTBinOp::Op::NE:
		this->ret = this->builder.CreateICmpNE(lhs, rhs, "netmp");
		break;
	case LangASTBinOp::Op::LT:
		this->ret = this->builder.CreateICmpSLT(lhs, rhs, "lttmp");
		break;
	case LangASTBinOp::Op::GT:
		this->ret = this->builder.CreateICmpSGT(lhs, rhs, "gttmp");
		break;
	case LangASTBinOp::Op::LE:
		this->ret = this->builder.CreateICmpSLE(lhs, rhs, "letmp");
		break;
	case LangASTBinOp::Op::GE:
		this->ret = this->builder.CreateICmpSGE(lhs, rhs, "getmp");
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

	this->builder.CreateCondBr(cond, then_bb, else_bb);

	this->builder.SetInsertPoint(then_bb);
	v->block_if->accept(this);

	this->builder.CreateBr(merge_bb);

	f->getBasicBlockList().push_back(else_bb);
	this->builder.SetInsertPoint(else_bb);

	if (v->block_else != nullptr) {
		v->block_else->accept(this);
	}

	this->builder.CreateBr(merge_bb);

	f->getBasicBlockList().push_back(merge_bb);
	this->builder.SetInsertPoint(merge_bb);
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
	this->builder.CreateBr(cond_bb);
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
	llvm::FunctionType* ft = llvm::FunctionType::get(this->type_manager.get(v->return_type)->llvm_type, arg_types, false);
	llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, v->name, &(this->module));
	Int i = 0;
	for (llvm::Argument& a : f->args()) {
		a.setName(v->args.at(i)->name);
		i++;
	}
	this->ret = f;
}

void CodeGen::visit(LangASTFunction* v) {
	v->proto->accept(this);
	llvm::Function* f = llvm::dyn_cast<llvm::Function>(this->ret);
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(this->context, "entry", f);
	this->builder.SetInsertPoint(bb);

	this->push_scope();
	std::map<std::string, llvm::Value*>& m = this->frames.front();
	for (llvm::Argument& a : f->args()) {
		m[a.getName()] = &a;
	}
	v->body->accept(this);
	llvm::Value* z = llvm::ConstantInt::get(this->type_manager.get("Int")->llvm_type, 0, true);
	this->builder.CreateRet(z);

	llvm::verifyFunction(*f);

	f->print(llvm::errs());

	this->pop_scope();
	this->ret = f;
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
	this->ret = this->builder.CreateCall(f, args, "calltmp");
}

void CodeGen::push_scope() {
	this->frames.emplace_front();
}

void CodeGen::pop_scope() {
	this->frames.pop_front();
}

llvm::Value* CodeGen::named_value(std::string s) {
	for (std::map<std::string, llvm::Value*> const& f : this->frames) {
		std::map<std::string, llvm::Value*>::const_iterator it = f.find(s);
		if (it != f.end()) {
			return it->second;
		}
	}
	return nullptr;
}

llvm::Function* CodeGen::get_function(std::string name) {
	if (llvm::Function* f = this->module.getFunction(name)) {
		return f;
	}
	return nullptr;
}
