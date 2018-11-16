#ifndef VS_ERROR_H
#define VS_ERROR_H

#include <exception>

#define THROW ::vs::ErrorHelper{__FILE__,__LINE__}.throw_exception

#define DEFINE_EXCEPTION(name) \
	class name : public std::exception { \
		public: \
			name (const std::string& msg) : mMessage(stringify(msg, " (" #name ")")) { } \
			const char *what( ) const noexcept override { return mMessage.data(); } \
		private: \
			const std::string mMessage; \
	}

namespace vs
{
	class ErrorHelper
	{
		public:
			ErrorHelper(const std::string& file, int line) : mFile(file), mLine(line) { }

			template<typename E, typename ... T>
			void throw_exception(const T& ... a)
			{
				throw E{stringify("[", mFile, ":", mLine, "] ", a...)};
			}

		private:
			const std::string mFile;
			const int mLine;
	};
}

#endif

