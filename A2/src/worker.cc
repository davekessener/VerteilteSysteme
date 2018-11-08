#include <iostream>
#include <chrono>

#include "worker.h"

#define MXT_O(n) ((118*sqrt(sqrt(n)))/100)

#define MXT_SKIP 1000

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
			else
			{
				self->state.set(v, a, l);

				auto start = clock_type::now();
				for(uint c = MXT_SKIP ; !self->state.done() ; --c)
				{
					if(c)
					{
						self->state.step();
					}
					else
					{
						c = MXT_SKIP;

						if(!self->mailbox().empty()) break;
					}
				}
				self->state.update(std::chrono::duration_cast<std::chrono::microseconds>(clock_type::now() - start).count());

				return self->state.get();
			}
		},
		[=](action::abort) { }
	};
}

// # --------------------------------------------------------------------------

void State::set(uint512_t n, uint a, uint l)
{
	using boost::multiprecision::sqrt;

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
}

void State::reset(void)
{
	mFact = { mNumber, mRNG(mMT), mA };
	mLeft = mCount;
	++mTries;
}

}}

