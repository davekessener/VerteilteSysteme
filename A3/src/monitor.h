#ifndef VS_SYNCBUF_H
#define VS_SYNCBUF_H

#include <mutex>

#include "util.h"

namespace vs
{
	template
	<
		typename T,
		typename Mutex = std::mutex,
		typename Lock = std::unique_lock<Mutex>
	>
	class Monitor
	{
		public:
		typedef Monitor<T, Mutex, Lock> Self;
		typedef T value_type;
		typedef Mutex mutex_type;
		typedef Lock lock_type;

		private:
			T mObj;
			mutex_type mMtx;

		public:
			Monitor( ) { }
			template<typename TT>
				Monitor(TT&& o) : mObj(std::forward<TT>(o)) { }
			template<typename TT>
				void set(TT&&);
			T get( );
			template<typename F>
				auto access(F&& f) -> decltype(f(mObj));

			Monitor(const Self&) = delete;
			Self& operator=(const Self&) = delete;
	};

	template<typename T, typename M, typename L>
	template<typename TT>
	void Monitor<T, M, L>::set(TT&& o)
	{
		lock_type guard(mMtx);

		mObj = std::forward<TT>(o);
	}

	template<typename T, typename M, typename L>
	T Monitor<T, M, L>::get(void)
	{
		lock_type guard(mMtx);

		return mObj;
	}

	template<typename T, typename M, typename L>
	template<typename F>
	auto Monitor<T, M, L>::access(F&& f) -> decltype(f(mObj))
	{
		lock_type guard(mMtx);

		return f(mObj);
	}
}

#endif

