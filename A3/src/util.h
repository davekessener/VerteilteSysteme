#ifndef VS_UTIL_H
#define VS_UTIL_H

#include <chrono>
#include <thread>
#include <functional>
#include <iomanip>
#include <sstream>
#include <cstdint>

using std::uint8_t;
using std::int8_t;
using std::uint16_t;
using std::int16_t;
using std::uint32_t;
using std::int32_t;
using std::uint64_t;
using std::int64_t;

typedef unsigned uint;
typedef uint8_t byte_t;

#define SLOTS_PER_FRAME 25
#define FRAME_DURATION 1000
#define SLOT_DURATION (FRAME_DURATION/SLOTS_PER_FRAME)

#define PACKET_SIZE 34
#define NAME_LENGTH 10
#define PAYLOAD_SIZE 24

namespace vs
{
	template<typename T1, typename T2>
	T1 lexical_cast(const T2& o)
	{
		std::stringstream ss;
		T1 r;

		ss << o;
		ss >> r;

		return r;
	}

	template<typename S>
	void stringify_impl(S&)
	{
	}

	template<typename S, typename T, typename ... TT>
	void stringify_impl(S& ss, const T& o, const TT& ... a)
	{
		ss << o;

		stringify_impl(ss, a...);
	}

	template<typename ... T>
	std::string stringify(const T& ... a)
	{
		std::stringstream ss;

		stringify_impl(ss, a...);

		return ss.str();
	}

	template<size_t N, typename T>
	std::string hex(const T& v)
	{
		return stringify(std::hex, std::setfill('0'), std::setw(N / 4), static_cast<uint64_t>(v));
	}

	class ScopeHook
	{
		public:
		typedef std::function<void(void)> callback_fn;

		public:
			ScopeHook( ) { }
			ScopeHook(callback_fn f) : mCallback(std::move(f)) { }
			~ScopeHook( ) { if(static_cast<bool>(mCallback)) mCallback(); }
			ScopeHook& operator=(callback_fn f) { mCallback = std::move(f); return *this; }

		private:
			callback_fn mCallback;
	};

	template<typename F>
	ScopeHook scoped(F&& f)
	{
		return ScopeHook{std::forward<F>(f)};
	}

	template<typename T>
	class Averager
	{
		public:
			Averager( ) : mSum(0), mCount(0) { }
			T get( ) { T v = 0; if(mCount) { v = mSum / mCount; mSum = mCount = 0; } return v; }
			void add(T v) { mSum += v; ++mCount; }
		private:
			T mSum;
			uint mCount;
	};

	template<typename T>
	constexpr T difference(const T& a, const T& b)
	{
		return a > b ? (a - b) : (b - a);
	}
}

#endif

