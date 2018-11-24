#ifndef VS_THREAD_H
#define VS_THREAD_H

#include <thread>
#include <exception>
#include <iostream>

#include "util.h"

namespace vs
{
	class Thread
	{
		public:
			Thread( ) { }
			template<typename F>
				Thread(F&&);
			~Thread( );
			Thread(Thread&& t) : mThread(std::move(t.mThread)) { }
			Thread& operator=(Thread&&);
			void join( );
			void detach( );
			bool joinable( ) const { return mThread.joinable(); }

			Thread(const Thread&) = delete;
			Thread& operator=(const Thread&) = delete;

		private:
			std::thread mThread;
	};

	template<typename F>
	Thread::Thread(F&& f)
	{
		mThread = std::thread{[f](void) {
			try
			{
				f();
			}
			catch(const std::exception& e)
			{
				std::cerr << stringify("Stray exception in thread: ", e.what(), "\n") << std::flush;

				throw;
			}
		}};
	}
}

#endif

