//
// Created by h46incon on 2017/11/29.
//

#ifndef _IJST_TYPES_STD_LAYOUT_WRAPPER_HPP_INCLUDE_
#define _IJST_TYPES_STD_LAYOUT_WRAPPER_HPP_INCLUDE_

#include "ijst.h"
#include "types_container.h"

namespace ijst {
/**
 * Common wrapper to hold object via a standard-layout object, that required by offset() macro.
 *
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

#if __cplusplus >= 201103L
	explicit operator _T&() { return *m_pVal; }
	explicit operator const _T&() const { return *m_pVal; }
#else
	operator _T&() { return *m_pVal; }
	operator const _T&() const { return *m_pVal; }
#endif

	bool operator== (const T_Wrapper& rhs) const { return *m_pVal == *rhs.m_pVal; }
	bool operator!= (const T_Wrapper& rhs) const { return *m_pVal != *rhs.m_pVal; }
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

namespace detail {
	template<typename _T>
	class FSerializer<T_Wrapper<_T> > : public SerializerInterface {
	public:
		typedef T_Wrapper<_T> VarType;

		virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
		{
			const VarType& field = *static_cast<const VarType*>(req.pField);
			SerializeReq elemReq(req.writer, &field.Val(), req.fPushMode);
			return IJSTI_FSERIALIZER_INS(_T)->Serialize(elemReq);
		}

#if IJST_ENABLE_TO_JSON_OBJECT
		virtual int ToJson(const ToJsonReq &req) IJSTI_OVERRIDE
		{
			const VarType& field = *static_cast<const VarType*>(req.pField);
			ToJsonReq elemReq(req.buffer, req.allocator, &field.Val(), req.canMoveSrc, req.fPushMode);
			return IJSTI_FSERIALIZER_INS(_T)->ToJson(elemReq);
		}

		virtual int SetAllocator(void* pField, JsonAllocator& allocator) IJSTI_OVERRIDE
		{
			VarType& field = *static_cast<VarType*>(pField);
			return IJSTI_FSERIALIZER_INS(_T)->SetAllocator(&field.Val(), allocator);
		}
#endif

		virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
		{
			VarType& field = *static_cast<VarType*>(req.pFieldBuffer);
			FromJsonReq elemReq(req.stream, req.allocator, req.unknownMode, req.canMoveSrc, req.checkField, &field.Val());
			return IJSTI_FSERIALIZER_INS(_T)->FromJson(elemReq, resp);
		}
	};
}
}
#endif //_IJST_TYPES_STD_LAYOUT_WRAPPER_HPP_INCLUDE_
