#ifndef VS_THREAD_H
#define VS_THREAD_H

#include <thread>

namespace vs
{
	class Thread
	{
		public:
			Thread( ) { }
			template<typename F>
				Thread(F&& f) : mThread(std::forward<F>(f)) { }
			~Thread( );
			Thread(Thread&& t) : mThread(std::move(t.mThread)) { }
			Thread& operator=(Thread&&);
			void join( );
			bool joinable( ) const { return mThread.joinable(); }

			Thread(const Thread&) = delete;
			Thread& operator=(const Thread&) = delete;

		private:
			std::thread mThread;
	};
}

#endif

