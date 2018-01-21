/**************************************************************************************************
 *		Serialization implementation of Primitive types
 *		Created by h46incon on 2017/11/2.
 **************************************************************************************************/

#ifndef _IJST_TYPES_STD_HPP_INCLUDE_
#define	_IJST_TYPES_STD_HPP_INCLUDE_

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
//! string -> std::string
typedef std::string 	T_string;
//! anything -> a wrapper of rapidjson::Value
class 					T_raw;

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
	template <typename T> T_wbool(T);	// deleted
	bool m_val;
};

/**
 * @brief Object that contain raw rapidjson::Value and Allocator.
 */
class T_raw {
public:
	T_raw()
	{
		m_pOwnDoc = new rapidjson::Document();
		m_pAllocator = &m_pOwnDoc->GetAllocator();
	}

	T_raw(const T_raw &rhs)
	{
		m_pOwnDoc = new rapidjson::Document();
		m_pAllocator = &m_pOwnDoc->GetAllocator();
		v.CopyFrom(rhs.v, *m_pAllocator);
	}

#if __cplusplus >= 201103L
	T_raw(T_raw &&rhs) IJSTI_NOEXCEPT
	{
		m_pOwnDoc = IJST_NULL;
		m_pAllocator = IJST_NULL;
		Steal(rhs);
	}
#endif

	T_raw &operator=(T_raw rhs)
	{
		Steal(rhs);
		return *this;
	}

	void Steal(T_raw& rhs) IJSTI_NOEXCEPT
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

	~T_raw()
	{
		delete m_pOwnDoc;
		m_pOwnDoc = IJST_NULL;
	}

	//! Get actually value in object
	JsonValue& V() {return v;}
	const JsonValue& V() const {return v;}
	//! See ijst::Accessor::GetAllocator
	JsonAllocator& GetAllocator() {return *m_pAllocator;}
	const JsonAllocator& GetAllocator() const {return *m_pAllocator;}
	//! See ijst::Accessor::GetOwnAllocator
	JsonAllocator& GetOwnAllocator() {return m_pOwnDoc->GetAllocator();}
	const JsonAllocator& GetOwnAllocator() const {return m_pOwnDoc->GetAllocator();}

private:
	friend class detail::FSerializer<T_raw>;
	JsonValue v;
	JsonAllocator* m_pAllocator;
	rapidjson::Document* m_pOwnDoc;		// use pointer to make T_raw be a standard-layout type
};

}	// namespace ijst

namespace ijst {
namespace detail {

#define IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(_Type)						\
	template<> class FSerializer< _Type> : public SerializerInterface {		\
		typedef _Type VarType;												\
	public:

#define IJSTI_DEFINE_SERIALIZE_INTERFACE_END()								\
	};

IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_ubool)
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		return (req.writer.Bool((*pField) != 0) ? 0 : ErrorCode::kWriteFailed);
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsBool()) {
			resp.errDoc.ElementTypeMismatch("bool", req.stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = static_cast<VarType >(req.stream.GetBool() ? 1 : 0);
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
		if (!req.stream.IsBool()) {																				\
			resp.errDoc.ElementTypeMismatch("bool", req.stream);												\
			return ErrorCode::kDeserializeValueTypeError;														\
		}																										\
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);												\
		*pField = req.stream.GetBool();																			\
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
		if (!req.stream.IsInt()) {
			resp.errDoc.ElementTypeMismatch("int", req.stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetInt();
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
		if (!req.stream.IsInt64()) {
			resp.errDoc.ElementTypeMismatch("int64", req.stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetInt64();
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
		if (!req.stream.IsUint()) {
			resp.errDoc.ElementTypeMismatch("uint", req.stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetUint();
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
		if (!req.stream.IsUint64()) {
			resp.errDoc.ElementTypeMismatch("uint64", req.stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetUint64();
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
		if (!req.stream.IsNumber()) {
			resp.errDoc.ElementTypeMismatch("number", req.stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetDouble();
		return 0;
	}

IJSTI_DEFINE_SERIALIZE_INTERFACE_END()


IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_string)
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType& field = *static_cast<const VarType *>(req.pField);
		return (req.writer.String(field.data(), static_cast<rapidjson::SizeType>(field.size())) ? 0 : ErrorCode::kWriteFailed);
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsString()) {
			resp.errDoc.ElementTypeMismatch("string", req.stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = std::string(req.stream.GetString(), req.stream.GetStringLength());
		return 0;
	}
IJSTI_DEFINE_SERIALIZE_INTERFACE_END()


IJSTI_DEFINE_SERIALIZE_INTERFACE_BEGIN(T_raw)
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
			pField->v.Swap(req.stream);
			pField->m_pAllocator = &req.allocator;
		}
		else {
			pField->v.CopyFrom(req.stream, *pField->m_pAllocator);
		}
		return 0;
	}
IJSTI_DEFINE_SERIALIZE_INTERFACE_END()

}	//namespace detail
}	//namespace ijst

#endif //_IJST_TYPES_STD_HPP_INCLUDE_
