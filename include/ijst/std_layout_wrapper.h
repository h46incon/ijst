//
// Created by h46incon on 2017/11/29.
//

#ifndef _IJST_STD_LAYOUT_WRAPPER_HPP_INCLUDE_
#define _IJST_STD_LAYOUT_WRAPPER_HPP_INCLUDE_

#include <vector>
#include <map>
#include <deque>
#include <list>
#include <string>

namespace ijst {
	namespace detail {
		//* SFINAE
		template <typename T>
		struct IsStdVectorOrDeque {
		};

		template<typename TElem>
		struct IsStdVectorOrDeque<std::vector<TElem> > {
			static const bool value = true;
		};

		template<typename TElem>
		struct IsStdVectorOrDeque<std::deque<TElem> > {
			static const bool value = true;
		};

		template <typename T>
		struct IsStdMap {
		};

		template<typename TKey, typename TVal>
		struct IsStdMap<std::map<TKey, TVal> > {
			static const bool value = true;
		};
	}

	#define IJSTI_SLWRAPPER_DEFINE(Class, T)													\
		public:																					\
			typedef T TVal;																		\
			Class()																				\
			{ m_pVal = new T(); }																\
			Class(const Class& rhs)																\
			{ m_pVal = new T(*rhs.m_pVal); }													\
			Class(const T& _val)																\
			{ m_pVal = new T(_val); }															\
																								\
			Class& operator=(const Class& rhs)													\
			{ if (this != &rhs) { (*m_pVal) = (*rhs.m_pVal); } return *this; }					\
			Class& operator=(const T& _val)														\
			{ (*m_pVal) = _val; return *this; }													\
																								\
			T& Val() { return *m_pVal; }														\
			const T& Val() const { return *m_pVal; }											\
																								\
			T* operator->() { return m_pVal; }													\
			const T* operator->() const { return m_pVal; }										\
																								\
			T& operator*() { return *m_pVal; }													\
			const T& operator*() const { return *m_pVal; }										\
																								\
			bool operator== (const T& rhs) const { return *m_pVal == rhs; }						\
			bool operator!= (const T& rhs) const { return *m_pVal != rhs; }						\
		private:																				\
			T* m_pVal;																			\

#if __cplusplus >= 201103L
	#define IJSTI_SLWRAPPER_DEFINE_BY_CPP_VERSION(Class, T)										\
		public:																					\
			Class(Class&& rhs) noexcept															\
			{ m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr; }										\
																								\
			Class& operator=(Class&& rhs) noexcept												\
			{ if (this != &rhs) { delete m_pVal; m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr;}	\
			  return *this; }																	\
																								\
			Class(T&& _val) noexcept															\
			{ m_pVal = new T(std::move(_val)); }												\
																								\
			Class& operator=(T&& _val) noexcept													\
			{ (*m_pVal) = std::move(_val); return *this;}										\
																								\
			~Class() noexcept																	\
			{ delete m_pVal; m_pVal = nullptr; }
#else
	#define IJSTI_SLWRAPPER_DEFINE_BY_CPP_VERSION(Class, T)										\
		public:																					\
			~Class()																			\
			{ delete m_pVal; m_pVal = NULL; }
#endif

	/**
	 * Common wrapper
	 * @tparam T: Value type, must implement default constructor, copy constructor, assignment
	 */
	template<typename T>
	class SLWrapper {
		IJSTI_SLWRAPPER_DEFINE(SLWrapper, T)
		IJSTI_SLWRAPPER_DEFINE_BY_CPP_VERSION(SLWrapper, T)

	public:
#if __cplusplus >= 201103L
		template <typename U=T, bool = detail::IsStdVectorOrDeque<U>::value>
		typename U::value_type& operator[](typename U::size_type i) { return (*m_pVal)[i]; }

		template <typename U=T, bool = detail::IsStdVectorOrDeque<U>::value>
		const typename U::value_type& operator[](typename U::size_type i) const { return (*m_pVal)[i]; }

		template <typename U=T, bool = detail::IsStdMap<U>::value>
		typename U::mapped_type& operator[](const typename U::key_type& _key) { return (*m_pVal)[_key]; }

		template <typename U=T, bool = detail::IsStdMap<U>::value>
		typename U::mapped_type& operator[](typename U::key_type&& _key) { return (*m_pVal)[_key]; }
#endif
	private:
	};


	/**
	 * Wrapper for std::vector. Use a special template here to make IDE happy
	 * Implement operator []
	 */
	template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
	class Vector{
	public:
		typedef std::vector<_Tp, _Alloc> T;
		IJSTI_SLWRAPPER_DEFINE(Vector, T);
		IJSTI_SLWRAPPER_DEFINE_BY_CPP_VERSION(Vector, T)

		typename T::value_type& operator[](typename T::size_type i) { return (*m_pVal)[i]; }
		const typename T::value_type& operator[](typename T::size_type i) const { return (*m_pVal)[i]; }
	};


	/**
	 * Wrapper for std::map. Use a special template here to make IDE happy
	 * Implement operator []
	 */
	template<typename _Key, typename _Tp, typename _Compare = std::less<_Key>,
			typename _Alloc = std::allocator<std::pair<const _Key, _Tp> > >
	class Map {
	public:
		typedef std::map<_Key, _Tp, _Compare, _Alloc> T;
		IJSTI_SLWRAPPER_DEFINE(Map, T)
		IJSTI_SLWRAPPER_DEFINE_BY_CPP_VERSION(Map, T)

		typename T::mapped_type& operator[](const typename T::key_type& _key) { return (*m_pVal)[_key]; }
#if __cplusplus >= 201103L
		typename T::mapped_type& operator[](typename T::key_type&& _key) { return (*m_pVal)[_key]; }
#endif
	};

	/**
	 * Wrapper for std::deque. Use a special template here to make IDE happy
	 * Implement operator []
	 */
	template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
	class Deque{
	public:
		typedef std::deque<_Tp, _Alloc> T;
		IJSTI_SLWRAPPER_DEFINE(Deque, T);
		IJSTI_SLWRAPPER_DEFINE_BY_CPP_VERSION(Deque, T)

		typename T::value_type& operator[](typename T::size_type i) { return (*m_pVal)[i]; }
		const typename T::value_type& operator[](typename T::size_type i) const { return (*m_pVal)[i]; }
	};
}
#endif //_IJST_STD_LAYOUT_WRAPPER_HPP_INCLUDE_
