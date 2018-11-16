#ifndef VS_POSIXERROR_H
#define VS_POSIXERROR_H

#include <string>

#include "util.h"
#include "error.h"

namespace vs
{
	class PosixError : public std::exception
	{
		public:
			PosixError(const std::string&);
			const char *what( ) const noexcept override { return mMessage.data(); }

		private:
			const std::string mMessage;
	};
}

#endif

