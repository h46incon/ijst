//
// Created by h46incon on 2017/12/26.
//

#ifndef _IJST_DETAIL_HPP_INCLUDE_
#define _IJST_DETAIL_HPP_INCLUDE_

namespace ijst{
namespace detail{

#if __cplusplus >= 201103L
	#define IJSTI_MOVE(val) 	std::move((val))
	#define IJSTI_OVERRIDE		override
	#define IJSTI_NOEXCEPT		noexcept
#else
	#define IJSTI_MOVE(val) 	(val)
	#define IJSTI_OVERRIDE
	#define IJSTI_NOEXCEPT
#endif

/**
 * Singleton interface
 * @tparam _T type
 */
template<typename _T>
class Singleton {
public:
	inline static _T *GetInstance()
	{
		static _T instance;
		return &instance;
	}

	inline static void InitInstanceBeforeMain()
	{
		// When accessing initInstanceTag in code, the GetInstance() function will be called before main
		volatile void* dummy = initInstanceTag;
		(void)dummy;
	}

private:
	static void* initInstanceTag;
};
template<typename _T> void *Singleton<_T>::initInstanceTag = Singleton<_T>::GetInstance();

}	// namespace detail
}	// namespace ijst

#endif //_IJST_DETAIL_HPP_INCLUDE_
