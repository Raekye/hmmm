#ifndef __NUMBER_H_
#define __NUMBER_H_

#include <cstdint>

typedef union tagNumberValue {
	int8_t b;
	uint8_t ub;
	int16_t is;
	uint16_t us;
	int32_t i;
	uint32_t ui;
	int64_t l;
	uint64_t ul;
	float f;
	double d;
} UNumberValue;

typedef enum tagNumberType {
	eBYTE,
	eUBYTE,
	eSHORT,
	eUSHORT,
	eINT,
	eUINT,
	eLONG,
	eULONG,
	eFLOAT,
	eDOUBLE,
} ENumberType;

/*
 * - primitive <-> primitive closed
 * - primitive integral <-> bigint closed
 * - number -> complex closed
 * - rational <-> primitive closed
 * - rational <-> bigint closed
 *
 * - byte -> short -> int -> long -> float -> double
 * - anything -> complex
 * - primitive -> rational
 * - primitive -> bigint
 * - rational <-> bigint (strong)
 */

class Number {
public:
	static virtual Number add(Number, Number);
	static virtual Number subtract(Number, Number);
	static virtual Number multiply(Number, Number);
	static virtual Number divide(Number, Number);
}

class PrimitiveNumber : Number {
private:
	UNumberValue val;
	ENumberType type;
}

class ComplexNumber : Number {
private:
	Number real;
	Number imaginary;
}

class RationalNumber : Number {
private:

}

class BigIntNumber : Number {

}

#endif // __NUMBER_H_