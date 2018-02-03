//
// Created by h46incon on 2017/11/29.
//

#ifndef IJST_TYPES_STD_LAYOUT_WRAPPER_HPP_INCLUDE_
#define IJST_TYPES_STD_LAYOUT_WRAPPER_HPP_INCLUDE_

#include "ijst.h"
#include "types_container.h"

namespace ijst {
/**
 * @brief Common wrapper to hold object via a standard-layout object.
 *
 * offset() macro requires the object is standard-layout, * i.e, all members in the object
 * is standard-layout. This wrapper could wrap any type to a standard-layout type.
 *
 * @tparam T: Value type, must implement default constructor, copy constructor, assignment
 */
template<typename T>
class T_Wrapper {
public:
	typedef T TVal;
	T_Wrapper()
	{ m_pVal = new T(); }
	T_Wrapper(const T_Wrapper& rhs)
	{ m_pVal = new T(*rhs.m_pVal); }
	T_Wrapper(const T& _val)
	{ m_pVal = new T(_val); }

	T_Wrapper& operator=(const T_Wrapper& rhs)
	{ if (this != &rhs) { (*m_pVal) = (*rhs.m_pVal); } return *this; }
	T_Wrapper& operator=(const T& _val)
	{ (*m_pVal) = _val; return *this; }

	T& Val() { return *m_pVal; }
	const T& Val() const { return *m_pVal; }

	T* operator->() { return m_pVal; }
	const T* operator->() const { return m_pVal; }

	T& operator*() { return *m_pVal; }
	const T& operator*() const { return *m_pVal; }

#if __cplusplus >= 201103L
	explicit operator T&() { return *m_pVal; }
	explicit operator const T&() const { return *m_pVal; }
#else
	operator T&() { return *m_pVal; }
	operator const T&() const { return *m_pVal; }
#endif

	bool operator== (const T_Wrapper& rhs) const { return *m_pVal == *rhs.m_pVal; }
	bool operator!= (const T_Wrapper& rhs) const { return *m_pVal != *rhs.m_pVal; }
	bool operator== (const T& rhs) const { return *m_pVal == rhs; }
	bool operator!= (const T& rhs) const { return *m_pVal != rhs; }
#if __cplusplus >= 201103L
	T_Wrapper(T_Wrapper&& rhs) noexcept
	{ m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr; }

	T_Wrapper& operator=(T_Wrapper&& rhs) noexcept
	{ if (this != &rhs) { delete m_pVal; m_pVal = rhs.m_pVal; rhs.m_pVal = nullptr;}
		return *this; }

	T_Wrapper(T&& _val) noexcept
	{ m_pVal = new T(std::move(_val)); }

	T_Wrapper& operator=(T&& _val) noexcept
	{ (*m_pVal) = std::move(_val); return *this;}

	~T_Wrapper() noexcept
	{ delete m_pVal; m_pVal = nullptr; }
#else
	~T_Wrapper()
		{ delete m_pVal; m_pVal = NULL; }
#endif

private:
	T* m_pVal;
};

namespace detail {
	template<typename T>
	class FSerializer<T_Wrapper<T> > : public SerializerInterface {
	public:
		typedef T_Wrapper<T> VarType;

		virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
		{
			const VarType& field = *static_cast<const VarType*>(req.pField);
			SerializeReq elemReq(req.writer, &field.Val(), req.serFlag);
			return IJSTI_FSERIALIZER_INS(T)->Serialize(elemReq);
		}

		virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
		{
			VarType& field = *static_cast<VarType*>(req.pFieldBuffer);
			FromJsonReq elemReq(req.stream, req.allocator, req.deserFlag, req.canMoveSrc, &field.Val());
			return IJSTI_FSERIALIZER_INS(T)->FromJson(elemReq, resp);
		}

		virtual void ShrinkAllocator(void *pField) IJSTI_OVERRIDE
		{
			VarType& field = *static_cast<VarType*>(pField);
			return IJSTI_FSERIALIZER_INS(T)->ShrinkAllocator(&field.Val());
		}
	};
}
}
#endif //IJST_TYPES_STD_LAYOUT_WRAPPER_HPP_INCLUDE_
