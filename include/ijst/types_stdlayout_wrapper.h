//
// Created by h46incon on 2017/11/29.
//

#ifndef _IJST_TYPES_STD_LAYOUT_WRAPPER_HPP_INCLUDE_
#define _IJST_TYPES_STD_LAYOUT_WRAPPER_HPP_INCLUDE_

#include "ijst.h"
#include "types_container.h"

namespace ijst {
/**
 * Common wrapper
 * @tparam _T: Value type, must implement default constructor, copy constructor, assignment
 */
template<typename _T>
class T_Wrapper {
public:
	typedef _T TVal;
	T_Wrapper()
	{ m_pVal = new _T(); }
	T_Wrapper(const T_Wrapper& rhs)
	{ m_pVal = new _T(*rhs.m_pVal); }
	T_Wrapper(const _T& _val)
	{ m_pVal = new _T(_val); }

	T_Wrapper& operator=(const T_Wrapper& rhs)
	{ if (this != &rhs) { (*m_pVal) = (*rhs.m_pVal); } return *this; }
	T_Wrapper& operator=(const _T& _val)
	{ (*m_pVal) = _val; return *this; }

	_T& Val() { return *m_pVal; }
	const _T& Val() const { return *m_pVal; }

	_T* operator->() { return m_pVal; }
	const _T* operator->() const { return m_pVal; }

	_T& operator*() { return *m_pVal; }
	const _T& operator*() const { return *m_pVal; }

	bool operator== (const _T& rhs) const { return *m_pVal == rhs; }
	bool operator!= (const _T& rhs) const { return *m_pVal != rhs; }
#if __cplusplus >= 201103L
	T_Wrapper(T_Wrapper&& rhs) noexcept
	{ m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr; }

	T_Wrapper& operator=(T_Wrapper&& rhs) noexcept
	{ if (this != &rhs) { delete m_pVal; m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr;}
		return *this; }

	T_Wrapper(_T&& _val) noexcept
	{ m_pVal = new _T(std::move(_val)); }

	T_Wrapper& operator=(_T&& _val) noexcept
	{ (*m_pVal) = std::move(_val); return *this;}

	~T_Wrapper() noexcept
	{ delete m_pVal; m_pVal = nullptr; }
#else
	~T_Wrapper()
		{ delete m_pVal; m_pVal = NULL; }
#endif

private:
	_T* m_pVal;
};
}
#endif //_IJST_TYPES_STD_LAYOUT_WRAPPER_HPP_INCLUDE_
