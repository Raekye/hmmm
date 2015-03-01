#include <iostream>
#include "regex.h"

int main() {
	std::string str = "abc|def|ghi{3,5}";
	RegexParser parser;
	RegexAST* r = parser.parse(str);
	RegexASTPrinter printer;
	r->accept(&printer);
	return 0;
}
