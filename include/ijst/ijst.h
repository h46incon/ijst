//
// Created by h46incon on 2017/9/19.
//

#ifndef _IJST_HPP_INCLUDE_
#define _IJST_HPP_INCLUDE_

#include "detail/detail.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

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
 * 	By default, ijst can only serialize the structure to string.
 * 	If users need to serialize the structure to a rapidjson::Value, specify this flag to 1.
 */
#ifndef IJST_ENABLE_TO_JSON_OBJECT
	#define IJST_ENABLE_TO_JSON_OBJECT 			0
#endif

/**
 * @ingroup IJST_CONFIG
 *
 * 	By default, ijst can only deserialize the structure from string.
 * 	If users need to deserialize the structure from a rapidjson::Value, specify this flag to 1.
 */
#ifndef IJST_ENABLE_FROM_JSON_OBJECT
	#define IJST_ENABLE_FROM_JSON_OBJECT 		0
#endif

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
 *	If define IJST_AUTO_META_INIT before include this header, the meta class information will init before main.
 *	That's will make it thread-safe to init meta class information before C++11.
 *	The feature is enable default before C++11. So set the value to 0 to force disable it.
 */
#ifndef IJST_AUTO_META_INIT
	#if __cplusplus < 201103L
		#define IJST_AUTO_META_INIT		1
	#else
		#define IJST_AUTO_META_INIT		0
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

/**
 * @ingroup IJST_CONFIG
 *
 *	By default, the offset of each field is computed by "((Struct*)0)->field".
 *	User could overwrite by define this macro such as "offsetof()".
 */
#ifndef IJST_OFFSETOF
	#define IJST_OFFSETOF(_T, member)	((size_t)&(((_T*)0)->member))
#endif

/** @defgroup IJST_MACRO_API ijst macro API
 *  @brief macro API
 *
 */

//! @brief Declare a ijst struct.
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_STRUCT(...) \
    IJSTI_DEFINE_STRUCT_IMPL(IJSTI_PP_NFIELD(__VA_ARGS__), false, F, __VA_ARGS__)
//! @brief Declare a ijst struct with getter.
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_STRUCT_WITH_GETTER(...) \
    IJSTI_DEFINE_STRUCT_IMPL(IJSTI_PP_NFIELD(__VA_ARGS__), false, T, __VA_ARGS__)
//! @brief Declare a ijst struct which represent a value instead of members insides a object
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_VALUE(stName, type, fName, desc)	\
    IJSTI_DEFINE_STRUCT_IMPL(1, true, F, stName, (type, fName, "JSON_ITSELF", desc))
//! @brief Declare a ijst struct which represent a value instead of members insides a object with getter
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_VALUE_WITH_GETTER(stName, type, fName, desc)	\
    IJSTI_DEFINE_STRUCT_IMPL(1, true, T, stName, (type, fName, "JSON_ITSELF", desc))

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
#define IJST_TYPE(...)			::ijst::detail::ArgumentType<void( __VA_ARGS__)>::type

namespace ijst {
typedef rapidjson::Value JsonValue;
typedef rapidjson::MemoryPoolAllocator<> 	JsonAllocator;

//! Field description.
struct FDesc {
	typedef unsigned int Mode;
	//! Field is optional.
	static const Mode Optional 			= 0x00000001;
	//! Field can be null.
	static const Mode Nullable 			= 0x00000002;
	//! Field is a container, and has at least one element.
	static const Mode ElemNotEmpty 		= 0x00000004;
	// FDesc of Element (i.e. Nullable, ElemNotEmpty) inside container is hard to represent, has not plan to implement it
};

//! Field status.
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
typedef FStatus::_E EFStatus;

/**
 * @brief Serialization options about fields.
 *
 * Options can be combined by bitwise OR operator (|).
 */
struct FPush {
	typedef unsigned int Mode;
	//! does not set any option.
	static const Mode kNoneFlag							= 0x00000000;
	//! set if serialize only valid fields, otherwise will serialize all fields.
	static const Mode kOnlyValidField					= 0x00000001;
	//! set if ignore unknown fields, otherwise will serialize all unknown fields.
	static const Mode kIgnoreUnknown					= 0x00000002;
};

/**
 * @brief Behaviour when meeting unknown member in json object.
 *
 * @see Accessor::GetUnknown()
 */
struct UnknownMode {
public:
	enum _E {
		//! Keep unknown fields.
		kKeep,
		//! Ignore unknown fields.
		kIgnore,
		//! Error when unknown fields occurs.
		kError
	};
};
typedef UnknownMode::_E EUnknownMode;

/**
 * @brief A virtual based class implement rapidjson::Handler concept.
 *
 * Using raw rapidjson::Handler concept have to make function as a template, which is not convince is some situations.
 *
 * @tparam _Ch	character type of string
 *
 * @see HandlerWrapper
 */
template<typename _Ch>
class GenericHandlerBase {
public:
	typedef _Ch Ch;

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
typedef GenericHandlerBase<char> HandlerBase;

/**
 * @brief A wrapper that convert raw rapidjson::Handler instance to derived class of GenericHandlerBase.
 *
 * @tparam Handler		rapidjson::Handler
 */
template<typename Handler>
class HandlerWrapper : public GenericHandlerBase<typename Handler::Ch>
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
struct Err {
	static const int kSucc 							= 0x00000000;
	static const int kDeserializeValueTypeError 	= 0x00001001;
	static const int kDeserializeSomeFiledsInvalid 	= 0x00001002;
	static const int kDeserializeParseFaild 		= 0x00001003;
	static const int kDeserializeElemEmpty 			= 0x00001004;
	static const int kDeserializeSomeUnknownMember	= 0x00001005;
	static const int kDeserializeMapKeyDuplicated	= 0x00001006;
	static const int kInnerError 					= 0x00002001;
	static const int kWriteFailed					= 0x00003001;
};

#define IJSTI_OPTIONAL_BASE_DEFINE(_T)						\
	public:													\
		/** @brief Constructor */ 							\
		explicit Optional(_T* _pVal) : m_pVal(_pVal) {}		\
		/** @brief Get holding pointer */ 					\
		_T* Ptr() const { return m_pVal; }					\
	private:												\
		_T* const m_pVal;

/**
 * @brief Helper for implementing getter chaining.
 *
 * @tparam _T 	type
 *
 * @note	The specialized template for container is declared in "types_container.h",
 * 			which implements operator [].
 */
template <typename _T, typename = void>
class Optional
{
	typedef _T ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
};

/**
 * @brief Specialization for ijst struct
 *
 * Specialization for ijst struct (defined via IJST_DEFINE_STRUCT and so on) of Optional template.
 * This specialization add operator->() for getter chaining.
 *
 * @tparam _T	ijst struct type
 */
template <typename _T>
class Optional<_T, typename detail::HasType<typename _T::_ijstStructAccessorType>::Void >
{
	typedef _T ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	/**
	 * @brief Get pointer
	 *
	 * @return 	valid instance when data is not null, invalid instance when data is null
	 */
	_T* operator->() const
	{
		static _T empty(false);
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
 * @see MetaClassInfo
 */
struct MetaFieldInfo { // NOLINT
	//! The index of this fields in the meta information in the class. (Fields are sorted by offset inside class)
	int index;
	//! Field description.
	FDesc::Mode desc;
	//! Field's offset inside class.
	std::size_t offset;
	//! Json name when (de)serialization.
	std::string jsonName;
	//! field name.
	std::string fieldName;
	//! @private private serializer interface.
	detail::SerializerInterface *serializerInterface;
};

/**
 * @brief Meta information of class.
 *
 * @see MetaFieldInfo
 */
class MetaClassInfo {
public:
	/**
	 * @brief Get meta information for ijst struct _T.
	 *
	 * @tparam _T 	ijst struct
	 * @return		MetaClassInfo instance
	 */
	template<typename _T>
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
				detail::BinarySearch(m_offsets.begin(), m_offsets.end(), offset, IntComp);
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
	const MetaFieldInfo* FindFieldByJsonName(const std::string &name) const
	{
		std::vector<NameMap>::const_iterator it =
				detail::BinarySearch(m_nameMap.begin(), m_nameMap.end(), name, NameMapComp);
		if (it != m_nameMap.end() && (*it->pName) == name) {
			return it->metaField;
		}
		else {
			return IJST_NULL;
		}
	}

	//! Get meta information of all fields in class. The returned vector is sorted by offset.
	const std::vector<MetaFieldInfo>& GetFieldsInfo() const { return fieldsInfo; }
	//! Get name of class.
	const std::string& GetClassName() const { return structName; }
	//! Get the offset of Accessor object.
	std::size_t GetAccessorOffset() const { return accessorOffset; }

private:
	friend class detail::MetaClassInfoSetter;
	template<typename _T> friend class detail::MetaClassInfoIniter;
	MetaClassInfo() : accessorOffset(0), mapInited(false) { }

	MetaClassInfo(const MetaClassInfo&);	// = delete
	MetaClassInfo& operator=(MetaClassInfo);		// = delete

	struct NameMap {
		NameMap(const std::string* _pName, const MetaFieldInfo* _metaField)
				: pName(_pName), metaField(_metaField) {}

		bool operator<(const NameMap &r) const
		{ return (*pName) < (*r.pName); }

		const std::string* pName;
		const MetaFieldInfo* metaField;
	};

	static int NameMapComp(const NameMap &l, const std::string &name)
	{
		return l.pName->compare(name);
	}

	static int IntComp(size_t l, size_t r)
	{
		return (int)((long)l - (long)r);
	}

	std::vector<MetaFieldInfo> fieldsInfo;
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
	typedef rapidjson::Writer<rapidjson::StringBuffer> JsonWriter;

	//! return Err::kWriteFailed if action return false
	#define IJSTI_RET_WHEN_WRITE_FAILD(action) 					\
		do { if(!(action)) return Err::kWriteFailed; } while (false)
	//! return if action return non-0
	#define IJSTI_RET_WHEN_NOT_ZERO(action) 					\
		do { int ret = (action); if(ret != 0) return (ret); } while (false)

	#if IJST_AUTO_META_INIT
		#define IJSTI_TRY_INIT_META_BEFORE_MAIN(_T)			::ijst::detail::Singleton< _T>::InitInstanceBeforeMain();
	#else
		#define IJSTI_TRY_INIT_META_BEFORE_MAIN(_T)
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

	class SerializerInterface {
	public:
		virtual ~SerializerInterface() { }

		struct SerializeReq {
			// Serialize option about fields
			FPush::Mode fPushMode;

			// Pointer of field to serialize.
			// The actual type of field should be decide in the derived class
			const void* pField;

			HandlerBase& writer;

			SerializeReq(HandlerBase& _writer, const void *_pField, FPush::Mode _fPushMode)
					: fPushMode(_fPushMode)
					, pField(_pField)
					, writer(_writer)
			{ }
		};

		virtual int Serialize(const SerializeReq &req) = 0;

#if IJST_ENABLE_TO_JSON_OBJECT
		struct ToJsonReq {
			// ToJson option about fields
			FPush::Mode fPushMode;

			// Pointer of field to serialize.
			// The actual type of field should be decide in the derived class
			const void* pField;

			// true if move context in pField to avoid copy when possible
			bool canMoveSrc;

			// Output buffer info. The instance should serialize in this object and use allocator
			IJST_OUT JsonValue& buffer;
			JsonAllocator& allocator;

			ToJsonReq(JsonValue &_buffer, JsonAllocator &_allocator,
					  const void *_pField, bool _canMoveSrc,
					  FPush::Mode _fPushMode)
					: fPushMode(_fPushMode)
					  , pField(_pField)
					  , canMoveSrc(_canMoveSrc)
					  , buffer(_buffer)
					  , allocator(_allocator)
			{ }
		};

		virtual int ToJson(const ToJsonReq &req)= 0;

		virtual int SetAllocator(void *pField, JsonAllocator &allocator)
		{ (void) pField; (void) allocator; return 0; }
#endif // #if IJST_ENABLE_TO_JSON_OBJECT

		struct FromJsonReq {
			// Pointer of deserialize output.
			// The instance should deserialize in this object
			// The actual type of field should be decide in the derived class
			void* pFieldBuffer;

			// The input stream and allocator
			JsonValue& stream;
			JsonAllocator& allocator;

			// true if move context in stream to avoid copy when possible
			bool canMoveSrc;

			EUnknownMode unknownMode;

			// true if check field status is matched requirement
			bool checkField;

			FromJsonReq(JsonValue &_stream, JsonAllocator &_allocator,
						EUnknownMode _unknownMode, bool _canMoveSrc, bool _checkField,
						void *_pField)
					: pFieldBuffer(_pField)
					  , stream(_stream)
					  , allocator(_allocator)
					  , canMoveSrc(_canMoveSrc)
					  , unknownMode(_unknownMode)
					  , checkField(_checkField)
			{ }
		};

		struct FromJsonResp {
			size_t fieldCount;
			DeserializeErrDoc& errDoc;

			explicit FromJsonResp(DeserializeErrDoc& _errDoc) :
					fieldCount(0),
					errDoc(_errDoc)
			{ }

		};

		virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp)= 0;

	};

	/**
	 * Template interface of serialization class
	 * This template is unimplemented, and will throw a compile error when use it.
	 * @tparam _T class
	 */
	template<typename _T, typename = void>
	class FSerializer : public SerializerInterface {
	public:
		#if __cplusplus >= 201103L
		static_assert(!std::is_same<_T, _T>::value,
					  "This base template should not be instantiated. (Maybe use wrong param when define ijst struct)");
		#endif

		typedef void VarType;

		virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE = 0;

#if IJST_ENABLE_TO_JSON_OBJECT
		virtual int ToJson(const ToJsonReq &req) IJSTI_OVERRIDE = 0;
		virtual int SetAllocator(void *pField, JsonAllocator &allocator) IJSTI_OVERRIDE;
#endif

		virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE = 0;
	};

	#define IJSTI_FSERIALIZER_INS(_T) ::ijst::detail::Singleton< ::ijst::detail::FSerializer< _T> >::GetInstance()

	/**	========================================================================================
	 *				Private
	 */
	/**
	 * Meta info initer
	 * Push meta class info of _T in specialized constructor MetaInfo<_T>().
	 * @tparam _T: class. Concept require _T::_InitMetaInfo(MetaInfo*)
	 */
	template<typename _T>
	class MetaClassInfoIniter {
	public:
		MetaClassInfo metaClass;

	private:
		friend class Singleton<MetaClassInfoIniter<_T> >;

		MetaClassInfoIniter()
		{
			_T::_InitMetaInfo(this);
		}
	};

	class MetaClassInfoSetter {
	public:
		explicit MetaClassInfoSetter(MetaClassInfo& _d) : d(_d) { }

		void InitBegin(const std::string& _tag, std::size_t _fieldCount, std::size_t _accessorOffset)
		{
			d.structName = _tag;
			d.accessorOffset = _accessorOffset;
			d.fieldsInfo.reserve(_fieldCount);
		}

		void PushMetaField(const std::string &fieldName, const std::string& jsonName,
						   std::size_t offset, FDesc::Mode desc, SerializerInterface *serializerInterface)
		{
			MetaFieldInfo metaField;
			metaField.jsonName = jsonName;
			metaField.fieldName = fieldName;
			metaField.offset = offset;
			metaField.desc = desc;
			metaField.serializerInterface = serializerInterface;
			d.fieldsInfo.push_back(IJSTI_MOVE(metaField));
		}

		void InitEnd()
		{
			// assert MetaClassInfo's map has not inited before
			assert(!d.mapInited);
			SortMetaFieldsByOffset();

			d.m_offsets.reserve(d.fieldsInfo.size());
			d.m_nameMap.resize(d.fieldsInfo.size(), MetaClassInfo::NameMap(IJSTI_NULL, IJSTI_NULL));

			for (size_t i = 0; i < d.fieldsInfo.size(); ++i) {
				MetaFieldInfo *ptrMetaField = &(d.fieldsInfo[i]);
				ptrMetaField->index = static_cast<int>(i);

				d.m_offsets.push_back(ptrMetaField->offset);
				// Assert field offset is sorted and not exist before
				assert(i == 0 || d.m_offsets[i]  > d.m_offsets[i-1]);

				// Insert name Map
				InsertNameMap(i, MetaClassInfo::NameMap(&(ptrMetaField->jsonName), ptrMetaField));
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
					detail::Swap(d.fieldsInfo[j], d.fieldsInfo[j - 1]);
				}
			}
		}

		void InsertNameMap(size_t len, const MetaClassInfo::NameMap& v)
		{
			std::vector<MetaClassInfo::NameMap>::iterator it =
					BinarySearch(d.m_nameMap.begin(), d.m_nameMap.begin() + len, *v.pName, MetaClassInfo::NameMapComp);
			size_t i = static_cast<size_t>(it - d.m_nameMap.begin());
			// assert name is unique
			assert(i == len || (*v.pName) != (*it->pName));

			for (size_t j = len; j > i; --j) {
				d.m_nameMap[j] = IJSTI_MOVE(d.m_nameMap[j - 1]);
			}
			d.m_nameMap[i] = v;
		}

		MetaClassInfo& d;
	};

	/**
	 * Serialization of ijst struct types
	 * @tparam _T class
	 */
	template<class _T>
	class FSerializer<_T, typename HasType<typename _T::_ijstStructAccessorType>::Void>: public SerializerInterface {
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

}	// namespace detail

/**
 * @brief Accessor of ijst struct
 *
 * User can access and modify fields, serialize and deserialize of a structure via it.
 */
class Accessor {
public:
	//! Constructor
	explicit Accessor(const MetaClassInfo *pMetaClass, bool isParentVal, bool isValid) :
			m_pMetaClass(pMetaClass), m_isValid(isValid), m_isParentVal(isParentVal)
	{
		IJST_ASSERT(!m_isParentVal || m_pMetaClass->GetFieldsInfo().size() == 1);
		m_r = static_cast<Resource *>(operator new(sizeof(Resource)));
		new(&m_r->fieldStatus) FieldStatusType(m_pMetaClass->GetFieldsInfo().size(), FStatus::kMissing);
		new(&m_r->unknown)rapidjson::Value(rapidjson::kObjectType);
		new(&m_r->ownDoc) rapidjson::Document();
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
		new(&m_r->unknown)rapidjson::Value(rapidjson::kObjectType);
		new(&m_r->ownDoc) rapidjson::Document();
		m_pAllocator = &m_r->ownDoc.GetAllocator();

		m_pMetaClass = rhs.m_pMetaClass;
		InitOuterPtr();

		m_r->unknown.CopyFrom(rhs.m_r->unknown, *m_pAllocator, true);
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
	const MetaClassInfo& GetMetaInfo() const { return *m_pMetaClass; }

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
	template<typename _T1, typename _T2>
	inline void Set(_T1 &field, const _T2 &value)
	{
		MarkValid(&field);
		field = value;
	}

	//! Set field to val and mark it valid. The type of field and value must be same.
	template<typename _T>
	inline void SetStrict(_T &field, const _T &value)
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
	inline JsonValue &GetUnknown() { return m_r->unknown; }
	inline const JsonValue &GetUnknown() const { return m_r->unknown; }

	/**
	 * @brief Get allocator used in object.
	 *
	 * The inner allocator is own allocator when init,
	 * but may change to other allocator when calling SetMembersAllocator() or Deserialize().
	 */
	inline JsonAllocator &GetAllocator() { return *m_pAllocator; }
	inline const JsonAllocator &GetAllocator() const { return *m_pAllocator; }

	/**
	 * @brief Get own allocator that used to manager resource.
	 *
	 * User could use the returned value to check if this object use outer allocator.
	 */
	inline JsonAllocator &GetOwnAllocator() { return m_r->ownDoc.GetAllocator(); }
	inline const JsonAllocator &GetOwnAllocator() const { return m_r->ownDoc.GetAllocator(); }

	/**
	 * @brief Serialize the structure to string.
	 *
	 * @param writer 			writer
	 * @param fieldPushMode 	Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return					Error code
	 */
	int Serialize(HandlerBase& writer, FPush::Mode fieldPushMode = FPush::kNoneFlag)  const
	{
		return DoSerialize(writer, fieldPushMode);
	}

	/**
	 * @brief Serialize the structure to string.
	 *
	 * @param strOutput 		The output of result
	 * @param fieldPushMode 	Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return					Error code
	 */
	int Serialize(IJST_OUT std::string &strOutput, FPush::Mode fieldPushMode = FPush::kNoneFlag)  const
	{
		rapidjson::StringBuffer buffer;
		detail::JsonWriter writer(buffer);
		HandlerWrapper<detail::JsonWriter> writerWrapper(writer);
		IJSTI_RET_WHEN_NOT_ZERO(DoSerialize(writerWrapper, fieldPushMode));

		strOutput = std::string(buffer.GetString(), buffer.GetLength());
		return 0;
	}

	/**
	 * @brief Deserialize from C-style string.
	 *
	 * @tparam parseFlags		parseFlags of rapidjson parse method
	 *
	 * @param cstrInput			Input C string
	 * @param length			Length of string
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param checkField		true if need check whether field status meet requirement
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note The input string can contain '\0'
	 */
	template <unsigned parseFlags>
	int Deserialize(const char *cstrInput, std::size_t length,
					EUnknownMode unknownMode = UnknownMode::kKeep, bool checkField = true,
					std::string *pErrMsgOut = IJST_NULL)
	{
		// The new object will call FromJson() interfaces soon in most situation
		// So clear own allocator will not bring much benefice
		m_pAllocator = &m_r->ownDoc.GetAllocator();
		rapidjson::Document doc(m_pAllocator);
		doc.Parse<parseFlags>(cstrInput, length);
		if (doc.HasParseError())
		{
			if (pErrMsgOut != IJST_NULL)
			{
				*pErrMsgOut = IJSTI_MOVE(std::string(
						rapidjson::GetParseError_En(doc.GetParseError())
				));
			}
			return Err::kDeserializeParseFaild;
		}

		return DoFromJsonWrap<JsonValue>(&Accessor::DoMoveFromJson, doc, unknownMode, checkField, pErrMsgOut);
	}

	/**
	 * @brief Deserialize from C-style string.
	 *
	 * @param cstrInput			Input C string
	 * @param length			Length of string
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param checkField		true if need check whether field status meet requirement
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note The input string can contain '\0'
	 */
	int Deserialize(const char *cstrInput, std::size_t length,
					EUnknownMode unknownMode = UnknownMode::kKeep, bool checkField = true,
					std::string *pErrMsgOut = IJST_NULL)
	{
		return this->template Deserialize<rapidjson::kParseDefaultFlags>(
				cstrInput, length, unknownMode, checkField, pErrMsgOut);
	}

	/**
	 * @brief Deserialize from std::string.
	 *
	 * @param strInput			Input string
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param checkField		true if need check whether field status meet requirement
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 */
	inline int Deserialize(const std::string &strInput,
						   EUnknownMode unknownMode = UnknownMode::kKeep, bool checkField = true,
						   std::string *pErrMsgOut = IJST_NULL)
	{
		return Deserialize(strInput.data(), strInput.size(), unknownMode, checkField, pErrMsgOut);
	}

	/**
	 * @brief Deserialize from std::string.
	 *
	 * @param strInput			Input string
	 * @param errMsgOut			Error message output
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param checkField		true if need check whether field status meet requirement
	 * @return					Error code
	 */
	inline int Deserialize(const std::string &strInput, IJST_OUT std::string& errMsgOut,
						   EUnknownMode unknownMode = UnknownMode::kKeep, bool checkField = true)
	{
		return Deserialize(strInput.data(), strInput.size(), unknownMode, checkField, &errMsgOut);
	}

	/**
	 * @brief Deserialize insitu from str.
	 *
	 * @tparam parseFlags		parseFlags of rapidjson parse method
	 *
	 * @param str				Input C string
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param checkField		true if need check whether field status meet requirement
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note The context in str may be changed after deserialize
	 * @note Make sure the lifecycle of str is longer than this object
	 */
	 template<unsigned parseFlags>
	int DeserializeInsitu(char *str,
						  EUnknownMode unknownMode = UnknownMode::kKeep, bool checkField = true,
						  std::string *pErrMsgOut = IJST_NULL)
	{
		// The new object will call FromJson() interfaces in most situation
		// So clear own allocator will not bring much benefice
		m_pAllocator = &m_r->ownDoc.GetAllocator();
		rapidjson::Document doc(m_pAllocator);
		doc.ParseInsitu<parseFlags>(str);
		if (doc.HasParseError())
		{
			if (pErrMsgOut != IJST_NULL)
			{
				*pErrMsgOut = IJSTI_MOVE(std::string(
						rapidjson::GetParseError_En(doc.GetParseError())
				));
			}
			return Err::kDeserializeParseFaild;
		}
		return DoFromJsonWrap<JsonValue>(&Accessor::DoMoveFromJson, doc, unknownMode, checkField, pErrMsgOut);
	}

	/**
	 * @brief Deserialize insitu from str.
	 *
	 * @param str				Input C string
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param checkField		true if need check whether field status meet requirement
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note The context in str may be changed after deserialize
	 * @note Make sure the lifecycle of str is longer than this object
	 */
	int DeserializeInsitu(char *str,
						  EUnknownMode unknownMode = UnknownMode::kKeep, bool checkField = true,
						  std::string *pErrMsgOut = IJST_NULL)
	{
		return this->template DeserializeInsitu<rapidjson::kParseDefaultFlags>(str, unknownMode, checkField, pErrMsgOut);
	}

#if IJST_ENABLE_TO_JSON_OBJECT || IJSTI_DOXYGEN_RUNNING
	/**
	 * @brief Serialize the structure to a JsonValue object.
	 *
	 * @param output 			The output of result
	 * @param allocator	 		Allocator when adding members to output
	 * @param fieldPushMode 	Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return					Error code
	 *
	 * @note Need to set IJST_ENABLE_TO_JSON_OBJECT to 1 to enable this method
	 * @note If using inner allocator, user should carefully handler the structure's life cycle
	 */
	inline int ToJson(IJST_OUT JsonValue &output, JsonAllocator &allocator,
					  FPush::Mode fieldPushMode = FPush::kNoneFlag) const
	{
		return Accessor::template DoToJson<false, const Accessor>
				(*this, fieldPushMode, output, allocator);
	}

	/**
	 * @brief Serialize the structure to a JsonValue object.
	 *
	 * @param output 			The output of result
	 * @param allocator	 		Allocator when adding members to output.
	 * @param fieldPushMode 	Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return					Error code
	 *
	 * @note The object may be invalid after serialization
	 * @note Need to set IJST_ENABLE_TO_JSON_OBJECT to 1 to enable this method
	 * @note If using inner allocator, user should carefully handler the structure's life cycle
	 */
	inline int MoveToJson(IJST_OUT JsonValue &output, JsonAllocator &allocator,
					  FPush::Mode fieldPushMode = FPush::kNoneFlag)
	{
		return Accessor::template DoToJson<true, Accessor>
				(*this, fieldPushMode, output, allocator);
	}

	/**
	 * @brief Init allocator of members to self's allocator.
	 *
	 * @return 					Error code
	 *
	 * @note Need to set IJST_ENABLE_TO_JSON_OBJECT to 1 to enable this method
	 */
	int InitMembersAllocator()
	{
		return SetMembersAllocator(*m_pAllocator);
	}

	/**
	 * @brief Set Inner allocator of object and members.
	 *
	 * @param allocator			New allocator
	 * @return					Error code
	 *
	 * @note Need to set IJST_ENABLE_TO_JSON_OBJECT to 1 to enable this method
	 * @note This method will NOT clear pervious allocator
	 */
	int SetMembersAllocator(JsonAllocator &allocator)
	{
		if (m_pAllocator != &allocator) {
			// copy buffer when need
			JsonValue temp;
			temp = m_r->unknown;
			// The life cycle of const string in temp should be same as this object
			m_r->unknown.CopyFrom(temp, allocator, false);
		}

		m_pAllocator = &allocator;

		// Set allocator in members
		for (std::vector<MetaFieldInfo>::const_iterator itMetaField = m_pMetaClass->GetFieldsInfo().begin();
			 itMetaField != m_pMetaClass->GetFieldsInfo().end(); ++itMetaField)
		{
			void *pField = GetFieldByOffset(itMetaField->offset);
			IJSTI_RET_WHEN_NOT_ZERO(
					itMetaField->serializerInterface->SetAllocator(pField, allocator)
			);
		}
		return 0;
	}
#endif

#if IJST_ENABLE_FROM_JSON_OBJECT || IJSTI_DOXYGEN_RUNNING
	/**
	 * @brief Deserialize from json object.
	 *
	 * @param stream			Input json object
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param checkField		true if need check whether field status meet requirement
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note Need to set IJST_ENABLE_FROM_JSON_OBJECT to 1 to enable this method
	 */
	inline int FromJson(const JsonValue &stream,
						EUnknownMode unknownMode = UnknownMode::kKeep, bool checkField = true,
						std::string *pErrMsgOut = IJST_NULL)
	{
		return DoFromJsonWrap<const JsonValue>(&Accessor::DoFromJson, stream, unknownMode, checkField, pErrMsgOut);
	}

	/**
	 * @brief Move deserialize form json document
	 *
	 * Deserialize from json document. The source object may be stolen after deserialize.
	 * Because the accessor need manager the input allocator, but the Allocator class has no Swap() interface,
	 * so use document object instead.
	 *
	 * @param srcDocStolen		Input document object
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param checkField		true if need check whether field status meet requirement
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note Need to set IJST_ENABLE_FROM_JSON_OBJECT to 1 to enable this method
	 * @note The source document may be changed after deserialize
	 * @note Make sure srcDocStolen use own allocator, or use allocator in this object
	 */
	inline int MoveFromJson(rapidjson::Document &srcDocStolen,
							EUnknownMode unknownMode = UnknownMode::kKeep, bool checkField = true,
							std::string *pErrMsgOut = IJST_NULL)
	{
		// Store document to manager allocator
		m_r->ownDoc.Swap(srcDocStolen);
		m_pAllocator = &m_r->ownDoc.GetAllocator();
		return DoFromJsonWrap<JsonValue>(&Accessor::DoMoveFromJson, m_r->ownDoc, unknownMode, checkField, pErrMsgOut);
	}
#endif

private:
	// #region Implement SerializeInterface
	template <typename, typename> friend class detail::FSerializer;
	typedef detail::SerializerInterface::SerializeReq SerializeReq;
	inline int ISerialize(const SerializeReq &req) const
	{
		assert(req.pField == this);
		return DoSerialize(req.writer, req.fPushMode);
	}

#if IJST_ENABLE_TO_JSON_OBJECT
	typedef detail::SerializerInterface::ToJsonReq ToJsonReq;

	inline int IToJson(const ToJsonReq &req)
	{
		assert(req.pField == this);
		if (req.canMoveSrc) {
			return Accessor::template DoToJson<true, Accessor>
					(*this, req.fPushMode, req.buffer, req.allocator);
		}
		else {
			return Accessor::template DoToJson<false, const Accessor>
					(*this, req.fPushMode, req.buffer, req.allocator);
		}
	}

	inline int ISetAllocator(void* pField, JsonAllocator& allocator)
	{
		(void) pField;
		assert(pField == this);
		return SetMembersAllocator(allocator);
	}
#endif

	typedef detail::SerializerInterface::FromJsonReq FromJsonReq;
	typedef detail::SerializerInterface::FromJsonResp FromJsonResp;
	struct FromJsonParam{
		EUnknownMode unknownMode;
		bool checkField;
		detail::DeserializeErrDoc& errDoc;
		size_t& fieldCount;

		FromJsonParam(EUnknownMode _unknownMode, bool _checkField,
					  detail::DeserializeErrDoc& _errDoc, size_t& _fieldCount)
				: unknownMode(_unknownMode), checkField(_checkField), errDoc(_errDoc), fieldCount(_fieldCount)
		{}
	};

	inline int IFromJson(const FromJsonReq &req, IJST_OUT FromJsonResp& resp)
	{
		assert(req.pFieldBuffer == this);

		m_pAllocator = &req.allocator;

		FromJsonParam param(req.unknownMode, req.checkField, resp.errDoc, resp.fieldCount);
		if (req.canMoveSrc) {
			return DoMoveFromJson(req.stream, param);
		}
		else {
			return DoFromJson(req.stream, param);
		}
	}

	// #endregion

	//! Serialize to string using SAX API
	int DoSerialize(HandlerBase &writer, FPush::Mode fPushMode) const
	{
		if (m_isParentVal) {
			return DoSerializeFields(writer, fPushMode);
			// Unknown will be ignored
		}

		IJSTI_RET_WHEN_WRITE_FAILD(writer.StartObject());

		// Write fields
		IJSTI_RET_WHEN_NOT_ZERO(DoSerializeFields(writer, fPushMode));

		// Write buffer if need
		if (!isBitSet(fPushMode, FPush::kIgnoreUnknown))
		{
			assert(m_r->unknown.IsObject());
			for (rapidjson::Value::ConstMemberIterator itMember = m_r->unknown.MemberBegin();
				 itMember != m_r->unknown.MemberEnd(); ++itMember)
			{
				// Write key
				const JsonValue& key = itMember->name;
				IJSTI_RET_WHEN_WRITE_FAILD(
						writer.Key(key.GetString(), key.GetStringLength()) );
				// Write value
				IJSTI_RET_WHEN_WRITE_FAILD(
						itMember->value.Accept(writer) );
			}
		}

		IJSTI_RET_WHEN_WRITE_FAILD(writer.EndObject());
		return 0;
	}

	int DoSerializeFields(HandlerBase &writer, FPush::Mode fPushMode) const
	{
		IJST_ASSERT(!m_isParentVal || m_pMetaClass->GetFieldsInfo().size() == 1);
		for (std::vector<MetaFieldInfo>::const_iterator itMetaField = m_pMetaClass->GetFieldsInfo().begin();
			 itMetaField != m_pMetaClass->GetFieldsInfo().end(); ++itMetaField)
		{
			// Check field state
			const EFStatus fstatus = m_r->fieldStatus[itMetaField->index];
			switch (fstatus) {
				case FStatus::kValid:
				case FStatus::kMissing:
				case FStatus::kParseFailed:
				{
					if (fstatus != FStatus::kValid && isBitSet(fPushMode, FPush::kOnlyValidField)) {
						continue;
					}

					const void *pFieldValue = GetFieldByOffset(itMetaField->offset);
					if (!m_isParentVal) {
						// write key
						IJSTI_RET_WHEN_WRITE_FAILD(
								writer.Key(itMetaField->jsonName.data(), (rapidjson::SizeType)itMetaField->jsonName.size()) );
					}
					// write value
					SerializeReq req(writer, pFieldValue, fPushMode);
					IJSTI_RET_WHEN_NOT_ZERO(
							itMetaField->serializerInterface->Serialize(req));
				}
					break;

				case FStatus::kNull:
				{
					if (!m_isParentVal) {
						// write key
						IJSTI_RET_WHEN_WRITE_FAILD(
								writer.Key(itMetaField->jsonName.data(), (rapidjson::SizeType)itMetaField->jsonName.size()) );
					}
					// write value
					IJSTI_RET_WHEN_WRITE_FAILD(writer.Null());
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

#if IJST_ENABLE_TO_JSON_OBJECT
	int DoAllFieldsToJson(FPush::Mode fPushMode, bool canMoveSrc,
						  IJST_OUT JsonValue &buffer, JsonAllocator &allocator) const
	{
		// Serialize fields to buffer

		if (!m_isParentVal)
		{
			// Init buffer to object and reserve space
			buffer.SetObject();

			do {
				const size_t unknwonSize = isBitSet(fPushMode, FPush::kIgnoreUnknown) ? 0 : m_r->unknown.MemberCount();
				const size_t maxSize = m_pMetaClass->GetFieldsInfo().size() + unknwonSize;
				if (buffer.MemberCapacity() >= maxSize) {
					break;
				}

				size_t fieldSize;
				if (isBitSet(fPushMode, FPush::kOnlyValidField)) {
					fieldSize = unknwonSize;
					for (std::vector<MetaFieldInfo>::const_iterator itMetaField = m_pMetaClass->GetFieldsInfo().begin();
						 itMetaField != m_pMetaClass->GetFieldsInfo().end(); ++itMetaField)
					{
						const EFStatus fstatus = m_r->fieldStatus[itMetaField->index];
						if (fstatus == FStatus::kValid || fstatus == FStatus::kNull) {
							++fieldSize;
						}
					}
				}
				else {
					fieldSize = maxSize;
				}
				buffer.MemberReserve((rapidjson::SizeType)fieldSize, allocator);
			} while (false);
		}

		#ifndef NDEBUG
		// assert that buffer will not reallocate memory during serialization
		const rapidjson::SizeType oldCapcity = m_isParentVal ? 0 : buffer.MemberCapacity();
		#endif

		for (std::vector<MetaFieldInfo>::const_iterator itMetaField = m_pMetaClass->GetFieldsInfo().begin();
			 itMetaField != m_pMetaClass->GetFieldsInfo().end(); ++itMetaField)
		{
			// Check field state
			const EFStatus fstatus = m_r->fieldStatus[itMetaField->index];
			switch (fstatus) {
				case FStatus::kValid:
				case FStatus::kMissing:
				case FStatus::kParseFailed:
				{
					if (fstatus != FStatus::kValid && isBitSet(fPushMode, FPush::kOnlyValidField)) {
						continue;
					}
					IJSTI_RET_WHEN_NOT_ZERO(
							DoFieldToJson(itMetaField, canMoveSrc, fPushMode, buffer, allocator) );
				}
					break;

				case FStatus::kNull:
				{
					IJSTI_RET_WHEN_NOT_ZERO(
							DoNullFieldToJson(itMetaField, buffer, allocator) );
				}
					break;


				case FStatus::kNotAField:
				default:
					// Error occurs
					assert(false);
					return Err::kInnerError;
			}

		}

		assert(m_isParentVal || buffer.MemberCapacity() == oldCapcity);
		// Unknown fields will be add in AppendUnknownToBuffer
		return 0;
	}

	template <bool kCanMoveSrc, class _TAccessor>
	static int DoToJson(_TAccessor &accessor, FPush::Mode fPushMode, IJST_OUT JsonValue &buffer,
						JsonAllocator &allocator)
	{
		const Accessor& rThis = accessor;
		IJSTI_RET_WHEN_NOT_ZERO(
				rThis.DoAllFieldsToJson(fPushMode, kCanMoveSrc, buffer, allocator) );
		if (!isBitSet(fPushMode, FPush::kIgnoreUnknown)) {
			Accessor::template AppendUnknownToBuffer<kCanMoveSrc, _TAccessor>(accessor, buffer, allocator);
		}
		return 0;
	}

	int DoFieldToJson(std::vector<MetaFieldInfo>::const_iterator itMetaField,
					  bool canMoveSrc, unsigned fPushMode,
					  JsonValue &buffer, JsonAllocator &allocator) const
	{
		// Init
		const void *pFieldValue = GetFieldByOffset(itMetaField->offset);

		if (m_isParentVal) {
			// Set buffer itself to json value
			ToJsonReq elemSerializeReq(buffer, allocator, pFieldValue, canMoveSrc, fPushMode);
			return itMetaField->serializerInterface->ToJson(elemSerializeReq);
		}
		// Add a member to buffer

		// Serialize field
		JsonValue elemOutput;
		ToJsonReq elemSerializeReq(elemOutput, allocator, pFieldValue, canMoveSrc, fPushMode);
		IJSTI_RET_WHEN_NOT_ZERO(
				itMetaField->serializerInterface->ToJson(elemSerializeReq));

		// Add member, copy field name because the fieldName store in Meta info maybe release when dynamical
		// library unload, and the memory pool should be fast to copy field name.
		// Do not check existing key, that's a feature
		rapidjson::GenericStringRef<char> fieldNameRef =
				rapidjson::StringRef(itMetaField->jsonName.data(), itMetaField->jsonName.size());
		buffer.AddMember(
				rapidjson::Value().SetString(fieldNameRef, allocator),
				elemOutput,
				allocator
		);
		return 0;
	}

	int DoNullFieldToJson(std::vector<MetaFieldInfo>::const_iterator itMetaField,
						  JsonValue &buffer, JsonAllocator &allocator) const
	{
		if (m_isParentVal) {
			buffer.SetNull();
			return 0;
		}

		// Init
		rapidjson::GenericStringRef<char> fieldNameRef =
				rapidjson::StringRef(itMetaField->jsonName.data(), itMetaField->jsonName.size());
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

	template<bool kCanMoveSrc, typename _TAccessor>
	static inline void AppendUnknownToBuffer(_TAccessor &accessor, JsonValue &buffer, JsonAllocator &allocator);
#endif // #if IJST_ENABLE_TO_JSON_OBJECT

	template<typename TJsonValue, typename Func>
	int DoFromJsonWrap(Func func, TJsonValue &stream, EUnknownMode unknownMode, bool checkField, std::string* pErrMsgOut)
	{
		size_t fieldCount = 0;
		JsonAllocator* allocator = pErrMsgOut == IJSTI_NULL ? IJSTI_NULL : new JsonAllocator();
		detail::MemoryGuarder<JsonAllocator> ptrGuarder(allocator);
		detail::DeserializeErrDoc errDoc(allocator);
		FromJsonParam param(unknownMode, checkField, errDoc, fieldCount);
		int ret = (this->*func)(stream, param);
		if (ret != 0 && pErrMsgOut != IJSTI_NULL) {
			assert(allocator != IJSTI_NULL);
			rapidjson::StringBuffer sb;
			detail::JsonWriter writer(sb);
			errDoc.errMsg.Accept(writer);
			(*pErrMsgOut) = std::string(sb.GetString(), sb.GetLength());
		}
		return ret;
	}

	/**
	 * Deserialize move from json object
	 * @note Make sure the lifecycle of allocator of the stream is longer than this object
	 */
	int DoMoveFromJson(JsonValue &stream, FromJsonParam& p)
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
			return Err::kDeserializeValueTypeError;
		}

		p.fieldCount = 0;
		// For each member
		rapidjson::Value::MemberIterator itNextRemain = stream.MemberBegin();
		for (rapidjson::Value::MemberIterator itMember = stream.MemberBegin();
			 itMember != stream.MemberEnd(); ++itMember)
		{

			// Get related field info
			const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
			const MetaFieldInfo *pMetaField = m_pMetaClass->FindFieldByJsonName(fieldName);

			if (pMetaField == IJST_NULL) {
				// Not a field in struct
				switch (p.unknownMode) {
					case UnknownMode::kKeep:
						// Move unknown fields to the front of array first
						// TODO: This is relay on the implementation details of rapidjson's object storage (array), how to check?
						if (itNextRemain != itMember) {
							itNextRemain->name.SetNull().Swap(itMember->name);
							itNextRemain->value.SetNull().Swap(itMember->value);
						}
						++itNextRemain;
						break;
					case UnknownMode::kError:
						p.errDoc.ErrorInObject("UnknownMember", fieldName);
						return Err::kDeserializeSomeUnknownMember;
					case UnknownMode::kIgnore:
						break;
					default:
						assert(false);
				}
				continue;
			}

			// Move member out of object
			JsonValue memberStream(rapidjson::kNullType);
			memberStream.Swap(itMember->value);

			IJSTI_RET_WHEN_NOT_ZERO(
					DoFieldFromJson(pMetaField, memberStream, /*canMoveSrc=*/true, p) );
			++p.fieldCount;
		}

		// Clean deserialized
		if (stream.MemberCount() != 0) {
			stream.EraseMember(itNextRemain, stream.MemberEnd());
		}
		if (p.unknownMode == UnknownMode::kKeep) {
			m_r->unknown.SetNull().Swap(stream);
		}
		else {
			m_r->unknown.SetObject();
		}

		if (p.checkField) {
			return CheckFieldState(p.errDoc);
		}
		else {
			return 0;
		}
	}

	//! Deserialize from stream
	int DoFromJson(const JsonValue &stream, FromJsonParam& p)
	{
		if (m_isParentVal) {
			// Serialize field by stream itself
			assert(m_pMetaClass->GetFieldsInfo().size() == 1);
			return DoFieldFromJson(
					&m_pMetaClass->GetFieldsInfo()[0], const_cast<JsonValue &>(stream), /*canMoveSrc=*/true, p);
		}

		// Serialize fields by members of stream
		if (!stream.IsObject()) {
			p.errDoc.ElementTypeMismatch("object", stream);
			return Err::kDeserializeValueTypeError;
		}

		m_r->unknown.SetObject();
		p.fieldCount = 0;
		// For each member
		for (rapidjson::Value::ConstMemberIterator itMember = stream.MemberBegin();
			 itMember != stream.MemberEnd(); ++itMember)
		{
			// Get related field info
			const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
			const MetaFieldInfo *pMetaField = m_pMetaClass->FindFieldByJsonName(fieldName);

			if (pMetaField == IJST_NULL)
			{
				// Not a field in struct
				switch (p.unknownMode) {
					case UnknownMode::kKeep:
						m_r->unknown.AddMember(
								rapidjson::Value().SetString(fieldName.data(), (rapidjson::SizeType)fieldName.size(), *m_pAllocator),
								rapidjson::Value().CopyFrom(itMember->value, *m_pAllocator, true),
								*m_pAllocator
						);
						break;
					case UnknownMode::kIgnore:
						break;
					case UnknownMode::kError:
						p.errDoc.ErrorInObject("UnknownMember", fieldName);
						return Err::kDeserializeSomeUnknownMember;
					default:
						assert(false);
				}
				continue;
			}

			JsonValue& memberStream = const_cast<JsonValue&>(itMember->value);
			IJSTI_RET_WHEN_NOT_ZERO(
					DoFieldFromJson(pMetaField, memberStream, /*canMoveSrc=*/false, p) );
			++p.fieldCount;
		}

		if (p.checkField) {
			return CheckFieldState(p.errDoc);
		}
		else {
			return 0;
		}
	}


	int DoFieldFromJson(const MetaFieldInfo *metaField, JsonValue &stream, bool canMoveSrc, FromJsonParam& p)
	{
		// Check nullable
		if (isBitSet(metaField->desc, FDesc::Nullable)
			&& stream.IsNull())
		{
			m_r->fieldStatus[metaField->index] = FStatus::kNull;
		}
		else
		{
			void *pField = GetFieldByOffset(metaField->offset);
			FromJsonReq elemReq(stream, *m_pAllocator, p.unknownMode, canMoveSrc, p.checkField, pField);
			FromJsonResp elemResp(p.errDoc);
			int ret = metaField->serializerInterface->FromJson(elemReq, elemResp);
			// Check return
			if (ret != 0)
			{
				m_r->fieldStatus[metaField->index] = FStatus::kParseFailed;
				p.errDoc.ErrorInObject("ErrInObject", metaField->jsonName);
				return ret;
			}
			// Check elem size
			if (isBitSet(metaField->desc, FDesc::ElemNotEmpty)
				&& elemResp.fieldCount == 0)
			{
				p.errDoc.ErrorInObject("ElemIsEmpty", metaField->jsonName);
				return Err::kDeserializeElemEmpty;
			}
			// succ
			m_r->fieldStatus[metaField->index] = FStatus::kValid;
		}
		return 0;
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

	int CheckFieldState(detail::DeserializeErrDoc& errDoc) const
	{
		// Check all required field status
		bool hasErr = false;

		for (std::vector<MetaFieldInfo>::const_iterator itField = m_pMetaClass->GetFieldsInfo().begin();
			 itField != m_pMetaClass->GetFieldsInfo().end(); ++itField)
		{
			if (isBitSet(itField->desc, FDesc::Optional))
			{
				// Optional
				continue;
			}

			const EFStatus fStatus = m_r->fieldStatus[itField->index];
			if (fStatus == FStatus::kValid
				|| fStatus == FStatus::kNull)
			{
				// Correct
				continue;
			}

			// Has error
			hasErr = true;
			errDoc.ElementAddMemberName(itField->jsonName);
		}
		if (hasErr)
		{
			errDoc.MissingMember();
			return Err::kDeserializeSomeFiledsInvalid;
		}
		return 0;
	}

	inline std::size_t GetFieldOffset(const void *ptr) const
	{
		const unsigned char *filed_ptr = static_cast<const unsigned char *>(ptr);
		return filed_ptr - m_pOuter;
	}

	inline void *GetFieldByOffset(std::size_t offset) const
	{
		return (void *) (m_pOuter + offset);
	}

	static inline bool isBitSet(unsigned int val, unsigned int bit)
	{
		return (val & bit) != 0;
	}

	typedef std::vector<EFStatus> FieldStatusType;
	// Note: Use pointers to make class Accessor be a standard-layout type struct
	struct Resource {
		rapidjson::Value unknown;
		// Should use document instead of Allocator because document can swap allocator
		rapidjson::Document ownDoc;
		FieldStatusType fieldStatus;
	};
	Resource* m_r;

	const MetaClassInfo* m_pMetaClass;
	JsonAllocator* m_pAllocator;
	const unsigned char *m_pOuter;

	bool m_isValid;
	bool m_isParentVal;
	//</editor-fold>
};

///! { implementations
template<typename _T>
inline const MetaClassInfo &MetaClassInfo::GetMetaInfo()
{
	IJSTI_TRY_INIT_META_BEFORE_MAIN(detail::MetaClassInfoIniter<_T>);
	return detail::Singleton<detail::MetaClassInfoIniter<_T> >::GetInstance()->metaClass;
}

#if IJST_ENABLE_TO_JSON_OBJECT
//! copy version of Accessor::AppendInnerToBuffer
template <>
inline void Accessor::template AppendUnknownToBuffer<false, const Accessor>(
		const Accessor &accessor, JsonValue &buffer, JsonAllocator &allocator)
{
	const Accessor& rThis = accessor;
	// Copy
	for (rapidjson::Value::ConstMemberIterator itMember = rThis.m_r->unknown.MemberBegin();
		 itMember != rThis.m_r->unknown.MemberEnd(); ++itMember)
	{
		rapidjson::Value name;
		rapidjson::Value val;
		name.CopyFrom(itMember->name, allocator, true);
		val.CopyFrom(itMember->value, allocator, true);
		buffer.AddMember(name, val, allocator);
	}
}

//! move version of Accessor::AppendInnerToBuffer
template <>
inline void Accessor::template AppendUnknownToBuffer<true, Accessor>(
		Accessor& accessor, JsonValue&buffer, JsonAllocator& allocator)
{
	Accessor& rThis = accessor;
	// append inner data to buffer
	if (&allocator == rThis.m_pAllocator) {
		// Move
		for (rapidjson::Value::MemberIterator itMember = rThis.m_r->unknown.MemberBegin();
			 itMember != rThis.m_r->unknown.MemberEnd(); ++itMember)
		{
			buffer.AddMember(itMember->name, itMember->value, allocator);
			// both itMember->name, itMember->value are null now
		}
	}
	else
	{
		// Copy
		Accessor::template AppendUnknownToBuffer<false, const Accessor>(accessor, buffer, allocator);
		// The object will be release after serialize in most suitation, so do not need clear own allocator
	}

	rThis.m_r->unknown.SetObject();
}
#endif
///! }

//! IJSTI_DEFINE_STRUCT_IMPL
//! Wrapper of IJST_DEFINE_STRUCT_IMPL_*
//! @param N			fields size
//! @param isRawVal		is struct a raw value: true/false
//! @param needGetter	need Get* function: T/F
//! @param stName		struct name
//! @param ...			fields define: [(fType, fName, sName, desc)]*
#ifdef _MSC_VER
	//! @params	N, isRawVal, needGetter, stName, ...
	#define IJSTI_DEFINE_STRUCT_IMPL(N, ...) \
		IJSTI_PP_CONCAT(IJSTI_PP_CONCAT(IJSTI_DEFINE_STRUCT_IMPL_, N)(__VA_ARGS__), )
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
		IJSTI_PP_CONCAT(IJSTI_PP_CONCAT(IJSTI_DEFINE_GETTER_IMPL_, N)(__VA_ARGS__), )
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

	#define IJSTI_STRUCT_PUBLIC_DEFINE()														\
		typedef ::ijst::Accessor _ijstStructAccessorType;										\
		_ijstStructAccessorType _;

	#define IJSTI_DEFINE_FIELD(fType, fName, ... )												\
			fType fName;

	#define IJSTI_FIELD_GETTER(fType, fName, ... )												\
			::ijst::Optional<const fType> IJSTI_PP_CONCAT(IJST_GETTER_PREFIX, fName)() const 	\
			{																					\
				if (!this->_.IsValid() || this->_.GetStatus(&fName) != ijst::FStatus::kValid)	\
					{ return ::ijst::Optional<const fType>(IJST_NULL); }						\
				return ::ijst::Optional<const fType>(&fName);									\
			}																					\
			::ijst::Optional< fType> IJSTI_PP_CONCAT(IJST_GETTER_PREFIX, fName)()				\
			{																					\
				if (!this->_.IsValid() || this->_.GetStatus(&fName) != ijst::FStatus::kValid)	\
					{ return ::ijst::Optional< fType>(IJST_NULL); }								\
				return ::ijst::Optional< fType>(&fName);										\
			}

	#define IJSTI_METAINFO_DEFINE_START(stName, N)												\
			typedef ::ijst::detail::MetaClassInfoIniter<stName> MetaInfoT;						\
			typedef ::ijst::detail::Singleton<MetaInfoT> MetaInfoS;								\
			friend MetaInfoT;																	\
			static void _InitMetaInfo(MetaInfoT* metaInfo)										\
			{																					\
				IJSTI_TRY_INIT_META_BEFORE_MAIN(MetaInfoT);										\
				/*Do not call MetaInfoS::GetInstance() int this function */			 			\
				detail::MetaClassInfoSetter mSetter(metaInfo->metaClass);						\
				mSetter.InitBegin(#stName, N, IJST_OFFSETOF(stName, _));

	#define IJSTI_METAINFO_ADD(stName, fDef)  													\
			mSetter.PushMetaField(																\
				IJSTI_IDL_FNAME_STR fDef,														\
				IJSTI_IDL_SNAME fDef, 															\
				IJST_OFFSETOF(stName, IJSTI_IDL_FNAME fDef),									\
				IJSTI_IDL_DESC fDef, 															\
				IJSTI_FSERIALIZER_INS(IJSTI_IDL_FTYPE fDef)										\
			);

	#define IJSTI_METAINFO_DEFINE_END()															\
				mSetter.InitEnd();																\
			}

}	// namespace ijst

#include "detail/ijst_repeat_def.inc"

#endif //_IJST_HPP_INCLUDE_