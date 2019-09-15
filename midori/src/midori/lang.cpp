#include "lang.h"
#include "regex_engine.h"
#include <cstdlib>

typedef ParserValue<std::unique_ptr<LangAST>> ParserValueLang;
typedef ParserValue<std::unique_ptr<LangASTBlock>> ParserValueBlock;
typedef ParserValue<std::vector<std::unique_ptr<LangASTExpression>>> ParserValueExprList;
typedef ParserValue<std::vector<std::unique_ptr<LangASTDecl>>> ParserValueDeclList;

LangAST::~LangAST() {
	return;
}

LangASTExpression::~LangASTExpression() {
	return;
}

LangASTVoid::~LangASTVoid() {
	return;
}

ILangASTVisitor::~ILangASTVisitor() {
	return;
}

void LangASTBlock::accept(ILangASTVisitor* v) {
	v->visit(this);
}

void LangASTIdent::accept(ILangASTVisitor* v) {
	v->visit(this);
}

void LangASTDecl::accept(ILangASTVisitor* v) {
	v->visit(this);
}

void LangASTUnOp::accept(ILangASTVisitor* v) {
	v->visit(this);
}

void LangASTBinOp::accept(ILangASTVisitor* v) {
	v->visit(this);
}

template <typename T> void LangASTLiteral<T>::accept(ILangASTVisitor* v) {
	v->visit(this);
}

void LangASTIf::accept(ILangASTVisitor* v) {
	v->visit(this);
}

void LangASTWhile::accept(ILangASTVisitor* v) {
	v->visit(this);
}

void LangASTPrototype::accept(ILangASTVisitor* v) {
	v->visit(this);
}

void LangASTFunction::accept(ILangASTVisitor* v) {
	v->visit(this);
}

void LangASTCall::accept(ILangASTVisitor* v) {
	v->visit(this);
}

LangASTPrinter::LangASTPrinter() : indents(0) {
	return;
}

void LangASTPrinter::f() {
	std::cout << std::string(this->indents, '\t');
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
	std::cout << "{ decl " << v->name << ": " << v->type << " }" << std::endl;
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
	std::cout << "prototype " << v->name << "(";
	for (std::unique_ptr<LangASTDecl> const& a : v->args) {
		std::cout << a->name << ": " << a->type << ", ";
	}
	std::cout << ")" << std::endl;
}

void LangASTPrinter::visit(LangASTFunction* v) {
	f();
	std::cout << "function " << v->proto->name << "(";
	for (std::unique_ptr<LangASTDecl> const& a : v->proto->args) {
		std::cout << a->name << ": " << a->type << ", ";
	}
	std::cout << ") {" << std::endl;
	this->indents++;
	v->body->accept(this);
	this->indents--;
	f();
	std::cout << "} endfunction " << v->proto->name << std::endl;
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
	p->add_token("SEMICOLON", re.parse(";"));
	p->add_token("COLON", re.parse(":"));
	p->add_token("PLUS", re.parse("\\+"));
	p->add_token("MINUS", re.parse("-"));
	p->add_token("STAR", re.parse("\\*"));
	p->add_token("SLASH", re.parse("/"));
	p->add_token("ARROW", re.parse("->"));

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

	p->add_production("statement", { "var_declaration" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("statement", { "expression" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});

	p->add_production("block", { "if_block" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("block", { "while_block" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("block", { "function_proto", "SEMICOLON" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});

	p->add_production("var_declaration", { "VAR", "declaration" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(1)->value);
	});

	p->add_production("declaration", { "IDENTIFIER", "COLON", "TYPE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangAST> d(new LangASTDecl(m->terminal(0)->token->lexeme, m->terminal(2)->token->lexeme));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(d)));
	});

	p->add_production("expression", { "IDENTIFIER" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangAST> l(new LangASTIdent(m->terminal(0)->token->lexeme));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(l)));
	});
	p->add_production("expression", { "literal" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("expression", { "LPAREN", "statement", "RPAREN" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(1)->value);
	});
	p->add_production("expression", { "un_op", "statement" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		LangASTUnOp::Op op = m->nonterminal(0)->value->get<LangASTUnOp::Op>();
		std::unique_ptr<LangAST> e = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTExpression> e2(dynamic_cast<LangASTExpression*>(e.release()));
		std::unique_ptr<LangAST> u(new LangASTUnOp(op, std::move(e2)));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(u)));
	});
	p->add_production("expression", { "statement", "bin_op", "statement" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		LangASTBinOp::Op op = m->nonterminal(1)->value->get<LangASTBinOp::Op>();
		std::unique_ptr<LangAST> l = std::move(m->nonterminal(0)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangAST> r = std::move(m->nonterminal(2)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTExpression> l2(dynamic_cast<LangASTExpression*>(l.release()));
		std::unique_ptr<LangASTExpression> r2(dynamic_cast<LangASTExpression*>(r.release()));
		std::unique_ptr<LangAST> b(new LangASTBinOp(op, std::move(l2), std::move(r2)));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(b)));
	});
	p->add_production("expression", { "IDENTIFIER", "LPAREN", "args_list", "RPAREN" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string name = m->terminal(0)->token->lexeme;
		std::vector<std::unique_ptr<LangASTExpression>> args = std::move(m->nonterminal(2)->value->get<std::vector<std::unique_ptr<LangASTExpression>>>());
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::unique_ptr<LangAST>(new LangASTCall(name, std::move(args)))));
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
	p->add_production("bin_op", { "EQUALS" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::ASSIGN));
	});
	p->add_production("bin_op", { "EQUALS", "EQUALS" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::EQ));
	});
	p->add_production("bin_op", { "NOT", "EQUALS" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
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
	p->add_production("bin_op", { "LANGLE", "EQUALS" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::LE));
	});
	p->add_production("bin_op", { "RANGLE", "EQUALS" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValue<LangASTBinOp::Op>(LangASTBinOp::Op::GE));
	});

	p->add_production("args_list", { "args_list", "expression" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::vector<std::unique_ptr<LangASTExpression>> args = std::move(m->nonterminal(0)->value->get<std::vector<std::unique_ptr<LangASTExpression>>>());
		std::unique_ptr<LangAST> e = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTExpression> e2(dynamic_cast<LangASTExpression*>(e.release()));
		args.push_back(std::move(e2));
		return std::unique_ptr<ParserAST>(new ParserValueExprList(std::move(args)));
	});
	p->add_production("args_list", {}, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValueExprList({}));
	});

	p->add_production("if_block", { "IF", "statement", "LBRACE", "lines", "RBRACE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangAST> p = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTExpression> p2(dynamic_cast<LangASTExpression*>(p.release()));
		std::unique_ptr<LangASTBlock> b = std::move(m->nonterminal(3)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangAST> i(new LangASTIf(std::move(p2), std::move(b), nullptr));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(i)));
	});
	p->add_production("if_block", { "IF", "statement", "LBRACE", "lines", "RBRACE", "ELSE", "LBRACE", "lines", "RBRACE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangAST> p = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTExpression> p2(dynamic_cast<LangASTExpression*>(p.release()));
		std::unique_ptr<LangASTBlock> b1 = std::move(m->nonterminal(3)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangASTBlock> b2 = std::move(m->nonterminal(5)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangAST> i(new LangASTIf(std::move(p2), std::move(b1), std::move(b2)));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(i)));
	});
	p->add_production("if_block", { "IF", "statement", "LBRACE", "lines", "RBRACE", "ELSE", "if_block" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangAST> p = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTExpression> p2(dynamic_cast<LangASTExpression*>(p.release()));
		std::unique_ptr<LangASTBlock> b1 = std::move(m->nonterminal(3)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangAST> elif = std::move(m->nonterminal(5)->value->get<std::unique_ptr<LangAST>>());
		std::vector<std::unique_ptr<LangAST>> v;
		v.push_back(std::move(elif));
		std::unique_ptr<LangASTBlock> b2(new LangASTBlock(std::move(v)));
		std::unique_ptr<LangAST> i(new LangASTIf(std::move(p2), std::move(b1), std::move(b2)));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(i)));
	});

	p->add_production("while_block", { "WHILE", "statement", "LBRACE", "lines", "RBRACE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<LangAST> p = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTExpression> p2(dynamic_cast<LangASTExpression*>(p.release()));
		std::unique_ptr<LangASTBlock> b = std::move(m->nonterminal(3)->value->get<std::unique_ptr<LangASTBlock>>());
		std::unique_ptr<LangAST> i(new LangASTWhile(std::move(p2), std::move(b)));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(i)));
	});

	p->add_production("function_proto", { "DEF", "IDENTIFIER", "LPAREN", "decls_list", "RPAREN", "ARROW", "TYPE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string name = m->terminal(1)->token->lexeme;
		std::string return_type = m->terminal(6)->token->lexeme;
		std::vector<std::unique_ptr<LangASTDecl>> args = std::move(m->nonterminal(3)->value->get<std::vector<std::unique_ptr<LangASTDecl>>>());
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::unique_ptr<LangAST>(new LangASTPrototype(name, return_type, std::move(args)))));
	});

	p->add_production("decls_list", { "decls_list", "declaration" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::vector<std::unique_ptr<LangASTDecl>> args = std::move(m->nonterminal(0)->value->get<std::vector<std::unique_ptr<LangASTDecl>>>());
		std::unique_ptr<LangAST> e = std::move(m->nonterminal(1)->value->get<std::unique_ptr<LangAST>>());
		std::unique_ptr<LangASTDecl> e2(dynamic_cast<LangASTDecl*>(e.release()));
		args.push_back(std::move(e2));
		return std::unique_ptr<ParserAST>(new ParserValueDeclList(std::move(args)));
	});
	p->add_production("decls_list", {}, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserValueDeclList({}));
	});

	p->add_production("literal", { "FLOAT" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		double d = std::strtod(m->terminal(0)->token->lexeme.c_str(), nullptr);
		std::unique_ptr<LangAST> l(new LangASTDouble(d));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(l)));
	});
	p->add_production("literal", { "INT" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		Long x = std::strtod(m->terminal(0)->token->lexeme.c_str(), nullptr);
		std::unique_ptr<LangAST> l(new LangASTInt(x));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(l)));
	});
	p->add_production("literal", { "HEX" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		Long x = std::strtod(m->terminal(0)->token->lexeme.c_str(), nullptr);
		std::unique_ptr<LangAST> l(new LangASTInt(x));
		return std::unique_ptr<ParserAST>(new ParserValueLang(std::move(l)));
	});

	p->generate(Parser::Type::LALR1, "code");
}
