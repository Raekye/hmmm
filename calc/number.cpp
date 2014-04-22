#include "number.h"
#include <stdexcept>
#include <cstdlib>
#include <algorithm>

static ENumberType primitive_number_normalized_type(ENumberType, ENumberType);
static double primitive_number_double_value(PrimitiveNumber*);
static int64_t primitive_number_long_value(PrimitiveNumber*);

PrimitiveNumber::PrimitiveNumber(ENumberType type, UNumberValue val) {
	this->type = type;
	this->val = val;
}

PrimitiveNumber::~PrimitiveNumber() {
	return;
}

Number* PrimitiveNumber::add(Number* y) {
	PrimitiveNumber* pn = (PrimitiveNumber*) y;//dynamic_cast<PrimitiveNumber*>(y);
	if (pn != NULL) {
		ENumberType larger = primitive_number_normalized_type(this->type, pn->type);
		if (larger == eLONG) {
			int64_t x = primitive_number_long_value(this);
			int64_t z = primitive_number_long_value(pn);
			UNumberValue val;
			val.l = x + z;
			return new PrimitiveNumber(larger, val);
		} else if (larger == eDOUBLE) {
			double x = primitive_number_double_value(this);
			double z = primitive_number_double_value(pn);
			UNumberValue val;
			val.d = x + z;
			return new PrimitiveNumber(larger, val);
		}
	}
	throw std::logic_error("Unhandled class");
	return NULL;
}

Number* PrimitiveNumber::sub(Number* y) {
	PrimitiveNumber* pn = dynamic_cast<PrimitiveNumber*>(y);
	if (pn != NULL) {
		ENumberType larger = primitive_number_normalized_type(this->type, pn->type);
		if (larger == eLONG) {
			int64_t x = primitive_number_long_value(this);
			int64_t z = primitive_number_long_value(pn);
			UNumberValue val;
			val.l = x - z;
			return new PrimitiveNumber(larger, val);
		} else if (larger == eDOUBLE) {
			double x = primitive_number_double_value(this);
			double z = primitive_number_double_value(pn);
			UNumberValue val;
			val.d = x - z;
			return new PrimitiveNumber(larger, val);
		}
	}
	throw std::logic_error("Unhandled class");
	return NULL;
}

Number* PrimitiveNumber::mul(Number* y) {
	PrimitiveNumber* pn = dynamic_cast<PrimitiveNumber*>(y);
	if (pn != NULL) {
		ENumberType larger = primitive_number_normalized_type(this->type, pn->type);
		if (larger == eLONG) {
			int64_t x = primitive_number_long_value(this);
			int64_t z = primitive_number_long_value(pn);
			UNumberValue val;
			val.f = x * z;
			return new PrimitiveNumber(larger, val);
		} else if (larger == eDOUBLE) {
			double x = primitive_number_double_value(this);
			double z = primitive_number_double_value(pn);
			UNumberValue val;
			val.d = x * z;
			return new PrimitiveNumber(larger, val);
		}
	}
	throw std::logic_error("Unhandled class");
	return NULL;
}

Number* PrimitiveNumber::div(Number* y) {
	PrimitiveNumber* pn = dynamic_cast<PrimitiveNumber*>(y);
	if (pn != NULL) {
		ENumberType larger = primitive_number_normalized_type(this->type, pn->type);
		if (larger == eLONG) {
			int64_t x = primitive_number_long_value(this);
			int64_t z = primitive_number_long_value(pn);
			UNumberValue val;
			val.l = x / z;
			return new PrimitiveNumber(larger, val);
		} else if (larger == eDOUBLE) {
			double x = primitive_number_double_value(this);
			double z = primitive_number_double_value(pn);
			UNumberValue val;
			val.d = x / z;
			return new PrimitiveNumber(larger, val);
		}
	}
	throw std::logic_error("Unhandled class");
	return NULL;
}

Number* PrimitiveNumber::pow(Number* y) {
	PrimitiveNumber* pn = dynamic_cast<PrimitiveNumber*>(y);
	if (pn != NULL) {
		ENumberType larger = primitive_number_normalized_type(this->type, pn->type);
		if (larger == eLONG) {
			int64_t x = primitive_number_long_value(this);
			int64_t z = primitive_number_long_value(pn);
			UNumberValue val;
			val.l = x + z;
			return new PrimitiveNumber(larger, val);
		} else if (larger == eDOUBLE) {
			double x = primitive_number_double_value(this);
			double z = primitive_number_double_value(pn);
			UNumberValue val;
			val.d = x + z;
			return new PrimitiveNumber(larger, val);
		}
	}
	throw std::logic_error("Unhandled class");
	return NULL;
}

static ENumberType primitive_number_normalized_type(ENumberType a, ENumberType b) {
	if (a == b) {
		return a;
	}
	ENumberType larger = std::max(a, b);
	if (larger < eLONG) {
		return eLONG;
	}
	return eDOUBLE;
}

static double primitive_number_double_value(PrimitiveNumber* x) {
	switch (x->type) {
		case eBYTE:
			return (double) x->val.b;
		case eUBYTE:
			return (double) x->val.ub;
		case eSHORT:
			return (double) x->val.s;
		case eUSHORT:
			return (double) x->val.us;
		case eINT:
			return (double) x->val.i;
		case eUINT:
			return (double) x->val.ui;
		case eLONG:
			return (double) x->val.l;
		case eULONG:
			return (double) x->val.ul;
		case eFLOAT:
			return (double) x->val.f;
		case eDOUBLE:
			return x->val.d;
	}
	throw std::logic_error("Unhandled type");
	return 0;
}

static int64_t primitive_number_long_value(PrimitiveNumber* x) {
	switch (x->type) {
		case eBYTE:
			return (int64_t) x->val.b;
		case eUBYTE:
			return (int64_t) x->val.ub;
		case eSHORT:
			return (int64_t) x->val.s;
		case eUSHORT:
			return (int64_t) x->val.us;
		case eINT:
			return (int64_t) x->val.i;
		case eUINT:
			return (int64_t) x->val.ui;
		case eLONG:
			return (int64_t) x->val.l;
		case eULONG:
			return x->val.ul;
		case eFLOAT:
			return (int64_t) x->val.f;
		case eDOUBLE:
			return (int64_t) x->val.d;
	}
	throw std::logic_error("Unhandled type");
	return 0;
}
