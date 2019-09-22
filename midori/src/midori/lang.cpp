#include "lang.h"
#include "regex_engine.h"
#include <cstdlib>
#include <cassert>

typedef ParserValue<std::unique_ptr<LangAST>> ParserValueLang;
typedef ParserValue<std::unique_ptr<LangASTExpression>> ParserValueExpr;
typedef ParserValue<std::unique_ptr<LangASTBlock>> ParserValueBlock;
typedef ParserValue<std::vector<std::unique_ptr<LangASTExpression>>> ParserValueExprList;
typedef ParserValue<std::vector<std::unique_ptr<LangASTDecl>>> ParserValueDeclList;
typedef ParserValue<std::unique_ptr<LangASTType>> ParserValueType;

LangAST::~LangAST() {
	return;
}

LangASTExpression::~LangASTExpression() {
	return;
}

LangASTVoid::~LangASTVoid() {
	return;
}

LangASTType::~LangASTType() {
	return;
}

ILangASTVisitor::~ILangASTVisitor() {
	return;
}

LangASTPrinter::LangASTPrinter() : indents(0) {
	return;
}

void LangASTPrinter::f() {
	std::cout << std::string(this->indents, '\t');
}

void LangASTPrinter::visit(LangASTBasicType* v) {
	std::cout << v->name;
}

void LangASTPrinter::visit(LangASTPointerType* v) {
	std::cout << v->name;
}

void LangASTPrinter::visit(LangASTArrayType* v) {
	std::cout << v->name;
}

void LangASTPrinter::visit(LangASTBlock* v) {
	f();
	std::cout << "{" << std::endl;
	this->indents++;
	for (std::unique_ptr<LangAST> const& l : v->lines) {
		l->accept(this);
	}
	this->indents--;
	f();
	std::cout << "}" << std::endl;
}

void LangASTPrinter::visit(LangASTIdent* v) {
	f();
	std::cout << "{ ident " << v->name << " }" << std::endl;
}

void LangASTPrinter::visit(LangASTDecl* v) {
	f();
	std::cout << "{ decl " << v->name << ": ";
	v->decl_type->accept(this);
	std::cout << " }" << std::endl;
}

void LangASTPrinter::visit(LangASTUnOp* v) {
	f();
	std::cout << "unop " << v->op << " {" << std::endl;
	this->indents++;
	v->expr->accept(this);
	this->indents--;
	f();
	std::cout << "} endunop " << (char) v->op << std::endl;
}

void LangASTPrinter::visit(LangASTBinOp* v) {
	f();
	std::cout << "binop " << v->op << " {" << std::endl;
	this->indents++;
	v->left->accept(this);
	f();
	std::cout << std::string(8, '=') << std::endl;
	v->right->accept(this);
	this->indents--;
	f();
	std::cout << "} endbinop " << (char) v->op << std::endl;
}

void LangASTPrinter::visit(LangASTInt* v) {
	f();
	std::cout << "{ literal int " << v->value << " }" << std::endl;
}

void LangASTPrinter::visit(LangASTDouble* v) {
	f();
	std::cout << "{ literal float " << v->value << " }" << std::endl;
}

void LangASTPrinter::visit(LangASTIf* v) {
	f();
	std::cout << "if {" << std::endl;
	this->indents++;
	v->predicate->accept(this);
	f();
	std::cout << std::string(8, '=') << std::endl;
	v->block_if->accept(this);
	if (v->block_else != nullptr) {
		this->indents--;
		f();
		std::cout << "} else {" << std::endl;
		this->indents++;
		v->block_else->accept(this);

	}
	this->indents--;
	f();
	std::cout << "} endif " << std::endl;
}

void LangASTPrinter::visit(LangASTWhile* v) {
	f();
	std::cout << "while {" << std::endl;
	this->indents++;
	v->predicate->accept(this);
	f();
	std::cout << std::string(8, '=') << std::endl;
	v->block->accept(this);
	this->indents--;
	f();
	std::cout << "} endwhile " << std::endl;
}

void LangASTPrinter::visit(LangASTPrototype* v) {
	f();
	std::cout << "prototype " << v->name << " {" << std::endl;
	this->indents++;
	for (std::unique_ptr<LangASTDecl> const& a : v->args) {
		a->accept(this);
	}
	this->indents--;
	f();
	std::cout << "}" << std::endl;
}

void LangASTPrinter::visit(LangASTFunction* v) {
	f();
	std::cout << "function " << v->proto->name << " {" << std::endl;
	this->indents++;
	for (std::unique_ptr<LangASTDecl> const& a : v->proto->args) {
		a->accept(this);
	}
	this->indents--;
	f();
	std::cout << "} body {" << std::endl;
	this->indents++;
	v->body->accept(this);
	this->indents--;
	f();
	std::cout << "} endfunction " << v->proto->name << std::endl;
}

void LangASTPrinter::visit(LangASTReturn* v) {
	f();
	std::cout << "return";
	if (v->val == nullptr) {
		std::cout << std::endl;
	} else {
		std::cout << " ";
		v->val->accept(this);
	}
}

void LangASTPrinter::visit(LangASTCall* v) {
	f();
	std::cout << "call " << v->function << " {" << std::endl;
	this->indents++;
	for (std::unique_ptr<LangASTExpression> const& a : v->args) {
		a->accept(this);
	}
	this->indents--;
	f();
	std::cout << "} endcall " << v->function << std::endl;
}

Lang::Lang() {
	this->generate();
}

std::unique_ptr<LangASTBlock> Lang::parse(IInputStream* is) {
	std::unique_ptr<MatchedNonterminal> code = this->parser.parse(is);
	return std::move(code->value->get<std::unique_ptr<LangASTBlock>>());
}

void Lang::generate() {
	RegexEngine re;
	Parser* p = &(this->parser);

	p->add_token("WHITESPACE", re.parse("[ \\t\\n]+"));
	p->add_skip("WHITESPACE");
	p->add_token("FLOAT", re.parse("[0-9]+(.[0-9]+)?(e-?[0-9]+)"));
	p->add_token("INT", re.parse("[0-9]+"));
	p->add_token("HEX", re.parse("0x[0-9a-f]+"));
	p->add_token("VAR", re.parse("var"));
	p->add_token("LET", re.parse("let"));
	p->add_token("DEF", re.parse("def"));
	p->add_token("IF", re.parse("if"));
	p->add_token("ELSE", re.parse("else"));
	p->add_token("WHILE", re.parse("while"));
	p->add_token("BREAK", re.parse("break"));
	p->add_token("RETURN", re.parse("return"));
	p->add_token("IDENTIFIER", re.parse("[a-z][a-zA-Z0-9_]*"));
	p->add_token("TYPE", re.parse("[A-Z][a-zA-Z0-9_]*"));
	p->add_token("EQUALS", re.parse("="));
	p->add_token("NOT", re.parse("!"));
	p->add_token("LPAREN", re.parse("\\("));
	p->add_token("RPAREN", re.parse("\\)"));
	p->add_token("LBRACE", re.parse("\\{"));
	p->add_token("RBRACE", re.parse("\\}"));
	p->add_token("LANGLE", re.parse("<"));
	p->add_token("RANGLE", re.parse(">"));
	p->add_token("LBRACKET", re.parse("\\["));
	p->add_token("RBRACKET", re.parse("\\]"));
	p->add_token("SEMICOLON", re.parse(";"));
	p->add_token("COLON", re.parse(":"));
	p->add_token("COMMA", re.parse(","));
	p->add_token("PLUS", re.parse("\\+"));
	p->add_token("MINUS", re.parse("-"));
	p->add_token("STAR", re.parse("\\*"));
	p->add_token("SLASH", re.parse("/"));
	p->add_token("ARROW", re.parse("->"));
	p->add_token("EQ", re.parse("=="));
	p->add_token("NE", re.parse("!="));
	p->add_token("LE", re.parse("<="));
	p->add_token("GE", re.parse(">="));

	p->add_production("code", { "lines" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});

	p->add_production("lines", { "lines", "line_entry" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTBlock> b = std::move(m->nonterminal(0)->value->get<std::unique_ptr<LangASTBlock>>());
		b->lines.push_back(std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangAST>>()));
		return std::unique_ptr<ParserAST>(new ParserValueBlock(std::move(b)));
	});
	p->add_production("lines", {}, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValueBlock(std::unique_ptr<LangASTBlock>(new LangASTBlock({}))));
	});

	p->add_production("line_entry", { "statement", "SEMICOLON" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("line_entry", { "block" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});

	p->add_production("statement", { "expression_void" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("statement", { "expression" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTExpression> e = std::move(m->nonterminal(0)->value->get<std::unique_ptr<LangASTExpression>>());
		std::unique_ptr<LangAST> e2 = std::move(e);
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(e2)));
	});

	p->add_production("block", { "if_block" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("block", { "while_block" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("block", { "plain_block" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("block", { "function_def" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});

	p->add_production("expression_void", { "var_declaration" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("expression_void", { "function_proto" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("expression_void", { "function_return" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});

	p->add_production("var_declaration", { "VAR", "declaration" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(1)->value);
	});

	p->add_production("declaration", { "IDENTIFIER", "COLON", "type" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTType> t = std::move(m->nonterminal(2)->value->get<std::unique_ptr<LangASTType>>());
		std::unique_ptr<LangAST> d(new LangASTDecl(m->terminal(0)->token->lexeme, std::move(t)));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(d)));
	});

	p->add_production("type", { "TYPE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::unique_ptr<ParserAST>(new ParserValueType(std::unique_ptr<LangASTType>(new LangASTBasicType(m->terminal(0)->token->lexeme))));
	});
	p->add_production("type", { "type", "STAR" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTType> t = std::move(m->nonterminal(0)->value->get<std::unique_ptr<LangASTType>>());
		return std::unique_ptr<ParserAST>(new ParserValueType(std::unique_ptr<LangASTType>(new LangASTPointerType(std::move(t)))));
	});
	p->add_production("type", { "type", "LBRACKET", "RBRACKET" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTType> t = std::move(m->nonterminal(0)->value->get<std::unique_ptr<LangASTType>>());
		return std::unique_ptr<ParserAST>(new ParserValueType(std::unique_ptr<LangASTType>(new LangASTArrayType(std::move(t)))));
	});

	p->add_production("function_proto", { "DEF", "IDENTIFIER", "LPAREN", "decls_list", "RPAREN", "ARROW", "type" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string name = m->terminal(1)->token->lexeme;
		std::unique_ptr<LangASTType> return_type = std::move(m->nonterminal(6)->value->get<std::unique_ptr<LangASTType>>());
		std::vector<std::unique_ptr<LangASTDecl>> args = std::move(m->nonterminal(3)->value->get<std::vector<std::unique_ptr<LangASTDecl>>>());
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::unique_ptr<LangAST>(new LangASTPrototype(name, std::move(return_type), std::move(args)))));
	});

	p->add_production("decls_list", { "decls_list_nonempty" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("decls_list", {}, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValueDeclList({}));
	});

	p->add_production("decls_list_nonempty", { "declaration" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::vector<std::unique_ptr<LangASTDecl>> v;
		std::unique_ptr<LangAST> d = std::move(m->nonterminal(0)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTDecl> d2(dynamic_cast<LangASTDecl*>(d.release()));
		v.push_back(std::move(d2));
		return std::unique_ptr<ParserAST>(new ParserValueDeclList(std::move(v)));
	});
	p->add_production("decls_list_nonempty", { "decls_list_nonempty", "COMMA", "declaration" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::vector<std::unique_ptr<LangASTDecl>> args = std::move(m->nonterminal(0)->value->get<std::vector<std::unique_ptr<LangASTDecl>>>());
		std::unique_ptr<LangAST> d = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTDecl> d2(dynamic_cast<LangASTDecl*>(d.release()));
		args.push_back(std::move(d2));
		return std::unique_ptr<ParserAST>(new ParserValueDeclList(std::move(args)));
	});

	p->add_production("function_return", { "RETURN" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::unique_ptr<LangAST>(new LangASTReturn(nullptr))));
	});
	p->add_production("function_return", { "RETURN", "expression" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTExpression> v = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangASTExpression>>());
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::unique_ptr<LangAST>(new LangASTReturn(std::move(v)))));
	});

	p->add_production("expression", { "identifier" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("expression", { "literal" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("expression", { "LPAREN", "expression", "RPAREN" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(1)->value);
	});
	p->add_production("expression", { "assignment" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("expression", { "un_op", "expression" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		LangASTUnOp::Op op = m->nonterminal(0)->value->get<LangASTUnOp::Op>();
		std::unique_ptr<LangASTExpression> e = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangASTExpression>>());
		std::unique_ptr<LangASTExpression> u(new LangASTUnOp(op, std::move(e)));
		return std::unique_ptr<ParserAST>(new ParserValueExpr(std::move(u)));
	});
	p->add_production("expression", { "expression", "bin_op", "expression" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		LangASTBinOp::Op op = m->nonterminal(1)->value->get<LangASTBinOp::Op>();
		std::unique_ptr<LangASTExpression> l = std::move(m->nonterminal(0)->value->get<std::unique_ptr<LangASTExpression>>());
		std::unique_ptr<LangASTExpression> r = std::move(m->nonterminal(2)->value->get<std::unique_ptr<LangASTExpression>>());
		std::unique_ptr<LangASTExpression> b(new LangASTBinOp(op, std::move(l), std::move(r)));
		return std::unique_ptr<ParserAST>(new ParserValueExpr(std::move(b)));
	});
	p->add_production("expression", { "IDENTIFIER", "LPAREN", "args_list", "RPAREN" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string name = m->terminal(0)->token->lexeme;
		std::vector<std::unique_ptr<LangASTExpression>> args = std::move(m->nonterminal(2)->value->get<std::vector<std::unique_ptr<LangASTExpression>>>());
		return std::unique_ptr<ParserAST>(new ParserValueExpr(std::unique_ptr<LangASTExpression>(new LangASTCall(name, std::move(args)))));
	});

	p->add_production("identifier", { "IDENTIFIER" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTExpression> l(new LangASTIdent(m->terminal(0)->token->lexeme));
		return std::unique_ptr<ParserAST>(new ParserValueExpr(std::move(l)));
	});

	p->add_production("assignment", { "var_declaration", "EQUALS", "expression" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangAST> v = std::move(m->nonterminal(0)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTExpression> v2(dynamic_cast<LangASTExpression*>(v.release()));
		std::unique_ptr<LangASTExpression> e = std::move(m->nonterminal(2)->value->get<std::unique_ptr<LangASTExpression>>());
		std::unique_ptr<LangASTExpression> b(new LangASTBinOp(LangASTBinOp::Op::ASSIGN, std::move(v2), std::move(e)));
		return std::unique_ptr<ParserAST>(new ParserValueExpr(std::move(b)));
	});
	p->add_production("assignment", { "identifier", "EQUALS", "expression" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTExpression> v = std::move(m->nonterminal(0)->value->get<std::unique_ptr<LangASTExpression>>());
		std::unique_ptr<LangASTExpression> e = std::move(m->nonterminal(2)->value->get<std::unique_ptr<LangASTExpression>>());
		std::unique_ptr<LangASTExpression> b(new LangASTBinOp(LangASTBinOp::Op::ASSIGN, std::move(v), std::move(e)));
		return std::unique_ptr<ParserAST>(new ParserValueExpr(std::move(b)));
	});

	p->add_production("un_op", { "MINUS" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTUnOp::Op>(LangASTUnOp::Op::MINUS));
	});
	p->add_production("un_op", { "NOT" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTUnOp::Op>(LangASTUnOp::Op::NOT));
	});

	p->add_production("bin_op", { "PLUS" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::PLUS));
	});
	p->add_production("bin_op", { "MINUS" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::MINUS));
	});
	p->add_production("bin_op", { "STAR" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::STAR));
	});
	p->add_production("bin_op", { "SLASH" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::SLASH));
	});
	p->add_production("bin_op", { "EQ" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::EQ));
	});
	p->add_production("bin_op", { "NE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::NE));
	});
	p->add_production("bin_op", { "LANGLE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::LT));
	});
	p->add_production("bin_op", { "RANGLE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::GT));
	});
	p->add_production("bin_op", { "LE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::LE));
	});
	p->add_production("bin_op", { "GE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::GE));
	});

	p->add_production("args_list", { "args_list_nonempty" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("args_list", {}, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValueExprList({}));
	});

	p->add_production("args_list_nonempty", { "expression" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::vector<std::unique_ptr<LangASTExpression>> v;
		v.push_back(std::move(m->nonterminal(0)->value->get<std::unique_ptr<LangASTExpression>>()));
		return std::unique_ptr<ParserAST>(new ParserValueExprList(std::move(v)));
	});
	p->add_production("args_list_nonempty", { "args_list_nonempty", "COMMA", "expression" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::vector<std::unique_ptr<LangASTExpression>> args = std::move(m->nonterminal(0)->value->get<std::vector<std::unique_ptr<LangASTExpression>>>());
		std::unique_ptr<LangASTExpression> e = std::move(m->nonterminal(2)->value->get<std::unique_ptr<LangASTExpression>>());
		args.push_back(std::move(e));
		return std::unique_ptr<ParserAST>(new ParserValueExprList(std::move(args)));
	});

	p->add_production("if_block", { "IF", "expression", "LBRACE", "lines", "RBRACE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTExpression> p = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangASTExpression>>());
		std::unique_ptr<LangASTBlock> b = std::move(m->nonterminal(3)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangAST> i(new LangASTIf(std::move(p), std::move(b), nullptr));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(i)));
	});
	p->add_production("if_block", { "IF", "expression", "LBRACE", "lines", "RBRACE", "ELSE", "LBRACE", "lines", "RBRACE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTExpression> p = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangASTExpression>>());
		std::unique_ptr<LangASTBlock> b1 = std::move(m->nonterminal(3)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangASTBlock> b2 = std::move(m->nonterminal(7)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangAST> i(new LangASTIf(std::move(p), std::move(b1), std::move(b2)));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(i)));
	});
	p->add_production("if_block", { "IF", "expression", "LBRACE", "lines", "RBRACE", "ELSE", "if_block" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTExpression> p = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangASTExpression>>());
		std::unique_ptr<LangASTBlock> b1 = std::move(m->nonterminal(3)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangAST> elif = std::move(m->nonterminal(5)->value->get<std::unique_ptr<LangAST>>());
		std::vector<std::unique_ptr<LangAST>> v;
		v.push_back(std::move(elif));
		std::unique_ptr<LangASTBlock> b2(new LangASTBlock(std::move(v)));
		std::unique_ptr<LangAST> i(new LangASTIf(std::move(p), std::move(b1), std::move(b2)));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(i)));
	});

	p->add_production("while_block", { "WHILE", "expression", "LBRACE", "lines", "RBRACE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTExpression> p = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangASTExpression>>());
		std::unique_ptr<LangASTBlock> b = std::move(m->nonterminal(3)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangAST> i(new LangASTWhile(std::move(p), std::move(b)));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(i)));
	});

	p->add_production("plain_block", { "LBRACE", "lines", "RBRACE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangASTBlock> b = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangAST> b2 = std::move(b);
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(b2)));
	});

	p->add_production("function_def", { "function_proto", "LBRACE", "lines", "RBRACE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangAST> proto = std::move(m->nonterminal(0)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTPrototype> proto2(dynamic_cast<LangASTPrototype*>(proto.release()));
		std::unique_ptr<LangASTBlock> b = std::move(m->nonterminal(2)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangASTFunction> f(new LangASTFunction(std::move(proto2), std::move(b)));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(f)));
	});

	p->add_production("literal", { "FLOAT" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		double d = std::strtod(m->terminal(0)->token->lexeme.c_str(), nullptr);
		std::unique_ptr<LangASTExpression> l(new LangASTDouble(d));
		return std::unique_ptr<ParserAST>(new ParserValueExpr(std::move(l)));
	});
	p->add_production("literal", { "INT" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		Long x = std::strtod(m->terminal(0)->token->lexeme.c_str(), nullptr);
		std::unique_ptr<LangASTExpression> l(new LangASTInt(x));
		return std::unique_ptr<ParserAST>(new ParserValueExpr(std::move(l)));
	});
	p->add_production("literal", { "HEX" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		Long x = std::strtod(m->terminal(0)->token->lexeme.c_str(), nullptr);
		std::unique_ptr<LangASTExpression> l(new LangASTInt(x));
		return std::unique_ptr<ParserAST>(new ParserValueExpr(std::move(l)));
	});

	p->set_precedence_class("assign", 10, Precedence::Associativity::LEFT);
	p->set_precedence("EQUALS", "assign");
	p->set_precedence_class("binop_left_eq", 20, Precedence::Associativity::LEFT);
	p->set_precedence("EQ", "binop_left_eq");
	p->set_precedence("NE", "binop_left_eq");
	p->set_precedence("LANGLE", "binop_left_eq");
	p->set_precedence("RANGLE", "binop_left_eq");
	p->set_precedence("LE", "binop_left_eq");
	p->set_precedence("GE", "binop_left_eq");
	p->set_precedence_class("binop_left_add", 30, Precedence::Associativity::LEFT);
	p->set_precedence("PLUS", "binop_left_add");
	p->set_precedence("MINUS", "binop_left_add");
	p->set_precedence_class("binop_left_mul", 40, Precedence::Associativity::LEFT);
	p->set_precedence("STAR", "binop_left_mul");
	p->set_precedence("SLASH", "binop_left_mul");
	p->set_precedence_class("right", 50, Precedence::Associativity::RIGHT);
	p->set_precedence("LPAREN", "right");
	p->set_precedence("NOT", "right");

	p->generate(Parser::Type::LALR1, "code");
	std::cout << "===== LANG" << std::endl;
	p->debug();
	std::cout << "===== CONFLICTS" << std::endl;
	for (GrammarConflict const& gc : p->conflicts()) {
		p->debug_grammar_conflict(gc);
	}
	std::cout << "===== " << p->conflicts().size() << " conflicts" << std::endl;
	assert(p->conflicts().size() == 0);
}
