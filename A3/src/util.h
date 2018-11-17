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

#define SLOT_IDX_OFFSET 1

#define SLOTS_PER_FRAME 25
#define FRAME_DURATION 1000
#define SLOT_DURATION (FRAME_DURATION/SLOTS_PER_FRAME)

namespace vs
{
	typedef std::chrono::high_resolution_clock clock_t;
	
	inline uint64_t now( ) { return std::chrono::duration_cast<std::chrono::milliseconds>(clock_t::now().time_since_epoch()).count(); }
	inline void sleep(int p) { if(p > 0) std::this_thread::sleep_for(std::chrono::microseconds(p * 1000 - 500)); }
	inline void sleep_until(uint64_t t) { uint64_t n = now(); if(t > n) sleep(t - n); }
	inline void sync(uint n) { sleep((n - now() % n) % n); }

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
}

#endif

