#ifndef VS_COLLIDEABLE_H
#define VS_COLLIDEABLE_H

#include "util.h"
#include "error.h"

namespace vs
{
	template<typename T>
	class Collideable
	{
		public:
		typedef Collideable<T> Self;
		typedef T value_type;

		DEFINE_EXCEPTION(NotPresentError);
		DEFINE_EXCEPTION(CollisionError);

		public:
			Collideable( ) : mCollision(false), mPresent(false) { }
			Collideable(Self&);
			template<typename TT>
				Self& operator=(TT&&);
			explicit operator value_type( );
			void clear( );
			bool isPresent( ) const { return mPresent; }
			bool hasCollided( ) const { return mCollision; }

			Collideable(const Self&) = delete;
			Collideable& operator=(const Self&) = delete;

		private:
			bool mCollision, mPresent;
			value_type mObj;
	};

	template<typename T>
	Collideable<T>::Collideable(Self& c)
		: mCollision(c.mCollision)
		, mPresent(true)
		, mObj(std::move(c.mObj))
	{
		c.mCollision = false;
		c.mPresent = false;
	}

	template<typename T>
	template<typename TT>
	typename Collideable<T>::Self& Collideable<T>::operator=(TT&& o)
	{
		if(mPresent || mCollision)
		{
			mCollision = true;
		}
		else
		{
			mPresent = true;
			mObj = std::forward<TT>(o);
		}

		return *this;
	}

	template<typename T>
	Collideable<T>::operator value_type(void)
	{
		auto hook = scoped([this](void) { clear(); });

		if(!mPresent)
			THROW<NotPresentError>();

		if(mCollision)
			THROW<CollisionError>();

		return std::move(mObj);
	}

	template<typename T>
	void Collideable<T>::clear(void)
	{
		mPresent = false;
		mCollision = false;
	}
}

#endif

