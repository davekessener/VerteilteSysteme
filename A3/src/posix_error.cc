#include <errno.h>
#include <string.h>

#include "posix_error.h"

namespace vs {

PosixError::PosixError(const std::string& msg)
	: mMessage(stringify(msg, ((!msg.empty() || errno) ? ": " : ""), ((errno) ? strerror(errno) : "")))
{
}

}

