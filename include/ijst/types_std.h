/**************************************************************************************************
 *		Serialization implementation of Primitive types
 *		Created by h46incon on 2017/11/2.
 **************************************************************************************************/

#ifndef _IJST_TYPES_STD_HPP_INCLUDE_
#define	_IJST_TYPES_STD_HPP_INCLUDE_

#include "ijst.h"
#include <cstdint>
#include <ctime>

#define IJST_TPRI(_T)	::ijst::detail::TypeClassPrim< ::ijst::FType::_T>

namespace ijst{

struct FType {
public:
	enum _E {
		Bool,
		Int,
		UInt32,
		UInt64,
		Int32,
		Int64,
		String,
		Raw,
		Time,
	};
};

namespace detail{

template<FType::_E _T>
struct TypeClassPrim {
	// nothing
};

}


typedef unsigned char FStoreBool; 		// Could not use bool type because std::vector<bool> is not a container!
typedef int FStoreInt;
typedef std::uint32_t FStoreUInt32;
typedef std::uint64_t FStoreUInt64;
typedef std::int32_t FStoreInt32;
typedef std::int64_t FStoreInt64;
typedef std::string FStoreString;
typedef std::time_t FStoreTime;

class FStoreRaw {
public:
	FStoreRaw()
	{
		m_pOwnDoc = new rapidjson::Document();
		m_pAllocator = &m_pOwnDoc->GetAllocator();
	}

	FStoreRaw(const FStoreRaw &rhs)
	{
		m_pOwnDoc = new rapidjson::Document();
		m_pAllocator = &m_pOwnDoc->GetAllocator();
		v.CopyFrom(rhs.v, *m_pAllocator);
	}

#if __cplusplus >= 201103L
	FStoreRaw(FStoreRaw &&rhs)
	{
		m_pOwnDoc = IJSTI_NULL;
		m_pAllocator = IJSTI_NULL;
		Steal(rhs);
	}
#endif

	FStoreRaw &operator=(FStoreRaw rhs)
	{
		Steal(rhs);
		return *this;
	}

	void Steal(FStoreRaw& raw)
	{
		delete m_pOwnDoc;
		m_pOwnDoc = raw.m_pOwnDoc;
		raw.m_pOwnDoc = IJSTI_NULL;

		m_pAllocator = raw.m_pAllocator;
		raw.m_pAllocator = IJSTI_NULL;
		v = raw.v;
	}

	~FStoreRaw()
	{
		delete m_pOwnDoc;
		m_pOwnDoc = IJSTI_NULL;
	}

	StoreType& V() {return v;}
	const StoreType& V() const {return v;}
	AllocatorType& GetAllocator() {return *m_pAllocator;}
	const AllocatorType& GetAllocator() const {return *m_pAllocator;}

private:
	friend class detail::FSerializer<detail::TypeClassPrim<FType::Raw> >;
	StoreType v;
	AllocatorType* m_pAllocator;
	rapidjson::Document* m_pOwnDoc;		// use pointer to make FStoreRaw be a standard-layout type
};

namespace detail {

template<>
class FSerializer<TypeClassPrim<FType::Bool> > : public SerializerInterface {
public:
	typedef ijst::FStoreBool VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		req.buffer.SetBool((*pField) != 0);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsBool()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not Bool");
			return Err::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = static_cast<unsigned char>(req.stream.GetBool() ? 1 : 0);
		return 0;
	}
};

template<>
class FSerializer<TypeClassPrim<FType::Int> > : public SerializerInterface {
public:
	typedef ijst::FStoreInt VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		req.buffer.SetInt(*pField);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsInt()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not Int");
			return Err::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetInt();
		return 0;
	}
};

template<>
class FSerializer<TypeClassPrim<FType::UInt32> > : public SerializerInterface {
public:
	typedef ijst::FStoreUInt32 VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		req.buffer.SetUint(*pField);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsUint()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not Uint");
			return Err::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetUint();
		return 0;
	}
};

template<>
class FSerializer<TypeClassPrim<FType::UInt64> > : public SerializerInterface {
public:
	typedef ijst::FStoreUInt64 VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		req.buffer.SetUint64(*pField);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsUint64()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not Uint64");
			return Err::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetUint64();
		return 0;
	}
};

template<>
class FSerializer<TypeClassPrim<FType::Int32> > : public SerializerInterface {
public:
	typedef ijst::FStoreInt32 VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		req.buffer.SetInt(*pField);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsInt()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not Int");
			return Err::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetInt();
		return 0;
	}
};

template<>
class FSerializer<TypeClassPrim<FType::Int64> > : public SerializerInterface {
public:
	typedef ijst::FStoreInt64 VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		req.buffer.SetInt64(*pField);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsInt64()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not Int64");
			return Err::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = req.stream.GetInt64();
		return 0;
	}
};

template<>
class FSerializer<TypeClassPrim<FType::String> > : public SerializerInterface {
public:
	typedef ijst::FStoreString VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		req.buffer.SetString(pField->c_str(), pField->length(), req.allocator);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsString()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not String");
			return Err::kDeserializeValueTypeError;
		}

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = std::string(req.stream.GetString(), req.stream.GetStringLength());
		return 0;
	}
};

template<>
class FSerializer<TypeClassPrim<FType::Raw> > : public SerializerInterface {
public:
	typedef ijst::FStoreRaw VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		req.buffer.CopyFrom(pField->V(), req.allocator);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		pField->v.Swap(req.stream);
		pField->m_pAllocator = &req.allocator;
		return 0;
	}

	virtual int SetAllocator(void *pField, AllocatorType &allocator) IJSTI_OVERRIDE
	{
		VarType *pFieldT = static_cast<VarType *>(pField);
		if (pFieldT->m_pAllocator == &allocator) {
			return 0;
		}
		StoreType temp;
		temp = pFieldT->v;
		pFieldT->v.CopyFrom(temp, allocator);
		pFieldT->m_pAllocator = &allocator;
		return 0;
	}
};

template<>
class FSerializer<TypeClassPrim<FType::Time> > : public SerializerInterface {
public:
	typedef ijst::FStoreTime VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		tm *p = localtime(pField);
		char strBuf[32];
		snprintf(strBuf, 49, "%04d-%02d-%02d %02d:%02d:%02d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour,
				 p->tm_min, p->tm_sec
		);
		req.buffer.SetString(strBuf, req.allocator);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsString()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not Int64");
			return Err::kDeserializeValueTypeError;
		}

		tm t;
		// use a dummy to check if any not-whitespace trailing
		char dummy;
		int matched = sscanf(req.stream.GetString(),
							 "%d-%d-%d %d:%d:%d %c",
							 &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &dummy
		);
		if (matched != 6) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is string but not a time format");
			return Err::kDeserializeValueTypeError;
		}
		t.tm_year -= 1900;
		t.tm_mon -= 1;
		t.tm_isdst = 0;

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = mktime(&t);
		return 0;
	}
};

}	//namespace detail
}	//namespace ijst
#endif //_IJST_TYPES_STD_HPP_INCLUDE_
