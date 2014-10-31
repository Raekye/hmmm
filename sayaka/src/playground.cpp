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

void test_string_stream() {
	std::cout << "Testing string stream" << std::endl;
	std::stringstream ss;
	//std::cout << { ss << "Hello " << "World"; ss.str() } << std::endl;
}

class A {
public:
	virtual ~A() { std::cout << "A" << std::endl; };
};

class B : public A {
};

class C : public B {
public:
	~C() { std::cout << "C" << std::endl; };
};

class D : public C {
public:
	~D() { std::cout << "D" << std::endl; };
};

void test_virtual_destructors() {
	std::cout << "Testing virtual destructors" << std::endl;
	{
		D d;
	}
	std::cout << "---" << std::endl;
	{
		B b;
	}
	std::cout << "---" << std::endl;
}

int main() {
	test_map_nonexisting_key();
	test_pointer_size();
	test_string_concat();
	test_string_stream();
	test_virtual_destructors();
	return 0;
}
