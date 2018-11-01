#include <iostream>

#include "worker.h"

#define MXT_O(n) ((118*sqrt(sqrt(n)))/100)

namespace vs { namespace worker {

actor::behavior_type behavior(actor::stateful_pointer<State> self, uint16_t port)
{
	return {
		[=](action::process, const std::string& n, uint a, uint l) -> caf::result<result> {
			uint512_t v(n);

			if(v <= 1)
			{
				return result{};
			}
			else
			{
				auto promise = self->make_response_promise<result>();

				self->state.abort();
				self->state.set(v, a, l, promise);

				if(self->mailbox().empty())
				{
					self->send(self, action::resume::value);
				}

				return promise;
			}
		},
		[=](action::resume) {
			while(!self->state.done())
			{
				self->state.step();

				if(!self->mailbox().empty())
				{
					self->send(self, action::resume::value);

					break;
				}
			}
		},
		[=](action::abort) {
			self->state.abort();
		}
	};
}

// # --------------------------------------------------------------------------

void State::set(uint512_t n, uint a, uint l, caf::response_promise p)
{
	using boost::multiprecision::sqrt;

	mRunning = true;
	mPromise = p;
	mTime = mCycles = mTries = 0;
	mNumber = n;
	mA = a;
	mCount = MXT_O(mNumber) / l;

	mRNG.param(d_t::param_type{0, mNumber - 1});

	reset();
}

result State::get(void) const
{
	result r;

	r.factor = stringify(mFact.get());
	r.source = stringify(mNumber);
	r.cpu_time = mTime;
	r.cycles = mCycles;
	r.tries = mTries;

	if(r.factor == r.source)
	{
		r.factor = "";
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
		mPromise.deliver(get());
	}
}

void State::abort(void)
{
	if(mRunning)
	{
		mRunning = false;
		mPromise.deliver(result{});
	}
}

void State::reset(void)
{
	mFact = { mNumber, mRNG(mMT), mA };
	mLeft = mCount;
	++mTries;

//	std::cout << stringify("On try ", mTries, "; ", mLeft, " steps left\n") << std::flush;
}

}}

