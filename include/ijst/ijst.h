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

#define IJST_TPRI(_T)	::ijst::detail::TypeClassPrim< ::ijst::FType::_T>
#define IJST_TVEC(_T)	::ijst::detail::TypeClassVec< _T>
#define IJST_TMAP(_T)	::ijst::detail::TypeClassMap< _T>
#define IJST_TOBJ(_T)	::ijst::detail::TypeClassObj< _T>
#define IJST_SET(obj, field, val)				obj._.Set(obj.field, val)
#define IJST_SET_STRICT(obj, field, val)		obj._.SetStrict(obj.field, val)
#define IJST_MAKE_VALID(obj, field)				obj._.MakeValid(obj.field)

#define IJST_DEFINE_STRUCT(...) \
    IJSTI_DEFINE_STRUCT_IMPL(IJSTI_PP_NFIELD(__VA_ARGS__), __VA_ARGS__)

#define IJSTI_STORE_MOVE(dest, src)			(dest) = (src)						// RapidJson's assigment behaviour is move
#define IJSTI_STORE_APPEND(list, elem, allocator)		(list).PushBack(elem, allocator)


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

typedef rapidjson::GenericDocument::AllocatorType 	AllocatorType;

#define IJSTI_MAP_TYPE    		std::map

#if __cplusplus >= 201103
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
		// When try to access initInstanceTag, the GetInstance() function will called before main
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
		StoreType buffer;
		AllocatorType* allocator;

		SerializeReq(): buffer(rapidjson::kNullType), allocator(IJSTI_NULL) {}
	};

	struct SerializeResp {
	};

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp)= 0;

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
	Serialize(const SerializeReq &req, SerializeResp &resp) = 0;

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

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp)
	{
		const _T *ptr = (const _T *) req.field;
		int ret = ptr->_.DoSerialize(req.pushAllField, req.tryInPlace, req.buffer);
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
			SerializeReq elemSerializeReq;
			elemSerializeReq.field = &(*itera);
			elemSerializeReq.pushAllField = req.pushAllField;
			elemSerializeReq.tryInPlace = req.tryInPlace;
			elemSerializeReq.allocator = req.allocator;
			SerializeResp elemSerializeResp;
			int ret = interface->Serialize(elemSerializeReq, elemSerializeResp);
			if (ret != 0) {
				return ret;
			}
			req.buffer.PushBack(elemSerializeReq.buffer, *req.allocator);
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

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp)
	{
		const VarType *ptr = static_cast<const VarType *>(req.field);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);
		if (!req.buffer.IsObject()) {
			req.buffer.SetObject();
		}
		for (typename VarType::const_iterator itera = ptr->begin(); itera != ptr->end(); ++itera) {
			SerializeReq elemSerializeReq;

			rapidjson::Value::ConstMemberIterator itMember = req.buffer.FindMember(itera->first.c_str());
			if (itMember != req.buffer.MemberEnd()) {
				elemSerializeReq.buffer = itMember->value;
			}
			elemSerializeReq.field = &(*itera);
			elemSerializeReq.pushAllField = req.pushAllField;
			elemSerializeReq.tryInPlace = req.tryInPlace;
			elemSerializeReq.allocator = req.allocator;

			SerializeResp elemSerializeResp;
			int ret = interface->Serialize(elemSerializeReq, elemSerializeResp);
			if (ret != 0) {
				return ret;
			}
			// no need to check same ptr
			req.buffer.AddMember(rapidjson::Value(itera->first.c_str(), *(req.allocator)).Move(),
								 elemSerializeReq.buffer, *(req.allocator));
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

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp)
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

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp)
	{
		const VarType *filedV = static_cast<const VarType *>(req.field);
		req.buffer.SetString(filedV->c_str(), filedV->length(), *(req.allocator));
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
	Accessor(const MetaClass *_metaClass, const void *_parentPtr) :
			metaClass(_metaClass),
			parentPtr(static_cast<const unsigned char *>(_parentPtr))
	{
		dummyDocument.SetObject();
		docAllocator = &dummyDocument.GetAllocator();
		ptrInnerStream = &dummyDocument;
		needReleaseStream = false;
	}

	~Accessor()
	{
		freeInnerStream();
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
		std::string name = (metaClass->mapOffset).find(offset)->second->name;
		std::cout << "offset: " << offset << ", name: " << name << std::endl;
		if (metaClass->mapOffset.find(offset) == metaClass->mapOffset.end()) {
			throw std::runtime_error("could not find field with expected offset: " + offset);
		}

		fieldStatus[offset] = FStatus::Valid;
	}

	template<typename _T>
	inline FStatus::_E GetStatus(const _T &field) const
	{
		const size_t offset = GetFieldOffset(&field);
		return GetStatusByOffset(offset);
	}

	inline StoreType &InnerStream( )
	{
		return *ptrInnerStream;
	}

	inline const StoreType &InnerStream( ) const
	{
		return *ptrInnerStream;
	}

	int SerializeInplace(bool pushAllField)
	{
		StoreType dummyBuffer;
		StoreType* pDummyOutput;
		int ret = DoSerialize(pushAllField, /*tryInPlace=*/true, dummyBuffer, pDummyOutput);
		assert(ret != 0 || pDummyOutput == ptrInnerStream);
		return ret;
	}

	int Serialize(StoreType &buffer, bool pushAllField) const
	{
		StoreType *pOutput = IJSTI_NULL;
		int ret = DoSerialize(pushAllField, /*tryInPlace=*/false, buffer, pOutput);
		if (ret != 0) {
			return ret;
		}
		assert(&buffer == pOutput);
	}

	void ShowTag() const
	{
		std::cout << "tag:" << metaClass->tag << std::endl;
		const size_t count = metaClass->metaFields.size();
		for (size_t i = 0; i < count; ++i) {
			std::cout << "name:" << metaClass->metaFields[i].name << std::endl
					  << "offset:" << metaClass->metaFields[i].offset << std::endl;
		}
	}

//	template <class _T>
//	friend class FSerializer<TypeClassObj<_T> >;

public:
	// TODO make it private
	int DoSerialize(bool pushAllField, bool tryInPlace, StoreType& buffer, AllocatorType* pAllocator) const
	{
		if (tryInPlace) {
			// copy inner data to buffer
		}

		// Loop each field
		for (std::vector<MetaField>::const_iterator itMetaField = metaClass->metaFields.begin();
			 itMetaField != metaClass->metaFields.end(); ++itMetaField) {

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

			// Push field
			const void *field = GetFieldByOffset(itMetaField->offset);
			rapidjson::Document doc;
			doc.GetAllocator()
			StoreType buffer;
			StoreType *pFieldStream;
			int ret = itMetaField->serializerInterface->Serialize(SerializerInterface::SerializeReq(), <#initializer#>);
			if (ret != 0) {
				return ret;
			}
			(*pOutput)[itMetaField->name].swap(*pFieldStream);
		}

		return 0;
	}
	/*
	 * Deserialize by stream
	 */
	int Deserialize(StoreType &stream)
	{
		freeInnerStream();
		ptrInnerStream = &stream;
		return 0;
	}

	inline std::size_t GetFieldOffset(const void *const ptr) const
	{
		const unsigned char *filed_ptr = static_cast<const unsigned char *>(ptr);
		return filed_ptr - parentPtr;
	}

	inline void *GetFieldByOffset(std::size_t offset) const
	{
		return (void *) (parentPtr + offset);
	}

	inline void freeInnerStream()
	{
		dummyDocument.SetObject();
		if (needReleaseStream) {
			delete ptrInnerStream;
			needReleaseStream = false;
			docAllocator = &dummyDocument.GetAllocator();
			ptrInnerStream = &dummyDocument;
		}
	}

	FStatus::_E GetStatusByOffset(const size_t offset) const
	{
		IJSTI_MAP_TYPE<size_t, FStatus::_E>::const_iterator itera = fieldStatus.find(offset);
		if (itera != fieldStatus.end()) {
			return itera->second;
		}

		if (metaClass->mapOffset.find(offset) != metaClass->mapOffset.end()) {
			return FStatus::Null;
		}

		return FStatus::NotAField;
	}

	typedef IJSTI_MAP_TYPE<std::size_t, FStatus::_E> FieldStatusType;
	FieldStatusType fieldStatus;
	const MetaClass *metaClass;

	rapidjson::Document dummyDocument;
	AllocatorType *docAllocator;
	StoreType *ptrInnerStream;
	bool needReleaseStream;
	const unsigned char *parentPtr;
};

}	// namespace detail

}	// namespace ijst

#include "ijst.inc"

#endif //_IJST_HPP_INCLUDE_
