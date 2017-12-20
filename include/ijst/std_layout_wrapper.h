//
// Created by h46incon on 2017/11/29.
//

#ifndef _IJST_STD_LAYOUT_WRAPPER_HPP_INCLUDE_
#define _IJST_STD_LAYOUT_WRAPPER_HPP_INCLUDE_

#include <vector>
#include <map>
#include <deque>
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
	/**
	 * Common wrapper
	 * @tparam T: Value type, must implement default constructor, copy constructor, assignment
	 */
	template<typename T>
	class SLWrapper {
	public:
		typedef T TVal;

		SLWrapper()
		{ m_pVal = new T(); }

		SLWrapper(const SLWrapper& rhs)
		{ m_pVal = new T(*rhs.m_pVal); }

		SLWrapper(const T& _val)
		{ m_pVal = new T(_val); }

		SLWrapper& operator=(const SLWrapper& rhs)
		{
			if (this != &rhs) { (*m_pVal) = (*rhs.m_pVal); }
			return *this;
		}

		SLWrapper& operator=(const T& _val)
		{ (*m_pVal) = _val; return *this; }

#if __cplusplus >= 201103L

		SLWrapper(SLWrapper&& rhs) noexcept
		{ m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr; }

		SLWrapper& operator=(SLWrapper&& rhs) noexcept
		{
			if (this != &rhs) { delete m_pVal; m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr;}
			return *this;
		}

		SLWrapper(T&& _val) noexcept
		{ m_pVal = new T(std::move(_val)); }

		SLWrapper& operator=(T&& _val) noexcept
		{ (*m_pVal) = std::move(_val); return *this;}
#endif

#if __cplusplus >= 201103L
		~SLWrapper() noexcept
		{ delete m_pVal; m_pVal = nullptr; }
#else
		~SLWrapper()
		{ delete m_pVal; m_pVal = NULL; }
#endif

		T& Val() { return *m_pVal; }
		const T& Val() const { return *m_pVal; }

		T* operator->() { return m_pVal; }
		const T* operator->() const { return m_pVal; }
		T& operator*() { return *m_pVal; }
		const T& operator*() const { return *m_pVal; }

		bool operator== (const T& rhs) const { return *m_pVal == rhs; }
		bool operator!= (const T& rhs) const { return *m_pVal != rhs; }

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
		T* m_pVal;
	};


	/**
	 * Wrapper for std::vector. Use a special template here to make IDE happy
	 * Implement operator []
	 */
	template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
	class Vector{
	public:
		typedef std::vector<_Tp, _Alloc> T;
		typedef T TVal;

		Vector()
		{ m_pVal = new T(); }

		Vector(const Vector& rhs)
		{ m_pVal = new T(*rhs.m_pVal); }

		Vector(const T& _val)
		{ m_pVal = new T(_val); }

		Vector& operator=(const Vector& rhs)
		{
			if (this != &rhs) { (*m_pVal) = (*rhs.m_pVal); }
			return *this;
		}

		Vector& operator=(const T& _val)
		{ (*m_pVal) = _val; return *this; }

#if __cplusplus >= 201103L

		Vector(Vector&& rhs) noexcept
		{ m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr; }

		Vector& operator=(Vector&& rhs) noexcept
		{
			if (this != &rhs) { delete m_pVal; m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr;}
			return *this;
		}

		Vector(T&& _val) noexcept
		{ m_pVal = new T(std::move(_val)); }

		Vector& operator=(T&& _val) noexcept
		{ (*m_pVal) = std::move(_val); return *this;}
#endif

#if __cplusplus >= 201103L
		~Vector() noexcept
		{ delete m_pVal; m_pVal = nullptr; }
#else
		~Vector()
		{ delete m_pVal; m_pVal = NULL; }
#endif

		T& Val() { return *m_pVal; }
		const T& Val() const { return *m_pVal; }

		T* operator->() { return m_pVal; }
		const T* operator->() const { return m_pVal; }
		T& operator*() { return *m_pVal; }
		const T& operator*() const { return *m_pVal; }

		typename T::value_type& operator[](typename T::size_type i) { return (*m_pVal)[i]; }
		const typename T::value_type& operator[](typename T::size_type i) const { return (*m_pVal)[i]; }
	private:
		T* m_pVal;
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
		typedef T TVal;

		Map()
		{ m_pVal = new T(); }

		Map(const Map& rhs)
		{ m_pVal = new T(*rhs.m_pVal); }

		Map(const T& _val)
		{ m_pVal = new T(_val); }

		Map& operator=(const Map& rhs)
		{
			if (this != &rhs) { (*m_pVal) = (*rhs.m_pVal); }
			return *this;
		}

		Map& operator=(const T& _val)
		{ (*m_pVal) = _val; return *this; }

#if __cplusplus >= 201103L

		Map(Map&& rhs) noexcept
		{ m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr; }

		Map& operator=(Map&& rhs) noexcept
		{
			if (this != &rhs) { delete m_pVal; m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr;}
			return *this;
		}

		Map(T&& _val) noexcept
		{ m_pVal = new T(std::move(_val)); }

		Map& operator=(T&& _val) noexcept
		{ (*m_pVal) = std::move(_val); return *this;}
#endif

#if __cplusplus >= 201103L
		~Map() noexcept
		{ delete m_pVal; m_pVal = nullptr; }
#else
		~Map()
		{ delete m_pVal; m_pVal = NULL; }
#endif

		T& Val() { return *m_pVal; }
		const T& Val() const { return *m_pVal; }

		T* operator->() { return m_pVal; }
		const T* operator->() const { return m_pVal; }
		T& operator*() { return *m_pVal; }
		const T& operator*() const { return *m_pVal; }

		typename T::mapped_type& operator[](const typename T::key_type& _key) { return (*m_pVal)[_key]; }
#if __cplusplus >= 201103L
		typename T::mapped_type& operator[](typename T::key_type&& _key) { return (*m_pVal)[_key]; }
#endif
	private:
		T* m_pVal;
	};
}
#endif //_IJST_STD_LAYOUT_WRAPPER_HPP_INCLUDE_
