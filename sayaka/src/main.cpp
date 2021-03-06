#include <iostream>
#include <fstream>
#include <boost/throw_exception.hpp>
#include "compiler.h"

#ifdef BOOST_NO_EXCEPTIONS
namespace boost {
	void throw_exception(const std::exception& e) {
		throw e;
	}
}
#endif // BOOST_NO_EXCEPTIONS

extern "C" {
	int32_t test_fn(int32_t x) {
		std::cout << x << std::endl;
		return x * 2;
	}
}

int main(int argc, char* argv[]) {
	Compiler compiler;
	compiler.llvm_initialize();
	compiler.initialize();
	std::cout << "Started." << std::endl;
	if (argc > 1) { std::string str = "";
		std::ifstream f(argv[1]);
		if (f.is_open()) {
			std::string line;
			while (std::getline(f, line)) {
				str += line + "\n";
			}
		}
		std::cout << "Reading from file, contents:\n" << str << std::endl;

		ASTNode* node = compiler.parse(str);
		if (node == NULL) {
			std::cout << "Root node was null" << std::endl;
		} else {
			compiler.run_code(node);
		}
		//delete node;
	} else {
		std::string line;
		while (true) {
			std::cout << "> ";
			if (!std::getline(std::cin, line)) {
				break;
			}
			ASTNode* node = compiler.parse(line);
			if (node == NULL) {
				std::cout << "Root node was null" << std::endl;
			} else {
				compiler.run_code(node);
			}
			//delete node;
		}
	}
	compiler.code_gen_context.module->dump();
	compiler.shutdown();
	compiler.llvm_shutdown();
	std::cout << "Done." << std::endl;
	return 0;
}
