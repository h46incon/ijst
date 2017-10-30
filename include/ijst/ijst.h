//
// Created by h46incon on 2017/9/19.
//

#ifndef _IJST_HPP_INCLUDE_
#define _IJST_HPP_INCLUDE_

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <cassert>
#include <sstream>

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
#define IJST_SET(obj, field, val)				obj._.Set((obj).field, (val))
#define IJST_SET_STRICT(obj, field, val)		obj._.SetStrict((obj).field, (val))

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
	};
};

struct Err {
	static const int kSucc 							= 0x00000000;
	static const int kDeserializeValueTypeError 	= 0x00001001;
	static const int kDeserializeSomeFiledsInvalid 	= 0x00001002;
	static const int kParseFaild 					= 0x00001003;
	static const int kInnerError 					= 0x00002001;
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
            /*rapidjson's assigment behaviour is move */ 	\
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


class SerializerInterface {
public:
	struct SerializeReq {
		// Pointer of field to serialize.
		// The actual type of field should be decide in the derived class
		const void* pField;

		// true if need serialize all field, false if serialize only valid field
		bool pushAllField;

		// Output buffer info. The instance should serialize in this object and use allocator
		IJST_OUT StoreType& buffer;
		AllocatorType& allocator;

		SerializeReq(StoreType &_buffer, AllocatorType &_allocator,
					 const void *_pField, bool _pushAllfield)
				: buffer(_buffer)
				, allocator(_allocator)
				, pField(_pField)
				, pushAllField(_pushAllfield)
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

		DeserializeReq(StoreType &_stream, AllocatorType &_allocator, void *_pField)
				: stream(_stream)
				, allocator(_allocator)
				, pFieldBuffer(_pField)
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
		int ret = ptr->_.ISerialize(req, resp);
		return ret;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp)
	{
		_T *ptr = (_T *) req.pFieldBuffer;
		return ptr->_.IDeserialize(req, resp);
	}

	virtual int Reset(void* pField)
	{
		_T *ptr = (_T *) pField;
		return ptr->_.IReset();
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
		req.buffer.Reserve(static_cast<rapidjson::SizeType>(ptr->size()), req.allocator);
		for (typename VarType::const_iterator itera = ptr->begin(); itera != ptr->end(); ++itera) {
			req.buffer.PushBack(
					rapidjson::Value(rapidjson::kNullType).Move(),
					req.allocator
			);
			StoreType &newElem = req.buffer[req.buffer.Size() - 1];

			SerializeReq elemReq(
					newElem, req.allocator, &(*itera), req.pushAllField);

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
			DeserializeReq elemReq(*itVal, req.allocator, &pBufferT->back());

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

			SerializeReq elemReq(
					*pNewElem, req.allocator, pFieldValue, req.pushAllField);

			SerializeResp elemResp;
			int ret = interface->Serialize(elemReq, elemResp);
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
			ElemVarType &elemBuffer = (*pBufferT)[fieldName];
			DeserializeReq elemReq(itMember->value, req.allocator, &elemBuffer);

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
	//region constructors
	Accessor(const MetaClass *_metaClass) :
			m_metaClass(_metaClass)
	{
		m_pBuffer = new rapidjson::Value(rapidjson::kObjectType);
		m_pOwnDoc = new rapidjson::Document();
		m_pAllocator = &m_pOwnDoc->GetAllocator();
		InitParentPtr();
	}

	Accessor(const Accessor &rhs) :
			m_fieldStatus(rhs.m_fieldStatus)
	{
		assert(this != &rhs);

		m_pBuffer = new rapidjson::Value(rapidjson::kObjectType);
		m_pOwnDoc = new rapidjson::Document();
		m_pAllocator = &m_pOwnDoc->GetAllocator();

		m_metaClass = rhs.m_metaClass;
		InitParentPtr();

		m_pBuffer->CopyFrom(*rhs.m_pBuffer, *m_pAllocator);
	}

#if __cplusplus >= 201103L
	Accessor(Accessor &&rhs)
	{
		m_pBuffer = IJSTI_NULL;
		m_pOwnDoc = IJSTI_NULL;
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
		delete m_pBuffer;
		m_pBuffer = IJSTI_NULL;
		delete m_pOwnDoc;
		m_pOwnDoc = IJSTI_NULL;
	}
	//endregion

	void Steal(Accessor &rhs)
	{
		if (IJSTI_UNLIKELY(this == &rhs)) {
			return;
		}

		// Handler resource
		delete m_pBuffer;
		m_pBuffer = rhs.m_pBuffer;
		rhs.m_pBuffer = IJSTI_NULL;

		delete m_pOwnDoc;
		m_pOwnDoc = rhs.m_pOwnDoc;
		rhs.m_pOwnDoc = IJSTI_NULL;

		// other simple field
		m_metaClass = rhs.m_metaClass;
		rhs.m_metaClass = IJSTI_NULL;
		m_pAllocator = rhs.m_pAllocator;
		rhs.m_pAllocator = IJSTI_NULL;

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

		m_pBuffer->SetObject();
		// TODO: need clear own allocator
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

	inline FStatus::_E GetStatus(const void *fieldptr) const
	{
		const size_t offset = GetFieldOffset(fieldptr);
		return GetStatusByOffset(offset);
	}

	//! Get inner buffer
	inline StoreType &InnerBuffer() { return *m_pBuffer; }
	inline const StoreType &InnerBuffer() const { return *m_pBuffer; }

	//! Get allocator used in object
	//! The inner allocator is own allocator when init, but may change to other allocator
	//! when calling SetParent() or Deserialize()
	inline AllocatorType &InnerAllocator() { return *m_pAllocator; }
	inline const AllocatorType &InnerAllocator( ) const { return *m_pAllocator; }

	//! Get own allocator that manager resource
	inline AllocatorType &OwnAllocator() { return m_pOwnDoc->GetAllocator(); }
	inline const AllocatorType &OwnAllocator( ) const { return m_pOwnDoc->GetAllocator(); }

	/**
	 * Serialize the structure.
	 * @param pushAllField 		true if push all field, false if push only valid or null field
	 * @param output 			the output of result
	 * @param allocator	 		allocator. If using inner allocator, user should carefully handler the structure's life cycle
	 * @return
	 */
	inline int Serialize(bool pushAllField, IJST_OUT StoreType& output, AllocatorType& allocator)
	{
		return DoSerialize(pushAllField, output, allocator);
	}

	/**
	 * Serialize the structure with inner allocator.
	 * User should make sure the structure's life cycle is longer than output
	 * @param pushAllField 		true if push all field, false if push only valid or null field
	 * @param output 			the output of result
	 * @return
	 */
	inline int SerializeInInnerAlloc(bool pushAllField, IJST_OUT StoreType &output)
	{
		return DoSerialize(pushAllField, output, *m_pAllocator);
	}

	/**
	 * Deserialize
	 * NOTE: Make sure srcDocStolen use own allocator
	 * @param errMsg. Output of error message, null if cancel error output
	 */
	inline int DeserializeMoved(rapidjson::Document &srcDocStolen, IJST_INOUT std::string *errMsg)
	{
		m_pOwnDoc->Swap(srcDocStolen);
		*m_pBuffer = *reinterpret_cast<StoreType*>(m_pOwnDoc);
		m_pAllocator = &m_pOwnDoc->GetAllocator();
		return DoDeserializeWrap(errMsg);
	}

	inline int Deserialize(const rapidjson::Document& srcDoc, IJST_INOUT std::string* errMsg)
	{
		m_pBuffer->CopyFrom(srcDoc, *m_pAllocator);
		return DoDeserializeWrap(errMsg);
	}

	inline int Deserialize(const std::string& input, IJST_INOUT std::string* errMsg)
	{
		return Deserialize(input.c_str(), input.length(), errMsg);
	}

	int Deserialize(const char* str, std::size_t length, IJST_INOUT std::string* errMsg)
	{
		// The new object will call Deserialize() interfaces in most suitation
		// So clear own allocator will not bring much benefice
		rapidjson::Document doc(m_pAllocator);
		doc.Parse(str, length);
		if (IJSTI_UNLIKELY(doc.HasParseError()))
		{
			if (errMsg != IJSTI_NULL)
			{
				*errMsg = IJSTI_MOVE(std::string(
						rapidjson::GetParseError_En(doc.GetParseError())
				));
			}
			return Err::kParseFaild;
		}
		*m_pBuffer = reinterpret_cast<StoreType&>(doc.Move());
		return DoDeserializeWrap(errMsg);
	}

	int DeserializeInsitu(char* str, IJST_INOUT std::string* errMsg)
	{
		// The new object will call Deserialize() interfaces in most suitation
		// So clear own allocator will not bring much benefice
		rapidjson::Document doc(m_pAllocator);
		doc.ParseInsitu(str);
		if (IJSTI_UNLIKELY(doc.HasParseError()))
		{
			if (errMsg != IJSTI_NULL)
			{
				*errMsg = IJSTI_MOVE(std::string(
						rapidjson::GetParseError_En(doc.GetParseError())
				));
			}
			return Err::kParseFaild;
		}
		*m_pBuffer = reinterpret_cast<StoreType&>(doc.Move());
		return DoDeserializeWrap(errMsg);
	}

	bool WriteInnerBuffer(IJST_OUT std::string &strOutput)
	{
		rapidjson::StringBuffer buffer;
		buffer.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		bool bSucc = m_pBuffer->Accept(writer);
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

	//region Implement SerializeInterface
	inline int ISerialize(const SerializeReq &req, SerializeResp &resp)
	{
		assert(req.pField == this);
		return DoSerialize(req.pushAllField, req.buffer, req.allocator);
	}

	inline int IDeserialize(const DeserializeReq &req, IJST_OUT DeserializeResp& resp)
	{
		assert(req.pFieldBuffer == this);

		IJSTI_STORE_MOVE(*m_pBuffer, req.stream);
		m_pAllocator = &req.allocator;

		return DoDeserialize(resp);
	}

	inline int IReset()
	{
		return Reset();
	}
	//endregion

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

	int DoSerialize(bool pushAllField, IJST_OUT StoreType& buffer, AllocatorType& allocator)
	{
		// Serialize fields to buffer
		buffer.SetObject();

		// Reserve space
#if false
		do {
			const size_t maxSize = m_metaClass->metaFields.size() + m_pBufferDoc->MemberCount();
			if (buffer.MemberCapacity() >= maxSize) {
				break;
			}

			size_t fieldSize;
			if (pushAllField) {
				fieldSize = maxSize;
			}
			else {
				fieldSize = m_pBufferDoc->MemberCount();
				for (std::vector<MetaField>::const_iterator itMetaField = m_metaClass->metaFields.begin();
					 itMetaField != m_metaClass->metaFields.end(); ++itMetaField)
				{
					FStatus::_E fstatus = GetStatusByOffset(itMetaField->offset);
					if (fstatus == FStatus::kValid || fstatus == FStatus::kNull) {
						++fieldSize;
					}
				}

			}
			buffer.MemberReserve(fieldSize, allocator);
		} while (false);
#endif

		std::size_t fieldSize = m_metaClass->metaFields.size();
		for (std::vector<MetaField>::const_iterator itMetaField = m_metaClass->metaFields.begin();
			 itMetaField != m_metaClass->metaFields.end(); ++itMetaField)
		{
			// Check field state
			FStatus::_E fstatus = GetStatusByOffset(itMetaField->offset);
			switch (fstatus) {
				case FStatus::kValid:
				{
					int ret = DoSerializeField(itMetaField, pushAllField, buffer, allocator);
					if (ret != 0) {
						return ret;
					}
				}
					break;

				case FStatus::kNull:
				{
					int ret = DoSerializeNullField(itMetaField, buffer, allocator);
					if (ret != 0) {
						return ret;
					}
				}
					break;

				case FStatus::kMissing:
				case FStatus::kParseFailed:
				{
					if (!pushAllField) {
						continue;
					}
					int ret = DoSerializeField(itMetaField, pushAllField, buffer, allocator);
					if (ret != 0) {
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

		// append inner data to buffer
		if (&allocator == m_pAllocator) {
			// Move
			for (rapidjson::Value::MemberIterator itMember = m_pBuffer->MemberBegin();
				 itMember != m_pBuffer->MemberEnd(); ++itMember)
			{
				buffer.AddMember(itMember->name, itMember->value, allocator);
				// both itMember->name, itMember->value are null now
			}
		}
		else {
			// Copy
			for (rapidjson::Value::ConstMemberIterator itMember = m_pBuffer->MemberBegin();
				 itMember != m_pBuffer->MemberEnd(); ++itMember)
			{
				rapidjson::Value name;
				rapidjson::Value val;
				name.CopyFrom(itMember->name, allocator);
				val.CopyFrom(itMember->value, allocator);
				buffer.AddMember(name, val, allocator);
			}
			// The object will be release after serialize in most suitation, so do not need clear own allocator
		}
		m_pBuffer->SetObject();

		return 0;
	}

	int DoSerializeField(std::vector<MetaField>::const_iterator itMetaField, bool pushAllField,
						 StoreType &buffer, AllocatorType &allocator) const
	{
		// Init
		const void *pFieldValue = GetFieldByOffset(itMetaField->offset);
		rapidjson::GenericStringRef<char> fieldNameRef =
				rapidjson::StringRef(itMetaField->name.c_str(), itMetaField->name.length());
		rapidjson::Value fieldNameVal;
		fieldNameVal.SetString(fieldNameRef);

		// Serialize field
		StoreType elemOutput;
		SerializeReq elemSerializeReq(
				elemOutput, allocator, pFieldValue, pushAllField);

		SerializeResp elemSerializeResp;
		int ret = itMetaField->serializerInterface->Serialize(elemSerializeReq, elemSerializeResp);
		if (IJSTI_UNLIKELY(ret != 0)) {
			return ret;
		}

		// Add member, field name is not need deep copy
		// Do not check existing key, that's a feature
		buffer.AddMember(
				rapidjson::Value().SetString(fieldNameRef),
				elemOutput,
				allocator
		);
		return 0;
	}

	int DoSerializeNullField(std::vector<MetaField>::const_iterator itMetaField,
							 StoreType &buffer, AllocatorType &allocator)
	{
		// Init
		const void *pFieldValue = GetFieldByOffset(itMetaField->offset);
		rapidjson::GenericStringRef<char> fieldNameRef =
				rapidjson::StringRef(itMetaField->name.c_str(), itMetaField->name.length());
		rapidjson::Value fieldNameVal;
		fieldNameVal.SetString(fieldNameRef);

		// Add member, field name is not need deep copy
		// Do not check if there is a existing key, that's a feature
		buffer.AddMember(
				rapidjson::Value().SetString(fieldNameRef),
				rapidjson::Value().SetNull(),
				allocator
		);
		return 0;
	}

	//! Deserialize in inner buffer
	int DoDeserialize(IJST_OUT DeserializeResp &resp)
	{
		if (!m_pBuffer->IsObject())
		{
			return Err::kDeserializeValueTypeError;
		}

		resp.fieldCount = 0;
		// For each member
		rapidjson::Value::MemberIterator itNextRemain = m_pBuffer->MemberBegin();
		for (rapidjson::Value::MemberIterator itMember = m_pBuffer->MemberBegin();
			 itMember != m_pBuffer->MemberEnd(); ++itMember)
		{

			// Get related field info
			// TODO: Performance issue of copy string?
			const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
			IJSTI_MAP_TYPE<std::string, const MetaField *>::const_iterator itMetaField =
					m_metaClass->mapName.find(fieldName);

			if (itMetaField == m_metaClass->mapName.end()) {
				// TODO: This is relay on the implementation details of rapidjson's storage, how to check?
				// Not a field in struct
				if (itNextRemain != itMember) {
					IJSTI_STORE_MOVE(*itNextRemain, *itMember);
				}
				++itNextRemain;
				continue;
			}

			// Move member out of object
			StoreType memberStream(rapidjson::kNullType);
			memberStream.Swap(itMember->value);

			const MetaField *metaField = itMetaField->second;
			// Check nullable
			if (isBitSet(metaField->desc, FDesc::Nullable)
					&& memberStream.IsNull())
			{
				m_fieldStatus[metaField->offset] = FStatus::kNull;
			}
			else
			{
				void *pField = GetFieldByOffset(metaField->offset);
				DeserializeReq elemReq(memberStream, *m_pAllocator, pField);
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

		// Clean
		if (m_pBuffer->MemberCount() != 0)
		{
			m_pBuffer->EraseMember(itNextRemain, m_pBuffer->MemberEnd());
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

	inline int DoDeserializeWrap(IJST_INOUT std::string *errMsg)
	{
		bool needErrMessage = (errMsg != IJSTI_NULL);
		DeserializeResp resp(needErrMessage);
		int ret = DoDeserialize(resp);
		if (needErrMessage) {
			*errMsg = IJSTI_MOVE(resp.errMsg);
		}
		return ret;
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

	// Must be a pointer to make class Accessor be a standard-layout type struct
	rapidjson::Value* m_pBuffer;
	// Should use document instead of Allocator because document can swap allocator
	rapidjson::Document* m_pOwnDoc;

	AllocatorType* m_pAllocator;
	const unsigned char *m_parentPtr;
	//</editor-fold>
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