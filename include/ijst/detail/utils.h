#ifndef IJST_DETAIL_UTILS_HPP_INCLUDE_
#define IJST_DETAIL_UTILS_HPP_INCLUDE_

#include "../ijst.h"
#include <vector>
#include <cstddef>	// NULL, size_t

namespace ijst {
namespace detail {

#if __cplusplus >= 201103L
	#define IJSTI_OVERRIDE						override
	#define IJSTI_NOEXCEPT						noexcept
	#define IJSTI_DELETED						= delete
	#define IJSTI_STATIC_ASSERT(cond, msg) 		static_assert((cond), msg)
#else
	#define IJSTI_OVERRIDE
	#define IJSTI_NOEXCEPT
	#define IJSTI_DELETED
	#define IJSTI_STATIC_ASSERT(cond, msg)
#endif

#if IJST_HAS_CXX11_RVALUE_REFS
	#define IJSTI_MOVE(val) 					std::move((val))
#else
	#define IJSTI_MOVE(val) 					(val)
#endif

// Expands to the concatenation of its two arguments.
#define IJSTI_PP_CONCAT(x, y) 		IJSTI_PP_CONCAT_I(x, y)
#define IJSTI_PP_CONCAT_I(x, y) 	x ## y

// Expand __VA_ARGS__ for msvc preprocessor
#define IJSTI_EXPAND(...)			__VA_ARGS__


#if IJST_TRY_INIT_META_BEFORE_MAIN
	#define IJSTI_TRY_INIT_META_BEFORE_MAIN(T)			::ijst::detail::Singleton< T >::InitInstanceInGlobal();
#else
	#define IJSTI_TRY_INIT_META_BEFORE_MAIN(T)
#endif

/**
 * Singleton interface
 * @tparam T type
 */
template<typename T>
class Singleton {
public:
	inline static T& GetInstance()
	{
		static T instance;
		return instance;
	}

	inline static void InitInstanceInGlobal()
	{
		// When accessing gInstance in code, the GetInstance() function will be called in
		// global scope (before main in many compilers)
		volatile T* dummy = gInstance;
		(void)dummy;
	}

private:
	static T* gInstance;
};
// static member of template class could declare in header
template<typename T> T* Singleton<T>::gInstance = &Singleton<T>::GetInstance();

template <typename T>
struct HasType {
	typedef void Void;
};

/**
 * ArugmentType. Helper template to declare macro argument with comma
 *
 * @see https://stackoverflow.com/questions/13842468/comma-in-c-c-macro/13842784
 */
template<typename T>
struct ArgumentType;
template<typename T, typename U>
struct ArgumentType<U(T)> {typedef T type;};

struct Util {
	/**
	 * Custom swap() to avoid dependency on C++ <algorithm> header
	 * @tparam T 	Type of the arguments to swap, should be instantiated with primitive C++ types only.
	 * @note This has the same semantics as std::swap().
	 */
	template <typename T>
	static inline void Swap(T& a, T& b) IJSTI_NOEXCEPT {
		T tmp = IJSTI_MOVE(a);
		a = IJSTI_MOVE(b);
		b = IJSTI_MOVE(tmp);
	}

	struct VectorBinarySearchResult {
		bool isFind;
		size_t index;

		explicit VectorBinarySearchResult(bool _isFind = false, size_t _index = 0)
				: isFind(_isFind), index(_index) {}
	};

	template<typename VType>
	static VectorBinarySearchResult VectorBinarySearch(const std::vector<VType>& vec, const VType& target)
	{
		size_t beg = 0;
		size_t end = vec.size();

		while (beg < end) {
			const size_t mid = beg + (end - beg) / 2;
			const VType& vMid = vec[mid];
			if (target < vMid) {
				// target is in left half
				end = mid;
			}
			else if (vMid < target) {
				// target is in right half
				beg = mid + 1;
			}
			else {
				return VectorBinarySearchResult(true, mid);
			}
		}
		return VectorBinarySearchResult(false, end);
	};

	static bool IsBitSet(unsigned int val, unsigned int bit)
	{
		return (val & bit) != 0;
	}

};

} // namespace detail
} // namespace ijst
#endif //IJST_DETAIL_UTILS_HPP_INCLUDE_
