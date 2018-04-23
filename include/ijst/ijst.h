//
// Created by h46incon on 2017/9/19.
//

#ifndef IJST_HPP_INCLUDE_
#define IJST_HPP_INCLUDE_

#include "detail/detail.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cassert>		// assert
#include <cstddef>		// NULL, size_t, offsetof
#include <vector>
#include <string>

/**	========================================================================================
 *				Public Interface
 */

/** @file */

/** @defgroup IJST_CONFIG ijst configuration
 *  @brief Configuration macros for library features
 *
 *  Some features are configurable to adapt the library to a wide
 *  variety of platforms, environments and usage scenarios.  Most of the
 *  features can be configured in terms of overriden or predefined
 *  preprocessor macros at compile-time.
 *
 *  @note These macros should be given on the compiler command-line
 *        (where applicable)  to avoid inconsistent values when compiling
 *        different translation units of a single application.
 */

/**
 * @ingroup IJST_CONFIG
 *
 * ijst generator getter methods when using IJST_DEFINE_STRUCT_WITH_GETTER or IJST_DEFINE_VALUE_WITH_GETTER that names
 * are IJST_GETTER_PREFIX + FIELD_NAME. By default, the prefix is "get", e.g., a getter method could name get_int1().
 *
 * @see IJST_DEFINE_STRUCT_WITH_GETTER, IJST_DEFINE_VALUE_WITH_GETTER
 */
#ifndef IJST_GETTER_PREFIX
	#define IJST_GETTER_PREFIX get_
#endif

/**
 * @ingroup IJST_CONFIG
 *
 *  ijst use static object in function to implement singleton, but it's not thread-safe before C++11.
 *  If declare IJST_TRY_INIT_META_BEFORE_MAIN to 1, ijst will init the singleton's instance when
 *  a static member of template class initialization, that init before main() in many compilers to avoid thread-safety problem.
 *	The feature is enable default before C++11.
 */
#ifndef IJST_TRY_INIT_META_BEFORE_MAIN
	#if __cplusplus < 201103L
		#define IJST_TRY_INIT_META_BEFORE_MAIN		1
	#else
		#define IJST_TRY_INIT_META_BEFORE_MAIN		0
	#endif
#endif

/**
 * @ingroup IJST_CONFIG
 *
 *	By default, ijst uses assert() for errors that indicate a bug.
 *	User can override it by defining IJST_ASSERT(x) macro.
 */
#ifndef IJST_ASSERT
	#define IJST_ASSERT(x) assert(x)
#endif

/** @defgroup IJST_MACRO_API ijst macro API
 *  @brief macro API
 *
 */

/**
 * @ingroup IJST_MACRO_API
 *
 * ijst support extern template to speed up compilation. To enable this option,
 * set the macro to 1 before including ijst headers. User should declare
 * IJST_EXPLICIT_TEMPLATE in one of cpp files to instantiation the templates.
 *
 * @note require extern template support (since C++11) to enable this option
 *
 * @see IJST_EXPLICIT_TEMPLATE
 */
#ifndef IJST_EXTERN_TEMPLATE
	#define IJST_EXTERN_TEMPLATE	0
#endif

/**
 * @ingroup IJST_MACRO_API
 *
 * ijst support extern template to speed up compilation. User should declare
 * IJST_EXPLICIT_TEMPLATE in one of cpp files to instantiation the templates.
 *
 * @see IJST_EXTERN_TEMPLATE
 */
#ifndef IJST_EXPLICIT_TEMPLATE
	#define IJST_EXPLICIT_TEMPLATE	0
#endif

//! @brief Declare a ijst struct with specify encoding
//! @param ...			encoding, struct_name [,(field_type, field_name, json_key, field_desc)]*
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_GENERIC_STRUCT(...) \
    IJSTI_DEFINE_STRUCT_IMPL(IJSTI_PP_NFIELD(__VA_ARGS__), false, F, __VA_ARGS__)
//! @brief Declare a ijst struct with specify encoding with getter
//! @param ...			encoding, struct_name [,(field_type, field_name, json_key, field_desc)]*
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_GENERIC_STRUCT_WITH_GETTER(...) \
    IJSTI_DEFINE_STRUCT_IMPL(IJSTI_PP_NFIELD(__VA_ARGS__), false, T, __VA_ARGS__)
//! @brief Declare a ijst struct with specify encoding which represent a value instead of members insides a object
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_GENERIC_VALUE(encoding, stName, type, fName, desc)	\
    IJSTI_DEFINE_STRUCT_IMPL(1, true, F, encoding, stName, (type, fName, "", desc))
//! @brief Declare a ijst struct with specify encoding which represent a value instead of members insides a object with getter
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_GENERIC_VALUE_WITH_GETTER(encoding, stName, type, fName, desc)	\
    IJSTI_DEFINE_STRUCT_IMPL(1, true, T, encoding, stName, (type, fName, "", desc))

//! @brief Declare a ijst struct.
//! @param ...			struct_name [,(field_type, field_name, json_key, field_desc)]*
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_STRUCT(...) \
	IJST_DEFINE_GENERIC_STRUCT(::rapidjson::UTF8<>, __VA_ARGS__)
//! @brief Declare a ijst struct with getter.
//! @param ...			struct_name [,(field_type, field_name, json_key, field_desc)]*
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_STRUCT_WITH_GETTER(...) \
    IJST_DEFINE_GENERIC_STRUCT_WITH_GETTER(::rapidjson::UTF8<>, __VA_ARGS__)
//! @brief Declare a ijst struct which represent a value instead of members insides a object
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_VALUE(stName, type, fName, desc)	\
    IJST_DEFINE_GENERIC_VALUE(::rapidjson::UTF8<>, stName, type, fName, desc)
//! @brief Declare a ijst struct which represent a value instead of members insides a object with getter
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_VALUE_WITH_GETTER(stName, type, fName, desc)	\
    IJST_DEFINE_GENERIC_VALUE_WITH_GETTER(::rapidjson::UTF8<>, stName, type, fName, desc)

//! @brief Get status of field in obj.
//! @ingroup IJST_MACRO_API
#define IJST_GET_STATUS(obj, field)				obj._.GetStatus(& ((obj).field))
//! @brief Mark status of field in obj to FStatus::kValid.
//! @ingroup IJST_MACRO_API
#define IJST_MARK_VALID(obj, field)				obj._.MarkValid(& ((obj).field))
//! @brief Mark status of field in obj to FStatus::kNull.
//! @ingroup IJST_MACRO_API
#define IJST_MARK_NULL(obj, field)				obj._.MarkNull(& ((obj).field))
//! @brief Mark status of field in obj to FStatus::kMissing.
//! @ingroup IJST_MACRO_API
#define IJST_MARK_MISSING(obj, field)			obj._.MarkMissing(& ((obj).field))
//! @brief Set field in obj to val and mark it valid.
//! @ingroup IJST_MACRO_API
#define IJST_SET(obj, field, val)				obj._.Set((obj).field, (val))
//! @brief Set field in obj to val and mark it valid. Type of field and val must be same.
//! @ingroup IJST_MACRO_API
#define IJST_SET_STRICT(obj, field, val)		obj._.SetStrict((obj).field, (val))

//! @brief NULL definition. (NULL before C++11, nullptr else)
//! @ingroup IJST_MACRO_API
#define IJST_NULL				IJSTI_NULL
//! @brief Empty macro to mark a param is a output.
//! @ingroup IJST_MACRO_API
#define IJST_OUT
//! @brief Helper declare macro with comma.
//! @ingroup IJST_MACRO_API
#if __cplusplus < 201103L
	#define IJST_TYPE(...)			::ijst::detail::ArgumentType<void(__VA_ARGS__)>::type
#else
	#define IJST_TYPE(...)			decltype(__VA_ARGS__())
#endif

namespace ijst {
//! Field description.
struct FDesc {
	typedef unsigned int Mode;
	//! Field is optional.
	static const Mode Optional 			= 0x00000001;
	//! Field can be null.
	static const Mode Nullable 			= 0x00000002;
	//! Field's value is not default, e.g, int is not 0, string, vector, map is not empty
	static const Mode NotDefault 		= 0x00000004;
	// FDesc of Element (i.e. Nullable, NotDefault) inside container is hard to represent, has not plan to implement it
};

//! Field status.
struct FStatus {
public:
	enum E_ {
		kNotAField,
		kMissing,
		kNull,
		kValid,
	};
};
typedef FStatus::E_ EFStatus;

/**
 * @brief Serialization options about fields.
 *
 * Options can be combined by bitwise OR operator (|).
 */
struct SerFlag {
	typedef unsigned int Flag;
	//! does not set any option.
	static const Flag kNoneFlag							= 0x00000000;
	//! set if ignore fields with kMissing status.
	static const Flag kIgnoreMissing					= 0x00000001;
	//! set if ignore unknown fields, otherwise will serialize all unknown fields.
	static const Flag kIgnoreUnknown					= 0x00000002;
	//! set if ignore fields with kNull status.
	static const Flag kIgnoreNull						= 0x00000004;
};

/**
 * @brief Deserialization options about fields.
 *
 * Options can be combined by bitwise OR operator (|).
 */
struct DeserFlag {
	typedef unsigned int Flag;
	//! Does not set any option.
	static const Flag kNoneFlag							= 0x00000000;
	//! Set if return error when meet unknown fields, otherwise will keep all unknown fields.
	static const Flag kErrorWhenUnknown					= 0x00000001;
	//! Set if ignore unknown fields, otherwise will keep all unknown fields.
	static const Flag kIgnoreUnknown					= 0x00000002;
	//! Set if ignore field status, otherwise will check if field status is matched requirement
	static const Flag kNotCheckFieldStatus				= 0x00000004;
	/**
	 * @brief  Set if move resource (to unknown or T_raw fields, copy by default) from intermediate document when deserialize.
	 *
	 * This option will speed up deserialization. But the allocator document will not free
	 * before the deserialized object destroyed. And the nested object will use parent's allocator.
	 * Be careful when moving the nested object to another object (e.g, calling
	 * RValue copy constructor), because the parent will free allocator when destroy.
	 *
	 * @note	Using this option carefully
	 */
	static const Flag kMoveFromIntermediateDoc			= 0x00000008;
};

/**
 * @brief A virtual based class implement rapidjson::Handler concept.
 *
 * Using raw rapidjson::Handler concept have to make function as a template, which is not convince is some situations.
 *
 * @tparam CharType	character type of string
 *
 * @see HandlerWrapper
 */
template<typename CharType = char>
class HandlerBase {
public:
	typedef CharType Ch;

	virtual bool Null()= 0;
	virtual bool Bool(bool b)= 0;
	virtual bool Int(int i)= 0;
	virtual bool Uint(unsigned i)= 0;
	virtual bool Int64(int64_t i)= 0;
	virtual bool Uint64(uint64_t i)= 0;
	virtual bool Double(double d)= 0;
	// enabled via kParseNumbersAsStringsFlag, string is not null-terminated (use length)
	virtual bool RawNumber(const Ch* str, rapidjson::SizeType length, bool copy = false)= 0;
	virtual bool String(const Ch* str, rapidjson::SizeType length, bool copy = false)= 0;
	virtual bool StartObject()= 0;
	virtual bool Key(const Ch* str, rapidjson::SizeType length, bool copy = false)= 0;
	virtual bool EndObject(rapidjson::SizeType memberCount = 0)= 0;
	virtual bool StartArray()= 0;
	virtual bool EndArray(rapidjson::SizeType elementCount = 0)= 0;
};

/**
 * @brief A wrapper that convert raw rapidjson::Handler instance to derived class of GenericHandlerBase.
 *
 * @tparam Handler		rapidjson::Handler
 */
template<typename Handler>
class HandlerWrapper : public HandlerBase<typename Handler::Ch>
{
public:
	typedef typename Handler::Ch Ch;
	Handler& h;
	explicit HandlerWrapper(Handler& _h) : h(_h) {}

	bool Null() IJSTI_OVERRIDE
	{ return h.Null(); }
	bool Bool(bool b) IJSTI_OVERRIDE
	{ return h.Bool(b); }
	bool Int(int i) IJSTI_OVERRIDE
	{ return h.Int(i); }
	bool Uint(unsigned i) IJSTI_OVERRIDE
	{ return h.Uint(i); }
	bool Int64(int64_t i) IJSTI_OVERRIDE
	{ return h.Int64(i); }
	bool Uint64(uint64_t i) IJSTI_OVERRIDE
	{ return h.Uint64(i); }
	bool Double(double d) IJSTI_OVERRIDE
	{ return h.Double(d); }
	bool RawNumber(const Ch *str, rapidjson::SizeType length, bool copy = false) IJSTI_OVERRIDE
	{ return h.RawNumber(str, length, copy); }
	bool String(const Ch *str, rapidjson::SizeType length, bool copy = false) IJSTI_OVERRIDE
	{ return h.String(str, length, copy); }
	bool StartObject() IJSTI_OVERRIDE
	{ return h.StartObject(); }
	bool Key(const Ch *str, rapidjson::SizeType length, bool copy = false) IJSTI_OVERRIDE
	{ return h.Key(str, length, copy); }
	bool EndObject(rapidjson::SizeType memberCount = 0) IJSTI_OVERRIDE
	{ return h.EndObject(memberCount); }
	bool StartArray() IJSTI_OVERRIDE
	{ return h.StartArray(); }
	bool EndArray(rapidjson::SizeType elementCount = 0) IJSTI_OVERRIDE
	{ return h.EndArray(elementCount); }
};


//! Error codes.
struct ErrorCode {
	static const int kSucc 							= 0x00000000;
	static const int kDeserializeValueTypeError 	= 0x00001001;
	static const int kDeserializeSomeFieldsInvalid 	= 0x00001002;
	static const int kDeserializeParseFailed 		= 0x00001003;
	static const int kDeserializeValueIsDefault		= 0x00001004;
	static const int kDeserializeSomeUnknownMember	= 0x00001005;
	static const int kDeserializeMapKeyDuplicated	= 0x00001006;
	static const int kInnerError 					= 0x00002001;
	static const int kWriteFailed					= 0x00003001;
};

#define IJSTI_OPTIONAL_BASE_DEFINE(T)						\
	public:													\
		/** @brief Constructor */ 							\
		explicit Optional(T* _pVal) : m_pVal(_pVal) {}		\
		/** @brief Get holding pointer */ 					\
		T* Ptr() const { return m_pVal; }					\
	private:												\
		T* const m_pVal;

/**
 * @brief Helper for implementing getter chaining.
 *
 * @tparam T 		type
 * @tparam Enable	type for SFINAE
 *
 * @note	The specialized template for container is declared in "types_container.h",
 * 			which implements operator [].
 */
template <typename T, typename Enable = void>
class Optional
{
	typedef T ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
};

/**
 * @brief Specialization for ijst struct
 *
 * Specialization for ijst struct (defined via IJST_DEFINE_STRUCT and so on) of Optional template.
 * This specialization add operator->() for getter chaining.
 *
 * @tparam T	ijst struct type
 */
template <typename T>
class Optional<T, /*EnableIf*/ typename detail::HasType<typename T::_ijst_AccessorType>::Void >
{
	typedef T ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	/**
	 * @brief Get pointer
	 *
	 * @return 	valid instance when data is not null, invalid instance when data is null
	 */
	T* operator->() const
	{
		static T empty(false);
		if (m_pVal == IJST_NULL) {
			return &empty;
		}
		else {
			return m_pVal;
		}
	}
};

/**
 * @brief Meta information of field.
 *
 * @tparam CharType		character type of string
 *
 * @see MetaClassInfo
 */
 template<typename CharType = char>
struct MetaFieldInfo { // NOLINT
 	typedef CharType Ch;
	//! The index of this fields in the meta information in the class. (Fields are sorted by offset inside class)
	int index;
	//! Field description.
	FDesc::Mode desc;
	//! Field's offset inside class.
	std::size_t offset;
	//! Json name when (de)serialization.
	std::basic_string<Ch> jsonName;
	//! field name.
	std::string fieldName;
	//! @private private serializer interface.
	void* serializerInterface;		// type: detail::SerializerInterface<Encoding>*
};

/**
 * @brief Meta information of class.
 *
 * @tparam CharType		character type of string
 *
 * @see MetaFieldInfo
 */
 template <typename CharType = char>
class MetaClassInfo {
public:
	typedef CharType Ch;
	/**
	 * @brief Get meta information for ijst struct T.
	 *
	 * @tparam T 	ijst struct
	 * @return		MetaClassInfo instance
	 */
	template<typename T>
	static const MetaClassInfo& GetMetaInfo();

	/**
	 * @brief Find index of field by offset.
	 *
	 * @param offset 	field's offset
	 * @return 			index if offset found, -1 else
	 *
	 * @note log(FieldSize) complexity.
	 */
	int FindIndex(size_t offset) const
	{
		std::vector<size_t>::const_iterator it =
				detail::Util::BinarySearch(m_offsets.begin(), m_offsets.end(), offset, IntComp);
		if (it != m_offsets.end() && *it == offset) {
			return static_cast<int>(it - m_offsets.begin());
		}
		else {
			return -1;
		}
	}

	/**
	 * @brief Find meta information of filed by json name.
	 *
	 * @param name		field's json name
	 * @return			pointer of info if found, null else
	 *
	 * @note log(FieldSize) complexity.
	 */
	const MetaFieldInfo<Ch>* FindFieldByJsonName(const std::basic_string<Ch>& name) const
	{
		typename std::vector<NameMap>::const_iterator it =
				detail::Util::BinarySearch(m_nameMap.begin(), m_nameMap.end(), name, NameMapComp);
		if (it != m_nameMap.end() && (*it->pName) == name) {
			return it->metaField;
		}
		else {
			return IJST_NULL;
		}
	}

	//! Get meta information of all fields in class. The returned vector is sorted by offset.
	const std::vector<MetaFieldInfo<Ch> >& GetFieldsInfo() const { return fieldsInfo; }
	//! Get name of class.
	const std::string& GetClassName() const { return structName; }
	//! Get the offset of Accessor object.
	std::size_t GetAccessorOffset() const { return accessorOffset; }

private:
	friend class detail::MetaClassInfoSetter<CharType>;		// use CharType instead of Ch to make IDE happy
	template<typename T> friend class detail::MetaClassInfoTyped;
	MetaClassInfo() : accessorOffset(0), mapInited(false) { }

	MetaClassInfo(const MetaClassInfo&) IJSTI_DELETED;
	MetaClassInfo& operator=(MetaClassInfo) IJSTI_DELETED;

	struct NameMap {
		NameMap(const std::basic_string<Ch>* _pName, const MetaFieldInfo<Ch>* _metaField)
				: pName(_pName), metaField(_metaField) {}

		bool operator<(const NameMap &r) const
		{ return (*pName) < (*r.pName); }

		const std::basic_string<Ch>* pName;
		const MetaFieldInfo<Ch>* metaField;
	};

	static int NameMapComp(const NameMap &l, const std::basic_string<Ch> &name)
	{
		return l.pName->compare(name);
	}

	static int IntComp(size_t l, size_t r)
	{
		return (int)((long)l - (long)r);
	}

	std::vector<MetaFieldInfo<Ch> > fieldsInfo;
	std::string structName;
	std::size_t accessorOffset;

	std::vector<NameMap> m_nameMap;
	std::vector<size_t> m_offsets;

	bool mapInited;
};

/**	========================================================================================
 *				Inner Interface
 */
namespace detail {

	//! return Err::kWriteFailed if action return false
	#define IJSTI_RET_WHEN_WRITE_FAILD(action) 						\
		do { if(!(action)) return ErrorCode::kWriteFailed; } while (false)
	//! return if action return non-0
	#define IJSTI_RET_WHEN_NOT_ZERO(action) 						\
		do { int ret = (action); if(ret != 0) return (ret); } while (false)
	//! return error and set error doc when type mismatch
	#define IJSTI_RET_WHEN_TYPE_MISMATCH(checkCode, expType)			\
		if (!(checkCode)) {												\
			resp.errDoc.ElementTypeMismatch(expType, req.stream);		\
			return ErrorCode::kDeserializeValueTypeError;				\
		}
	//! return error and set error doc when value is default
	#define IJSTI_RET_WHEN_VALUE_IS_DEFAULT(checkCode)					\
		if (detail::Util::IsBitSet(req.fDesc, FDesc::NotDefault) 		\
				&& (checkCode)) {										\
            resp.errDoc.ElementValueIsDefault();						\
            return ErrorCode::kDeserializeValueIsDefault;				\
		}


	#if IJST_TRY_INIT_META_BEFORE_MAIN
		#define IJSTI_TRY_INIT_META_BEFORE_MAIN(T)			::ijst::detail::Singleton< T >::InitInstanceInGlobal();
	#else
		#define IJSTI_TRY_INIT_META_BEFORE_MAIN(T)
	#endif

	/**
	 * ArugmentType. Helper template to declare macro argument with comma
	 *
	 * @see https://stackoverflow.com/questions/13842468/comma-in-c-c-macro/13842784
	 */
	template<typename T>
	struct ArgumentType;
	template<typename T, typename U>
	struct ArgumentType<U(T)> {typedef T type;};

	template<typename Encoding>
	class SerializerInterface {
	public:
		typedef typename Encoding::Ch Ch;
		virtual ~SerializerInterface() { }

		struct SerializeReq {
			// Serialize option about fields
			SerFlag::Flag serFlag;

			// Pointer of field to serialize.
			// The actual type of field should be decide in the derived class
			const void* pField;

			HandlerBase<Ch>& writer;

			SerializeReq(HandlerBase<Ch>& _writer, const void *_pField, SerFlag::Flag _serFlag)
					: serFlag(_serFlag)
					, pField(_pField)
					, writer(_writer)
			{ }
		};

		virtual int Serialize(const SerializeReq &req) = 0;

		struct FromJsonReq {
			// Pointer of deserialize output.
			// The instance should deserialize in this object
			// The actual type of field should be decide in the derived class
			void* pFieldBuffer;

			// The input stream and allocator
			// The stream maybe cast from const value if canMoveSrc is false
			rapidjson::GenericValue<Encoding>& stream;
			JsonAllocator& allocator;

			// true if move context in stream to avoid copy when possible
			bool canMoveSrc;

			DeserFlag::Flag deserFlag;
			FDesc::Mode fDesc;

			FromJsonReq(rapidjson::GenericValue<Encoding>& _stream, JsonAllocator& _allocator,
						DeserFlag::Flag _deserFlag, bool _canMoveSrc,
						void* _pField, FDesc::Mode _fDesc)
					: pFieldBuffer(_pField)
					, stream(_stream)
					, allocator(_allocator)
					, canMoveSrc(_canMoveSrc)
					, deserFlag(_deserFlag)
					, fDesc(_fDesc)
			{ }
		};

		struct FromJsonResp {
			ErrorDocSetter<Encoding>& errDoc;

			explicit FromJsonResp(ErrorDocSetter<Encoding>& _errDoc) :
					errDoc(_errDoc)
			{ }

		};

		virtual int FromJson(const FromJsonReq& req, IJST_OUT FromJsonResp& resp)= 0;

		virtual void ShrinkAllocator(void * pField)
		{ (void)pField; }
	};

	//! Propagate structs' define in SerializeInterface<Encoding>
	#define IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding)										\
		typedef typename SerializerInterface<Encoding>::SerializeReq SerializeReq;			\
		typedef typename SerializerInterface<Encoding>::FromJsonReq FromJsonReq;			\
		typedef typename SerializerInterface<Encoding>::FromJsonResp FromJsonResp;

	/**
	 * Template interface of serialization class
	 * This template is unimplemented, and will throw a compile error when use it.
	 *
	 * @tparam T 		class
	 * @tparam Encoding	encoding of json struct
	 * @tparam Enable	type for SFINAE
	 */
	template<typename T, typename Encoding, typename Enable = void>
	class FSerializer : public SerializerInterface<Encoding> {
	public:
#if __cplusplus >= 201103L
		static_assert(!std::is_same<T, T>::value,	// always failed
					  "This base template should not be instantiated. (Maybe use wrong param when define ijst struct)");
#endif
		typedef void VarType;
		IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);

		virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE = 0;
		virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE = 0;
		virtual void ShrinkAllocator(void * pField) IJSTI_OVERRIDE { (void)pField; }
	};

	#define IJSTI_FSERIALIZER_INS(T, Encoding) 		\
		::ijst::detail::Singleton< ::ijst::detail::FSerializer< T, Encoding > >::GetInstance()

	/**
	 * Get and cast serializerInterface in metaFieldInfo to specify type
	 *
	 * @tparam Encoding 		encoding of output
	 *
	 * @param metaFieldInfo 	metaFieldInfo
	 * @return 					typed SerializerInterface
	 */
	template<typename Encoding>
	SerializerInterface<Encoding>* GetSerializerInterface(const MetaFieldInfo<typename Encoding::Ch>& metaFieldInfo)
	{
		return reinterpret_cast<SerializerInterface<Encoding>*>(metaFieldInfo.serializerInterface);
	}

	/**	========================================================================================
	 *				Private
	 */
	/**
	 * MetaClassInfo of T
	 * Push meta class info of T in specialized constructor MetaInfo<T>().
	 *
	 * @tparam T 	class. Concept require T::_ijst_InitMetaInfo<bool>(MetaInfo*), typedef T::_ijst_Ch
	 *
	 * @note		Use Singleton<MetaClassInfoTyped<T> > to get the instance
	 */
	template<typename T>
	class MetaClassInfoTyped {
	public:
		MetaClassInfo<typename T::_ijst_Ch> metaClass;

	private:
		friend class Singleton<MetaClassInfoTyped<T> >;

		MetaClassInfoTyped()
		{
			T::template _ijst_InitMetaInfo<true>(this);
		}
	};

	template<typename CharType>
	class MetaClassInfoSetter {
	public:
		typedef CharType Ch;
		explicit MetaClassInfoSetter(MetaClassInfo<Ch>& _d) : d(_d) { }

		void InitBegin(const std::string& _tag, std::size_t _fieldCount, std::size_t _accessorOffset)
		{
			d.structName = _tag;
			d.accessorOffset = _accessorOffset;
			d.fieldsInfo.reserve(_fieldCount);
		}

		void PushMetaField(const std::string &fieldName, const std::basic_string<Ch>& jsonName,
						   std::size_t offset, FDesc::Mode desc, void* pSerializeInterface)
		{
			MetaFieldInfo<Ch> metaField;
			metaField.jsonName = jsonName;
			metaField.fieldName = fieldName;
			metaField.offset = offset;
			metaField.desc = desc;
			metaField.serializerInterface = pSerializeInterface;
			d.fieldsInfo.push_back(IJSTI_MOVE(metaField));
		}

		void InitEnd()
		{
			// assert MetaClassInfo's map has not inited before
			assert(!d.mapInited);
			SortMetaFieldsByOffset();

			d.m_offsets.reserve(d.fieldsInfo.size());
			d.m_nameMap.resize(d.fieldsInfo.size(), typename MetaClassInfo<Ch>::NameMap(IJSTI_NULL, IJSTI_NULL));

			for (size_t i = 0; i < d.fieldsInfo.size(); ++i) {
				MetaFieldInfo<Ch>* ptrMetaField = &(d.fieldsInfo[i]);
				ptrMetaField->index = static_cast<int>(i);

				d.m_offsets.push_back(ptrMetaField->offset);
				// Assert field offset is sorted and not exist before
				assert(i == 0 || d.m_offsets[i]  > d.m_offsets[i-1]);

				// Insert name Map
				InsertNameMap(i, typename MetaClassInfo<Ch>::NameMap(&(ptrMetaField->jsonName), ptrMetaField));
			}

			d.mapInited = true;
		}

	private:
		void SortMetaFieldsByOffset()
		{
			// fieldsInfo is already sorted in most case, use insertion sort
			const size_t n = d.fieldsInfo.size();
			for (size_t i = 1; i < n; i++) {
				for (size_t j = i; j > 0 && d.fieldsInfo[j - 1].offset > d.fieldsInfo[j].offset; j--) {
					detail::Util::Swap(d.fieldsInfo[j], d.fieldsInfo[j - 1]);
				}
			}
		}

		void InsertNameMap(size_t len, const typename MetaClassInfo<Ch>::NameMap& v)
		{
			typename std::vector<typename MetaClassInfo<Ch>::NameMap>::iterator it =
					Util::BinarySearch(d.m_nameMap.begin(), d.m_nameMap.begin() + len, *v.pName, MetaClassInfo<Ch>::NameMapComp);
			size_t i = static_cast<size_t>(it - d.m_nameMap.begin());
			// assert name is unique
			assert(i == len || (*v.pName) != (*it->pName));

			for (size_t j = len; j > i; --j) {
				d.m_nameMap[j] = IJSTI_MOVE(d.m_nameMap[j - 1]);
			}
			d.m_nameMap[i] = v;
		}

		MetaClassInfo<Ch>& d;
	};

	/**
	 * Serialization of ijst struct types
	 * @tparam T 			class
	 * @tparam Encoding		encoding of json struct
	 */
	template<class T, typename Encoding>
	class FSerializer<T, Encoding, /*EnableIf*/ typename HasType<typename T::_ijst_AccessorType>::Void>
	        : public SerializerInterface<Encoding>
	{
	public:
		typedef T VarType;
		IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);

		virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
		{
			T *pField = (T *) req.pField;
			return pField->_.ISerialize(req);
		}

		virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
		{
			T *pField = (T *) req.pFieldBuffer;
			return pField->_.IFromJson(req, resp);
		}

		virtual void ShrinkAllocator(void *pField) IJSTI_OVERRIDE
		{
			((T*)pField)->_.IShrinkAllocator(pField);
		}
	};

}	// namespace detail

/**
 * @brief Accessor of ijst struct
 *
 * @tparam Encoding		Encoding for json struct
 *
 * User can access and modify fields, serialize and deserialize of a structure via it.
 */
template<typename Encoding = rapidjson::UTF8<> >
class Accessor {
public:
	typedef typename Encoding::Ch Ch;
	//! Constructor
	explicit Accessor(const MetaClassInfo<Ch>* pMetaClass, bool isParentVal, bool isValid) :
			m_pMetaClass(pMetaClass), m_isValid(isValid), m_isParentVal(isParentVal)
	{
		IJST_ASSERT(!m_isParentVal || m_pMetaClass->GetFieldsInfo().size() == 1);
		m_r = static_cast<Resource *>(operator new(sizeof(Resource)));
		new(&m_r->fieldStatus) FieldStatusType(m_pMetaClass->GetFieldsInfo().size(), FStatus::kMissing);
		new(&m_r->unknown)TValue(rapidjson::kObjectType);
		new(&m_r->ownDoc) TDocument();
		m_pAllocator = &m_r->ownDoc.GetAllocator();
		InitOuterPtr();
	}

	//! Copy constructor
	Accessor(const Accessor &rhs)
	{
		assert(this != &rhs);

		m_isValid = rhs.m_isValid;
		m_isParentVal = rhs.m_isParentVal;

		m_r = static_cast<Resource *>(operator new(sizeof(Resource)));
		new(&m_r->fieldStatus)FieldStatusType(rhs.m_r->fieldStatus);
		new(&m_r->unknown)TValue(rapidjson::kObjectType);
		new(&m_r->ownDoc) TDocument();
		m_pAllocator = &m_r->ownDoc.GetAllocator();

		m_pMetaClass = rhs.m_pMetaClass;
		InitOuterPtr();

		m_r->unknown.CopyFrom(rhs.m_r->unknown, *m_pAllocator);
	}

	#if __cplusplus >= 201103L
	//! Move copy constructor
	Accessor(Accessor &&rhs) IJSTI_NOEXCEPT
	{
		m_r = IJSTI_NULL;
		Steal(rhs);
	}
	#endif

	//! Assigment
	Accessor &operator=(Accessor rhs)
	{
		Steal(rhs);
		return *this;
	}

	//! Destructor
	~Accessor() IJSTI_NOEXCEPT
	{
		delete m_r;
		m_r = IJSTI_NULL;
	}

	//! Stealer
	void Steal(Accessor &rhs)
	{
		if (this == &rhs) {
			return;
		}

		// Handler resource
		delete m_r;
		m_r = rhs.m_r;
		rhs.m_r = IJSTI_NULL;

		// other simple field
		m_pMetaClass = rhs.m_pMetaClass;
		rhs.m_pMetaClass = IJST_NULL;
		m_pAllocator = rhs.m_pAllocator;
		rhs.m_pAllocator = IJST_NULL;

		m_isValid = rhs.m_isValid;
		m_isParentVal = rhs.m_isParentVal;

		InitOuterPtr();
	}

	inline bool IsValid() const { return m_isValid; }
	inline bool IsParentVal() const { return m_isParentVal; }
	const MetaClassInfo<Ch>& GetMetaInfo() const { return *m_pMetaClass; }

	/*
	 * Field accessor.
	 */

	//! Check if pField is a filed in this object.
	inline bool HasField(const void *pField) const
	{
		size_t offset = GetFieldOffset(pField);
		return (m_pMetaClass->FindIndex(offset) != -1);
	}

	//! Set field to val and mark it valid.
	template<typename T1, typename T2>
	inline void Set(T1 &field, const T2 &value)
	{
		MarkValid(&field);
		field = value;
	}

	//! Set field to val and mark it valid. The type of field and value must be same.
	template<typename T>
	inline void SetStrict(T &field, const T &value)
	{
		Set(field, value);
	}

	//! Mark status of field to FStatus::kValid.
	inline void MarkValid(const void* pField) { MarkFieldStatus(pField, FStatus::kValid); }
	//! Mark status of field to FStatus::kNull.
	inline void MarkNull(const void* pField) { MarkFieldStatus(pField, FStatus::kNull); }
	//! Mark status of field to FStatus::kMissing.
	inline void MarkMissing(const void* pField) { MarkFieldStatus(pField, FStatus::kMissing); }

	//! Get status of field.
	inline EFStatus GetStatus(const void *pField) const
	{
		const size_t offset = GetFieldOffset(pField);
		const int index = m_pMetaClass->FindIndex(offset);
		return index == -1 ? FStatus::kNotAField : m_r->fieldStatus[index];
	}

	//! Get unknwon fields
	inline rapidjson::GenericValue<Encoding> &GetUnknown() { return m_r->unknown; }
	inline const rapidjson::GenericValue<Encoding> &GetUnknown() const { return m_r->unknown; }

	/**
	 * @brief Get allocator used in object.
	 *
	 * The inner allocator is own allocator when init,
	 * but may change to other allocator when calling SetMembersAllocator() or Deserialize().
	 */
	inline rapidjson::MemoryPoolAllocator<> &GetAllocator() { return *m_pAllocator; }
	inline const rapidjson::MemoryPoolAllocator<> &GetAllocator() const { return *m_pAllocator; }

	/**
	 * @brief Get own allocator that used to manager resource.
	 *
	 * User could use the returned value to check if this object use outer allocator.
	 */
	inline rapidjson::MemoryPoolAllocator<> &GetOwnAllocator() { return m_r->ownDoc.GetAllocator(); }
	inline const rapidjson::MemoryPoolAllocator<> &GetOwnAllocator() const { return m_r->ownDoc.GetAllocator(); }

	/**
	 * @brief Get Optional wrapper of field
	 *
	 * @tparam T 		Field type
	 * @param field 	Field reference in the parent object
	 * @return 			Optional(&field) if struct and field is valid, Optional(nullptr) else
	 */
	template <typename T>
	Optional<const T> GetOptional(const T& field) const
	{
		IJST_ASSERT(HasField(&field));
		if (m_isValid && GetStatus(&field) == ijst::FStatus::kValid) {
			return ::ijst::Optional<const T>(&field);
		}
		else {
			return ::ijst::Optional<const T>(IJSTI_NULL);
		}
	}

	/**
	 * @brief Get Optional wrapper of field
	 *
	 * @tparam T 		Field type
	 * @param field 	Field reference in the parent object
	 * @return 			Optional(&field) if struct and field is valid, Optional(nullptr) else
	 */
	template <typename T>
	Optional<T> GetOptional(T& field)
	{
		IJST_ASSERT(HasField(&field));
		if (m_isValid && GetStatus(&field) == ijst::FStatus::kValid) {
			return ::ijst::Optional<T>(&field);
		}
		else {
			return ::ijst::Optional<T>(IJSTI_NULL);
		}
	}

	/**
	 * @brief Serialize the structure to string.
	 *
	 * @param writer 		writer
	 * @param serFlag	 	Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return				Error code
	 */
	int Serialize(HandlerBase<Ch>& writer, SerFlag::Flag serFlag = SerFlag::kNoneFlag)  const
	{
		return DoSerialize(writer, serFlag);
	}

	/**
	 * @brief Serialize the structure to string.
	 *
	 * @param strOutput 	The output of result
	 * @param serFlag 		Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return				Error code
	 */
	int Serialize(IJST_OUT std::basic_string<Ch> &strOutput, SerFlag::Flag serFlag = SerFlag::kNoneFlag)  const
	{
		TStringBuffer buffer;
		rapidjson::Writer<TStringBuffer> writer(buffer);
		HandlerWrapper<rapidjson::Writer<TStringBuffer> > writerWrapper(writer);
		IJSTI_RET_WHEN_NOT_ZERO(DoSerialize(writerWrapper, serFlag));

		strOutput = std::basic_string<Ch>(buffer.GetString(), buffer.GetSize() / sizeof(typename TStringBuffer::Ch));
		return 0;
	}

	/**
	 * @brief Deserialize from C-style string.
	 *
	 * @tparam parseFlags		parseFlags of rapidjson parse method
	 * @tparam SourceEncoding	encoding of source
	 *
	 * @param cstrInput			Input C string
	 * @param length			Length of string
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note The input string can contain '\0'
	 */
	template <unsigned parseFlags, typename SourceEncoding>
	int Deserialize(const typename SourceEncoding::Ch *cstrInput, std::size_t length,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::GenericDocument<Encoding> *pErrDocOut = IJST_NULL)
	{
		//! helper
#define IJSTI_PARSE_AND_RET_WHEN_ERROR									\
	do {																\
		doc.template Parse<parseFlags, SourceEncoding>(					\
				cstrInput, length);										\
		if (doc.HasParseError()) {										\
			detail::ErrorDocSetter<Encoding> errDocSetter(pErrDocOut);	\
			errDocSetter.ParseFailed(doc.GetParseError());				\
			return ErrorCode::kDeserializeParseFailed;					\
		}																\
	} while (false)

		// The new object will call FromJson() interfaces soon in most situation
		// So clear own allocator will not bring much benefit
		m_pAllocator = &m_r->ownDoc.GetAllocator();

		if (detail::Util::IsBitSet(deserFlag, DeserFlag::kMoveFromIntermediateDoc)) {
			TDocument doc(m_pAllocator);
			IJSTI_PARSE_AND_RET_WHEN_ERROR;
			return DoFromJsonWrap<TValue>(&Accessor::DoMoveFromJson, doc, deserFlag, pErrDocOut);
		}
		else {
			TDocument doc;
			IJSTI_PARSE_AND_RET_WHEN_ERROR;
			return DoFromJsonWrap<const TValue >(&Accessor::DoFromJson, doc, deserFlag, pErrDocOut);
		}

#undef IJSTI_PARSE_AND_RET_WHEN_ERROR
	}

	/**
	 * @brief Deserialize from C-style string.
	 *
	 * @tparam parseFlags		parseFlags of rapidjson parse method
	 *
	 * @param cstrInput			Input C string
	 * @param length			Length of string
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note The input string can contain '\0'
	 */
	template <unsigned parseFlags>
	int Deserialize(const Ch *cstrInput, std::size_t length,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::GenericDocument<Encoding> *pErrDocOut = IJST_NULL)
	{
		return this->template Deserialize<parseFlags, Encoding>(
				cstrInput, length, deserFlag, pErrDocOut);
	}

	/**
	 * @brief Deserialize from C-style string.
	 *
	 * @param cstrInput			Input C string
	 * @param length			Length of string
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note The input string can contain '\0'
	 */
	int Deserialize(const Ch *cstrInput, std::size_t length,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::GenericDocument<Encoding> *pErrDocOut = IJST_NULL)
	{
		return this->template Deserialize<rapidjson::kParseDefaultFlags>(
				cstrInput, length, deserFlag, pErrDocOut);
	}

	/**
	 * @brief Deserialize from std::basic_string.
	 *
	 * @param strInput			Input string
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 */
	inline int Deserialize(const std::basic_string<Ch> &strInput,
						   DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
						   rapidjson::GenericDocument<Encoding> *pErrDocOut = IJST_NULL)
	{
		return Deserialize(strInput.data(), strInput.size(), deserFlag, pErrDocOut);
	}

	/**
	 * @brief Deserialize from std::basic_string.
	 *
	 * @param strInput			Input string
	 * @param errMsgOut			Error message output
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @return					Error code
	 */
	inline int Deserialize(const std::basic_string<Ch> &strInput, IJST_OUT std::basic_string<Ch>& errMsgOut,
						   DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag)
	{
		TDocument errDoc;
		int ret = Deserialize(strInput.data(), strInput.size(), deserFlag, &errDoc);
		if (ret != 0) {
			TStringBuffer sb;
			rapidjson::Writer<TStringBuffer> writer(sb);
			errDoc.Accept(writer);
			errMsgOut = std::basic_string<Ch>(sb.GetString(), sb.GetSize() / sizeof(typename TStringBuffer::Ch));
		}
		return ret;
	}

	/**
	 * @brief Deserialize from json object.
	 *
	 * @param srcJson			Input json object
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note 	It will not copy const string reference in source json. Be careful if handler such situation,
	 * 			e.g, json object is generated by ParseInsitu().
	 */
	inline int FromJson(const rapidjson::GenericValue<Encoding> &srcJson,
						DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
						rapidjson::GenericDocument<Encoding> *pErrDocOut = IJST_NULL)
	{
		m_pAllocator = &m_r->ownDoc.GetAllocator();
		return DoFromJsonWrap<const TValue>(&Accessor::DoFromJson, srcJson, deserFlag, pErrDocOut);
	}

	/**
	 * @brief Move deserialize form json document
	 *
	 * Deserialize from json document. The source object may be stolen after deserialize.
	 * Because the accessor need manager the input allocator, but the Allocator class has no Swap() interface,
	 * so use document object instead.
	 *
	 * @param srcDocStolen		Input document object
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note 	The source document may be changed after deserialize
	 * @note 	Make sure srcDocStolen use own allocator, or use allocator in this object
	 * @note	Handler allocator of nested object carefully. see DeserFlag::kMoveFromIntermediateDoc.
	 * @note 	It will not copy const string reference in source json. Be careful if handler such situation,
	 * 			e.g, json object is generated by ParseInsitu().
	 *
	 * @see DeserFlag::kMoveFromIntermediateDoc
	 */
	inline int MoveFromJson(rapidjson::GenericDocument<Encoding> &srcDocStolen,
							DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
							rapidjson::GenericDocument<Encoding> *pErrDocOut = IJST_NULL)
	{
		// Store document to manager allocator
		m_r->ownDoc.Swap(srcDocStolen);
		m_pAllocator = &m_r->ownDoc.GetAllocator();
		return DoFromJsonWrap<TValue>(&Accessor::DoMoveFromJson, m_r->ownDoc, deserFlag, pErrDocOut);
	}

	/**
	 * @brief Shrink allocator of each member by recopy unknown fields using own allocator
	 *
	 * The allocator of source json after move deserialize. The memory is wasted if the
	 * deserialized object exists in long time.
	 *
	 * @see DeserFlag::kMoveFromIntermediateDoc, MoveFromJson
	 */
	inline void ShrinkAllocator()
	{
		DoShrinkAllocator();
	}

private:
	typedef rapidjson::GenericDocument<Encoding> TDocument;
	typedef rapidjson::GenericValue<Encoding> TValue;
	typedef rapidjson::GenericStringBuffer<Encoding> TStringBuffer;
	typedef MetaFieldInfo<Ch> TMetaFieldInfo;
	typedef MetaClassInfo<Ch> TMetaClassInfo;

	// #region Implement SerializeInterface
	template <typename, typename> friend class detail::FSerializer;
	typedef typename detail::SerializerInterface<Encoding>::SerializeReq SerializeReq;
	inline int ISerialize(const SerializeReq &req) const
	{
		assert(req.pField == this);
		return DoSerialize(req.writer, req.serFlag);
	}

	typedef typename detail::SerializerInterface<Encoding>::FromJsonReq FromJsonReq;
	typedef typename detail::SerializerInterface<Encoding>::FromJsonResp FromJsonResp;
	struct FromJsonParam{
		DeserFlag::Flag deserFlag;
		detail::ErrorDocSetter<Encoding>& errDoc;

		FromJsonParam(DeserFlag::Flag _deserFlag, detail::ErrorDocSetter<Encoding>& _errDoc)
				: deserFlag(_deserFlag), errDoc(_errDoc)
		{}
	};

	inline int IFromJson(const FromJsonReq &req, IJST_OUT FromJsonResp& resp)
	{
		assert(req.pFieldBuffer == this);

		FromJsonParam param(req.deserFlag, resp.errDoc);
		if (req.canMoveSrc) {
			m_pAllocator = &req.allocator;
			return DoMoveFromJson(req.stream, param);
		}
		else {
			m_pAllocator = &m_r->ownDoc.GetAllocator();
			return DoFromJson(req.stream, param);
		}
	}

	inline void IShrinkAllocator(void* pField)
	{
		(void)pField;
		assert(pField == this);
		DoShrinkAllocator();
	}

	// #endregion

	//! Serialize to string using SAX API
	int DoSerialize(HandlerBase<Ch> &writer, SerFlag::Flag serFlag) const
	{
		rapidjson::SizeType fieldCount = 0;
		if (m_isParentVal) {
			return DoSerializeFields(writer, serFlag, fieldCount);
			// Unknown will be ignored
		}

		IJSTI_RET_WHEN_WRITE_FAILD(writer.StartObject());

		// Write fields
		IJSTI_RET_WHEN_NOT_ZERO(DoSerializeFields(writer, serFlag, fieldCount));

		// Write buffer if need
		if (!detail::Util::IsBitSet(serFlag, SerFlag::kIgnoreUnknown))
		{
			assert(m_r->unknown.IsObject());
			for (typename TValue::ConstMemberIterator itMember = m_r->unknown.MemberBegin();
				 itMember != m_r->unknown.MemberEnd(); ++itMember)
			{
				// Write key
				const TValue& key = itMember->name;
				IJSTI_RET_WHEN_WRITE_FAILD(
						writer.Key(key.GetString(), key.GetStringLength()) );
				// Write value
				IJSTI_RET_WHEN_WRITE_FAILD(
						itMember->value.Accept(writer) );
			}

			fieldCount += m_r->unknown.MemberCount();
		}

		IJSTI_RET_WHEN_WRITE_FAILD(writer.EndObject(fieldCount));
		return 0;
	}

	int DoSerializeFields(HandlerBase<Ch> &writer, SerFlag::Flag serFlag, IJST_OUT rapidjson::SizeType& fieldCountOut) const
	{
		IJST_ASSERT(!m_isParentVal || m_pMetaClass->GetFieldsInfo().size() == 1);
		for (typename std::vector<TMetaFieldInfo>::const_iterator itMetaField = m_pMetaClass->GetFieldsInfo().begin();
			 itMetaField != m_pMetaClass->GetFieldsInfo().end(); ++itMetaField)
		{
			// Check field state
			const EFStatus fstatus = m_r->fieldStatus[itMetaField->index];
			switch (fstatus) {
				case FStatus::kMissing:
					if (detail::Util::IsBitSet(serFlag, SerFlag::kIgnoreMissing)) {
						continue;
					}
					// Fall through
				case FStatus::kValid:
				{
					const void *pFieldValue = GetFieldByOffset(itMetaField->offset);
					if (!m_isParentVal) {
						// write key
						IJSTI_RET_WHEN_WRITE_FAILD(
								writer.Key(itMetaField->jsonName.data(), (rapidjson::SizeType)itMetaField->jsonName.size()) );
					}
					// write value
					SerializeReq req(writer, pFieldValue, serFlag);
					IJSTI_RET_WHEN_NOT_ZERO(
							detail::GetSerializerInterface<Encoding>(*itMetaField)->Serialize(req));
					++fieldCountOut;
				}
					break;

				case FStatus::kNull:
				{
					if (detail::Util::IsBitSet(serFlag, SerFlag::kIgnoreNull)) {
						continue;
					}

					if (!m_isParentVal) {
						// write key
						IJSTI_RET_WHEN_WRITE_FAILD(
								writer.Key(itMetaField->jsonName.data(), (rapidjson::SizeType)itMetaField->jsonName.size()) );
					}
					// write value
					IJSTI_RET_WHEN_WRITE_FAILD(writer.Null());
					++fieldCountOut;
				}
					break;

				case FStatus::kNotAField:
				default:
					// Error occurs
					assert(false);
					return ErrorCode::kInnerError;
			}
		}
		return 0;
	}

	template<typename TJsonValue, typename Func>
	int DoFromJsonWrap(Func func, TJsonValue &stream, DeserFlag::Flag deserFlag, TDocument* pErrDocOut)
	{
		detail::ErrorDocSetter<Encoding> errDoc(pErrDocOut);
		FromJsonParam param(deserFlag, errDoc);
		return (this->*func)(stream, param);
	}

	/**
	 * Deserialize move from json object
	 * @note Make sure the lifecycle of allocator of the stream is longer than this object
	 */
	int DoMoveFromJson(TValue &stream, FromJsonParam& p)
	{
		if (m_isParentVal) {
			// Set field by stream itself
			assert(m_pMetaClass->GetFieldsInfo().size() == 1);
			return DoFieldFromJson(
					&m_pMetaClass->GetFieldsInfo()[0], stream, /*canMoveSrc=*/true, p);
		}

		// Set fields by members of stream
		if (!stream.IsObject()) {
			p.errDoc.ElementTypeMismatch("object", stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		// For each member
		typename TValue::MemberIterator itNextRemain = stream.MemberBegin();
		for (typename TValue::MemberIterator itMember = stream.MemberBegin();
			 itMember != stream.MemberEnd(); ++itMember)
		{

			// Get related field info
			const std::basic_string<Ch> jsonKeyName(itMember->name.GetString(), itMember->name.GetStringLength());
			const TMetaFieldInfo *pMetaField = m_pMetaClass->FindFieldByJsonName(jsonKeyName);

			if (pMetaField == IJST_NULL) {
				// Not a field in struct
				if (detail::Util::IsBitSet(p.deserFlag, DeserFlag::kErrorWhenUnknown)) {
					p.errDoc.UnknownMember(jsonKeyName);
					return ErrorCode::kDeserializeSomeUnknownMember;
				}
				if (!detail::Util::IsBitSet(p.deserFlag, DeserFlag::kIgnoreUnknown)) {
					// Move unknown fields to the front of array first
					// TODO: This is relay on the implementation details of rapidjson's object storage (array), how to check?
					if (itNextRemain != itMember) {
						itNextRemain->name.SetNull().Swap(itMember->name);
						itNextRemain->value.SetNull().Swap(itMember->value);
					}
					++itNextRemain;
				}
				continue;
			}

			// Move member out of object
			TValue memberStream(rapidjson::kNullType);
			memberStream.Swap(itMember->value);

			IJSTI_RET_WHEN_NOT_ZERO(
					DoFieldFromJson(pMetaField, memberStream, /*canMoveSrc=*/true, p) );
		}

		// Clean deserialized
		if (stream.MemberCount() != 0) {
			stream.EraseMember(itNextRemain, stream.MemberEnd());
		}
		if (detail::Util::IsBitSet(p.deserFlag, DeserFlag::kIgnoreUnknown)) {
			m_r->unknown.SetObject();
		}
		else {
			m_r->unknown.SetNull().Swap(stream);
		}

		if (!detail::Util::IsBitSet(p.deserFlag, DeserFlag::kNotCheckFieldStatus)) {
			return CheckFieldState(p.errDoc);
		}
		else {
			return 0;
		}
	}

	//! Deserialize from stream
	int DoFromJson(const TValue &stream, FromJsonParam& p)
	{
		if (m_isParentVal) {
			// Serialize field by stream itself
			assert(m_pMetaClass->GetFieldsInfo().size() == 1);
			return DoFieldFromJson(
					&m_pMetaClass->GetFieldsInfo()[0], const_cast<TValue &>(stream), /*canMoveSrc=*/true, p);
		}

		// Serialize fields by members of stream
		if (!stream.IsObject()) {
			p.errDoc.ElementTypeMismatch("object", stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		m_r->unknown.SetObject();
		// For each member
		for (typename TValue::ConstMemberIterator itMember = stream.MemberBegin();
			 itMember != stream.MemberEnd(); ++itMember)
		{
			// Get related field info
			const std::basic_string<Ch> jsonKeyName(itMember->name.GetString(), itMember->name.GetStringLength());
			const TMetaFieldInfo *pMetaField = m_pMetaClass->FindFieldByJsonName(jsonKeyName);

			if (pMetaField == IJST_NULL) {
				// Not a field in struct
				if (detail::Util::IsBitSet(p.deserFlag, DeserFlag::kErrorWhenUnknown)) {
					p.errDoc.UnknownMember(jsonKeyName);
					return ErrorCode::kDeserializeSomeUnknownMember;
				}
				if (!detail::Util::IsBitSet(p.deserFlag, DeserFlag::kIgnoreUnknown)) {
					m_r->unknown.AddMember(
							TValue().SetString(jsonKeyName.data(), (rapidjson::SizeType)jsonKeyName.size(), *m_pAllocator),
							TValue().CopyFrom(itMember->value, *m_pAllocator),
							*m_pAllocator
					);
				}
				continue;
			}

			TValue& memberStream = const_cast<TValue&>(itMember->value);
			IJSTI_RET_WHEN_NOT_ZERO(
					DoFieldFromJson(pMetaField, memberStream, /*canMoveSrc=*/false, p) );
		}

		if (!detail::Util::IsBitSet(p.deserFlag, DeserFlag::kNotCheckFieldStatus)) {
			return CheckFieldState(p.errDoc);
		}
		else {
			return 0;
		}
	}


	int DoFieldFromJson(const TMetaFieldInfo* metaField, TValue &stream, bool canMoveSrc, FromJsonParam& p)
	{
		// Check nullable
		if (detail::Util::IsBitSet(metaField->desc, FDesc::Nullable)
			&& stream.IsNull())
		{
			m_r->fieldStatus[metaField->index] = FStatus::kNull;
		}
		else
		{
			void *pField = GetFieldByOffset(metaField->offset);
			FromJsonReq elemReq(stream, *m_pAllocator, p.deserFlag, canMoveSrc, pField, metaField->desc);
			FromJsonResp elemResp(p.errDoc);
			int ret = detail::GetSerializerInterface<Encoding>(*metaField)->FromJson(elemReq, elemResp);
			// Check return
			if (ret != 0) {
				m_r->fieldStatus[metaField->index] = FStatus::kMissing;
				p.errDoc.ErrorInObject(metaField->fieldName, metaField->jsonName);
				return ret;
			}
			// succ
			m_r->fieldStatus[metaField->index] = FStatus::kValid;
		}
		return 0;
	}

	void DoShrinkAllocator()
	{
		// Shrink allocator of each field
		for (typename std::vector<TMetaFieldInfo>::const_iterator itFieldInfo = m_pMetaClass->GetFieldsInfo().begin();
			 itFieldInfo != m_pMetaClass->GetFieldsInfo().end(); ++itFieldInfo)
		{
			void *pField = GetFieldByOffset(itFieldInfo->offset);
			detail::GetSerializerInterface<Encoding>(*itFieldInfo)->ShrinkAllocator(pField);
		}

		// Shrink self allocator
		detail::Util::ShrinkAllocatorWithOwnDoc(m_r->ownDoc, m_r->unknown, m_pAllocator);
	}

	inline void InitOuterPtr()
	{
		m_pOuter = reinterpret_cast<const unsigned char *>(this - m_pMetaClass->GetAccessorOffset());
	}

	void MarkFieldStatus(const void* field, EFStatus fStatus)
	{
		const std::size_t offset = GetFieldOffset(field);
		const int index = m_pMetaClass->FindIndex(offset);
		IJST_ASSERT(index >= 0 && (unsigned int)index < m_r->fieldStatus.size());
		m_r->fieldStatus[index] = fStatus;
	}

	int CheckFieldState(detail::ErrorDocSetter<Encoding>& errDoc) const
	{
		// Check all required field status
		bool hasErr = false;

		for (typename std::vector<TMetaFieldInfo>::const_iterator itFieldInfo = m_pMetaClass->GetFieldsInfo().begin();
			 itFieldInfo != m_pMetaClass->GetFieldsInfo().end(); ++itFieldInfo)
		{
			if (detail::Util::IsBitSet(itFieldInfo->desc, FDesc::Optional))
			{
				// Optional
				continue;
			}

			const EFStatus fStatus = m_r->fieldStatus[itFieldInfo->index];
			if (fStatus == FStatus::kValid
				|| fStatus == FStatus::kNull)
			{
				// Correct
				continue;
			}

			// Has error
			hasErr = true;
			errDoc.ElementAddMemberName(itFieldInfo->jsonName);
		}
		if (hasErr)
		{
			errDoc.MissingMember();
			return ErrorCode::kDeserializeSomeFieldsInvalid;
		}
		return 0;
	}

	inline std::size_t GetFieldOffset(const void *ptr) const
	{
		const unsigned char *filed_ptr = reinterpret_cast<const unsigned char *>(ptr);
		return filed_ptr - m_pOuter;
	}

	inline void *GetFieldByOffset(std::size_t offset) const
	{
		return (void *) (m_pOuter + offset);
	}

	typedef std::vector<EFStatus> FieldStatusType;
	// Note: Use pointers to make class Accessor be a standard-layout type struct
	struct Resource {
		TValue unknown;
		// Should use document instead of Allocator because document can swap allocator
		TDocument ownDoc;
		FieldStatusType fieldStatus;
	};
	Resource* m_r;

	const TMetaClassInfo* m_pMetaClass;
	detail::JsonAllocator* m_pAllocator;
	const unsigned char *m_pOuter;

	bool m_isValid;
	bool m_isParentVal;
	//</editor-fold>
};	// class Accessor

/**
 * @brief Provide `bool f(Handler)` functor used by rapidjson::Document.Populate()
 *
 * @tparam Handler		rapidjson::Handler
 * @tparam Encoding		Encoding of Accessor
 */
template<typename Handler, typename Encoding = rapidjson::UTF8<> >
class SAXGeneratorWrapper : public HandlerBase<typename Handler::Ch>
{
public:
#if __cplusplus >= 201103L
	static_assert(std::is_same<typename Handler::Ch, typename Encoding::Ch>::value,
				  "Handler::Ch and Encoding::Ch must be same");
#endif
	typedef typename Handler::Ch Ch;
	explicit SAXGeneratorWrapper(const Accessor<Encoding>& accessor, SerFlag::Flag serFlag = SerFlag::kNoneFlag) :
			m_accessor(accessor), m_serFlag (serFlag), m_h(IJST_NULL) {}

	bool operator() (Handler& h)
	{
		m_h = &h;
		int ret = m_accessor.Serialize(*this, m_serFlag);
		return ret == 0;
	}

	bool Null() IJSTI_OVERRIDE
	{ return m_h->Null(); }
	bool Bool(bool b) IJSTI_OVERRIDE
	{ return m_h->Bool(b); }
	bool Int(int i) IJSTI_OVERRIDE
	{ return m_h->Int(i); }
	bool Uint(unsigned i) IJSTI_OVERRIDE
	{ return m_h->Uint(i); }
	bool Int64(int64_t i) IJSTI_OVERRIDE
	{ return m_h->Int64(i); }
	bool Uint64(uint64_t i) IJSTI_OVERRIDE
	{ return m_h->Uint64(i); }
	bool Double(double d) IJSTI_OVERRIDE
	{ return m_h->Double(d); }
	bool RawNumber(const Ch *str, rapidjson::SizeType length, bool copy = false) IJSTI_OVERRIDE
	{ return m_h->RawNumber(str, length, copy); }
	bool String(const Ch *str, rapidjson::SizeType length, bool copy = false) IJSTI_OVERRIDE
	{ return m_h->String(str, length, copy); }
	bool StartObject() IJSTI_OVERRIDE
	{ return m_h->StartObject(); }
	bool Key(const Ch *str, rapidjson::SizeType length, bool copy = false) IJSTI_OVERRIDE
	{ return m_h->Key(str, length, copy); }
	bool EndObject(rapidjson::SizeType memberCount = 0) IJSTI_OVERRIDE
	{ return m_h->EndObject(memberCount); }
	bool StartArray() IJSTI_OVERRIDE
	{ return m_h->StartArray(); }
	bool EndArray(rapidjson::SizeType elementCount = 0) IJSTI_OVERRIDE
	{ return m_h->EndArray(elementCount); }
private:
	const Accessor<Encoding>& m_accessor;
	SerFlag::Flag m_serFlag;
	Handler* m_h;
};

template<typename Encoding>
template<typename T>
inline const MetaClassInfo<Encoding> &MetaClassInfo<Encoding>::GetMetaInfo()
{
	IJSTI_TRY_INIT_META_BEFORE_MAIN(detail::MetaClassInfoTyped<T>);
	return detail::Singleton<detail::MetaClassInfoTyped<T> >::GetInstance().metaClass;
}

	#define IJSTI_STRUCT_META_INITER_DECLARE(stName)	\
		template void stName::template _ijst_InitMetaInfo<true>(stName::_ijst_MetaInfoT*);

//! IJSTI_STRUCT_EXTERN_TEMPLATE
#if IJST_EXTERN_TEMPLATE
	#define IJSTI_STRUCT_EXTERN_TEMPLATE(stName)	\
		extern IJSTI_STRUCT_META_INITER_DECLARE(stName)
#else
	#define IJSTI_STRUCT_EXTERN_TEMPLATE(stName)	// empty
#endif

//! IJSTI_STRUCT_EXPLICIT_TEMPLATE
#if IJST_EXPLICIT_TEMPLATE
	#define IJSTI_STRUCT_EXPLICIT_TEMPLATE(stName)	\
		IJSTI_STRUCT_META_INITER_DECLARE(stName)
#else
	#define IJSTI_STRUCT_EXPLICIT_TEMPLATE(stName)	//emtpy
#endif

//! IJSTI_DEFINE_STRUCT_IMPL
//! Wrapper of IJST_DEFINE_STRUCT_IMPL_*
//! @param N			fields size
//! @param isRawVal		is struct a raw value: true/false
//! @param needGetter	need get_* function: T/F
//! @param encoding		encoding of json struct
//! @param stName		struct name
//! @param ...			fields define: [(fType, fName, sName, desc)]*
#ifdef _MSC_VER
	//! @params	N, isRawVal, needGetter, encoding, stName, ...
	#define IJSTI_DEFINE_STRUCT_IMPL(N, ...) \
		IJSTI_EXPAND(IJSTI_PP_CONCAT(IJSTI_DEFINE_STRUCT_IMPL_, N)(__VA_ARGS__))
#else
	//! Wrapper of IJST_DEFINE_STRUCT_IMPL_*
	//! @param needGetter: must be T or F
	#define IJSTI_DEFINE_STRUCT_IMPL(N, isRawVal, needGetter, ...) \
		IJSTI_PP_CONCAT(IJSTI_DEFINE_STRUCT_IMPL_, N)(isRawVal, needGetter, __VA_ARGS__)
#endif

//! IJSTI_DEFINE_GETTER_T
//! Define getter of fields
#ifdef _MSC_VER
	#define IJSTI_DEFINE_GETTER_T(N, ...)	\
		IJSTI_EXPAND(IJSTI_PP_CONCAT(IJSTI_DEFINE_GETTER_IMPL_, N)(__VA_ARGS__))
#else
	#define IJSTI_DEFINE_GETTER_T(N, ...)	\
		IJSTI_PP_CONCAT(IJSTI_DEFINE_GETTER_IMPL_, N)(__VA_ARGS__)
#endif
	#define IJSTI_DEFINE_GETTER_F(N, ...)	// empty

	#define IJSTI_IDL_FTYPE(fType, fName, sName, desc)		fType
	#define IJSTI_IDL_FNAME(fType, fName, sName, desc)		fName
	#define IJSTI_IDL_SNAME(fType, fName, sName, desc)		sName
	#define IJSTI_IDL_DESC(fType, fName, sName, desc)		desc
	#define IJSTI_IDL_FNAME_STR(fType, fName, sName, desc)		#fName

	#define IJSTI_OFFSETOF(base, member)	(size_t(&base->member) - size_t(base))

	#define IJSTI_STRUCT_PUBLIC_DEFINE(encoding)												\
		typedef encoding _ijst_Encoding;														\
		typedef typename encoding::Ch _ijst_Ch;													\
		typedef ::ijst::Accessor<_ijst_Encoding> _ijst_AccessorType;							\
		_ijst_AccessorType _;

	#define IJSTI_DEFINE_FIELD(fType, fName, ... )												\
			fType fName;

	#define IJSTI_FIELD_GETTER(fType, fName, ... )												\
			::ijst::Optional<const fType > IJSTI_PP_CONCAT(IJST_GETTER_PREFIX, fName)() const 	\
			{ return static_cast<const _ijst_AccessorType&>(this->_).GetOptional(this->fName); }\
			::ijst::Optional< fType > IJSTI_PP_CONCAT(IJST_GETTER_PREFIX, fName)()				\
			{ return this->_.GetOptional(this->fName); }

	#define IJSTI_METAINFO_DEFINE_START(stName, N)												\
			typedef ::ijst::detail::MetaClassInfoTyped< stName > _ijst_MetaInfoT;				\
			typedef ::ijst::detail::Singleton<_ijst_MetaInfoT> _ijst_MetaInfoS;					\
			friend class ::ijst::detail::MetaClassInfoTyped< stName >;							\
			template<bool DummyTrue	>															\
			static void _ijst_InitMetaInfo(_ijst_MetaInfoT* metaInfo)							\
			{																					\
				IJSTI_TRY_INIT_META_BEFORE_MAIN(_ijst_MetaInfoT);								\
				/* Do not call MetaInfoS::GetInstance() int this function */			 		\
				char dummyBuffer[sizeof(stName)];												\
				const stName* stPtr = reinterpret_cast< stName*>(dummyBuffer);					\
				::ijst::detail::MetaClassInfoSetter<_ijst_Ch>					 				\
							mSetter(metaInfo->metaClass);										\
				mSetter.InitBegin(#stName, N, IJSTI_OFFSETOF(stPtr, _));

	#define IJSTI_METAINFO_ADD(stName, fDef)  													\
			mSetter.PushMetaField(																\
				IJSTI_IDL_FNAME_STR fDef,														\
				IJSTI_IDL_SNAME fDef, 															\
				IJSTI_OFFSETOF(stPtr, IJSTI_IDL_FNAME fDef),									\
				IJSTI_IDL_DESC fDef, 															\
				&(IJSTI_FSERIALIZER_INS(IJSTI_IDL_FTYPE fDef, _ijst_Encoding))					\
			);

	#define IJSTI_METAINFO_DEFINE_END()															\
				mSetter.InitEnd();																\
			}

	#define IJSTI_DEFINE_CLASS_END(stName)														\
		};																						\
		IJSTI_STRUCT_EXTERN_TEMPLATE(stName)													\
		IJSTI_STRUCT_EXPLICIT_TEMPLATE(stName)


}	// namespace ijst

//! list of templates could been declared extern
#define IJSTI_EXTERNAL_TEMPLATE_XLIST											\
		IJSTX(struct rapidjson::UTF8<>)											\
		IJSTX(class ijst::Accessor<>)											\
		IJSTX(class rapidjson::GenericDocument<rapidjson::UTF8<> >)				\
		IJSTX(class rapidjson::GenericValue<rapidjson::UTF8<> >)				\
		IJSTX(class rapidjson::MemoryPoolAllocator<>)							\
		IJSTX(class rapidjson::GenericStringBuffer<rapidjson::UTF8<> >)			\
		IJSTX(class rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<> > >)

//! extern declare template list
#if IJST_EXTERN_TEMPLATE
	#define IJSTX(...)	extern template __VA_ARGS__;
	IJSTI_EXTERNAL_TEMPLATE_XLIST
	#undef IJSTX
#endif

//! explicit declare template list
#if IJST_EXPLICIT_TEMPLATE
	#define IJSTX(...)	template __VA_ARGS__;
	IJSTI_EXTERNAL_TEMPLATE_XLIST
	#undef IJSTX
#endif

#include "detail/ijst_repeat_def.inc"

#endif //IJST_HPP_INCLUDE_
