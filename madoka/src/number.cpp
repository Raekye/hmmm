#include "number.h"
#include <stdexcept>
#include <cstdlib>
#include <algorithm>

PrimitiveNumber::PrimitiveNumber(ENumberType type, UNumberValue val) {
	this->type = type;
	this->val = val;
}

PrimitiveNumber::~PrimitiveNumber() {
	return;
}

Number* PrimitiveNumber::add(Number* y) {
	return NULL;
}

Number* PrimitiveNumber::sub(Number* y) {
	return NULL;
}

Number* PrimitiveNumber::mul(Number* y) {
	return NULL;
}

Number* PrimitiveNumber::div(Number* y) {
	return NULL;
}

Number* PrimitiveNumber::pow(Number* y) {
	return NULL;
}
