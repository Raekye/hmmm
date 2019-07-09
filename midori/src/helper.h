#ifndef MIDORI_HELPER_H_INCLUDED
#define MIDORI_HELPER_H_INCLUDED

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
	inline void printf(const char* fmt, ...) {
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
	inline void logf(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}
}

#endif /* MIDORI_HELPER_H_INCLUDED */
