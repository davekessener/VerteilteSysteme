#ifndef VS_SCHEDULE_H
#define VS_SCHEDULE_H

#include <cstring>

#include "util.h"
#include "error.h"

namespace vs
{
	template<size_t N>
	class Schedule
	{
		static constexpr size_t BPB = 8;
		static constexpr size_t SIZE = (N + BPB - 1) / BPB; 

		public:
		DEFINE_EXCEPTION(IndexOutOfBoundsError);

		public:
			Schedule( );
			void set(uint);
			bool check(uint) const;
			void clear( );

		private:
			byte_t mSched[SIZE];
	};

	template<size_t N>
	Schedule<N>::Schedule(void)
	{
		clear();
	}

	template<size_t N>
	void Schedule<N>::set(uint idx)
	{
		if(idx >= N)
			THROW<IndexOutOfBoundsError>("Index ", idx, " is not within [0, ", N, ") !");

		mSched[idx / BPB] |= (1 << (idx % BPB));
	}

	template<size_t N>
	bool Schedule<N>::check(uint idx) const
	{
		if(idx >= N)
			THROW<IndexOutOfBoundsError>("Index ", idx, " is not within [0, ", N, ") !");

		return mSched[idx / BPB] & (1 << (idx % BPB));
	}

	template<size_t N>
	void Schedule<N>::clear(void)
	{
		std::memset(mSched, 0, SIZE);
	}
}

#endif

