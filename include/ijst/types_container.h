/**************************************************************************************************
 *		Serialization implementation of Container types
 *		Created by h46incon on 2017/12/26.
 **************************************************************************************************/

#ifndef _IJST_TYPES_CONTAINER_HPP_INCLUDE_
#define _IJST_TYPES_CONTAINER_HPP_INCLUDE_

#include "ijst.h"
#include <map>
#include <list>
#include <deque>

//! Declare a vector<_T> field.
#define IJST_TVEC(_T)	::std::vector< _T>
//! Declare a deque<_T> field.
#define IJST_TDEQUE(_T)	::std::deque< _T>
//! Declare a vector<_T> field.
#define IJST_TLIST(_T)	::std::list< _T>
//! Declare a map<string, _T> field.
#define IJST_TMAP(_T)	::ijst::TypeClassMap< _T>
//! Declare a object field which _T is a ijst struct type.
#define IJST_TOBJ(_T)	::ijst::TypeClassObj< _T>

namespace ijst {

template<class _T>
struct TypeClassMap {
	// nothing
};

template<class _T>
struct TypeClassObj {
	// nothing
};

template <typename _TElem>
class Optional <std::map<std::string, _TElem> > {
	typedef std::map<std::string, _TElem> ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	Optional<_TElem> operator[](const std::string& key) const
	{
		if (m_pVal == IJST_NULL) {
			return Optional<_TElem>(IJST_NULL);
		}
		typename std::map<std::string, _TElem>::iterator it = m_pVal->find(key);
		if (it == m_pVal->end()){
			return Optional<_TElem>(IJST_NULL);
		}
		else {
			return Optional<_TElem>(&it->second);
		}
	}
};

template <typename _TElem>
class Optional <const std::map<std::string, _TElem> >
{
	typedef const std::map<std::string, _TElem> ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	Optional<const _TElem> operator[](const std::string& key) const
	{
		if (m_pVal == IJST_NULL) {
			return Optional<const _TElem>(IJST_NULL);
		}
		typename std::map<std::string, _TElem>::const_iterator it = m_pVal->find(key);
		if (it == m_pVal->end()){
			return Optional<const _TElem>(IJST_NULL);
		}
		else {
			return Optional<const _TElem>(&it->second);
		}
	}
};

#define IJSTI_OPTIONAL_ARRAY_DEFINE(is_const, Container)													\
	template<typename _TElem>																				\
	class Optional<is_const Container<_TElem> >																\
	{ 																										\
		typedef is_const Container<_TElem> ValType;															\
		IJSTI_OPTIONAL_BASE_DEFINE(ValType)																	\
	public:																									\
		Optional<is_const _TElem> operator[](typename Container<_TElem>::size_type i) const					\
		{																									\
			if (m_pVal == IJST_NULL || m_pVal->size() <= i) {												\
				return Optional<is_const _TElem>(IJST_NULL);												\
			}																								\
			return Optional<is_const _TElem>(&(*m_pVal)[i]);												\
		}																									\
	};

IJSTI_OPTIONAL_ARRAY_DEFINE(, std::vector)
IJSTI_OPTIONAL_ARRAY_DEFINE(const, std::vector)
IJSTI_OPTIONAL_ARRAY_DEFINE(, std::deque)
IJSTI_OPTIONAL_ARRAY_DEFINE(const, std::deque)

}	// namespace ijst


namespace ijst {
namespace detail {
/**
 * Serialization class of Object types
 * @tparam _T class
 */
template<class _T>
class FSerializer<TypeClassObj<_T> > : public SerializerInterface {
public:
	typedef _T VarType;

	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		_T *pField = (_T *) req.pField;
		return pField->_.ISerialize(req);
	}

#if IJST_ENABLE_TO_JSON_OBJECT
	virtual int ToJson(const ToJsonReq &req) IJSTI_OVERRIDE
	{
		_T *pField = (_T *) req.pField;
		return pField->_.IToJson(req);
	}

	virtual int SetAllocator(void* pField, JsonAllocator& allocator) IJSTI_OVERRIDE
	{
		_T *pFieldT = (_T *) pField;
		return pFieldT->_.ISetAllocator(pField, allocator);
	}
#endif

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		_T *pField = (_T *) req.pFieldBuffer;
		return pField->_.IFromJson(req, resp);
	}

};

template<typename DefType, typename VarType>
class ContainerSerializer : public SerializerInterface {
public:
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != IJST_NULL);
		const VarType& field = *static_cast<const VarType*>(req.pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(DefType);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartArray());

		for (typename VarType::const_iterator itera = field.begin(); itera != field.end(); ++itera) {
			SerializeReq elemReq(req.writer, &(*itera), req.fPushMode);
			IJSTI_RET_WHEN_NOT_ZERO(interface->Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.EndArray());

		return 0;
	}

	virtual int FromJson(const FromJsonReq &req, FromJsonResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsArray()) {
			resp.errDoc.ElementTypeMismatch("array", req.stream);
			return Err::kDeserializeValueTypeError;
		}

		assert(req.pFieldBuffer != IJST_NULL);
		VarType& field = *static_cast<VarType *>(req.pFieldBuffer);
		field.clear();
		ReserveWrapper<VarType>::Do(field, req.stream.Size());
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(DefType);

		for (rapidjson::Value::ValueIterator itVal = req.stream.Begin();
			 itVal != req.stream.End(); ++itVal)
		{
			// Alloc buffer
			// New a elem buffer in container first to avoid copy
			// Use resize() instead of push_back() to avoid copy constructor in C++11
			field.resize(field.size() + 1);
			FromJsonReq elemReq(*itVal, req.allocator,
								   req.unknownMode, req.canMoveSrc, req.checkField, &field.back());
			FromJsonResp elemResp(resp.errDoc);
			// FromJson
			int ret = serializerInterface->FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				field.pop_back();
				resp.errDoc.ErrorInArray("ErrInArray", (rapidjson::SizeType)field.size());
				return ret;
			}
			++resp.fieldCount;
		}
		return 0;
	}

#if IJST_ENABLE_TO_JSON_OBJECT
	virtual int ToJson(const ToJsonReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != IJST_NULL);
		const VarType& field = *static_cast<const VarType*>(req.pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(DefType);
		req.buffer.SetArray();
		req.buffer.Reserve(static_cast<rapidjson::SizeType>(field.size()), req.allocator);

		for (typename VarType::const_iterator itera = field.begin(); itera != field.end(); ++itera) {
			JsonValue newElem;
			ToJsonReq elemReq(newElem, req.allocator, &(*itera), req.canMoveSrc, req.fPushMode);
			IJSTI_RET_WHEN_NOT_ZERO(interface->ToJson(elemReq));
			req.buffer.PushBack(newElem, req.allocator);
		}
		return 0;
	}

	virtual int SetAllocator(void *pField, JsonAllocator &allocator) IJSTI_OVERRIDE
	{
		assert(pField != IJST_NULL);
		VarType& field = *static_cast<VarType *>(pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(DefType);

		// Loop
		for (typename VarType::iterator itera = field.begin(); itera != field.end(); ++itera)
		{
			IJSTI_RET_WHEN_NOT_ZERO(interface->SetAllocator(&(*itera), allocator));
		}

		return 0;
	}
#endif
private:
	template<typename Container>
	struct ReserveWrapper {
		static void Do(Container& container, size_t capacity)
		{ (void)container; (void) capacity; }
	};

	template<typename _T>
	struct ReserveWrapper<std::vector<_T> > {
		static void Do(std::vector<_T>& container, size_t capacity)
		{
			// field.shrink_to_fit();
			container.reserve(capacity);
		}
	};
};

#define IJSTI_SERIALIZER_CONTAINER_DEFINE()																		\
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE												\
	{ return ContainerSerializerSingleton::GetInstance()->Serialize(req); }

#if IJST_ENABLE_TO_JSON_OBJECT
	#define IJSTI_SERIALIZER_CONTAINER_DEFINE_TO_JSON()																\
		virtual int ToJson(const ToJsonReq &req) IJSTI_OVERRIDE														\
		{ return ContainerSerializerSingleton::GetInstance()->ToJson(req); }										\
		virtual int SetAllocator(void *pField, JsonAllocator &allocator) IJSTI_OVERRIDE								\
		{ return ContainerSerializerSingleton::GetInstance()->SetAllocator(pField, allocator); }
#else
	#define IJSTI_SERIALIZER_CONTAINER_DEFINE_TO_JSON()		// empty
#endif

#define IJSTI_SERIALIZER_CONTAINER_DEFINE_FROM_JSON()															\
	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE					\
	{ return ContainerSerializerSingleton::GetInstance()->FromJson(req, resp); }

/**
 * Serialization class of Vector types
 * @tparam _T class
 */
template<class _T>
class FSerializer<std::vector<_T> > : public SerializerInterface {
public:
	typedef typename FSerializer<_T>::VarType ElemVarType;
	typedef std::vector<ElemVarType> VarType;

private:
	typedef Singleton<ContainerSerializer<_T, VarType> > ContainerSerializerSingleton;

public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
	IJSTI_SERIALIZER_CONTAINER_DEFINE_TO_JSON()
	IJSTI_SERIALIZER_CONTAINER_DEFINE_FROM_JSON()
};

/**
 * Serialization class of Deque types
 * @tparam _T class
 */
template<class _T>
class FSerializer<std::deque<_T> > : public SerializerInterface {
public:
	typedef typename FSerializer<_T>::VarType ElemVarType;
	typedef std::deque<ElemVarType> VarType;

private:
	typedef Singleton<ContainerSerializer<_T, VarType> > ContainerSerializerSingleton;

public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
	IJSTI_SERIALIZER_CONTAINER_DEFINE_TO_JSON()
	IJSTI_SERIALIZER_CONTAINER_DEFINE_FROM_JSON()
};

/**
 * Serialization class of Deque types
 * @tparam _T class
 */
template<class _T>
class FSerializer<std::list<_T> > : public SerializerInterface {
public:
	typedef typename FSerializer<_T>::VarType ElemVarType;
	typedef std::list<ElemVarType> VarType;

private:
	typedef Singleton<ContainerSerializer<_T, VarType> > ContainerSerializerSingleton;

public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
	IJSTI_SERIALIZER_CONTAINER_DEFINE_TO_JSON()
	IJSTI_SERIALIZER_CONTAINER_DEFINE_FROM_JSON()
};

/**
 * Serialization class of Map types
 * @tparam _T class
 */
template<class _T>
class FSerializer<TypeClassMap<_T> > : public SerializerInterface {
public:
	typedef typename FSerializer<_T>::VarType ElemVarType;
	typedef std::map<std::string, ElemVarType> VarType;

	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != IJST_NULL);
		const VarType& field = *static_cast<const VarType *>(req.pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartObject());

		for (typename VarType::const_iterator itFieldMember = field.begin(); itFieldMember != field.end(); ++itFieldMember) {
			const std::string& key = itFieldMember->first;
			IJSTI_RET_WHEN_WRITE_FAILD(
					req.writer.Key(key.data(), static_cast<rapidjson::SizeType>(key.size())) );

			SerializeReq elemReq(req.writer, &(itFieldMember->second), req.fPushMode);
			IJSTI_RET_WHEN_NOT_ZERO(interface->Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.EndObject());
		return 0;
	}

#if IJST_ENABLE_TO_JSON_OBJECT
	virtual int ToJson(const ToJsonReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != IJST_NULL);
		const VarType& field = *static_cast<const VarType *>(req.pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);
		if (!req.buffer.IsObject()) {
			req.buffer.SetObject();
		}

		for (typename VarType::const_iterator itFieldMember = field.begin(); itFieldMember != field.end(); ++itFieldMember)
		{
			// Init
			const void* pFieldValue = &itFieldMember->second;
			JsonValue newElem;
			ToJsonReq elemReq(newElem, req.allocator, pFieldValue, req.canMoveSrc, req.fPushMode);
			IJSTI_RET_WHEN_NOT_ZERO(interface->ToJson(elemReq));

			// Add member by copy key name
			rapidjson::GenericStringRef<char> fieldNameRef =
					rapidjson::StringRef(itFieldMember->first.data(), itFieldMember->first.size());
			req.buffer.AddMember(
					rapidjson::Value().SetString(fieldNameRef, req.allocator),
					newElem,
					req.allocator
			);
		}
		return 0;
	}

	virtual int SetAllocator(void* pField, JsonAllocator& allocator) IJSTI_OVERRIDE
	{
		assert(pField != IJST_NULL);
		VarType& field = *static_cast<VarType *>(pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);

		// Reset member
		for (typename VarType::iterator itera = field.begin(); itera != field.end(); ++itera)
		{
			IJSTI_RET_WHEN_NOT_ZERO(interface->SetAllocator(&(itera->second), allocator));
		}

		return 0;
	}
#endif

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsObject()) {
			resp.errDoc.ElementTypeMismatch("object", req.stream);
			return Err::kDeserializeValueTypeError;
		}

		assert(req.pFieldBuffer != IJST_NULL);
		VarType& field = *static_cast<VarType *>(req.pFieldBuffer);
		field.clear();
		// pField->shrink_to_fit();
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(_T);

		for (rapidjson::Value::MemberIterator itMember = req.stream.MemberBegin();
			 itMember != req.stream.MemberEnd(); ++itMember)
		{
			// Get information
			const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
			// New a elem buffer in container first to avoid copy
			typename VarType::value_type buf(fieldName, ElemVarType());
			std::pair<typename VarType::iterator, bool> insertRet = field.insert(IJSTI_MOVE(buf));
			const bool hasAlloc = insertRet.second;
			ElemVarType &elemBuffer = insertRet.first->second;
			FromJsonReq elemReq(itMember->value, req.allocator,
								req.unknownMode, req.canMoveSrc, req.checkField, &elemBuffer);

			// FromJson
			FromJsonResp elemResp(resp.errDoc);
			int ret = serializerInterface->FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				if (hasAlloc)
				{
					field.erase(fieldName);
				}
				resp.errDoc.ErrorInObject("ErrInMap", fieldName);
				return ret;
			}
			++resp.fieldCount;
		}
		return 0;
	}

};

}	// namespace detail
}	// namespace ijst

#endif //_IJST_TYPES_CONTAINER_HPP_INCLUDE_
