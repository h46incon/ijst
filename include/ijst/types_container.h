/**************************************************************************************************
 *		Serialization implementation of Container types
 *		Created by h46incon on 2017/12/26.
 **************************************************************************************************/

#ifndef IJST_TYPES_CONTAINER_HPP_INCLUDE_
#define IJST_TYPES_CONTAINER_HPP_INCLUDE_

#include "accessor.h"
#include <map>
#include <list>
#include <deque>

#ifdef _MSC_VER
	#define IJSTI_IMPL_WRAPPER(Name, ... )	IJSTI_EXPAND(IJSTI_PP_CONCAT(IJSTI_ ## Name ## _IMPL_, IJSTI_PP_NARGS(__VA_ARGS__))(__VA_ARGS__))
#else
	#define IJSTI_IMPL_WRAPPER(Name, ... )	IJSTI_PP_CONCAT(IJSTI_ ## Name ## _IMPL_, IJSTI_PP_NARGS(__VA_ARGS__))(__VA_ARGS__)
#endif

//! @brief IJST_TVEC(T, Alloc=std::allocator<T>), use for declaring a vector<T, Alloc> field in ijst struct.
//! #define IJST_TVEC(...) 		std::vector< _VA_ARGS__ >
//! @ingroup IJST_MACRO_API
#define IJST_TVEC(...) 						IJSTI_IMPL_WRAPPER(TVEC, __VA_ARGS__)
#define IJSTI_TVEC_IMPL_1(T) 				::std::vector< T >
#define IJSTI_TVEC_IMPL_2(T, Alloc) 		IJST_TYPE(::std::vector< T , Alloc >)

//! @brief IJST_TDEQUE(T, Alloc=std::allocator<T>), use for declaring a deque<T, Alloc> field in ijst struct.
//! #define IJST_TDEQUE(...)	std::deque< _VA_ARGS__ >
//! @ingroup IJST_MACRO_API
#define IJST_TDEQUE(...) 					IJSTI_IMPL_WRAPPER(TDEQUE, __VA_ARGS__)
#define IJSTI_TDEQUE_IMPL_1(T) 				::std::deque< T >
#define IJSTI_TDEQUE_IMPL_2(T, Alloc) 		IJST_TYPE(::std::deque< T , Alloc >)

//! @brief IJST_TLIST(T, Alloc=std::allocator<T>), use for declaring a list<T, Alloc> field in ijst struct.
//!	#define IJST_TLIST(...) 	std::list< _VA_ARGS__ >
//! @ingroup IJST_MACRO_API
#define IJST_TLIST(...) 					IJSTI_IMPL_WRAPPER(TLIST, __VA_ARGS__)
#define IJSTI_TLIST_IMPL_1(T) 				::std::list< T >
#define IJSTI_TLIST_IMPL_2(T, Alloc) 		IJST_TYPE(::std::list< T , Alloc >)

//! @brief IJST_TMAP(T, Comp=std::less, Alloc=std::allocator<T>), use for declaring a map<string, T, Comp, Alloc> field in ijst struct.
//! @ingroup IJST_MACRO_API
#define IJST_TMAP(...)						IJST_TYPE(::std::map< ::std::basic_string<_ijst_Ch>, __VA_ARGS__ >)

//! @brief Declare a vector of members of json object
//!	#define IJST_TOBJ(T, Alloc=DefaultAlloc) 	std::vector<ijst::T_Member<T>, Alloc>
//! @ingroup IJST_MACRO_API
#define IJST_TOBJ(...)						IJSTI_IMPL_WRAPPER(TOBJ, __VA_ARGS__)
#define IJSTI_TOBJ_IMPL_1(T)				IJST_TYPE(::std::vector< ::ijst::T_Member< T, _ijst_Ch> >)
#define IJSTI_TOBJ_IMPL_2(T, Alloc)			IJST_TYPE(::std::vector< ::ijst::T_Member< T, _ijst_Ch>, Alloc >)

//! @brief Declare a object field which T is a ijst struct type.
//! @ingroup IJST_MACRO_API
#define IJST_TST(T)							T

namespace ijst {

/**
 * @brief Memeber in json object
 *
 * @tparam T		value type
 * @tparam CharType	character type of string
 */
template<typename T, typename CharType = char>
struct T_Member {
	typedef T ValType;
	typedef CharType Ch;
	std::basic_string<Ch> name;
	T value;

	T_Member(): name(), value() {}
	T_Member(const std::basic_string<Ch>& _name, const T& _value): name(_name), value(_value) {}
#if IJST_HAS_CXX11_RVALUE_REFS
	T_Member(std::basic_string<Ch>&& _name, T&& _value): name(_name), value(_value) {}
#endif
};

/**
 * Specialization for map type of Optional template.
 * This specialization add operator[] (string key) for getter chaining.
 *
 * @tparam TElem		map value type
 * @tparam CharType		character type of map key
 */
template <typename TElem, typename CharType, typename Compare, typename Alloc>
class Optional <std::map<std::basic_string<CharType>, TElem, Compare, Alloc> >
{
	typedef std::map<std::basic_string<CharType>, TElem, Compare, Alloc> ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	/**
	 * Get element by key
	 *
	 * @param key 	key
	 * @return 		Optional(elemInstance) if key is found, Optional(null) else
	 */
	Optional<TElem> operator[](const std::basic_string<CharType>& key) const
	{
		if (m_pVal == NULL) {
			return Optional<TElem>(NULL);
		}
		typename ValType::iterator it = m_pVal->find(key);
		if (it == m_pVal->end()){
			return Optional<TElem>(NULL);
		}
		else {
			return Optional<TElem>(&it->second);
		}
	}
};

/**
 * const version Specialization for map type of Optional template.
 * This specialization add operator[] (string key) for getter chaining.
 *
 * @tparam TElem		map value type
 * @tparam CharType		character type of map key
 */
template <typename TElem, typename CharType, typename Compare, typename Alloc>
class Optional <const std::map<std::basic_string<CharType>, TElem, Compare, Alloc> >
{
	typedef const std::map<std::basic_string<CharType>, TElem, Compare, Alloc> ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	/**
	 * Get element by key
	 *
	 * @param key 	key
	 * @return 		Optional(const elemInstance) if key is found, Optional(null) else
	 */
	Optional<const TElem> operator[](const std::basic_string<CharType>& key) const
	{
		if (m_pVal == NULL) {
			return Optional<const TElem>(NULL);
		}
		typename ValType::const_iterator it = m_pVal->find(key);
		if (it == m_pVal->end()){
			return Optional<const TElem>(NULL);
		}
		else {
			return Optional<const TElem>(&it->second);
		}
	}
};

/**
 * Specialization for vector or deque type of Optional template.
 * This specialization add operator[] (size_type i) for getter chaining.
 *
 * @tparam TElem	Element type
 */
#define IJSTI_OPTIONAL_ARRAY_DEFINE(is_const, Container)													\
	template<typename TElem, typename Alloc>																\
	class Optional<is_const Container<TElem, Alloc> >														\
	{ 																										\
		typedef is_const Container<TElem, Alloc> ValType;													\
		IJSTI_OPTIONAL_BASE_DEFINE(ValType)																	\
	public:																									\
		/** return Optional(elemeInstance) if i is valid, Optional(null) else. */							\
		Optional<is_const TElem> operator[](typename Container<TElem, Alloc>::size_type i) const			\
		{																									\
			if (m_pVal == NULL || m_pVal->size() <= i) {													\
				return Optional<is_const TElem>(NULL);														\
			}																								\
			return Optional<is_const TElem>(&(*m_pVal)[i]);													\
		}																									\
	};

IJSTI_OPTIONAL_ARRAY_DEFINE(, std::vector)
IJSTI_OPTIONAL_ARRAY_DEFINE(const, std::vector)
IJSTI_OPTIONAL_ARRAY_DEFINE(, std::deque)
IJSTI_OPTIONAL_ARRAY_DEFINE(const, std::deque)

}	// namespace ijst


namespace ijst {
namespace detail {

template<typename ElemType, typename VarType, typename Encoding>
class ContainerSerializer : public SerializerInterface<Encoding> {
public:
	IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);

	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != NULL);
		const VarType& field = *static_cast<const VarType*>(req.pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ElemType, Encoding);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartArray());

		for (typename VarType::const_iterator itera = field.begin(), itEnd = field.end(); itera != itEnd; ++itera)
		{
			SerializeReq elemReq(req.writer, &(*itera), req.serFlag);
			IJSTI_RET_WHEN_NOT_ZERO(intf.Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(
				req.writer.EndArray(static_cast<rapidjson::SizeType>(field.size())) );

		return 0;
	}

	virtual int FromJson(const FromJsonReq &req, FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsArray()), "array");

		assert(req.pFieldBuffer != NULL);
		VarType& field = *static_cast<VarType *>(req.pFieldBuffer);
		field.clear();
		// Alloc buffer
		field.resize(req.stream.Size());
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ElemType, Encoding);

		size_t i = 0;
		typename VarType::iterator itField = field.begin();
		for (typename rapidjson::GenericValue<Encoding>::ValueIterator itVal = req.stream.Begin(), itEnd = req.stream.End();
				itVal != itEnd;
				++itVal, ++itField, ++i)
		{
			assert(i < field.size());
			assert(itField != field.end());
			FromJsonReq elemReq(*itVal, req.allocator,
								req.deserFlag, req.canMoveSrc, &*itField, FDesc::NoneFlag);	// element desc is always default
			FromJsonResp elemResp(resp.errDoc);
			// FromJson
			int ret = intf.FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				field.resize(i);
				resp.errDoc.ErrorInArray(static_cast<rapidjson::SizeType>(field.size()));
				return ret;
			}
		}
		assert(i == field.size());

		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((field.empty()));
		return 0;
	}

	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE
	{
		VarType& field = *static_cast<VarType *>(pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ElemType, Encoding);
		for (typename VarType::iterator itField = field.begin(), itEnd = field.end(); itField != itEnd; ++itField)
		{
			intf.ShrinkAllocator(&*itField);
		}
	}
};

#define IJSTI_SERIALIZER_CONTAINER_DEFINE()																		\
	IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);																	\
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE												\
	{ return Singleton<ContainerSerializer<T, VarType, Encoding> >().Serialize(req); }										\
	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE					\
	{ return Singleton<ContainerSerializer<T, VarType, Encoding> >().FromJson(req, resp); }									\
	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE													\
	{ return Singleton<ContainerSerializer<T, VarType, Encoding> >().ShrinkAllocator(pField); }

/**
 * Serialization class of Vector types
 * @tparam T class
 */
template<class T, typename Alloc, typename Encoding>
class FSerializer<std::vector<T, Alloc>, Encoding> : public SerializerInterface<Encoding> {
	typedef std::vector<T, Alloc> VarType;
public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
};

/**
 * Serialization class of Deque types
 * @tparam T class
 */
template<class T, typename Alloc, typename Encoding>
class FSerializer<std::deque<T, Alloc>, Encoding> : public SerializerInterface<Encoding> {
	typedef std::deque<T, Alloc> VarType;
public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
};

/**
 * Serialization class of Deque types
 * @tparam T class
 */
template<class T, typename Alloc, typename Encoding>
class FSerializer<std::list<T, Alloc>, Encoding> : public SerializerInterface<Encoding> {
	typedef std::list<T, Alloc> VarType;
public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
};

/**
 * Serialization class of Map types
 * @tparam T class
 */
template<class T, typename Compare, typename Alloc, typename Encoding>
class FSerializer<std::map<std::basic_string<typename Encoding::Ch>, T, Compare, Alloc>, Encoding> : public SerializerInterface<Encoding> {
	typedef typename Encoding::Ch Ch;
	typedef std::map<std::basic_string<Ch>, T, Compare, Alloc> VarType;
public:
	IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);

	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != NULL);
		const VarType& field = *static_cast<const VarType *>(req.pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(T, Encoding);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartObject());

		for (typename VarType::const_iterator itFieldMember = field.begin(), itEnd = field.end(); itFieldMember != itEnd; ++itFieldMember)
		{
			const std::basic_string<Ch>& key = itFieldMember->first;
			IJSTI_RET_WHEN_WRITE_FAILD(
					req.writer.Key(key.data(), static_cast<rapidjson::SizeType>(key.size())) );

			SerializeReq elemReq(req.writer, &(itFieldMember->second), req.serFlag);
			IJSTI_RET_WHEN_NOT_ZERO(intf.Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(
				req.writer.EndObject(static_cast<rapidjson::SizeType>(field.size())) );
		return 0;
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsObject()), "object");

		assert(req.pFieldBuffer != NULL);
		VarType& field = *static_cast<VarType *>(req.pFieldBuffer);
		field.clear();
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(T, Encoding);

		for (typename rapidjson::GenericValue<Encoding>::MemberIterator itMember = req.stream.MemberBegin(), itEnd = req.stream.MemberEnd();
			 itMember != itEnd; ++itMember)
		{
			// New a elem buffer in container first to avoid copy
			std::pair<typename VarType::iterator, bool> insertRet = field.insert(
					std::pair<const std::basic_string<Ch>, T>(GetJsonStr(itMember->name), T()));
			// Check duplicate
			if (!insertRet.second) {
				resp.errDoc.ElementMapKeyDuplicated(GetJsonStr(itMember->name));
				return ErrorCode::kDeserializeMapKeyDuplicated;
			}

			// Element FromJson
			T &elemBuffer = insertRet.first->second;
			FromJsonReq elemReq(itMember->value, req.allocator,
								req.deserFlag, req.canMoveSrc, &elemBuffer, FDesc::NoneFlag);	// element desc is always default
			FromJsonResp elemResp(resp.errDoc);
			int ret = intf.FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				const std::basic_string<Ch> fieldName = GetJsonStr(itMember->name);
				field.erase(fieldName);
				resp.errDoc.ErrorInMap(fieldName);
				return ret;
			}
		}
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((field.empty()));
		return 0;
	}

	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE
	{
		VarType& field = *static_cast<VarType *>(pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(T, Encoding);
		for (typename VarType::iterator itField = field.begin(), itEnd = field.end(); itField != itEnd; ++itField)
		{
			intf.ShrinkAllocator(&itField->second);
		}
	}
};

/**
 * Serialization class of Object types
 */
template<class T, typename Alloc, typename Encoding>
class FSerializer<std::vector<T_Member<T, typename Encoding::Ch>, Alloc>, Encoding> : public SerializerInterface<Encoding> {
	typedef typename Encoding::Ch Ch;
	typedef T_Member<T, Ch> MemberType;
	typedef typename MemberType::ValType ValType;
	typedef std::vector<MemberType, Alloc> VarType;
public:
	IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);

	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != NULL);
		const VarType& field = *static_cast<const VarType *>(req.pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ValType, Encoding);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartObject());

		for (typename VarType::const_iterator itMember = field.begin(), itEnd = field.end(); itMember != itEnd; ++itMember)
		{
			const std::basic_string<Ch>& key = itMember->name;
			IJSTI_RET_WHEN_WRITE_FAILD(
					req.writer.Key(key.data(), static_cast<rapidjson::SizeType>(key.size())) );

			SerializeReq elemReq(req.writer, &(itMember->value), req.serFlag);
			IJSTI_RET_WHEN_NOT_ZERO(intf.Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(
				req.writer.EndObject(static_cast<rapidjson::SizeType>(field.size())) );
		return 0;
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsObject()), "object");

		assert(req.pFieldBuffer != NULL);
		VarType& field = *static_cast<VarType *>(req.pFieldBuffer);
		field.clear();
		// pField->shrink_to_fit();
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ValType, Encoding);

		// Alloc buffer
		field.resize(req.stream.MemberCount());
		size_t i = 0;
		for (typename rapidjson::GenericValue<Encoding>::MemberIterator itMember = req.stream.MemberBegin(), itMemberEnd = req.stream.MemberEnd();
			 itMember != itMemberEnd;
			 ++itMember, ++i)
		{
			assert(i < field.size());
			MemberType& memberBuf = field[i];
			memberBuf.name = GetJsonStr(itMember->name);
			ValType &elemBuffer = memberBuf.value;
			FromJsonReq elemReq(itMember->value, req.allocator,
								req.deserFlag, req.canMoveSrc, &elemBuffer, FDesc::NoneFlag);	// element desc is always default
			FromJsonResp elemResp(resp.errDoc);

			int ret = intf.FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				resp.errDoc.ErrorInMap(memberBuf.name);
				field.resize(i);
				return ret;
			}
		}
		assert(i == field.size());
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((field.empty()));
		return 0;
	}

	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE
	{
		VarType& field = *static_cast<VarType*>(pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ValType, Encoding);
		for (typename VarType::iterator itField = field.begin(), itEnd = field.end(); itField != itEnd; ++itField)
		{
			intf.ShrinkAllocator(&itField->value);
		}
	}
};

}	// namespace detail
}	// namespace ijst

#endif //IJST_TYPES_CONTAINER_HPP_INCLUDE_
