#include <iostream>
#include <chrono>

#include "worker.h"

#define MXT_O(n) ((118*sqrt(sqrt(n)))/100)

#define MXT_MAILBOX_F 1000

namespace vs { namespace worker {

namespace
{
	using clock_type = std::chrono::high_resolution_clock;
}

actor::behavior_type behavior(actor::stateful_pointer<State> self, uint id)
{
	return {
		[=](action::process, const std::string& n, uint a, uint l) -> caf::result<result> {
			uint512_t v(n);

			aout(self) << "[" << id << "] Received request for " << n << std::endl;

			if(v <= 1 || (v % 2) != 1)
			{
				return result{};
			}
			else if(self->state.isRepeat(uint512_t{n}, a))
			{
				return self->state.get();
			}
			else
			{
				auto promise = self->make_response_promise<result>();

				self->state.set(v, a, l, promise);

				if(self->mailbox().empty())
				{
					self->send(self, action::resume::value);
				}

				return promise;
			}
		},
		[=](action::resume) {
			bool should_cont = false;

			auto start = clock_type::now();
			while(!self->state.done())
			{
				self->state.step();

				if((should_cont = !self->mailbox().empty())) break;
			}

			self->state.update(std::chrono::duration_cast<std::chrono::microseconds>(clock_type::now() - start).count());

			if(should_cont)
			{
				self->send(self, action::resume::value);
			}
		},
		[=](action::abort) {
			self->state.abort();
		},
		[=](action::kill) {
			self->quit();
		}
	};
}

// # --------------------------------------------------------------------------

void State::set(uint512_t n, uint a, uint l, caf::response_promise p)
{
	using boost::multiprecision::sqrt;

	if(mRunning && n == mNumber && a == mA)
	{
		mPromise.deliver(result{});
		mPromise = p;
	}
	else
	{
		abort();

		mRunning = true;
		mPromise = p;
		mTime = mCycles = mTries = 0;
		mNumber = n;
		mA = a;
		mCount = MXT_O(mNumber) / l;

		mRNG.param(d_t::param_type{0, mNumber - 1});

		reset();
	}
}

result State::get(void) const
{
	result r;

	r.cpu_time = mTime / 1000000.0;
	r.cycles = mCycles;
	r.tries = mTries;

	if(mFact.done())
	{
		r.factor = stringify(mFact.get());
		r.source = stringify(mNumber);

		if(r.factor == r.source)
		{
			r.factor = "";
		}
	}

	return r;
}

void State::step(void)
{
	++mCycles;
	
	if(mCount)
	{
		if(!--mLeft) reset();
	}

	mFact.step();

	if(mFact.done())
	{
		mRunning = false;
		std::cout << stringify("Finished calculating: ", mFact.get(), "\n") << std::endl;
		mPromise.deliver(get());
	}
}

void State::abort(void)
{
	if(mRunning)
	{
		mRunning = false;
		mPromise.deliver(get());
	}
}

bool State::isRepeat(uint512_t n, uint a) const
{
	return n == mNumber && a == mA && mFact.done() && mFact.get() != n;
}

void State::reset(void)
{
	mFact = { mNumber, mRNG(mMT), mA };
	mLeft = mCount;
	++mTries;

//	std::cout << stringify("On try ", mTries, "; ", mLeft, " steps left\n") << std::flush;
}

}}

