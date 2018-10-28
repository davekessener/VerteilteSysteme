#ifndef VS_RHO_H
#define VS_RHO_H

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/math/common_factor.hpp>

namespace vs
{
	template<typename T>
	bool isPrime(const T& v)
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

	template<typename T>
	class RhoFactorizer
	{
		public:
			RhoFactorizer(T v, T x, T a) : mX(x), mY(x), mP(1), mA(a), mN(v) { }
			bool done( ) const { return mP != 1; }
			const T& get( ) const { return mP; }
			void step( );

		private:
			T advance(T v) const { return (v * v + mA) % mN; }

		private:
			T mX, mY, mP;
			const T mA, mN;
	};

	template<typename T>
	void RhoFactorizer<T>::step(void)
	{
		mX = advance(mX);
		mY = advance(advance(mY));
		mP = boost::math::gcd((mY - mX) % mN, mN);
	}
}

#endif

