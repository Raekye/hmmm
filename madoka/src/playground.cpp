#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>

void test_map_nonexisting_key() {
	std::cout << "Testing map non-existing key" << std::endl;
	std::map<std::string, std::string*> m;
	std::cout << "Is: " << m["what"] << std::endl;
}

void test_pointer_size() {
	std::cout << "Testing pointer size" << std::endl;
	std::cout << sizeof(int*) << std::endl;
}

void test_string_concat() {
	std::cout << "Testing string concat" << std::endl;
	std::stringstream strstream;
	strstream << "Test " << 1.2;
	std::cout << strstream.str() << std::endl;
}

int main() {
	test_map_nonexisting_key();
	test_pointer_size();
	test_string_concat();
	return 0;
}
