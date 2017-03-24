#ifndef SIYU_HELPER_H_INCLUDED
#define SIYU_HELPER_H_INCLUDED

#include <iostream>
#include <cstdio>
#include <cstdarg>

namespace mdk {
	template <typename T> void print(T t) {
		std::cout << t;// << std::endl;
	}
	template <typename T, typename... Args> void print(T t, Args... args) {
		std::cout << t;
		print(args...);
	}
	void printf(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		vfprintf(stdout, fmt, args);
		va_end(args);
	}
	template <typename T> void log(T t) {
		std::cerr << t;// << std::endl;
	}
	template <typename T, typename... Args> void log(T t, Args... args) {
		std::cerr << t;
		log(args...);
	}
	void logf(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}
}

#endif /* SIYU_HELPER_H_INCLUDED */
