#ifndef VS_UTIL_HH
#define VS_UTIL_HH

#include <sstream>
#include <stdint.h>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random.hpp>

#include <caf/all.hpp>
#include <caf/io/all.hpp>

typedef unsigned uint;

using boost::multiprecision::uint512_t;

namespace vs
{
	namespace impl
	{
		template<typename S>
		void stringify(S&)
		{
		}
	
		template<typename S, typename T, typename ... TT>
		void stringify(S& ss, const T& o, const TT& ... a)
		{
			ss << o;
	
			stringify(ss, a...);
		}
	}
	
	template<typename ... T>
	std::string stringify(const T& ... a)
	{
		std::stringstream ss;
	
		impl::stringify(ss, a...);
	
		return ss.str();
	}

	template<typename T>
	uint divideAll(T& v, T f)
	{
		uint c = 0;

		while(true)
		{
			auto p = v / f;

			if(p * f != v) break;

			v = p;
			++c;
		}

		return c;
	}

	template<typename V>
	constexpr V min(const V& a, const V& b) noexcept
	{
		return a < b ? a : b;
	}

	template<typename V>
	constexpr V max(const V& a, const V& b) noexcept
	{
		return a > b ? a : b;
	}
}

#endif

