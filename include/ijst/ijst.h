//
// Created by h46incon on 2017/9/19.
//

#ifndef _IJST_HPP_INCLUDE_
#define _IJST_HPP_INCLUDE_

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
    IJSTI_DEFINE_STRUCT_IMPL(1, true, F, stName, (type, fName, "", desc))
//! @brief Declare a ijst struct which represent a value instead of members insides a object with getter
//! @ingroup IJST_MACRO_API
#define IJST_DEFINE_VALUE_WITH_GETTER(stName, type, fName, desc)	\
    IJSTI_DEFINE_STRUCT_IMPL(1, true, T, stName, (type, fName, "", desc))

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
struct SerFlag {
	typedef unsigned int Flag;
	//! does not set any option.
	static const Flag kNoneFlag							= 0x00000000;
	//! set if serialize only valid fields, otherwise will serialize all fields.
	static const Flag kOnlyValidField					= 0x00000001;
	//! set if ignore unknown fields, otherwise will serialize all unknown fields.
	static const Flag kIgnoreUnknown					= 0x00000002;
};

/**
 * @brief Deserialization options about fields.
 *
 * Options can be combined by bitwise OR operator (|).
 */
struct DeserFlag {
	typedef unsigned int Flag;
	//! does not set any option.
	static const Flag kNoneFlag							= 0x00000000;
	//! set if return error when meet unknown fields, otherwise will keep all unknown fields.
	static const Flag kErrorWhenUnknown					= 0x00000001;
	//! set if ignore unknown fields, otherwise will keep all unknown fields.
	static const Flag kIgnoreUnknown					= 0x00000002;
	//! set if ignore field status, otherwise will check if field status is matched requirement
	static const Flag kNotCheckFieldStatus				= 0x00000004;
};

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
struct ErrorCode {
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

	//! return Err::kWriteFailed if action return false
	#define IJSTI_RET_WHEN_WRITE_FAILD(action) 					\
		do { if(!(action)) return ErrorCode::kWriteFailed; } while (false)
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
			SerFlag::Flag serFlag;

			// Pointer of field to serialize.
			// The actual type of field should be decide in the derived class
			const void* pField;

			HandlerBase& writer;

			SerializeReq(HandlerBase& _writer, const void *_pField, SerFlag::Flag _serFlag)
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
			JsonValue& stream;
			JsonAllocator& allocator;

			// true if move context in stream to avoid copy when possible
			bool canMoveSrc;

			DeserFlag::Flag deserFlag;

			FromJsonReq(JsonValue &_stream, JsonAllocator &_allocator,
						DeserFlag::Flag _deserFlag, bool _canMoveSrc,
						void *_pField)
					: pFieldBuffer(_pField)
					  , stream(_stream)
					  , allocator(_allocator)
					  , canMoveSrc(_canMoveSrc)
					  , deserFlag(_deserFlag)
			{ }
		};

		struct FromJsonResp {
			size_t fieldCount;
			ErrorDocSetter& errDoc;

			explicit FromJsonResp(ErrorDocSetter& _errDoc) :
					fieldCount(0),
					errDoc(_errDoc)
			{ }

		};

		virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp)= 0;

		virtual void ShrinkAllocator(void * pField)
		{ (void)pField; }
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
		virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE = 0;
		virtual void ShrinkAllocator(void * pField) IJSTI_OVERRIDE {}
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

		virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
		{
			_T *pField = (_T *) req.pFieldBuffer;
			return pField->_.IFromJson(req, resp);
		}

		virtual void ShrinkAllocator(void *pField) IJSTI_OVERRIDE
		{
			((_T*)pField)->_.IShrinkAllocator(pField);
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
	 * @param writer 		writer
	 * @param serFlag	 	Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return				Error code
	 */
	int Serialize(HandlerBase& writer, SerFlag::Flag serFlag = SerFlag::kNoneFlag)  const
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
	int Serialize(IJST_OUT std::string &strOutput, SerFlag::Flag serFlag = SerFlag::kNoneFlag)  const
	{
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		HandlerWrapper<rapidjson::Writer<rapidjson::StringBuffer> > writerWrapper(writer);
		IJSTI_RET_WHEN_NOT_ZERO(DoSerialize(writerWrapper, serFlag));

		strOutput = std::string(buffer.GetString(), buffer.GetSize() / sizeof(rapidjson::StringBuffer::Ch));
		return 0;
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
	int Deserialize(const char *cstrInput, std::size_t length,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::Document *pErrDocOut = IJST_NULL)
	{
		// The new object will call FromJson() interfaces soon in most situation
		// So clear own allocator will not bring much benefice
		m_pAllocator = &m_r->ownDoc.GetAllocator();
		rapidjson::Document doc(m_pAllocator);
		doc.Parse<parseFlags>(cstrInput, length);
		if (doc.HasParseError())
		{
			detail::ErrorDocSetter errDocSetter(pErrDocOut);
			errDocSetter.ParseFailed(doc.GetParseError());
			return ErrorCode::kDeserializeParseFaild;
		}

		return DoFromJsonWrap<JsonValue>(&Accessor::DoMoveFromJson, doc, deserFlag, pErrDocOut);
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
	int Deserialize(const char *cstrInput, std::size_t length,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::Document *pErrDocOut = IJST_NULL)
	{
		return this->template Deserialize<rapidjson::kParseDefaultFlags>(
				cstrInput, length, deserFlag, pErrDocOut);
	}

	/**
	 * @brief Deserialize from std::string.
	 *
	 * @param strInput			Input string
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 */
	inline int Deserialize(const std::string &strInput,
						   DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
						   rapidjson::Document *pErrDocOut = IJST_NULL)
	{
		return Deserialize(strInput.data(), strInput.size(), deserFlag, pErrDocOut);
	}

	/**
	 * @brief Deserialize from std::string.
	 *
	 * @param strInput			Input string
	 * @param errMsgOut			Error message output
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @return					Error code
	 */
	inline int Deserialize(const std::string &strInput, IJST_OUT std::string& errMsgOut,
						   DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag)
	{
		rapidjson::Document errDoc;
		int ret = Deserialize(strInput.data(), strInput.size(), deserFlag, &errDoc);
		if (ret != 0) {
			rapidjson::StringBuffer sb;
			rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
			errDoc.Accept(writer);
			errMsgOut = std::string(sb.GetString(), sb.GetSize() / sizeof(rapidjson::StringBuffer::Ch));
		}
		return ret;
	}

	/**
	 * @brief Deserialize insitu from str.
	 *
	 * @tparam parseFlags		parseFlags of rapidjson parse method
	 *
	 * @param str				Input C string
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note The context in str may be changed after deserialize
	 * @note Make sure the lifecycle of str is longer than this object
	 */
	 template<unsigned parseFlags>
	int DeserializeInsitu(char *str,
						  DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
						  rapidjson::Document *pErrDocOut = IJST_NULL)
	{
		// The new object will call FromJson() interfaces in most situation
		// So clear own allocator will not bring much benefice
		m_pAllocator = &m_r->ownDoc.GetAllocator();
		rapidjson::Document doc(m_pAllocator);
		doc.ParseInsitu<parseFlags>(str);
		if (doc.HasParseError())
		{
			detail::ErrorDocSetter errDocSetter(pErrDocOut);
			errDocSetter.ParseFailed(doc.GetParseError());
			return ErrorCode::kDeserializeParseFaild;
		}
		return DoFromJsonWrap<JsonValue>(&Accessor::DoMoveFromJson, doc, deserFlag, pErrDocOut);
	}

	/**
	 * @brief Deserialize insitu from str.
	 *
	 * @param str				Input C string
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note The context in str may be changed after deserialize
	 * @note Make sure the lifecycle of str is longer than this object
	 */
	int DeserializeInsitu(char *str,
						  DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
						  rapidjson::Document *pErrDocOut = IJST_NULL)
	{
		return this->template DeserializeInsitu<rapidjson::kParseDefaultFlags>(str, deserFlag, pErrDocOut);
	}

#if IJST_ENABLE_FROM_JSON_OBJECT || IJSTI_DOXYGEN_RUNNING
	/**
	 * @brief Deserialize from json object.
	 *
	 * @param srcJson			Input json object
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note 	Need to set IJST_ENABLE_FROM_JSON_OBJECT to 1 to enable this method
	 * @note 	It will not copy const string reference in source json. Be careful if handler such situation,
	 * 			e.g, json object is generated by ParseInsitu().
	 */
	inline int FromJson(const JsonValue &srcJson,
						DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
						rapidjson::Document *pErrDocOut = IJST_NULL)
	{
		return DoFromJsonWrap<const JsonValue>(&Accessor::DoFromJson, srcJson, deserFlag, pErrDocOut);
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
	 * @note 	Need to set IJST_ENABLE_FROM_JSON_OBJECT to 1 to enable this method
	 * @note 	The source document may be changed after deserialize
	 * @note 	Make sure srcDocStolen use own allocator, or use allocator in this object
	 * @note 	It will not copy const string reference in source json. Be careful if handler such situation,
	 * 			e.g, json object is generated by ParseInsitu().
	 */
	inline int MoveFromJson(rapidjson::Document &srcDocStolen,
							DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
							rapidjson::Document *pErrDocOut = IJST_NULL)
	{
		// Store document to manager allocator
		m_r->ownDoc.Swap(srcDocStolen);
		m_pAllocator = &m_r->ownDoc.GetAllocator();
		return DoFromJsonWrap<JsonValue>(&Accessor::DoMoveFromJson, m_r->ownDoc, deserFlag, pErrDocOut);
	}
#endif

	/**
	 * @brief Shrink allocator of each member by recopy unknown fields
	 *
	 * The allocator of source json after move deserialize. The memory is wasted if the
	 * deserialized object exists in long time.
	 */
	inline void ShrinkAllocator()
	{
		DoShrinkAllocator();
	}

private:
	// #region Implement SerializeInterface
	template <typename, typename> friend class detail::FSerializer;
	typedef detail::SerializerInterface::SerializeReq SerializeReq;
	inline int ISerialize(const SerializeReq &req) const
	{
		assert(req.pField == this);
		return DoSerialize(req.writer, req.serFlag);
	}

	typedef detail::SerializerInterface::FromJsonReq FromJsonReq;
	typedef detail::SerializerInterface::FromJsonResp FromJsonResp;
	struct FromJsonParam{
		DeserFlag::Flag deserFlag;
		detail::ErrorDocSetter& errDoc;
		size_t& fieldCount;

		FromJsonParam(DeserFlag::Flag _deserFlag, detail::ErrorDocSetter& _errDoc, size_t& _fieldCount)
				: deserFlag(_deserFlag), errDoc(_errDoc), fieldCount(_fieldCount)
		{}
	};

	inline int IFromJson(const FromJsonReq &req, IJST_OUT FromJsonResp& resp)
	{
		assert(req.pFieldBuffer == this);

		m_pAllocator = &req.allocator;

		FromJsonParam param(req.deserFlag, resp.errDoc, resp.fieldCount);
		if (req.canMoveSrc) {
			return DoMoveFromJson(req.stream, param);
		}
		else {
			return DoFromJson(req.stream, param);
		}
	}

	inline void IShrinkAllocator(void* pField)
	{
		assert(pField == this);
		DoShrinkAllocator();
	}

	// #endregion

	//! Serialize to string using SAX API
	int DoSerialize(HandlerBase &writer, SerFlag::Flag serFlag) const
	{
		if (m_isParentVal) {
			return DoSerializeFields(writer, serFlag);
			// Unknown will be ignored
		}

		IJSTI_RET_WHEN_WRITE_FAILD(writer.StartObject());

		// Write fields
		IJSTI_RET_WHEN_NOT_ZERO(DoSerializeFields(writer, serFlag));

		// Write buffer if need
		if (!IsBitSet(serFlag, SerFlag::kIgnoreUnknown))
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

	int DoSerializeFields(HandlerBase &writer, SerFlag::Flag serFlag) const
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
					if (fstatus != FStatus::kValid && IsBitSet(serFlag, SerFlag::kOnlyValidField)) {
						continue;
					}

					const void *pFieldValue = GetFieldByOffset(itMetaField->offset);
					if (!m_isParentVal) {
						// write key
						IJSTI_RET_WHEN_WRITE_FAILD(
								writer.Key(itMetaField->jsonName.data(), (rapidjson::SizeType)itMetaField->jsonName.size()) );
					}
					// write value
					SerializeReq req(writer, pFieldValue, serFlag);
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
					return ErrorCode::kInnerError;
			}
		}
		return 0;
	}

	template<typename TJsonValue, typename Func>
	int DoFromJsonWrap(Func func, TJsonValue &stream, DeserFlag::Flag deserFlag, rapidjson::Document* pErrDocOut)
	{
		size_t fieldCount = 0;
		detail::ErrorDocSetter errDoc(pErrDocOut);
		FromJsonParam param(deserFlag, errDoc, fieldCount);
		return (this->*func)(stream, param);
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
			return ErrorCode::kDeserializeValueTypeError;
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
				if (IsBitSet(p.deserFlag, DeserFlag::kErrorWhenUnknown)) {
					p.errDoc.ErrorInObject("UnknownMember", fieldName);
					return ErrorCode::kDeserializeSomeUnknownMember;
				}
				if (!IsBitSet(p.deserFlag, DeserFlag::kIgnoreUnknown)) {
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
		if (IsBitSet(p.deserFlag, DeserFlag::kIgnoreUnknown)) {
			m_r->unknown.SetObject();
		}
		else {
			m_r->unknown.SetNull().Swap(stream);
		}

		if (!IsBitSet(p.deserFlag, DeserFlag::kNotCheckFieldStatus)) {
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
			return ErrorCode::kDeserializeValueTypeError;
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

			if (pMetaField == IJST_NULL) {
				// Not a field in struct
				if (IsBitSet(p.deserFlag, DeserFlag::kErrorWhenUnknown)) {
					p.errDoc.ErrorInObject("UnknownMember", fieldName);
					return ErrorCode::kDeserializeSomeUnknownMember;
				}
				if (!IsBitSet(p.deserFlag, DeserFlag::kIgnoreUnknown)) {
					m_r->unknown.AddMember(
							rapidjson::Value().SetString(fieldName.data(), (rapidjson::SizeType)fieldName.size(), *m_pAllocator),
							rapidjson::Value().CopyFrom(itMember->value, *m_pAllocator),
							*m_pAllocator
					);
				}
				continue;
			}

			JsonValue& memberStream = const_cast<JsonValue&>(itMember->value);
			IJSTI_RET_WHEN_NOT_ZERO(
					DoFieldFromJson(pMetaField, memberStream, /*canMoveSrc=*/false, p) );
			++p.fieldCount;
		}

		if (!IsBitSet(p.deserFlag, DeserFlag::kNotCheckFieldStatus)) {
			return CheckFieldState(p.errDoc);
		}
		else {
			return 0;
		}
	}


	int DoFieldFromJson(const MetaFieldInfo *metaField, JsonValue &stream, bool canMoveSrc, FromJsonParam& p)
	{
		// Check nullable
		if (IsBitSet(metaField->desc, FDesc::Nullable)
			&& stream.IsNull())
		{
			m_r->fieldStatus[metaField->index] = FStatus::kNull;
		}
		else
		{
			void *pField = GetFieldByOffset(metaField->offset);
			FromJsonReq elemReq(stream, *m_pAllocator, p.deserFlag, canMoveSrc, pField);
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
			if (IsBitSet(metaField->desc, FDesc::ElemNotEmpty)
				&& elemResp.fieldCount == 0)
			{
				p.errDoc.ErrorInObject("ElemIsEmpty", metaField->jsonName);
				return ErrorCode::kDeserializeElemEmpty;
			}
			// succ
			m_r->fieldStatus[metaField->index] = FStatus::kValid;
		}
		return 0;
	}

	void DoShrinkAllocator()
	{
		// Shrink allocator of each field
		for (std::vector<MetaFieldInfo>::const_iterator itFieldInfo = m_pMetaClass->GetFieldsInfo().begin();
			 itFieldInfo != m_pMetaClass->GetFieldsInfo().end(); ++itFieldInfo)
		{
			void *pField = GetFieldByOffset(itFieldInfo->offset);
			itFieldInfo->serializerInterface->ShrinkAllocator(pField);
		}

		// Shrink self allocator
		detail::ShrinkAllocatorWithOwnDoc(m_r->ownDoc, m_r->unknown, m_pAllocator);
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

	int CheckFieldState(detail::ErrorDocSetter& errDoc) const
	{
		// Check all required field status
		bool hasErr = false;

		for (std::vector<MetaFieldInfo>::const_iterator itFieldInfo = m_pMetaClass->GetFieldsInfo().begin();
			 itFieldInfo != m_pMetaClass->GetFieldsInfo().end(); ++itFieldInfo)
		{
			if (IsBitSet(itFieldInfo->desc, FDesc::Optional))
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
			return ErrorCode::kDeserializeSomeFiledsInvalid;
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

	static inline bool IsBitSet(unsigned int val, unsigned int bit)
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

template<typename _T>
inline const MetaClassInfo &MetaClassInfo::GetMetaInfo()
{
	IJSTI_TRY_INIT_META_BEFORE_MAIN(detail::MetaClassInfoIniter<_T>);
	return detail::Singleton<detail::MetaClassInfoIniter<_T> >::GetInstance()->metaClass;
}

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
			::ijst::Optional<const fType > IJSTI_PP_CONCAT(IJST_GETTER_PREFIX, fName)() const 	\
			{																					\
				if (!this->_.IsValid() || this->_.GetStatus(&fName) != ijst::FStatus::kValid)	\
					{ return ::ijst::Optional<const fType >(IJST_NULL); }						\
				return ::ijst::Optional<const fType >(&fName);									\
			}																					\
			::ijst::Optional< fType > IJSTI_PP_CONCAT(IJST_GETTER_PREFIX, fName)()				\
			{																					\
				if (!this->_.IsValid() || this->_.GetStatus(&fName) != ijst::FStatus::kValid)	\
					{ return ::ijst::Optional< fType >(IJST_NULL); }							\
				return ::ijst::Optional< fType >(&fName);										\
			}

	#define IJSTI_METAINFO_DEFINE_START(stName, N)												\
			typedef ::ijst::detail::MetaClassInfoIniter< stName > _MetaInfoT;					\
			typedef ::ijst::detail::Singleton<_MetaInfoT> _MetaInfoS;							\
			friend _MetaInfoT;																	\
			static void _InitMetaInfo(_MetaInfoT* metaInfo)										\
			{																					\
				IJSTI_TRY_INIT_META_BEFORE_MAIN(_MetaInfoT);									\
				/*Do not call MetaInfoS::GetInstance() int this function */			 			\
				::ijst::detail::MetaClassInfoSetter mSetter(metaInfo->metaClass);				\
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