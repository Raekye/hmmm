#include "hm.h"
#include <boost/throw_exception.hpp>

#ifdef BOOST_NO_EXCEPTIONS
namespace boost {
	void throw_exception(const std::exception& e) {
		throw e;
	}
}

#endif // BOOST_NO_EXCEPTIONS

