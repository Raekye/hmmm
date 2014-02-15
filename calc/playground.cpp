#include <cstdlib>
#include <iostream>
#include <map>

void test_map_nonexisting_key() {
	std::map<std::string, std::string*> m;
	std::cout << "Is: " << m["what"] << std::endl;
}

int main() {
	test_map_nonexisting_key();
	return 0;
}