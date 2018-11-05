#ifndef VS_RHO_H
#define VS_RHO_H

#include <boost/multiprecision/miller_rabin.hpp>
#include <boost/integer/common_factor.hpp>
//#include <boost/math/common_factor.hpp>
//#include <caf/detail/gcd.hpp>

#define MXT_PRIME_TRIALS 25
#define MXT_SMALL_NUMBER uint512_t{"10000000000"}

namespace vs
{
	template<typename T>
	bool isPrime(const T& v)
	{
		if(v < MXT_SMALL_NUMBER)
		{
			if(v <= 1) return false;
			if(v == 2 || v == 3) return true;
			if(!(v & 1) || (v % 3) == 0) return false;

			for(T i = 5 ; i * i <= v ; i += 6)
			{
				if((v % i) == 0 || (v % (i + 2)) == 0)
					return false;
			}
	
			return true;
		}
		else
		{
			return boost::multiprecision::miller_rabin_test(v, MXT_PRIME_TRIALS);
		}
	}

	template<typename T>
	class RhoFactorizer
	{
		public:
			RhoFactorizer( ) { }
			RhoFactorizer(T v, T x, T a) : mX(x), mY(x), mP(1), mA(a), mN(v) { }
			bool done( ) const { return mP != 1; }
			const T& get( ) const { return mP; }
			void step( );

		private:
			T advance(T v) const { return (v * v + mA) % mN; }

		private:
			T mX, mY, mP;
			T mA, mN;
	};

	template<typename T>
	void RhoFactorizer<T>::step(void)
	{
		mX = advance(mX);
		mY = advance(advance(mY));
		mP = boost::math::gcd((mY - mX) % mN, mN);
//		mP = caf::detail::gcd((mY - mX) % mN, mN);
	}
}

#endif

