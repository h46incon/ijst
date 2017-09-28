//
// Created by h46incon on 2017/9/19.
//

#ifndef _IJST_HPP_INCLUDE_
#define _IJST_HPP_INCLUDE_

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <map>
#include <assert.h>
#include "rapidjson/document.h"

/**	========================================================================================
 *				Public Interface
 */
typedef rapidjson::Value StoreType;

#define IJST_OUT

#define IJST_TPRI(_T)	::ijst::detail::TypeClassPrim< ::ijst::FType::_T>
#define IJST_TVEC(_T)	::ijst::detail::TypeClassVec< _T>
#define IJST_TMAP(_T)	::ijst::detail::TypeClassMap< _T>
#define IJST_TOBJ(_T)	::ijst::detail::TypeClassObj< _T>
#define IJST_SET(obj, field, val)				obj._.Set(obj.field, val)
#define IJST_SET_STRICT(obj, field, val)		obj._.SetStrict(obj.field, val)
#define IJST_MAKE_VALID(obj, field)				obj._.MakeValid(obj.field)

#define IJST_DEFINE_STRUCT(...) \
    IJSTI_DEFINE_STRUCT_IMPL(IJSTI_PP_NFIELD(__VA_ARGS__), __VA_ARGS__)



namespace ijst {

struct FType {
public:
	enum _E {
		Raw,
		Int,
		String,
	};
};

struct FDesc {
	static const unsigned int _MaskDesc 	= 0x000000FF;
	static const unsigned int Optional 		= 0x00000001;
};

struct FStatus {
public:
	enum _E {
		NotAField,
		Null,
		ParseFailed,
		Valid,
		Removed,
	};
};


/**	========================================================================================
 *				Inner Interface
 */
namespace detail {

//typedef rapidjson::GenericDocument::AllocatorType 	AllocatorType;
typedef rapidjson::MemoryPoolAllocator<> 	AllocatorType;

// LIKELY and UNLIKELY
#if defined(__GNUC__) || defined(__clang__)
	#define IJSTI_LIKELY(x)			__builtin_expect(!!(x), 1)
	#define IJSTI_UNLIKELY(x)		__builtin_expect(!!(x), 0)
#else
	#define IJSTI_LIKELY(x) 		(x)
	#define IJSTI_UNLIKELY(x)		(x)
#endif

#define IJSTI_MAP_TYPE    			std::map
#define IJSTI_STORE_MOVE(dest, src)			\
	do {                                    \
        if (&dest != &src) {                \
            /*RapidJson's assigment behaviour is move */ \
            (dest) = (src);                 \
        }                                   \
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
		const void* field;
		bool pushAllField;
		bool tryInPlace;
		StoreType& buffer;
		AllocatorType& allocator;

		SerializeReq(StoreType &_buffer, AllocatorType &_allocator,
					 const void *_field, bool _pushAllfield, bool _tryInPlace) :
				buffer(_buffer),
				allocator(_allocator),
				field(_field),
				pushAllField(_pushAllfield),
				tryInPlace(_tryInPlace)
		{

		}
	};

	struct SerializeResp {
	};

	virtual int Serialize(SerializeReq &req, SerializeResp &resp)= 0;

	virtual FStatus::_E Deserialize(const StoreType &srcStream, void *pBuffer)= 0;

	virtual ~SerializerInterface()
	{
	}
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
	Serialize(SerializeReq &req, SerializeResp &resp) = 0;

	virtual FStatus::_E Deserialize(const StoreType &srcStream, void *pBuffer) = 0;
};
#define IJSTI_FSERIALIZER_INS(_T) ::ijst::detail::Singleton< ::ijst::detail::FSerializer< _T> >::GetInstance()

/**
 * Serialization class of Object types
 * @tparam _T class
 */
template<class _T>
class FSerializer<TypeClassObj<_T> > : public SerializerInterface {
public:
	typedef _T VarType;

	virtual int Serialize(SerializeReq &req, SerializeResp &resp)
	{
		_T *ptr = (_T *) req.field;
		int ret = ptr->_.DoSerialize(req.pushAllField, req.tryInPlace, req.buffer, req.allocator);
		return ret;
	}

	virtual FStatus::_E Deserialize(const StoreType &srcStream, void *pBuffer)
	{

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

	virtual int Serialize(SerializeReq &req, SerializeResp &resp)
	{
		const VarType *ptr = static_cast<const VarType *>(req.field);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);
		req.buffer.SetArray();
		for (typename VarType::const_iterator itera = ptr->begin(); itera != ptr->end(); ++itera) {
			req.buffer.PushBack(
					rapidjson::Value(rapidjson::kNullType).Move(),
					req.allocator
			);
			StoreType &newElem = req.buffer[req.buffer.Size() - 1];

			SerializeReq elemSerializeReq(
					newElem, req.allocator, &(*itera), req.pushAllField, req.tryInPlace);

			SerializeResp elemSerializeResp;
			int ret = interface->Serialize(elemSerializeReq, elemSerializeResp);
			if (ret != 0) {
				req.buffer.PopBack();
				return ret;
			}
		}
		return 0;
	}

	virtual FStatus::_E Deserialize(const StoreType &srcStream, void *pBuffer)
	{
		VarType *pBufferT = static_cast<VarType *>(pBuffer);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);
		pBufferT->clear();

		if (!srcStream.IsArray()) {
			return FStatus::ParseFailed;
		}
		return FStatus::Valid;
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

	virtual int Serialize(SerializeReq &req, SerializeResp &resp)
	{
		const VarType *ptr = static_cast<const VarType *>(req.field);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);
		if (!req.buffer.IsObject()) {
			req.buffer.SetObject();
		}

		for (typename VarType::const_iterator itFieldMember = ptr->begin(); itFieldMember != ptr->end(); ++itFieldMember) {

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

			SerializeReq elemSerializeReq(
					*pNewElem, req.allocator, pFieldValue, req.pushAllField, req.tryInPlace);

			SerializeResp elemSerializeResp;
			int ret = interface->Serialize(elemSerializeReq, elemSerializeResp);
			if (ret != 0) {
				if (hasAllocMember) {
					req.buffer.RemoveMember(fieldNameVal);
				}
				return ret;
			}
			assert(&(req.buffer[fieldNameVal]) == &elemSerializeReq.buffer);
		}
		return 0;
	}

	virtual FStatus::_E Deserialize(const StoreType &srcStream, void *pBuffer)
	{
		// TODO: return 0
	}
};

/**
 *				Serialization implementation of Primitive types
 */

template<>
class FSerializer<TypeClassPrim<FType::Int> > : public SerializerInterface {
public:
	typedef int VarType;

	virtual int Serialize(SerializeReq &req, SerializeResp &resp)
	{
		const VarType *fieldI = static_cast<const VarType *>(req.field);
		req.buffer.SetInt(*fieldI);
		return 0;
	}

	virtual FStatus::_E Deserialize(const StoreType &srcStream, void *pBuffer)
	{
		return FStatus::Valid;
	}

};

template<>
class FSerializer<TypeClassPrim<FType::String> > : public SerializerInterface {
public:
	typedef std::string VarType;

	virtual int Serialize(SerializeReq &req, SerializeResp &resp)
	{
		const VarType *filedV = static_cast<const VarType *>(req.field);
		req.buffer.SetString(filedV->c_str(), filedV->length(), req.allocator);
		return 0;
	}

	virtual FStatus::_E Deserialize(const StoreType &srcStream, void *pBuffer)
	{
		return FStatus::Valid;
	}
};

/**	========================================================================================
 *				Private
 */

struct MetaField { // NOLINT
	std::string name;
	std::size_t offset;
	unsigned int desc;
	SerializerInterface *serializerInterface;
};

class MetaClass {
public:
	MetaClass() : mapInited(false) { }

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
		if (mapInited) {
			throw std::runtime_error("MetaClass's map has inited before");
		}

		for (size_t i = 0; i < metaFields.size(); ++i) {
			const MetaField *ptrMetaField = &(metaFields[i]);
			// Check key exist
			if (mapName.find(ptrMetaField->name) != mapName.end()) {
				throw std::runtime_error("MetaClass's name conflict:" + ptrMetaField->name);
			}
			if (mapOffset.find(ptrMetaField->offset) != mapOffset.end()) {
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
	Accessor(const MetaClass *_metaClass) :
			m_metaClass(_metaClass)
	{
		InitParentPtr();
		m_pDummyDoc = new rapidjson::Document(rapidjson::kObjectType);
		resetInnerStream();
	}

	~Accessor()
	{
		resetInnerStream();
		if (m_pDummyDoc != IJSTI_NULL) {
			delete m_pDummyDoc;
		}
		m_pDummyDoc = IJSTI_NULL;
	}

	static void swap(Accessor &lhs, Accessor &rhs)
	{
		if (IJSTI_UNLIKELY(&lhs == &rhs)) {
			return;
		}
		// Swap pointer
		using std::swap;
		swap(lhs.m_metaClass, rhs.m_metaClass);
		swap(lhs.m_pDummyDoc, rhs.m_pDummyDoc);
		swap(lhs.m_useDummyDoc, rhs.m_useDummyDoc);
		swap(lhs.m_pAllocator, rhs.m_pAllocator);
		swap(lhs.m_pInnerStream, rhs.m_pInnerStream);
		swap(lhs.m_fieldStatus, rhs.m_fieldStatus);
		lhs.InitParentPtr();
		rhs.InitParentPtr();
	}

	Accessor(const Accessor &rhs) :
			m_metaClass(rhs.m_metaClass),
			m_fieldStatus(rhs.m_fieldStatus)
	{
		InitParentPtr();
		assert(this != &rhs);
		m_pDummyDoc = new rapidjson::Document(rapidjson::kNullType);
		m_pDummyDoc->CopyFrom(*rhs.m_pDummyDoc, m_pDummyDoc->GetAllocator());
		m_useDummyDoc = rhs.m_useDummyDoc;
		if (m_useDummyDoc) {
			m_pInnerStream = m_pDummyDoc;
			m_pAllocator = &m_pDummyDoc->GetAllocator();
		}
		else {
			m_pInnerStream = rhs.m_pInnerStream;
			m_pAllocator = rhs.m_pAllocator;
		}
	}

#if __cplusplus >= 201103L
	Accessor(Accessor &&rhs)
	{
		m_pDummyDoc = IJSTI_NULL;
		swap(*this, rhs);
	}
#endif

	Accessor &operator=(Accessor rhs)
	{
		swap(*this, rhs);
		return *this;
	}

	template<typename _T1, typename _T2>
	inline void Set(_T1 &field, const _T2 &value)
	{
		MakeValid(field);
		field = value;
	}

	template<typename _T>
	inline void SetStrict(_T &field, const _T &value)
	{
		Set(field, value);
	}

	template<typename _T>
	void MakeValid(_T &field)
	{
		const std::size_t offset = GetFieldOffset(&field);
		// TODO: remove it:
		std::string name = (m_metaClass->mapOffset).find(offset)->second->name;
		std::cout << "offset: " << offset << ", name: " << name << std::endl;
		if (m_metaClass->mapOffset.find(offset) == m_metaClass->mapOffset.end()) {
			throw std::runtime_error("could not find field with expected offset: " + offset);
		}

		m_fieldStatus[offset] = FStatus::Valid;
	}

	template<typename _T>
	inline FStatus::_E GetStatus(const _T &field) const
	{
		const size_t offset = GetFieldOffset(&field);
		return GetStatusByOffset(offset);
	}

	inline StoreType &InnerStream( )
	{
		return *m_pInnerStream;
	}

	inline const StoreType &InnerStream( ) const
	{
		return *m_pInnerStream;
	}

	int SerializeInplace(bool pushAllField)
	{
		int ret = DoSerialize(pushAllField, /*tryInPlace=*/true, *m_pInnerStream, *m_pAllocator);
		return ret;
	}

	bool Serialize(bool pushAllField, IJST_OUT rapidjson::Document& docOutput) const
	{
		Accessor* pAccessor = const_cast<Accessor *>(this);
		int ret = pAccessor->DoSerialize(pushAllField, /*tryInPlace=*/false, docOutput, docOutput.GetAllocator());
		if (ret != 0) {
			return ret;
		}
	}

	void ShowTag() const
	{
		std::cout << "tag:" << m_metaClass->tag << std::endl;
		const size_t count = m_metaClass->metaFields.size();
		for (size_t i = 0; i < count; ++i) {
			std::cout << "name:" << m_metaClass->metaFields[i].name << std::endl
					  << "offset:" << m_metaClass->metaFields[i].offset << std::endl;
		}
	}

//	template <class _T>
//	friend class FSerializer<TypeClassObj<_T> >;

public:
	inline void InitParentPtr()
	{
		m_parentPtr = reinterpret_cast<const unsigned char *>(this - m_metaClass->accessorOffset);
	}

	// TODO make it private
	int DoSerialize(bool pushAllField, bool tryInPlace, StoreType& buffer, AllocatorType& allocator)
	{
		if (tryInPlace) {
			// copy inner data to buffer
			if (&allocator == m_pAllocator) {
				IJSTI_STORE_MOVE(buffer, *m_pInnerStream);
			} else {
				buffer.CopyFrom(*m_pInnerStream, allocator);
			}
			resetInnerStream();
			setInnerStream(&buffer, &allocator);
		}

		if (!buffer.IsObject()) {
			buffer.SetObject();
		}
		// Loop each field
		for (std::vector<MetaField>::const_iterator itMetaField = m_metaClass->metaFields.begin();
			 itMetaField != m_metaClass->metaFields.end(); ++itMetaField) {

			std::cout << "Serialize field: " << itMetaField->name << std::endl;
			// Check field state
			FStatus::_E fstatus = GetStatusByOffset(itMetaField->offset);
			switch (fstatus) {
				case FStatus::NotAField:
					return -1;
				case FStatus::Valid:
					// continue processing
					break;
					// TODO: add remove
				default:
					if (!pushAllField) {
						continue;
					}
					break;
			}

			const void *pFieldValue = GetFieldByOffset(itMetaField->offset);
			rapidjson::GenericStringRef<char> fieldNameRef =
					rapidjson::StringRef(itMetaField->name.c_str(), itMetaField->name.length());
			rapidjson::Value fieldNameVal;
			fieldNameVal.SetString(fieldNameRef);

			// Try add field when need
			StoreType* pNewElem = IJSTI_NULL;
			bool hasAllocMember = false;
			{
				rapidjson::Value::MemberIterator itMember = buffer.FindMember(fieldNameVal);
				if (itMember == buffer.MemberEnd()) {
					// Add member, field name is not need copy
					buffer.AddMember(
							rapidjson::Value().SetString(fieldNameRef),
							rapidjson::Value(rapidjson::kNullType).Move(),
							allocator
					);
					// Why rapidjson AddMember function do not return newly create member
					pNewElem = &buffer[fieldNameVal];
					hasAllocMember = true;
				}
				else {
					pNewElem = &(itMember->value);
				}
			}

			// Serialize field
			SerializerInterface::SerializeReq elemSerializeReq(
					*pNewElem, allocator, pFieldValue, pushAllField, tryInPlace);

			SerializerInterface::SerializeResp elemSerializeResp;
			int ret = itMetaField->serializerInterface->Serialize(elemSerializeReq, elemSerializeResp);
			if (ret != 0) {
				if (hasAllocMember) {
					buffer.RemoveMember(fieldNameVal);
				}
				return ret;
			}
			assert(&(buffer[fieldNameVal]) == &elemSerializeReq.buffer);
		}

		return 0;
	}
	/*
	 * Deserialize by stream
	 */
	int Deserialize(StoreType &stream)
	{
		resetInnerStream();
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

	inline void resetInnerStream()
	{
		m_pDummyDoc->SetObject();	// Clear object
		m_pAllocator = &m_pDummyDoc->GetAllocator();
		m_pInnerStream = m_pDummyDoc;
		m_useDummyDoc = true;
	}

	inline void setInnerStream(StoreType* stream, AllocatorType* allocator)
	{
		m_pInnerStream = stream;
		m_pAllocator = allocator;
		m_useDummyDoc = false;
	}

	FStatus::_E GetStatusByOffset(const size_t offset) const
	{
		IJSTI_MAP_TYPE<size_t, FStatus::_E>::const_iterator itera = m_fieldStatus.find(offset);
		if (itera != m_fieldStatus.end()) {
			return itera->second;
		}

		if (m_metaClass->mapOffset.find(offset) != m_metaClass->mapOffset.end()) {
			return FStatus::Null;
		}

		return FStatus::NotAField;
	}

	typedef IJSTI_MAP_TYPE<std::size_t, FStatus::_E> FieldStatusType;
	FieldStatusType m_fieldStatus;
	const MetaClass *m_metaClass;

	rapidjson::Document* m_pDummyDoc;		// Must be a pointer to make class Accessor be a standard-layout type struct
	AllocatorType *m_pAllocator;
	StoreType *m_pInnerStream;
	bool m_useDummyDoc;
	const unsigned char *m_parentPtr;
};

}	// namespace detail

}	// namespace ijst

#include "ijst.inc"

#endif //_IJST_HPP_INCLUDE_
