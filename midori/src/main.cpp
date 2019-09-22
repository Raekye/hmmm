#include <sstream>
#include <fstream>
#include "midori/helper.h"
#include "midori/regex_ast.h"
//#include "midori/lexer.h"
#include "midori/parser.h"
#include "midori/regex_engine.h"
#include "midori/interval_tree.h"
#include "midori/generator.h"
#include "midori/lang.h"
#include "midori/codegen.h"
#include "llvm/Support/TargetSelect.h"
//#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/ManagedStatic.h"

int test_regex_engine() {
	RegexEngine re;
	//std::string pattern = "(abc){0,3}[a-zA-Z]|def.\\.[^a-zA-Z]?+-^\\n+[^\\t\\xff-\\u12345678]";
	std::string pattern = "[-a-b-cd---]{3}abc";
	std::unique_ptr<RegexAST> r = re.parse(pattern);
	RegexASTPrinter printer;
	r->accept(&printer);
	return 0;
}

int test_generator() {
	std::fstream f("../src/parser.txt", std::fstream::in);
	std::unique_ptr<Parser> p = ParserGenerator::from_file(&f);
	std::stringstream ss;
	ss << "a1ab2b \tc3cd4d";
	FileInputStream fis(&ss);
	std::unique_ptr<MatchedNonterminal> m = p->parse(&fis);
	assert(m != nullptr);
	return 0;
}

int test_lang() {
	Lang l;
	std::fstream f("../src/program.txt", std::fstream::in);
	FileInputStream fis(&f);
	std::unique_ptr<LangASTBlock> program = l.parse(&fis);
	LangASTPrinter p;
	program->accept(&p);
	CodeGen cg;
	std::vector<std::unique_ptr<LangASTDecl>> v;
	//v.push_back(std::unique_ptr<LangASTDecl>(new LangASTDecl("foo", "Int")));
	std::unique_ptr<LangASTPrototype> proto(new LangASTPrototype("main", std::unique_ptr<LangASTType>(new LangASTBasicType("Int")), std::move(v)));
	LangASTFunction g(std::move(proto), std::move(program));
	cg.process(&g);
	cg.dump("program.bc");
	cg.run();
	return 0;
}

extern "C" {
	void putint(int x) {
		std::cout << "int " << x << std::endl;
	}
}

void foo(int& x) {
	x = 3;
}

int main(int argc, char* argv[]) {
	//llvm::InitLLVM _llvm(argc, argv);
	(void) argc;
	(void) argv;
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();
	ULong x = ~0;
	std::cout << "-1 is " << x << std::endl;
	test_regex_engine();
	test_generator();
	test_lang();
	int y = 2;
	foo(y);
	std::cout << "y is " << y << std::endl;
	llvm::llvm_shutdown();
	return 0;
}
