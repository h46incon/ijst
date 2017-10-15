//
// Created by h46incon on 2017/9/19.
//

#ifndef _IJST_HPP_INCLUDE_
#define _IJST_HPP_INCLUDE_

#include "rapidjson/document.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <cassert>
#include <sstream>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>
#include <w32api/tchar.h>

/**	========================================================================================
 *				Public Interface
 */

#define IJST_OUT
#define IJST_INOUT

#define IJST_TPRI(_T)	::ijst::detail::TypeClassPrim< ::ijst::FType::_T>
#define IJST_TVEC(_T)	::ijst::detail::TypeClassVec< _T>
#define IJST_TMAP(_T)	::ijst::detail::TypeClassMap< _T>
#define IJST_TOBJ(_T)	::ijst::detail::TypeClassObj< _T>
#define IJST_DEFINE_STRUCT(...) \
    IJSTI_DEFINE_STRUCT_IMPL(IJSTI_PP_NFIELD(__VA_ARGS__), __VA_ARGS__)

#define IJST_GET_STATUS(obj, field)				obj._.GetStatus(& ((obj).field))
#define IJST_MARK_VALID(obj, field)				obj._.MarkValid(& ((obj).field))
#define IJST_MARK_NULL(obj, field)				obj._.MarkNull(& ((obj).field))
#define IJST_MARK_REMOVED(obj, field)			obj._.MarkRemoved(& ((obj).field))
#define IJST_SET(obj, field, val)				obj._.Set((obj).field, (val))
#define IJST_SET_STRICT(obj, field, val)		obj._.SetStrict((obj).field, (val))
// TODO:
#define IJST_REMOVE(obj, field)				obj._.GetStatus(& ((obj).field))

namespace ijst {

typedef rapidjson::Value StoreType;
//typedef rapidjson::GenericDocument::AllocatorType 	AllocatorType;
typedef rapidjson::MemoryPoolAllocator<> 	AllocatorType;

struct FType {
public:
	enum _E {
		Bool,
		Int,
		String,
	};
};

struct FDesc {
	static const unsigned int _MaskDesc 		= 0x000000FF;
	static const unsigned int Optional 			= 0x00000001;
	static const unsigned int Nullable 			= 0x00000002;
	// Element nullable is hard to represent, has not plan to implement it
};

struct FStatus {
public:
	enum _E {
		kNotAField,
		kMissing,
		kNull,
		kParseFailed,
		kValid,
		kRemoved,
	};
};

struct Err {
	static const int kSucc 							= 0x00000000;
	static const int kDeserializeValueTypeError 	= 0x00001001;
	static const int kDeserializeSomeFiledsInvalid 	= 0x00001002;
	static const int kParseFaild 					= 0x00001003;
	static const int kInnerError 					= 0x00002001;
};

struct BufferInfo {
	BufferInfo(): BufferInfo(0, 0, true) {}
	BufferInfo(StoreType* _pBuffer, AllocatorType* _pAllocator, bool _useReferenceBuffer)
			: pBuffer(_pBuffer), pAllocator(_pAllocator), useReferenceBuffer(_useReferenceBuffer){}

	StoreType* pBuffer;
	AllocatorType* pAllocator;
	bool useReferenceBuffer;
};
/**	========================================================================================
 *				Inner Interface
 */
namespace detail {

// LIKELY and UNLIKELY
#if defined(__GNUC__) || defined(__clang__)
	#define IJSTI_LIKELY(x)			__builtin_expect(!!(x), 1)
	#define IJSTI_UNLIKELY(x)		__builtin_expect(!!(x), 0)
#else
	#define IJSTI_LIKELY(x) 		(x)
	#define IJSTI_UNLIKELY(x)		(x)
#endif

#define IJSTI_MAP_TYPE    			std::map
#define IJSTI_STORE_MOVE(dest, src)							\
	do {                                    				\
        if (IJSTI_UNLIKELY(&(dest) != &(src))) {            \
            /*RapidJson's assigment behaviour is move */ 	\
            (dest) = (src);                 				\
        }                                   				\
    } while (false)


#if __cplusplus >= 201103L
	#define IJSTI_MOVE(val) 	std::move(val)
	#define IJSTI_NULL 			nullptr

#else
	#define IJSTI_MOVE(val) 	(val)
	#define IJSTI_NULL 			0
#endif


template<FType::_E>
struct TypeClassPrim {
	// nothing
};

template<class _T>
struct TypeClassVec {
	// nothing
};

template<class _T>
struct TypeClassMap {
	// nothing
};

template<class _T>
struct TypeClassObj {
	// nothing
};

/**
 * Singleton interface
 * @tparam _T type
 */
template<typename _T>
class Singleton {
public:
	static _T *GetInstance()
	{
		static _T instance;
		return &instance;
	}

	inline static void InitInstanceBeforeMain()
	{
		// When accessing initInstanceTag in code, the GetInstance() function will be called before main
		volatile void* dummy = initInstanceTag;
	}

private:
	static void* initInstanceTag;
};
template<typename _T> void *Singleton<_T>::initInstanceTag = Singleton<_T>::GetInstance();


class BufferFinder {
public:
	explicit BufferFinder(BufferInfo& _bufferInfo)
	{
		SetBuffer(_bufferInfo);
	}

	BufferFinder(BufferFinder* _parent, const std::string& _key)
	{
		SetParent(_parent, _key);
	}

	void SetParent(BufferFinder *_parent, const std::string& _key)
	{
		parentInfo.parent = _parent;
		parentInfo.key = _key;
		hasParent = true;
	}

	void SetBuffer(BufferInfo& _bufferInfo)
	{
		assert(!_bufferInfo.useReferenceBuffer);
		bufferInfo = _bufferInfo;
		hasParent = false;
	}

	bool FindBuffer(IJST_OUT BufferInfo& out) const
	{
		if (!hasParent) {
			assert(!bufferInfo.useReferenceBuffer);
			out = bufferInfo;
			return true;
		}

		BufferInfo parentBuffer;
		bool succ = parentInfo.parent->FindBuffer(parentBuffer);
		if (!succ) {
			return false;
		}

		rapidjson::GenericStringRef<char> fileNameRef =
				rapidjson::StringRef(parentInfo.key.c_str(), parentInfo.key.length());
		const rapidjson::Value::MemberIterator itMember = parentBuffer.pBuffer->FindMember(fileNameRef);
		if (itMember == parentBuffer.pBuffer->MemberEnd()) {
			return false;
		}

		out.pBuffer = &(itMember->value);
		out.pAllocator = parentBuffer.pAllocator;
		out.useReferenceBuffer = true;
		return true;
	}

private:
	struct ParentInfo {
		BufferFinder* parent;
		std::string key;
	};
	union {
		ParentInfo parentInfo;
		BufferInfo bufferInfo;
	};
	bool hasParent;
};

class SerializerInterface {
public:
	struct SerializeReq {
		// Pointer of field to serialize.
		// The actual type of field should be decide in the derived class
		const void* pField;

		// true if need serialize all field, false if serialize only valid field
		bool pushAllField;

		// true if copy inner buffer to output buffer first
		bool useInnerBuffer;

		// Output buffer info. The instance should serialize in this object and use allocator
		StoreType& buffer;
		AllocatorType& allocator;

		// New buffer finder information
		// NULL if do not update buffer finder information (usually when serializing a vector, or use in outer buffer)
		BufferFinder* pBufferFinder;

		SerializeReq(StoreType &_buffer, AllocatorType &_allocator,
					 const void *_pField, bool _pushAllfield, bool useInnerBuffer, BufferFinder* _pBufferFinder)
				:buffer(_buffer)
				,allocator(_allocator)
				,pField(_pField)
				,pushAllField(_pushAllfield)
				,useInnerBuffer(useInnerBuffer)
				,pBufferFinder(_pBufferFinder)
		{ }
	};

	struct SerializeResp {
	};

	struct DeserializeReq {
		// Pointer of deserialize output.
		// The instance should deserialize in this object
		// The actual type of field should be decide in the derived class
		void* pFieldBuffer;

		// The input stream and allocator
		StoreType& stream;
		AllocatorType& allocator;

		// New buffer finder information
		// NULL if do not update buffer finder information (usually when deserializing a array)
		BufferFinder* pBufferFinder;

		DeserializeReq(StoreType &_stream, AllocatorType &_allocator, void *_pField, BufferFinder* _pBufferFinder)
				:stream(_stream)
				,allocator(_allocator)
				,pFieldBuffer(_pField)
				,pBufferFinder(_pBufferFinder)
		{ }
	};

	struct DeserializeResp {
		FStatus::_E fStatus;
		size_t fieldCount;
		const bool needErrMsg;
		std::string errMsg;

		explicit DeserializeResp(bool _needErrMsg = false) :
				fStatus(FStatus::kMissing),
				fieldCount(0),
				needErrMsg(_needErrMsg)
		{ }

		template<typename _T>
		bool SetErrMsg(const _T &_errMsg)
		{
			if (!needErrMsg) {
				return false;
			}
			errMsg = _errMsg;
			return true;
		}

		template<typename _T>
		bool CombineErrMsg(const _T & errMsg, const DeserializeResp& childResp)
		{
			if (!needErrMsg) {
				return false;
			}

			std::stringstream oss;
			oss << errMsg << "[" << childResp.errMsg << "]";
			return true;
		}
	};

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp)= 0;

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp)= 0;

	virtual int Reset(void *pField)= 0;

	virtual ~SerializerInterface()
	{ }
};

/**
 * Template interface of serialization class
 * This template is unimplemented,  and will throw complie error when use it.
 * @tparam _T class
 */
template<class _T>
class FSerializer : public SerializerInterface {
public:
	typedef void VarType;

	virtual int
	Serialize(const SerializeReq &req, SerializeResp &resp) = 0;

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) = 0;

	virtual int Reset(void *pField) = 0;
};
#define IJSTI_FSERIALIZER_INS(_T) ::ijst::detail::Singleton< ::ijst::detail::FSerializer< _T> >::GetInstance()

#include "ijst_serialize.inc"
/**	========================================================================================
 *				Private
 */

/**
 * Serialization class of Object types
 * @tparam _T class
 */
template<class _T>
class FSerializer<TypeClassObj<_T> > : public SerializerInterface {
public:
	typedef _T VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp)
	{
		_T *ptr = (_T *) req.pField;
		return ptr->_.DoSerialize(req);
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp)
	{
		_T *ptr = (_T *) req.pFieldBuffer;
		return ptr->_.DoDeserialize(req, resp);
	}

	virtual int Reset(void* pField)
	{
		_T *ptr = (_T *) pField;
		return ptr->_.Reset();
	}
};

/**
 * Serialization class of Vector types
 * @tparam _T class
 */
template<class _T>
class FSerializer<TypeClassVec<_T> > : public SerializerInterface {
private:
	typedef typename FSerializer<_T>::VarType ElemVarType;
public:
	typedef std::vector<ElemVarType> VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp)
	{
		const VarType *ptr = static_cast<const VarType *>(req.pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);
		req.buffer.SetArray();
		// Reserve first to make sure rapidjson will not reallocate buffer
		req.buffer.Reserve(ptr->size(), req.allocator);
		for (typename VarType::const_iterator itera = ptr->begin(); itera != ptr->end(); ++itera) {
			req.buffer.PushBack(
					rapidjson::Value(rapidjson::kNullType).Move(),
					req.allocator
			);
			StoreType &newElem = req.buffer[req.buffer.Size() - 1];

			SerializeReq elemReq(
					newElem, req.allocator, &(*itera), req.pushAllField, req.useInnerBuffer, IJSTI_NULL);

			SerializeResp elemResp;
			int ret = interface->Serialize(elemReq, elemResp);
			if (IJSTI_UNLIKELY(ret != 0))
			{
				req.buffer.PopBack();
				return ret;
			}
		}
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp)
	{
		if (!req.stream.IsArray()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not a Array");
			return Err::kDeserializeValueTypeError;
		}

		VarType *pBufferT = static_cast<VarType *>(req.pFieldBuffer);
		pBufferT->clear();
		// pBufferT->shrink_to_fit();
		pBufferT->reserve(req.stream.Size());
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(_T);

		for (rapidjson::Value::ValueIterator itVal = req.stream.Begin();
			 itVal != req.stream.End(); ++itVal)
		{
			// Alloc buffer
			// Use resize() instead of push_back() to avoid copy constructor in C++11
			pBufferT->resize(pBufferT->size() + 1);
			DeserializeReq elemReq(*itVal, req.allocator, &pBufferT->back(), IJSTI_NULL);

			// Deserialize
			DeserializeResp elemResp(resp.needErrMsg);
			int ret = serializerInterface->Deserialize(elemReq, elemResp);
			if (IJSTI_UNLIKELY(ret != 0))
			{
				pBufferT->pop_back();
				if (resp.needErrMsg)
				{
					std::stringstream oss;
					oss << "Deserialize elem error. index: " << pBufferT->size() << ", err: ";
					resp.CombineErrMsg(oss.str(), elemResp);
				}
				return ret;
			}
			resp.fStatus = FStatus::kParseFailed;
			++resp.fieldCount;
		}
		resp.fStatus = FStatus::kValid;
		return 0;
	}

	virtual int Reset(void* pField)
	{
		VarType *ptr = static_cast<VarType *>(pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);

		// Reset member
		for (typename VarType::iterator itera = ptr->begin(); itera != ptr->end(); ++itera)
		{
			int ret = interface->Reset(&(*itera));
			if (IJSTI_UNLIKELY(ret != 0))
			{
				return ret;
			}
		}

		// Clear
		ptr->clear();
		return 0;
	}
};

/**
 * Serialization class of Map types
 * @tparam _T class
 */
template<class _T>
class FSerializer<TypeClassMap<_T> > : public SerializerInterface {
private:
	typedef typename FSerializer<_T>::VarType ElemVarType;
public:
	typedef std::map<std::string, ElemVarType> VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp)
	{
		const VarType *ptr = static_cast<const VarType *>(req.pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);
		if (!req.buffer.IsObject()) {
			req.buffer.SetObject();
		}

		for (typename VarType::const_iterator itFieldMember = ptr->begin(); itFieldMember != ptr->end(); ++itFieldMember)
		{
			// Init
			const void* pFieldValue = &itFieldMember->second;
			rapidjson::GenericStringRef<char> fileNameRef =
					rapidjson::StringRef(itFieldMember->first.c_str(), itFieldMember->first.length());
			rapidjson::Value fieldNameVal;
			fieldNameVal.SetString(fileNameRef);

			// Add new member when need
			StoreType* pNewElem = IJSTI_NULL;
			bool hasAllocMember = false;
			{
				rapidjson::Value::MemberIterator itMember = req.buffer.FindMember(fieldNameVal);
				if (itMember == req.buffer.MemberEnd()) {
					// Add member by copy key name
					req.buffer.AddMember(
							rapidjson::Value(fieldNameVal, req.allocator).Move(),
							rapidjson::Value(rapidjson::kNullType).Move(),
							req.allocator
					);
					pNewElem = &req.buffer[fieldNameVal];
					hasAllocMember = true;
				}
				else {
					pNewElem = &(itMember->value);
				}
			}

			BufferFinder *pElemBufferFinder = IJSTI_NULL;
			if (req.pBufferFinder != IJSTI_NULL) {
				pElemBufferFinder = new BufferFinder(
						req.pBufferFinder,
						std::string(itFieldMember->first.c_str(), itFieldMember->first.length())
						);
			}
			SerializeReq elemReq(
					*pNewElem, req.allocator, pFieldValue, req.pushAllField, req.useInnerBuffer, pElemBufferFinder);

			SerializeResp elemResp;
			int ret = interface->Serialize(elemReq, elemResp);

			// clear new() object
			if (pElemBufferFinder != IJSTI_NULL) {
				delete pElemBufferFinder;
				pElemBufferFinder = IJSTI_NULL;
			}

			if (IJSTI_UNLIKELY(ret != 0)) {
				if (hasAllocMember) {
					req.buffer.RemoveMember(fieldNameVal);
				}
				return ret;
			}
			assert(&(req.buffer[fieldNameVal]) == &elemReq.buffer);
		}
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp)
	{
		if (!req.stream.IsObject()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not a Object");
			return Err::kDeserializeValueTypeError;
		}

		VarType *pBufferT = static_cast<VarType *>(req.pFieldBuffer);
		pBufferT->clear();
		// pBufferT->shrink_to_fit();
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(_T);

		for (rapidjson::Value::MemberIterator itMember = req.stream.MemberBegin();
			 itMember != req.stream.MemberEnd(); ++itMember)
		{
			// Get information
			const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
			bool hasAlloc = false;
			if (pBufferT->find(fieldName) == pBufferT->end()) {
				hasAlloc = true;
			}

			// Alloc buffer
			BufferFinder *pElemBufferFinder = IJSTI_NULL;
			BufferFinder elemBufferFinder(req.pBufferFinder, fieldName);
			if (req.pBufferFinder != IJSTI_NULL) {
				// Do not declare elemBufferFinder, otherwise it will be destroy after if() statement
				pElemBufferFinder = &elemBufferFinder;
			}
			ElemVarType &elemBuffer = (*pBufferT)[fieldName];
			DeserializeReq elemReq(itMember->value, req.allocator, &elemBuffer, pElemBufferFinder);

			// Deserialize
			DeserializeResp elemResp(resp.needErrMsg);
			int ret = serializerInterface->Deserialize(elemReq, elemResp);
			if (IJSTI_UNLIKELY(ret != 0))
			{
				if (hasAlloc)
				{
					pBufferT->erase(fieldName);
				}
				resp.needErrMsg &&
					resp.CombineErrMsg("Deserialize elem error. key: " + fieldName + ", err: ",
								   elemResp
				);
				resp.fStatus = FStatus::kParseFailed;
				return ret;
			}
			++resp.fieldCount;
		}
		resp.fStatus = FStatus::kValid;
		return 0;
	}

	virtual int Reset(void* pField)
	{
		VarType *ptr = static_cast<VarType *>(pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);

		// Reset member
		for (typename VarType::iterator itera = ptr->begin(); itera != ptr->end(); ++itera)
		{
			int ret = interface->Reset(&(itera->second));
			if (IJSTI_UNLIKELY(ret != 0))
			{
				return ret;
			}
		}

		// Clear
		ptr->clear();
		return 0;
	}
};

struct MetaField { // NOLINT
	std::string name;
	std::size_t offset;
	unsigned int desc;
	SerializerInterface *serializerInterface;
};

class MetaClass {
public:
	MetaClass() : mapInited(false), accessorOffset(0) { }

	void PushMetaField(const std::string &name, std::size_t offset,
					   unsigned int desc, SerializerInterface *serializerInterface)
	{
		MetaField metaField;
		metaField.name = name;
		metaField.offset = offset;
		metaField.desc = desc;
		metaField.serializerInterface = serializerInterface;

		metaFields.push_back(IJSTI_MOVE(metaField));
	}

	void InitMap()
	{
		if (IJSTI_UNLIKELY(mapInited)) {
			throw std::runtime_error("MetaClass's map has inited before");
		}

		for (size_t i = 0; i < metaFields.size(); ++i) {
			const MetaField *ptrMetaField = &(metaFields[i]);
			// Check key exist
			if (IJSTI_UNLIKELY(mapName.find(ptrMetaField->name) != mapName.end())) {
				throw std::runtime_error("MetaClass's name conflict:" + ptrMetaField->name);
			}
			if (IJSTI_UNLIKELY(mapOffset.find(ptrMetaField->offset) != mapOffset.end())) {
				throw std::runtime_error("MetaClass's offset conflict:" + ptrMetaField->offset);
			}

			mapName[ptrMetaField->name] = ptrMetaField;
			mapOffset[ptrMetaField->offset] = ptrMetaField;
		}
		mapInited = true;
	}

	std::vector<MetaField> metaFields;
	IJSTI_MAP_TYPE<std::string, const MetaField *> mapName;
	IJSTI_MAP_TYPE<std::size_t, const MetaField *> mapOffset;
	std::string tag;
	std::size_t accessorOffset;
private:
	bool mapInited;

};

/**
 * Reflection info
 * Push meta class info of _T in specialized constructor MetaInfo<_T>()
 * @tparam _T: class
 */
template<class _T>
class MetaInfo {
public:
	typedef _T FieldType;
	MetaClass metaClass;

private:
	friend class Singleton<MetaInfo<_T> >;

	MetaInfo()
	{
		_T::InitMetaInfo(this);
	}
};

template<class _T>
class MetaInfoIniter {
private:
	friend class Singleton<MetaInfoIniter<_T> >;
	MetaInfoIniter()
	{
		_T::InitMetaInfo();
	}
};

class Accessor {
public:
	/*
	 * Constructor
	 */
	Accessor(const MetaClass *_metaClass) :
			m_metaClass(_metaClass)
	{
		InitParentPtr();
		m_pDummyDoc = new rapidjson::Document(rapidjson::kObjectType);
		m_pBufferFinder = IJSTI_NULL;
		ResetInnerBuffer();
	}

	Accessor(const Accessor &rhs) :
			m_fieldStatus(rhs.m_fieldStatus)
	{
		assert(this != &rhs);

		m_metaClass = rhs.m_metaClass;
		InitParentPtr();
		m_pDummyDoc = new rapidjson::Document(rapidjson::kNullType);
		m_pDummyDoc->CopyFrom(*rhs.m_pDummyDoc, m_pDummyDoc->GetAllocator());
		m_useDummyDoc = rhs.m_useDummyDoc;
		m_pBufferFinder = IJSTI_NULL;
		ResetInnerBuffer();
		if (!m_useDummyDoc) {
			*m_pBufferFinder = *rhs.m_pBufferFinder;
		}
	}

#if __cplusplus >= 201103L
	Accessor(Accessor &&rhs)
	{
		m_pDummyDoc = IJSTI_NULL;
		Steal(rhs);
	}
#endif

	Accessor &operator=(Accessor rhs)
	{
		Steal(rhs);
		return *this;
	}

	~Accessor()
	{
		clearPtr();
	}

	void clearPtr()
	{
		if (m_pDummyDoc != IJSTI_NULL) {
			delete m_pDummyDoc;
			m_pDummyDoc = IJSTI_NULL;
		}
		if (m_pBufferFinder != IJSTI_NULL) {
			delete m_pBufferFinder;
			m_pBufferFinder = IJSTI_NULL;
		}
	}

	void Steal(Accessor &rhs)
	{
		if (IJSTI_UNLIKELY(this == &rhs)) {
			return;
		}

		clearPtr();
		// Handler pointer
		m_pDummyDoc = rhs.m_pDummyDoc;
		rhs.m_pDummyDoc = IJSTI_NULL;
		m_pBufferFinder = rhs.m_pBufferFinder;
		rhs.m_pBufferFinder = IJSTI_NULL;

		// other simple field
		m_metaClass = rhs.m_metaClass;
		rhs.m_metaClass = IJSTI_NULL;

		m_useDummyDoc = rhs.m_useDummyDoc;

		m_fieldStatus = IJSTI_MOVE(rhs.m_fieldStatus);
		InitParentPtr();
	}

	int Reset()
	{
		// Reset member
		for (std::vector<MetaField>::const_iterator itMetaField = m_metaClass->metaFields.begin();
			 itMetaField != m_metaClass->metaFields.end(); ++itMetaField) {

			void *pField = GetFieldByOffset(itMetaField->offset);
			int ret = itMetaField->serializerInterface->Reset(pField);
			if (IJSTI_UNLIKELY(ret != 0)){
				return ret;
			}
		}

		ResetInnerBuffer();
		m_fieldStatus.clear();
		return 0;
	}

	/*
	 * Field accessor
	 */
	template<typename _T1, typename _T2>
	inline void Set(_T1 &field, const _T2 &value)
	{
		MarkValid(&field);
		field = value;
	}

	template<typename _T>
	inline void SetStrict(_T &field, const _T &value)
	{
		Set(field, value);
	}

	inline void MarkValid(const void* fieldPtr)
	{
		MarkFieldStatus(fieldPtr, FStatus::kValid);
	}

	inline void MarkNull(const void* fieldPtr)
	{
		MarkFieldStatus(fieldPtr, FStatus::kNull);
	}

	inline void MarkRemoved(const void* fieldPtr)
	{
		MarkFieldStatus(fieldPtr, FStatus::kRemoved);
	}

	inline FStatus::_E GetStatus(const void *fieldptr) const
	{
		const size_t offset = GetFieldOffset(fieldptr);
		return GetStatusByOffset(offset);
	}

	/**
	 * Find Inner Buffer used the object
	 * Be careful: the inner buffer returned may be invalid in many case.
	 * @return
	 */
	BufferInfo FindUsedBuffer()
	{
		if (m_useDummyDoc) {
			return BufferInfo(m_pDummyDoc, &m_pDummyDoc->GetAllocator(), true);
		}
		else {
			BufferInfo bufferInfo;
			bool succ = m_pBufferFinder->FindBuffer(bufferInfo);
			if (succ) {
				return bufferInfo;
			}
			else {
				ResetInnerBuffer();
				return BufferInfo(m_pDummyDoc, &m_pDummyDoc->GetAllocator(), true);
			}
		}
	}

	void ForceUseInnerBuffer()
	{
		m_useDummyDoc = true;
	}

	/*
	 * Serialize
	 */
	inline int SerializeInplace(bool pushAllField)
	{
		BufferInfo usedBufferInfo = FindUsedBuffer();
		SerializeReq req(*usedBufferInfo.pBuffer, *usedBufferInfo.pAllocator, this, pushAllField, /*useInnerBuffer=*/true, IJSTI_NULL);
		return DoSerialize(req, &usedBufferInfo);
	}

	int Serialize(bool pushAllField, IJST_INOUT rapidjson::Document& docOutput) const
	{
		SerializeReq req(docOutput, docOutput.GetAllocator(), this, pushAllField, /*useInnerBuffer=*/false, IJSTI_NULL);
		return DoSerializeConst(req);
	}

	/**
	 * Deserialize
	 * @param errMsg. Output of error message, null if cancel error output
	 */
	inline int DeserializeMoveFrom(rapidjson::Document& srcDocStolen, IJST_INOUT std::string* errMsg)
	{
		ResetInnerBuffer();
		m_pDummyDoc->Swap(srcDocStolen);
		return DeserializeInInnerBuffer(errMsg);
	}

	inline int Deserialize(const rapidjson::Document& srcDoc, IJST_INOUT std::string* errMsg)
	{
		ResetInnerBuffer();
		m_pDummyDoc->CopyFrom(srcDoc, m_pDummyDoc->GetAllocator());
		return DeserializeInInnerBuffer(errMsg);
	}

	int Deserialize(const char* str, std::size_t length, IJST_INOUT std::string* errMsg)
	{
		ResetInnerBuffer();
		m_pDummyDoc->Parse(str, length);
		if (IJSTI_UNLIKELY(m_pDummyDoc->HasParseError()))
		{
			if (errMsg != IJSTI_NULL)
			{
				*errMsg = IJSTI_MOVE(std::string(
						rapidjson::GetParseError_En(m_pDummyDoc->GetParseError())
				));
			}
			return Err::kParseFaild;
		}
		return DeserializeInInnerBuffer(errMsg);
	}

	inline int Deserialize(const std::string& input, IJST_INOUT std::string* errMsg)
	{
		return Deserialize(input.c_str(), input.length(), errMsg);
	}


	bool WriteUsedBuffer(IJST_OUT std::string &strOutput)
	{
		rapidjson::StringBuffer buffer;
		buffer.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		BufferInfo useBufferInfo = FindUsedBuffer();
		bool bSucc = useBufferInfo.pBuffer->Accept(writer);
		if (IJSTI_LIKELY(bSucc))
		{
			strOutput = std::string(buffer.GetString(), buffer.GetSize());
		}
		return bSucc;
	}

//	void ShowTag() const
//	{
//		std::cout << "tag:" << m_metaClass->tag << std::endl;
//		const size_t count = m_metaClass->metaFields.size();
//		for (size_t i = 0; i < count; ++i) {
//			std::cout << "name:" << m_metaClass->metaFields[i].name << std::endl
//					  << "offset:" << m_metaClass->metaFields[i].offset << std::endl;
//		}
//	}

private:
	typedef SerializerInterface::SerializeReq SerializeReq;
	typedef SerializerInterface::SerializeResp SerializeResp;
	typedef SerializerInterface::DeserializeReq DeserializeReq;
	typedef SerializerInterface::DeserializeResp DeserializeResp;

	inline void InitParentPtr()
	{
		m_parentPtr = reinterpret_cast<const unsigned char *>(this - m_metaClass->accessorOffset);
	}

	void MarkFieldStatus(const void* field, FStatus::_E fStatus)
	{
		const std::size_t offset = GetFieldOffset(field);
		if (IJSTI_UNLIKELY(m_metaClass->mapOffset.find(offset) == m_metaClass->mapOffset.end())) {
			throw std::runtime_error("could not find field with expected offset: " + offset);
		}

		m_fieldStatus[offset] = fStatus;
	}

	inline int DeserializeInInnerBuffer(IJST_INOUT std::string *errMsg)
	{
		bool needErrMessage = (errMsg != IJSTI_NULL);
		DeserializeResp resp(needErrMessage);
		int ret = DoDeserializeInInnerBuffer(resp);
		if (needErrMessage) {
			*errMsg = IJSTI_MOVE(resp.errMsg);
		}
		return ret;
	}

	/**
	 * Serialize implement
	 * @param req 			serialize req
	 * @param pUsedBuffer 	cached result return from FindUsedBuffer(). NULL if not specify the cache
	 * @return
	 */
	int DoSerialize(const SerializeReq& req, BufferInfo* pUsedBuffer = IJSTI_NULL)
	{
		assert(req.pField == this || req.pField == IJSTI_NULL);
		if (req.useInnerBuffer) {
			// copy inner data to buffer
			BufferInfo tmp;
			if (pUsedBuffer == IJSTI_NULL) {
				tmp = FindUsedBuffer();
				pUsedBuffer = &tmp;
			}
			if (pUsedBuffer->pAllocator == &req.allocator) {
				IJSTI_STORE_MOVE(req.buffer, *pUsedBuffer->pBuffer);
			} else {
				req.buffer.CopyFrom(*pUsedBuffer->pBuffer, req.allocator);
			}
		}

		if (req.pBufferFinder != IJSTI_NULL) {
			SetBufferFinder(*req.pBufferFinder);
		}

		return DoSerializeConst(req);

	}

	int DoSerializeConst(const SerializeReq &req) const
	{
		if (!req.buffer.IsObject()) {
			req.buffer.SetObject();
		}

		// Loop each field
		for (std::vector::const_iterator itMetaField = m_metaClass->metaFields.begin();
			 itMetaField != m_metaClass->metaFields.end(); ++itMetaField) {

			// Check field state
			FStatus::_E fstatus = GetStatusByOffset(itMetaField->offset);
			switch (fstatus) {
				case FStatus::kValid:
				{
					int ret = SerializeField(req, itMetaField);
					if (IJSTI_UNLIKELY(ret != 0)) {
						return ret;
					}
				}
					break;

				case FStatus::kNull:
				{
					int ret = SerializeNullField(itMetaField, req.buffer, req.allocator);
					if (IJSTI_UNLIKELY(ret != 0)) {
						return ret;
					}
				}
					break;

				case FStatus::kRemoved:
					RemoveFieldInBuffer(itMetaField, req.buffer);
					break;

				case FStatus::kMissing:
				case FStatus::kParseFailed:
				{
					if (!req.pushAllField) {
						continue;
					}
					int ret = SerializeField(req, itMetaField);
					if (IJSTI_UNLIKELY(ret != 0)) {
						return ret;
					}
				}
					break;

				case FStatus::kNotAField:
				default:
					// Error occurs
					assert(false);
					return Err::kInnerError;
			}

		}

		return 0;
	}

	int SerializeField(const SerializeReq &req, const std::vector<MetaField>::const_iterator &itMetaField) const
	{
		// Init
		const void *pFieldValue = GetFieldByOffset(itMetaField->offset);
		rapidjson::GenericStringRef<char> fieldNameRef =
				rapidjson::StringRef(itMetaField->name.c_str(), itMetaField->name.length());
		rapidjson::Value fieldNameVal;
		fieldNameVal.SetString(fieldNameRef);

		// Try allocate new element buffer when need
		StoreType *pNewElem = IJSTI_NULL;
		bool hasAllocMember = false;
		{
			rapidjson::Value::MemberIterator itMember = req.buffer.FindMember(fieldNameVal);
			if (itMember == req.buffer.MemberEnd()) {
				// Add member, field name is not need deep copy
				req.buffer.AddMember(
						rapidjson::Value().SetString(fieldNameRef),
						rapidjson::Value(rapidjson::kNullType).Move(),
						req.allocator
				);
				// Why rapidjson AddMember function do not return newly create member
				pNewElem = &req.buffer[fieldNameVal];
				hasAllocMember = true;
			}
			else {
				pNewElem = &(itMember->value);
			}
		}

		// Serialize field
		BufferFinder* pElemBufferFinder = IJSTI_NULL;
		BufferFinder elemBufferFinder(req.pBufferFinder, itMetaField->name);
		if (req.pBufferFinder == IJSTI_NULL) {
			// elemBufferFinder could not declared here, otherwise it will be destory after if() statement
			pElemBufferFinder = &elemBufferFinder;
		}
		SerializeReq elemSerializeReq(
				*pNewElem, req.allocator, pFieldValue, req.pushAllField, req.useInnerBuffer, pElemBufferFinder);

		SerializeResp elemSerializeResp;
		int ret = itMetaField->serializerInterface->Serialize(elemSerializeReq, elemSerializeResp);
		if (IJSTI_UNLIKELY(ret != 0)) {
			if (hasAllocMember) {
				req.buffer.RemoveMember(fieldNameVal);
			}
			return ret;
		}
		assert(&(req.buffer[fieldNameVal]) == &elemSerializeReq.buffer);
		return 0;
	}

	int SerializeNullField(const std::vector<MetaField>::const_iterator& itMetaField,
					   StoreType& buffer, AllocatorType& allocator) const
	{
		// Init
		const void *pFieldValue = GetFieldByOffset(itMetaField->offset);
		rapidjson::GenericStringRef<char> fieldNameRef =
				rapidjson::StringRef(itMetaField->name.c_str(), itMetaField->name.length());
		rapidjson::Value fieldNameVal;
		fieldNameVal.SetString(fieldNameRef);

		rapidjson::Value::MemberIterator itMember = buffer.FindMember(fieldNameVal);
		if (itMember == buffer.MemberEnd()) {
			// Add member, field name is not need deep copy
			buffer.AddMember(
					rapidjson::Value().SetString(fieldNameRef),
					rapidjson::Value(rapidjson::kNullType).Move(),
					allocator
			);
		}
		else {
			itMember->value.SetNull();
		}

		return 0;
	}

	int RemoveFieldInBuffer(const std::vector<MetaField>::const_iterator &itMetaField, StoreType &buffer) const
	{
		// Reset field
		void* pField = GetFieldByOffset(itMetaField->offset);
		int ret = itMetaField->serializerInterface->Reset(pField);
		if (IJSTI_UNLIKELY(ret != 0)) {
			return ret;
		}

		// Remove member in buffer
		rapidjson::GenericStringRef<char> fieldNameRef =
				rapidjson::StringRef(itMetaField->name.c_str(), itMetaField->name.length());
		rapidjson::Value fieldNameVal;
		fieldNameVal.SetString(fieldNameRef);
		return buffer.RemoveMember(fieldNameVal);
	}

	/*
	 * Deserialize by stream
	 */
	int DoDeserialize(const DeserializeReq &req, IJST_OUT DeserializeResp& resp)
	{
		assert(req.pFieldBuffer == IJSTI_NULL || req.pFieldBuffer == this);

		// Store BufferFinder
		if (req.pBufferFinder != IJSTI_NULL) {
			SetBufferFinder(*req.pBufferFinder);
		}

		return DoDeserializeInInnerBuffer(resp);
	}

	int DoDeserializeInInnerBuffer(IJST_OUT DeserializeResp &resp)
	{
		if (!m_pInnerBuffer->IsObject())
		{
			return Err::kDeserializeValueTypeError;
		}

		// For each member
		for (rapidjson::Value::MemberIterator itMember = m_pInnerBuffer->MemberBegin();
			 itMember != m_pInnerBuffer->MemberEnd(); ++itMember) {

			// TODO: Performance issue?
			const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
			IJSTI_MAP_TYPE<std::string, const MetaField *>::const_iterator itMetaField =
					m_metaClass->mapName.find(fieldName);
			if (itMetaField == m_metaClass->mapName.end()) {
				// Not a field in struct
				continue;
			}

			const MetaField *metaField = itMetaField->second;

			// Check nullable
			if (isBitSet(metaField->desc, FDesc::Nullable)
					&& itMember->value.IsNull())
			{
				m_fieldStatus[metaField->offset] = FStatus::kNull;
			}
			else
			{
				void *pField = GetFieldByOffset(metaField->offset);
				DeserializeReq elemReq(itMember->value, *m_pAllocator, pField);
				DeserializeResp elemResp(resp.needErrMsg);
				int ret = metaField->serializerInterface->Deserialize(elemReq, elemResp);
				if (ret != 0) {
					m_fieldStatus[metaField->offset] = FStatus::kParseFailed;
					resp.needErrMsg &&
						resp.CombineErrMsg("Deserialize field error. name: " + metaField->name + ", err: ", elemResp);
					return ret;
				}
				// TODO: Check member state
				m_fieldStatus[metaField->offset] = FStatus::kValid;
			}

			++resp.fieldCount;
		}

		// Check all required field status
		std::stringstream invalidNameOss;
		bool hasErr = false;
		for (std::vector<MetaField>::const_iterator itField = m_metaClass->metaFields.begin();
			 itField != m_metaClass->metaFields.end(); ++itField)
		{
			if (isBitSet(itField->desc, FDesc::Optional))
			{
				// Optional
				continue;
			}

			FStatus::_E fStatus = GetStatusByOffset(itField->offset);
			if (fStatus == FStatus::kValid
					|| fStatus == FStatus::kNull)
			{
				// Correct
				continue;
			}

			// Has error
			hasErr = true;
			if (resp.needErrMsg)
			{
				invalidNameOss << itField->name << ", ";
			}
		}
		if (hasErr)
		{
			resp.needErrMsg &&
				resp.SetErrMsg("Some fields are invalid: " + invalidNameOss.str());
			return Err::kDeserializeSomeFiledsInvalid;
		}
		return 0;
	}

	inline std::size_t GetFieldOffset(const void *const ptr) const
	{
		const unsigned char *filed_ptr = static_cast<const unsigned char *>(ptr);
		return filed_ptr - m_parentPtr;
	}

	inline void *GetFieldByOffset(std::size_t offset) const
	{
		return (void *) (m_parentPtr + offset);
	}

	/**
	 * Reset inner buffer
	 * NOTE: it will clear m_pDummyDoc. Make sure that the content of m_pDummyDoc is not used in other place
	 */
	inline void ResetInnerBuffer()
	{
		m_pDummyDoc->SetObject();	// Clear object
		if (m_pBufferFinder != IJSTI_NULL) {
			delete m_pBufferFinder;
			m_pBufferFinder = IJSTI_NULL;
		}
		m_useDummyDoc = true;
		BufferInfo bufferInfo(m_pDummyDoc, &m_pDummyDoc->GetAllocator(), false);
		m_pBufferFinder = new BufferFinder(bufferInfo);
	}

	inline void SetBufferFinder(const BufferFinder &bufferFinder)
	{
		*m_pBufferFinder = bufferFinder;
		m_useDummyDoc = false;
		// Do not clear m_pDummyDoc because it may be use in other place
	}

	FStatus::_E GetStatusByOffset(const size_t offset) const
	{
		IJSTI_MAP_TYPE<size_t, FStatus::_E>::const_iterator itera = m_fieldStatus.find(offset);
		if (itera != m_fieldStatus.end()) {
			return itera->second;
		}

		if (IJSTI_LIKELY(m_metaClass->mapOffset.find(offset) != m_metaClass->mapOffset.end())) {
			return FStatus::kMissing;
		}

		return FStatus::kNotAField;
	}

	inline bool isBitSet(unsigned int val, unsigned int bit)
	{
		return (val & bit) != 0;
	}

	template <class _T>
	friend class FSerializer;

	typedef IJSTI_MAP_TYPE<std::size_t, FStatus::_E> FieldStatusType;
	FieldStatusType m_fieldStatus;
	const MetaClass *m_metaClass;

	rapidjson::Document* m_pDummyDoc;		// Must be a pointer to make class Accessor be a standard-layout type struct
	BufferFinder* m_pBufferFinder;			// Use pointer to make it easier to init
	bool m_useDummyDoc;
	const unsigned char *m_parentPtr;
};

#define IJSTI_DEFINE_STRUCT_IMPL(N, ...) \
    IJSTI_PP_CONCAT(IJSTI_DEFINE_STRUCT_IMPL_, N)(__VA_ARGS__)

// Expands to the concatenation of its two arguments.
#define IJSTI_PP_CONCAT(x, y) IJSTI_PP_CONCAT_PRIMITIVE(x, y)
#define IJSTI_PP_CONCAT_PRIMITIVE(x, y) x ## y

#define IJSTI_IDL_FTYPE(fType, fName, sName, desc)		fType
#define IJSTI_IDL_FNAME(fType, fName, sName, desc)		fName
#define IJSTI_IDL_SNAME(fType, fName, sName, desc)		sName
#define IJSTI_IDL_DESC(fType, fName, sName, desc)		desc

#define IJSTI_DEFINE_FIELD(fDef) 							\
        ::ijst::detail::FSerializer<IJSTI_IDL_FTYPE fDef>::VarType IJSTI_IDL_FNAME fDef

#define IJSTI_METAINFO_DEFINE_START(stName, N)												\
	private:																				\
		typedef ::ijst::detail::MetaInfo<stName> MetaInfoT;									\
		typedef ::ijst::detail::Singleton<MetaInfoT> MetaInfoS;								\
		friend MetaInfoT;																	\
		static void InitMetaInfo(MetaInfoT* metaInfo)										\
		{																					\
			MetaInfoS::InitInstanceBeforeMain();											\
			metaInfo->metaClass.tag = #stName;												\
			metaInfo->metaClass.accessorOffset = offsetof(stName, _);						\
			metaInfo->metaClass.metaFields.reserve(N);

#define IJSTI_METAINFO_ADD(stName, fDef)  					\
		metaInfo->metaClass.PushMetaField(					\
			IJSTI_IDL_SNAME fDef, 							\
			offsetof(stName, IJSTI_IDL_FNAME fDef),			\
			IJSTI_IDL_DESC fDef, 							\
			IJSTI_FSERIALIZER_INS(IJSTI_IDL_FTYPE fDef)		\
		)

#define IJSTI_METAINFO_DEFINE_END()															\
			metaInfo->metaClass.InitMap();													\
		}

}	// namespace detail

}	// namespace ijst

#include "ijst_repeat_def.inc"

#endif //_IJST_HPP_INCLUDE_
