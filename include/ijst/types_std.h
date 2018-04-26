/**************************************************************************************************
 *		Serialization implementation of Primitive types
 *		Created by h46incon on 2017/11/2.
 **************************************************************************************************/

#ifndef IJST_TYPES_STD_HPP_INCLUDE_
#define	IJST_TYPES_STD_HPP_INCLUDE_

#include "ijst.h"

// Use int types declared in rapidjson
#include <rapidjson/rapidjson.h>

namespace ijst {

//! bool -> bool. @note: Could not declare std::vector<T_bool>
typedef bool 			T_bool;
//! bool -> uint8_t
typedef uint8_t 		T_ubool;
//! bool -> a wrapper of bool
class 					T_wbool;
//! number -> int
typedef int 			T_int;
//! number -> int64_t
typedef int64_t 		T_int64;
//! number -> unsigned int
typedef unsigned int 	T_uint;
//! number -> uint64_t
typedef uint64_t 		T_uint64;
//! string -> std::string
typedef double 			T_double;
//! string -> std::basic_string<Encoding::Ch>
#define IJST_TSTR		::std::basic_string<_ijst_Ch>
//! string -> std::string, for backward compatibility
typedef std::string 	T_string;
//! anything -> a wrapper of rapidjson::GenericValue<Encoding>
#define IJST_TRAW		::ijst::T_GenericRaw<_ijst_Encoding>
//! anything -> a wrapper of rapidjson::Value, for backward compatibility
template<typename Encoding> class T_GenericRaw;
typedef T_GenericRaw<rapidjson::UTF8<> > T_raw;

/**
 * @brief Wrapper of bool to support normal vector<bool>.
 *
 * @note This class could convert to/from bool implicitly.
 */
class T_wbool {
public:
	T_wbool() : m_val(false) {}
	T_wbool(bool _val) : m_val(_val) {}
	operator bool() const { return m_val; }

private:
	template <typename T> T_wbool(T) IJSTI_DELETED;
	bool m_val;
};

/**
 * @brief Object that contain raw rapidjson::Value and Allocator.
 *
 * @tparam Encoding		encoding of json struct
 */
template<typename Encoding>
class T_GenericRaw {
public:
	T_GenericRaw()
	{
		m_pOwnDoc = new TDocument();
		m_pAllocator = &m_pOwnDoc->GetAllocator();
	}

	T_GenericRaw(const T_GenericRaw &rhs)
	{
		m_pOwnDoc = new TDocument();
		m_pAllocator = &m_pOwnDoc->GetAllocator();
		v.CopyFrom(rhs.v, *m_pAllocator);
	}

#if __cplusplus >= 201103L
	T_GenericRaw(T_GenericRaw &&rhs) IJSTI_NOEXCEPT
	{
		m_pOwnDoc = IJST_NULL;
		m_pAllocator = IJST_NULL;
		Steal(rhs);
	}
#endif

	T_GenericRaw &operator=(T_GenericRaw rhs)
	{
		Steal(rhs);
		return *this;
	}

	void Steal(T_GenericRaw& rhs) IJSTI_NOEXCEPT
	{
		if (this == &rhs) {
			return;
		}

		delete m_pOwnDoc;
		m_pOwnDoc = rhs.m_pOwnDoc;
		rhs.m_pOwnDoc = IJST_NULL;

		m_pAllocator = rhs.m_pAllocator;
		rhs.m_pAllocator = IJST_NULL;
		v = rhs.v;
	}

	~T_GenericRaw()
	{
		delete m_pOwnDoc;
		m_pOwnDoc = IJST_NULL;
	}

	//! Get actually value in object
	rapidjson::GenericValue<Encoding>& V() {return v;}
	const rapidjson::GenericValue<Encoding>& V() const {return v;}
	//! See ijst::Accessor::GetAllocator
	rapidjson::MemoryPoolAllocator<>& GetAllocator() {return *m_pAllocator;}
	const rapidjson::MemoryPoolAllocator<>& GetAllocator() const {return *m_pAllocator;}
	//! See ijst::Accessor::GetOwnAllocator
	rapidjson::MemoryPoolAllocator<>& GetOwnAllocator() {return m_pOwnDoc->GetAllocator();}
	const rapidjson::MemoryPoolAllocator<>& GetOwnAllocator() const {return m_pOwnDoc->GetAllocator();}

private:
	typedef rapidjson::GenericDocument<Encoding> TDocument;
	typedef rapidjson::GenericValue<Encoding> TValue;

	friend class detail::FSerializer<T_GenericRaw, Encoding>;
	TValue v;
	rapidjson::MemoryPoolAllocator<>* m_pAllocator;
	TDocument* m_pOwnDoc;		//TODO: use pointer to make T_raw be a standard-layout type
};

}	// namespace ijst

namespace ijst {
namespace detail {

#define IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T)										\
	template<typename Encoding> 														\
	class FSerializer< T, Encoding> : public SerializerInterface<Encoding> {			\
		typedef T VarType;																\
		IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);										\
	public:

#define IJSTI_DEFINE_SERIALIZE_INTERFACE_END()											\
	};

IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_ubool)
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		return (req.writer.Bool((*pField) != 0) ? 0 : ErrorCode::kWriteFailed);
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsBool()), "bool");
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((req.stream.GetBool() == false));

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = static_cast<VarType>(req.stream.GetBool() ? 1 : 0);
		return 0;
	}
IJSTI_DEFINE_SERIALIZE_INTERFACE_END()


#define IJSTI_SERIALIZER_BOOL_DEFINE()																			\
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE												\
	{																											\
		const VarType *pField = static_cast<const VarType *>(req.pField);										\
		return (req.writer.Bool(*pField) ? 0 : ErrorCode::kWriteFailed);										\
	}																											\
																												\

#define IJSTI_SERIALIZER_BOOL_DEFINE_FROM_JSON()																\
	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE					\
	{																											\
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsBool()), "bool");											\
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);												\
		*pField = req.stream.GetBool();																			\
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((*pField == false));													\
		return 0;																								\
	}


IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_bool)
	IJSTI_SERIALIZER_BOOL_DEFINE()
	IJSTI_SERIALIZER_BOOL_DEFINE_FROM_JSON()
IJSTI_DEFINE_SERIALIZE_INTERFACE_END()


IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_wbool)
	IJSTI_SERIALIZER_BOOL_DEFINE()
	IJSTI_SERIALIZER_BOOL_DEFINE_FROM_JSON()
IJSTI_DEFINE_SERIALIZE_INTERFACE_END()


IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_int)
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		return (req.writer.Int(*pField) ? 0 : ErrorCode::kWriteFailed);
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsInt()), "int");
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetInt();
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((*pField == 0));
		return 0;
	}
IJSTI_DEFINE_SERIALIZE_INTERFACE_END()


IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_int64)
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		return (req.writer.Int64(*pField) ? 0 : ErrorCode::kWriteFailed);
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsInt64()), "int64");
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetInt64();
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((*pField == 0));
		return 0;
	}
IJSTI_DEFINE_SERIALIZE_INTERFACE_END()


IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_uint)
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		return (req.writer.Uint(*pField) ? 0 : ErrorCode::kWriteFailed);
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsUint()), "uint");
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetUint();
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((*pField == 0));
		return 0;
	}
IJSTI_DEFINE_SERIALIZE_INTERFACE_END()


IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_uint64)
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		return (req.writer.Uint64(*pField) ? 0 : ErrorCode::kWriteFailed);
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsUint64()), "uint64");
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetUint64();
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((*pField == 0));
		return 0;
	}
IJSTI_DEFINE_SERIALIZE_INTERFACE_END()


IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_double)
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		return (req.writer.Double(*pField) ? 0 : ErrorCode::kWriteFailed);
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsNumber()), "number");
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetDouble();
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((*pField == 0.0));
		return 0;
	}

IJSTI_DEFINE_SERIALIZE_INTERFACE_END()


IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(std::basic_string<typename Encoding::Ch>)
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType& field = *static_cast<const VarType *>(req.pField);
		return (req.writer.String(field.data(), static_cast<rapidjson::SizeType>(field.size())) ? 0 : ErrorCode::kWriteFailed);
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsString()), "string");
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = std::basic_string<typename Encoding::Ch>(req.stream.GetString(), req.stream.GetStringLength());
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((pField->empty()));
		return 0;
	}
IJSTI_DEFINE_SERIALIZE_INTERFACE_END()


IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_GenericRaw<Encoding>)
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		return (pField->V().Accept(req.writer) ? 0 : ErrorCode::kWriteFailed);
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		(void) resp;
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);

		if (req.canMoveSrc) {
			pField->m_pAllocator = &req.allocator;
			pField->v.Swap(req.stream);
		}
		else {
			pField->m_pAllocator = &pField->m_pOwnDoc->GetAllocator();
			pField->v.CopyFrom(req.stream, *pField->m_pAllocator);
		}
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((pField->v.IsNull()));
		return 0;
	}

	virtual void ShrinkAllocator(void *pField) IJSTI_OVERRIDE
	{
		VarType& field = *static_cast<VarType*>(pField);
		detail::Util::ShrinkAllocatorWithOwnDoc(*field.m_pOwnDoc, field.v, field.m_pAllocator);
	}


IJSTI_DEFINE_SERIALIZE_INTERFACE_END()

}	//namespace detail
}	//namespace ijst

#endif //IJST_TYPES_STD_HPP_INCLUDE_
