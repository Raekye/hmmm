#include <cstdlib>
#include <iostream>
#include <map>

void test_map_nonexisting_key() {
	std::cout << "Testing map non-existing key" << std::endl;
	std::map<std::string, std::string*> m;
	std::cout << "Is: " << m["what"] << std::endl;
}

void test_pointer_size() {
	std::cout << "Testing pointer size" << std::endl;
	std::cout << sizeof(int*) << std::endl;
}

int main() {
	test_map_nonexisting_key();
	test_pointer_size();
	return 0;
}
