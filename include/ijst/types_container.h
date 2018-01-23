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

//! @brief Declare a vector<_T> field.
//! @ingroup IJST_MACRO_API
#define IJST_TVEC(_T)	::std::vector< _T >
//! @brief Declare a deque<_T> field.
//! @ingroup IJST_MACRO_API
#define IJST_TDEQUE(_T)	::std::deque< _T >
//! @brief Declare a vector<_T> field.
//! @ingroup IJST_MACRO_API
#define IJST_TLIST(_T)	::std::list< _T >
//! @brief Declare a map<string, _T> field of json object
//! @ingroup IJST_MACRO_API
#define IJST_TMAP(_T)	IJST_TYPE(::std::map< ::std::string, _T >)
//! @brief Declare a vector of members of json object
//! @ingroup IJST_MACRO_API
#define IJST_TOBJ(_T)	::std::vector< ::ijst::T_Member< _T > >
//! @brief Declare a object field which _T is a ijst struct type.
//! @ingroup IJST_MACRO_API
#define IJST_TST(_T)	_T

namespace ijst {

/**
 * @brief Memeber in json object
 *
 * @tparam _T	value type
 */
template<typename _T>
struct T_Member {
	typedef _T ValType;
	std::string name;
	_T value;

	T_Member(): name(), value() {}
	T_Member(const std::string& _name, const _T& _value): name(_name), value(_value) {}
#if __cplusplus >= 201103L
	T_Member(std::string&& _name, _T&& _value): name(_name), value(_value) {}
#endif
};

/**
 * Specialization for map type of Optional template.
 * This specialization add operator[] (string key) for getter chaining.
 *
 * @tparam _TElem
 */
template <typename _TElem>
class Optional <std::map<std::string, _TElem> > {
	typedef std::map<std::string, _TElem> ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	/**
	 * Get element by key
	 *
	 * @param key 	key
	 * @return 		Optional(elemInstance) if key is found, Optional(null) else
	 */
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

/**
 * const version Specialization for map type of Optional template.
 * This specialization add operator[] (string key) for getter chaining.
 *
 * @tparam _TElem	Element type
 */
template <typename _TElem>
class Optional <const std::map<std::string, _TElem> >
{
	typedef const std::map<std::string, _TElem> ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	/**
	 * Get element by key
	 *
	 * @param key 	key
	 * @return 		Optional(const elemInstance) if key is found, Optional(null) else
	 */
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

/**
 * Specialization for vector or deque type of Optional template.
 * This specialization add operator[] (size_type i) for getter chaining.
 *
 * @tparam _TElem	Element type
 */
#define IJSTI_OPTIONAL_ARRAY_DEFINE(is_const, Container)													\
	template<typename _TElem>																				\
	class Optional<is_const Container<_TElem> >																\
	{ 																										\
		typedef is_const Container<_TElem> ValType;															\
		IJSTI_OPTIONAL_BASE_DEFINE(ValType)																	\
	public:																									\
		/** return Optional(elemeInstance) if i is valid, Optional(null) else. */							\
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

template<typename ElemType, typename VarType>
class ContainerSerializer : public SerializerInterface {
public:
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != IJST_NULL);
		const VarType& field = *static_cast<const VarType*>(req.pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(ElemType);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartArray());

		for (typename VarType::const_iterator itera = field.begin(); itera != field.end(); ++itera) {
			SerializeReq elemReq(req.writer, &(*itera), req.serFlag);
			IJSTI_RET_WHEN_NOT_ZERO(interface->Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(
				req.writer.EndArray(static_cast<rapidjson::SizeType>(field.size())) );

		return 0;
	}

	virtual int FromJson(const FromJsonReq &req, FromJsonResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsArray()) {
			resp.errDoc.ElementTypeMismatch("array", req.stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		assert(req.pFieldBuffer != IJST_NULL);
		VarType& field = *static_cast<VarType *>(req.pFieldBuffer);
		field.clear();
		// Alloc buffer
		field.resize(req.stream.Size());
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(ElemType);

		size_t i = 0;
		typename VarType::iterator itField = field.begin();
		rapidjson::Value::ValueIterator  itVal = req.stream.Begin();
		for (; itVal != req.stream.End(); ++itVal, ++itField, ++i)
		{
			assert(i < field.size());
			assert(itField != field.end());
			FromJsonReq elemReq(*itVal, req.allocator,
								   req.deserFlag, req.canMoveSrc, &*itField);
			FromJsonResp elemResp(resp.errDoc);
			// FromJson
			int ret = serializerInterface->FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				field.resize(i);
				resp.errDoc.ErrorInArray("ErrInArray", (rapidjson::SizeType)field.size());
				return ret;
			}
			++resp.fieldCount;
		}
		assert(i == field.size());
		return 0;
	}

	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE
	{
		VarType& field = *static_cast<VarType *>(pField);
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(ElemType);
		for (typename VarType::iterator itField = field.begin(); itField != field.end(); ++itField)
		{
			serializerInterface->ShrinkAllocator(&*itField);
		}
	}
};

#define IJSTI_SERIALIZER_CONTAINER_DEFINE()																		\
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE												\
	{ return ContainerSerializerSingleton::GetInstance()->Serialize(req); }										\
	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE					\
	{ return ContainerSerializerSingleton::GetInstance()->FromJson(req, resp); }								\
	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE													\
	{ return ContainerSerializerSingleton::GetInstance()->ShrinkAllocator(pField); }							\

/**
 * Serialization class of Vector types
 * @tparam _T class
 */
template<class _T>
class FSerializer<std::vector<_T> > : public SerializerInterface {
	typedef std::vector<_T> VarType;
	typedef Singleton<ContainerSerializer<_T, VarType> > ContainerSerializerSingleton;
public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
};

/**
 * Serialization class of Deque types
 * @tparam _T class
 */
template<class _T>
class FSerializer<std::deque<_T> > : public SerializerInterface {
	typedef std::deque<_T> VarType;
	typedef Singleton<ContainerSerializer<_T, VarType> > ContainerSerializerSingleton;

public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
};

/**
 * Serialization class of Deque types
 * @tparam _T class
 */
template<class _T>
class FSerializer<std::list<_T> > : public SerializerInterface {
	typedef std::list<_T> VarType;
	typedef Singleton<ContainerSerializer<_T, VarType> > ContainerSerializerSingleton;
public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
};

/**
 * Serialization class of Map types
 * @tparam _T class
 */
template<class _T>
class FSerializer<std::map<std::string, _T> > : public SerializerInterface {
	typedef std::map<std::string, _T> VarType;
public:

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

			SerializeReq elemReq(req.writer, &(itFieldMember->second), req.serFlag);
			IJSTI_RET_WHEN_NOT_ZERO(interface->Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(
				req.writer.EndObject(static_cast<rapidjson::SizeType>(field.size())) );
		return 0;
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsObject()) {
			resp.errDoc.ElementTypeMismatch("object", req.stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		assert(req.pFieldBuffer != IJST_NULL);
		VarType& field = *static_cast<VarType *>(req.pFieldBuffer);
		field.clear();
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(_T);

		for (rapidjson::Value::MemberIterator itMember = req.stream.MemberBegin();
			 itMember != req.stream.MemberEnd(); ++itMember)
		{
			const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
			// New a elem buffer in container first to avoid copy
			typename VarType::value_type buf(fieldName, _T());
			std::pair<typename VarType::iterator, bool> insertRet = field.insert(IJSTI_MOVE(buf));
			// Check duplicate
			if (!insertRet.second) {
				resp.errDoc.ElementMapKeyDuplicated(fieldName);
				return ErrorCode::kDeserializeMapKeyDuplicated;
			}

			// Element FromJson
			_T &elemBuffer = insertRet.first->second;
			FromJsonReq elemReq(itMember->value, req.allocator,
								req.deserFlag, req.canMoveSrc, &elemBuffer);
			FromJsonResp elemResp(resp.errDoc);
			int ret = serializerInterface->FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				field.erase(fieldName);
				resp.errDoc.ErrorInObject("ErrInMap", fieldName);
				return ret;
			}
			++resp.fieldCount;
		}
		return 0;
	}

	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE
	{
		VarType& field = *static_cast<VarType *>(pField);
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(_T);
		for (typename VarType::iterator itField = field.begin(); itField != field.end(); ++itField)
		{
			serializerInterface->ShrinkAllocator(&itField->second);
		}
	}
};

/**
 * Serialization class of Object types
 * @tparam _T class
 */
template<class _T>
class FSerializer<std::vector<T_Member<_T> > > : public SerializerInterface {
	typedef T_Member<_T> MemberType;
	typedef typename MemberType::ValType ValType;
	typedef std::vector<MemberType> VarType;
public:
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != IJST_NULL);
		const VarType& field = *static_cast<const VarType *>(req.pField);
		SerializerInterface *interface = IJSTI_FSERIALIZER_INS(ValType);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartObject());

		for (typename VarType::const_iterator itMember = field.begin(); itMember != field.end(); ++itMember) {
			const std::string& key = itMember->name;
			IJSTI_RET_WHEN_WRITE_FAILD(
					req.writer.Key(key.data(), static_cast<rapidjson::SizeType>(key.size())) );

			SerializeReq elemReq(req.writer, &(itMember->value), req.serFlag);
			IJSTI_RET_WHEN_NOT_ZERO(interface->Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(
				req.writer.EndObject(static_cast<rapidjson::SizeType>(field.size())) );
		return 0;
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsObject()) {
			resp.errDoc.ElementTypeMismatch("object", req.stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		assert(req.pFieldBuffer != IJST_NULL);
		VarType& field = *static_cast<VarType *>(req.pFieldBuffer);
		field.clear();
		// pField->shrink_to_fit();
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(ValType);

		// Alloc buffer
		field.resize(req.stream.MemberCount());
		size_t i = 0;
		rapidjson::Value::MemberIterator itMember = req.stream.MemberBegin();
		for (; itMember != req.stream.MemberEnd(); ++itMember, ++i)
		{
			assert(i < field.size());
			MemberType& memberBuf = field[i];
			memberBuf.name = std::string(itMember->name.GetString(), itMember->name.GetStringLength());
			ValType &elemBuffer = memberBuf.value;
			FromJsonReq elemReq(itMember->value, req.allocator,
								req.deserFlag, req.canMoveSrc, &elemBuffer);
			FromJsonResp elemResp(resp.errDoc);

			int ret = serializerInterface->FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				resp.errDoc.ErrorInObject("ErrInMap", memberBuf.name);
				field.resize(i);
				return ret;
			}
			++resp.fieldCount;
		}
		assert(i == field.size());
		return 0;
	}

	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE
	{
		VarType& field = *static_cast<VarType*>(pField);
		SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(ValType);
		for (typename VarType::iterator itField = field.begin(); itField != field.end(); ++itField)
		{
			serializerInterface->ShrinkAllocator(&itField->value);
		}
	}
};

}	// namespace detail
}	// namespace ijst

#endif //_IJST_TYPES_CONTAINER_HPP_INCLUDE_
