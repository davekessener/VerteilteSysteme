#ifndef VS_WORKER_H
#define VS_WORKER_H

#include "util.h"
#include "rho.h"

namespace vs
{
	namespace worker
	{
		namespace action
		{
			using caf::atom;
			using caf::atom_constant;

			using process = atom_constant<atom("w_process")>;
			using resume = atom_constant<atom("w_resume")>;
			using abort = atom_constant<atom("w_abort")>;
			using kill = atom_constant<atom("w_kill")>;
		}

		struct result
		{
			std::string factor, source;
			double cpu_time = 0;
			uint cycles = 0;
			uint tries = 0;
		};

		using actor = caf::typed_actor<
			caf::replies_to<action::process, std::string, uint, uint>::with<result>,
			caf::reacts_to<action::resume>,
			caf::reacts_to<action::abort>,
			caf::reacts_to<action::kill>
		>;

		class State
		{
			typedef boost::random::uniform_int_distribution<uint512_t> d_t;

			public:
				void set(uint512_t, uint, uint, caf::response_promise);
				void step( );
				void abort( );
				result get( ) const;
				bool isRepeat(uint512_t, uint) const;

				bool done( ) const { return !mRunning || mFact.done(); }
				void update(uint64_t t) { mTime += t; }

			private:
				void reset( );

			private:
				boost::random::mt19937 mMT;
				d_t mRNG;
				bool mRunning;
				uint512_t mNumber;
				uint mA;
				uint512_t mCount, mLeft;
				uint mCycles, mTries;
				uint64_t mTime;
				RhoFactorizer<uint512_t> mFact;
				caf::response_promise mPromise;
		};

		actor::behavior_type behavior(actor::stateful_pointer<State>, uint);

		template<typename Inspector>
		typename Inspector::result_type inspect(Inspector& f, result& r)
		{
			return f(caf::meta::type_name("w_result"), r.factor, r.source, r.cpu_time, r.cycles, r.tries);
		}
	}
}

#endif

