#include "thread.h"

namespace vs {

Thread::~Thread(void)
{
	join();
}

Thread& Thread::operator=(Thread&& t)
{
	join();

	mThread = std::move(t.mThread);

	return *this;
}

void Thread::join(void)
{
	if(joinable())
	{
		mThread.join();
	}
}

void Thread::detach(void)
{
	mThread.detach();
}

}

