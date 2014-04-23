#ifndef __NUMBER_H_
#define __NUMBER_H_

#include <cstdint>

typedef union tagNumberValue {
	int8_t b;
	uint8_t ub;
	int16_t s;
	uint16_t us;
	int32_t i;
	uint32_t ui;
	int64_t l;
	uint64_t ul;
	float f;
	double d;
} UNumberValue;

typedef enum tagNumberType {
	eBYTE = 0,
	eUBYTE = 1,
	eSHORT = (1 << 1),
	eUSHORT = (1 << 1) | 1,
	eINT = (1 << 2),
	eUINT = (1 << 2) | 1,
	eLONG = (1 << 3),
	eULONG = (1 << 3) | 1,
	eFLOAT = (1 << 4),
	eDOUBLE = (1 << 5),
} ENumberType;

class Number {
public:
	virtual Number* add(Number*) = 0;
	virtual Number* sub(Number*) = 0;
	virtual Number* mul(Number*) = 0;
	virtual Number* div(Number*) = 0;
	virtual Number* pow(Number*) = 0;
};

class PrimitiveNumber : Number {
public:
	PrimitiveNumber(ENumberType, UNumberValue);
	~PrimitiveNumber();
	virtual Number* add(Number*) override;
	virtual Number* sub(Number*) override;
	virtual Number* mul(Number*) override;
	virtual Number* div(Number*) override;
	virtual Number* pow(Number*) override;
	ENumberType type;
	UNumberValue val;
};

class ComplexNumber : Number {
public:
	Number* real;
	Number* imag;
};

class RationalNumber : Number {
public:

};

class BigIntNumber : Number {
public:
};

#endif // __NUMBER_H_
