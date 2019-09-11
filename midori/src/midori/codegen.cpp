#include "codegen.h"
#include "types.h"
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

std::error_code CodeGen::dump(std::string path) {
	std::error_code ec;
	llvm::raw_fd_ostream o(path, ec, llvm::sys::fs::OpenFlags::F_None);
	llvm::WriteBitcodeToFile(&(this->module), o);
	o.close();
	return ec;
}

void CodeGen::visit(LangASTIdent* v) {
	this->ret = this->builder.CreateLoad(this->named_value(v->name), v->name.c_str());
}

void CodeGen::visit(LangASTDecl* v) {
	llvm::Type* t = this->type_manager.get(v->type)->llvm_type;
	llvm::Value* x = this->builder.CreateAlloca(t, 0, v->name.c_str());
	this->frames.front()[v->name] = x;
	this->ret = x;
}

void CodeGen::visit(LangASTBinOp* v) {
	v->right->accept(this);
	llvm::Value* rhs = this->ret;
	if (v->op == '=') {
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
	case '+':
		this->ret = this->builder.CreateAdd(lhs, rhs, "addtmp");
		break;
	case '-':
		this->ret = this->builder.CreateSub(lhs, rhs, "subtmp");
		break;
	case '*':
		this->ret = this->builder.CreateMul(lhs, rhs, "multmp");
		break;
	case '/':
		this->ret = this->builder.CreateSDiv(lhs, rhs, "divtmp");
		break;
	default:
		this->ret = nullptr;
	}
}

void CodeGen::visit(LangASTInt* v) {
	this->ret = llvm::ConstantInt::get(this->type_manager.get("Int")->llvm_type, v->value, true);
}

void CodeGen::visit(LangASTDouble* v) {
	this->ret = llvm::ConstantFP::get(this->type_manager.get("Double")->llvm_type, v->value);
}

void CodeGen::visit(LangASTIf* v) {
	(void) v;
	/*
	v->predicate->accept(this);
	llvm::Value* cond = this->builder.CreateICmpNE(this->ret, llvm::ConstantInt::get(llvm::Type::getInt64Ty(this->context), 0, true));
	llvm::BasicBlock* then_bb = llvm::BasicBlock::Create(this->context, "then");
	llvm::BasicBlock* else_bb = llvm::BasicBlock::Create(this->context, "else");
	llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(this->context, "ifcont");

	this->builder.CreateCondBr(cond, then_bb, else_bb);

	this->builder.SetInsertPoint(then_bb);
	llvm::Value* then_value = nullptr;
	for (std::unique_ptr<LangAST> const& l : v->block) {
		l->accept(this);
		then_value = this->ret;
	}
	*/
}

void CodeGen::visit(LangASTWhile* v) {
	(void) v;
}

void CodeGen::visit(LangASTPrototype* v) {
	std::vector<llvm::Type*> arg_types;
	for (std::unique_ptr<LangASTDecl> const& a : v->args) {
		arg_types.push_back(this->type_manager.get(a->type)->llvm_type);
	}
	llvm::FunctionType* ft = llvm::FunctionType::get(this->type_manager.get(v->return_type)->llvm_type, arg_types, false);
	llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, v->name, &(this->module));
	Int i = 0;
	for (llvm::Argument& a : f->args()) {
		a.setName(v->args.at(i)->name);
		i++;
	}
	this->ret_function = f;
}

void CodeGen::visit(LangASTFunction* v) {
	v->proto->accept(this);
	llvm::Function* f = dynamic_cast<llvm::Function*>(this->ret_function);
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(this->context, "entry", f);
	this->builder.SetInsertPoint(bb);

	this->frames.emplace_front();
	std::map<std::string, llvm::Value*>& m = this->frames.front();
	for (llvm::Argument& a : f->args()) {
		m[a.getName()] = &a;
	}
	for (std::unique_ptr<LangAST> const& l : v->body) {
		l->accept(this);
	}
	this->frames.pop_front();
	this->builder.CreateRet(this->ret);

	llvm::verifyFunction(*f);

	f->print(llvm::errs());

	this->ret_function = f;
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
