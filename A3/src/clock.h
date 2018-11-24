#ifndef VS_CLOCK_H
#define VS_CLOCK_H

#include <chrono>

#include "util.h"

namespace vs
{
	template
	<
		typename C = std::chrono::system_clock,
		typename T = std::chrono::milliseconds
	>
	class Clock
	{
		public:
		typedef C clock_type;
		typedef T resolution_type;
		typedef typename T::period period_type;
		typedef std::chrono::duration<int64_t, std::ratio<period_type::num, 10 * period_type::den>> highres_type;

		public:
			static void adjust(int64_t a) { sAdjust += a; }
			static void reset( ) { sAdjust = 0; }
			static uint64_t now( );
			static void sleep(int64_t);
			static void sleep_until(uint64_t);
			static void sync(uint);

		private:
			static int64_t sAdjust;
	};

	template<typename C, typename T>
	int64_t Clock<C, T>::sAdjust = 0;

	template<typename C, typename T>
	uint64_t Clock<C, T>::now(void)
	{
		return std::chrono::duration_cast<resolution_type>(clock_type::now().time_since_epoch()).count() + sAdjust;
	}

	template<typename C, typename T>
	void Clock<C, T>::sleep(int64_t p)
	{
		if(p > 0)
		{
			std::this_thread::sleep_for(highres_type{10 * p - 5});
		}
	}

	template<typename C, typename T>
	void Clock<C, T>::sleep_until(uint64_t t)
	{
		uint64_t n = now();

		if(t > n)
		{
			sleep(t - n);
		}
	}

	template<typename C, typename T>
	void Clock<C, T>::sync(uint n)
	{
		sleep((n - now() % n) % n);
	}
}

#endif

