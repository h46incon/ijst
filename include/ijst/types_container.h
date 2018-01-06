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
#define IJST_TVEC(_T)	::ijst::detail::TypeClassVec< _T>
//! Declare a deque<_T> field.
#define IJST_TDEQUE(_T)	::ijst::detail::TypeClassDeque< _T>
//! Declare a vector<_T> field.
#define IJST_TLIST(_T)	::ijst::detail::TypeClassList< _T>
//! Declare a map<string, _T> field.
#define IJST_TMAP(_T)	::ijst::detail::TypeClassMap< _T>
//! Declare a object field which _T is a ijst struct type.
#define IJST_TOBJ(_T)	::ijst::detail::TypeClassObj< _T>

//! Wrappers.
#if IJST_USE_SL_WRAPPER
	#define IJST_CONT_VEC(...)			ijst::Vector<__VA_ARGS__>
	#define IJST_CONT_MAP(...)			ijst::Map<__VA_ARGS__>
	#define IJST_CONT_DEQUE(...)		ijst::Deque<__VA_ARGS__>
	#define IJST_CONT_LIST(...)			ijst::SLWrapper<std::list<__VA_ARGS__> >
#else
	#define IJST_CONT_VEC(...)			std::vector<__VA_ARGS__>
	#define IJST_CONT_DEQUE(...)		std::deque<__VA_ARGS__>
	#define IJST_CONT_LIST(...)			std::list<__VA_ARGS__>
	#define IJST_CONT_MAP(...)			std::map<__VA_ARGS__>
#endif

namespace ijst {

template <typename _TElem>
class Optional <IJST_CONT_MAP(std::string, _TElem)>
{
	typedef IJST_CONT_MAP(std::string, _TElem) ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	Optional<_TElem> operator[](const std::string& key) const
	{
		if (m_pVal == IJST_NULL) {
			return Optional<_TElem>(IJST_NULL);
		}
		typename std::map<std::string, _TElem>::iterator it = IJST_CONT_VAL(*m_pVal).find(key);
		if (it == IJST_CONT_VAL(*m_pVal).end()){
			return Optional<_TElem>(IJST_NULL);
		}
		else {
			return Optional<_TElem>(&it->second);
		}
	}
};

template <typename _TElem>
class Optional <const IJST_CONT_MAP(std::string, _TElem)>
{
	typedef const IJST_CONT_MAP(std::string, _TElem) ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	Optional<const _TElem> operator[](const std::string& key) const
	{
		if (m_pVal == IJST_NULL) {
			return Optional<const _TElem>(IJST_NULL);
		}
		typename std::map<std::string, _TElem>::const_iterator it = IJST_CONT_VAL(*m_pVal).find(key);
		if (it == IJST_CONT_VAL(*m_pVal).end()){
			return Optional<const _TElem>(IJST_NULL);
		}
		else {
			return Optional<const _TElem>(&it->second);
		}
	}
};

#define IJSTI_OPTIONAL_ARRAY_DEFINE(is_const, CONT_WRAPPER, Container)										\
	template<typename _TElem>																				\
	class Optional<is_const CONT_WRAPPER(_TElem)>															\
	{ 																										\
		typedef is_const CONT_WRAPPER(_TElem) ValType;														\
		IJSTI_OPTIONAL_BASE_DEFINE(ValType)																	\
	public:																									\
		Optional<is_const _TElem> operator[](typename Container<_TElem>::size_type i) const					\
		{																									\
			if (m_pVal == IJST_NULL || IJST_CONT_VAL(*m_pVal).size() <= i) {								\
				return Optional<is_const _TElem>(IJST_NULL);												\
			}																								\
			return Optional<is_const _TElem>(&(*m_pVal)[i]);												\
		}																									\
	};

IJSTI_OPTIONAL_ARRAY_DEFINE(, IJST_CONT_VEC, std::vector)
IJSTI_OPTIONAL_ARRAY_DEFINE(const, IJST_CONT_VEC, std::vector)
IJSTI_OPTIONAL_ARRAY_DEFINE(, IJST_CONT_DEQUE, std::deque)
IJSTI_OPTIONAL_ARRAY_DEFINE(const, IJST_CONT_DEQUE, std::deque)

}	// namespace ijst


namespace ijst {
namespace detail {

template<class _T>
struct TypeClassVec {
	// nothing
};

template<class _T>
struct TypeClassDeque {
	// nothing
};

template<class _T>
struct TypeClassList {
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

template<typename DefType, typename VarType, typename RealVarType>
class ContainerSerializer : public SerializerInterface {
public:
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType *pFieldWrapper = static_cast<const VarType *>(req.pField);
		assert(pFieldWrapper != IJST_NULL);
		const RealVarType& field = IJST_CONT_VAL(*pFieldWrapper);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(DefType);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartArray());

		for (typename RealVarType::const_iterator itera = field.begin(); itera != field.end(); ++itera) {
			SerializeReq elemReq(req.writer, &(*itera), req.fPushMode);
			IJSTI_RET_WHEN_NOT_ZERO(interface->Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.EndArray());

		return 0;
	}

	virtual int FromJson(const FromJsonReq &req, FromJsonResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsArray()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.errDoc.TypeMismatch("array", req.stream);
			return Err::kDeserializeValueTypeError;
		}

		VarType *pFieldWrapper = static_cast<VarType *>(req.pFieldBuffer);
		assert(pFieldWrapper != IJST_NULL);
		RealVarType& field = IJST_CONT_VAL(*pFieldWrapper);
		field.clear();
		ReserveWrapper<RealVarType>::Do(field, req.stream.Size());
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(DefType);

		for (rapidjson::Value::ValueIterator itVal = req.stream.Begin();
			 itVal != req.stream.End(); ++itVal)
		{
			// Alloc buffer
			// New a elem buffer in container first to avoid copy
			// Use resize() instead of push_back() to avoid copy constructor in C++11
			field.resize(field.size() + 1);
			FromJsonReq elemReq(*itVal, req.allocator,
								   req.unknownMode, req.canMoveSrc, &field.back());
			FromJsonResp elemResp(resp.errDoc.pAllocator);
			// FromJson
			int ret = serializerInterface->FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				field.pop_back();
				resp.fStatus = FStatus::kParseFailed;
				resp.errDoc.ErrorInArray("ErrInArray", field.size(), &elemResp.errDoc);
				return ret;
			}
			++resp.fieldCount;
		}
		resp.fStatus = FStatus::kValid;
		return 0;
	}

#if IJST_ENABLE_TO_JSON_OBJECT
	virtual int ToJson(const ToJsonReq &req) IJSTI_OVERRIDE
	{
		const VarType *pFieldWrapper = static_cast<const VarType *>(req.pField);
		assert(pFieldWrapper != IJST_NULL);
		const RealVarType& field = IJST_CONT_VAL(*pFieldWrapper);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(DefType);
		req.buffer.SetArray();
		req.buffer.Reserve(static_cast<rapidjson::SizeType>(field.size()), req.allocator);

		for (typename RealVarType::const_iterator itera = field.begin(); itera != field.end(); ++itera) {
			JsonValue newElem;
			ToJsonReq elemReq(newElem, req.allocator, &(*itera), req.canMoveSrc, req.fPushMode);
			IJSTI_RET_WHEN_NOT_ZERO(interface->ToJson(elemReq));
			req.buffer.PushBack(newElem, req.allocator);
		}
		return 0;
	}

	virtual int SetAllocator(void *pField, JsonAllocator &allocator) IJSTI_OVERRIDE
	{
		VarType *pFieldWrapper = static_cast<VarType *>(pField);
		assert(pFieldWrapper != IJST_NULL);
		RealVarType& field = IJST_CONT_VAL(*pFieldWrapper);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(DefType);

		// Loop
		for (typename RealVarType::iterator itera = field.begin(); itera != field.end(); ++itera)
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
class FSerializer<TypeClassVec<_T> > : public SerializerInterface {
public:
	typedef typename FSerializer<_T>::VarType ElemVarType;
	typedef IJST_CONT_VEC(ElemVarType) VarType;

private:
	typedef std::vector<ElemVarType> RealVarType;
	typedef Singleton<ContainerSerializer<_T, VarType, RealVarType> > ContainerSerializerSingleton;

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
class FSerializer<TypeClassDeque<_T> > : public SerializerInterface {
public:
	typedef typename FSerializer<_T>::VarType ElemVarType;
	typedef IJST_CONT_DEQUE(ElemVarType) VarType;

private:
	typedef std::deque<ElemVarType> RealVarType;
	typedef Singleton<ContainerSerializer<_T, VarType, RealVarType> > ContainerSerializerSingleton;

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
class FSerializer<TypeClassList<_T> > : public SerializerInterface {
public:
	typedef typename FSerializer<_T>::VarType ElemVarType;
	typedef IJST_CONT_LIST(ElemVarType) VarType;

private:
	typedef std::list<ElemVarType> RealVarType;
	typedef Singleton<ContainerSerializer<_T, VarType, RealVarType> > ContainerSerializerSingleton;

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
private:
	typedef typename FSerializer<_T>::VarType ElemVarType;
	typedef std::map<std::string, ElemVarType> RealVarType;
public:
	typedef IJST_CONT_MAP(std::string, ElemVarType) VarType;

	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		const VarType *pFieldWrapper = static_cast<const VarType *>(req.pField);
		assert(pFieldWrapper != IJST_NULL);
		const RealVarType& field = IJST_CONT_VAL(*pFieldWrapper);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartObject());

		for (typename RealVarType::const_iterator itFieldMember = field.begin(); itFieldMember != field.end(); ++itFieldMember) {
			const std::string& key = itFieldMember->first;
			IJSTI_RET_WHEN_WRITE_FAILD(
					req.writer.Key(key.c_str(), static_cast<rapidjson::SizeType>(key.length())) );

			SerializeReq elemReq(req.writer, &(itFieldMember->second), req.fPushMode);
			IJSTI_RET_WHEN_NOT_ZERO(interface->Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.EndObject());
		return 0;
	}

#if IJST_ENABLE_TO_JSON_OBJECT
	virtual int ToJson(const ToJsonReq &req) IJSTI_OVERRIDE
	{
		const VarType *pFieldWrapper = static_cast<const VarType *>(req.pField);
		assert(pFieldWrapper != IJST_NULL);
		const RealVarType &field = IJST_CONT_VAL(*pFieldWrapper);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);
		if (!req.buffer.IsObject()) {
			req.buffer.SetObject();
		}

		for (typename RealVarType::const_iterator itFieldMember = field.begin(); itFieldMember != field.end(); ++itFieldMember)
		{
			// Init
			const void* pFieldValue = &itFieldMember->second;
			JsonValue newElem;
			ToJsonReq elemReq(newElem, req.allocator, pFieldValue, req.canMoveSrc, req.fPushMode);
			IJSTI_RET_WHEN_NOT_ZERO(interface->ToJson(elemReq));

			// Add member by copy key name
			rapidjson::GenericStringRef<char> fieldNameRef =
					rapidjson::StringRef(itFieldMember->first.c_str(), itFieldMember->first.length());
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
		VarType *pFieldWrapper = static_cast<VarType *>(pField);
		assert(pFieldWrapper != IJST_NULL);
		RealVarType&field = IJST_CONT_VAL(*pFieldWrapper);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);

		// Reset member
		for (typename RealVarType::iterator itera = field.begin(); itera != field.end(); ++itera)
		{
			IJSTI_RET_WHEN_NOT_ZERO(interface->SetAllocator(&(itera->second), allocator));
		}

		return 0;
	}
#endif

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsObject()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.errDoc.TypeMismatch("object", req.stream);
			return Err::kDeserializeValueTypeError;
		}

		VarType *pFieldWrapper = static_cast<VarType *>(req.pFieldBuffer);
		assert(pFieldWrapper != IJST_NULL);
		RealVarType&field = IJST_CONT_VAL(*pFieldWrapper);
		field.clear();
		// pField->shrink_to_fit();
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(_T);

		for (rapidjson::Value::MemberIterator itMember = req.stream.MemberBegin();
			 itMember != req.stream.MemberEnd(); ++itMember)
		{
			// Get information
			const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
			// New a elem buffer in container first to avoid copy
			bool hasAlloc = false;
			if (field.find(fieldName) == field.end()) {
				hasAlloc = true;
			}

			ElemVarType &elemBuffer = field[fieldName];
			FromJsonReq elemReq(itMember->value, req.allocator,
								req.unknownMode, req.canMoveSrc, &elemBuffer);

			// FromJson
			FromJsonResp elemResp(resp.errDoc.pAllocator);
			int ret = serializerInterface->FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				if (hasAlloc)
				{
					field.erase(fieldName);
				}
				resp.errDoc.ErrorInObject("ErrInMap", fieldName, &elemResp.errDoc);
				resp.fStatus = FStatus::kParseFailed;
				return ret;
			}
			++resp.fieldCount;
		}
		resp.fStatus = FStatus::kValid;
		return 0;
	}

};

}	// namespace detail
}	// namespace ijst

#endif //_IJST_TYPES_CONTAINER_HPP_INCLUDE_
