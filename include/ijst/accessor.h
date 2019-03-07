#ifndef IJST_ACCESSOR_HPP_INCLUDE_
#define IJST_ACCESSOR_HPP_INCLUDE_

#include "ijst.h"
#include "detail/utils.h"
#include "detail/detail.h"

/**
 * @ingroup IJST_CONFIG
 *
 *  ijst use rapidjson::kParseDefaultFlags when parsing in default.
 *	User can override it by defining IJST_PARSE_DEFAULT_FLAGS macro.
 */
#ifndef IJST_PARSE_DEFAULT_FLAGS
	#define IJST_PARSE_DEFAULT_FLAGS rapidjson::kParseDefaultFlags
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
#define IJST_SET(obj, field, val)				\
		do { (obj)._.MarkValid(&((obj).field) ); (obj).field = (val); } while (false)

//! @brief Helper declare macro with comma.
//! @ingroup IJST_MACRO_API
#if __cplusplus < 201103L
	#define IJST_TYPE(...)			::ijst::detail::ArgumentType<void(__VA_ARGS__)>::type
#else
	#define IJST_TYPE(...)			decltype(__VA_ARGS__())
#endif

namespace ijst {

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
class Optional<T, /*EnableIf*/ typename detail::HasType<typename T::_ijst_AccessorType>::Void>
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
		if (m_pVal == NULL) {
			return &empty;
		}
		else {
			return m_pVal;
		}
	}
};

/**
 * @brief Get meta information for ijst struct T.
 *
 * @tparam T 	ijst struct
 * @return		MetaClassInfo instance
 */
template<typename T>
const MetaClassInfo<typename T::_ijst_Ch>& GetMetaInfo()
{
	return detail::Singleton<detail::MetaClassInfoTyped<T> >::GetInstance().metaClass;
}

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
	explicit Accessor(const MetaClassInfo<Ch>* pMetaClass, bool isParentVal, bool isValid)
			: m_r(NULL)
	{
		IJST_ASSERT(!isParentVal || pMetaClass->GetFieldsInfo().size() == 1);

		// Allocate fieldStatus followed by m_r
		const size_t sizeFieldStatus = pMetaClass->GetFieldsInfo().size();
		m_r = static_cast<Resource *>(operator new(
				sizeof(Resource) + sizeFieldStatus * sizeof(EFStatus)));
		m_r->fieldStatus = reinterpret_cast<EFStatus*>(m_r + 1);

		m_r->pMetaClass = pMetaClass;
		InitOuterPtr();
		m_r->isValid = isValid;
		m_r->isParentVal = isParentVal;

		// Init fieldStatus with kMissing
		for (size_t i = 0; i < sizeFieldStatus; ++i) {
			m_r->fieldStatus[i] = FStatus::kMissing;
		}
		new(&m_r->unknown)TValue(rapidjson::kObjectType);
		new(&m_r->ownDoc) TDocument();
		m_r->pAllocator = &m_r->ownDoc.GetAllocator();
	}

	//! Copy constructor
	Accessor(const Accessor &rhs)
			: m_r(NULL)
	{
		assert(this != &rhs);

		// Allocate fieldStatus followed by m_r
		const size_t sizeFieldStatus = rhs.m_r->pMetaClass->GetFieldsInfo().size();
		m_r = static_cast<Resource *>(operator new(
				sizeof(Resource) + sizeFieldStatus * sizeof(EFStatus)));
		m_r->fieldStatus = reinterpret_cast<EFStatus*>(m_r + 1);

		m_r->pMetaClass = rhs.m_r->pMetaClass;
		InitOuterPtr();
		m_r->isValid = rhs.m_r->isValid;
		m_r->isParentVal = rhs.m_r->isParentVal;

		// Init fieldStatus from rhs
		for (size_t i = 0; i < sizeFieldStatus; ++i) {
			m_r->fieldStatus[i] = rhs.m_r->fieldStatus[i];
		}
		new(&m_r->unknown)TValue(rapidjson::kObjectType);
		new(&m_r->ownDoc) TDocument();
		m_r->pAllocator = &m_r->ownDoc.GetAllocator();

		m_r->unknown.CopyFrom(rhs.m_r->unknown, *(m_r->pAllocator));
	}

#if IJST_HAS_CXX11_RVALUE_REFS
	//! Move copy constructor
	//! @note Do not use source object after move
	Accessor(Accessor &&rhs) IJSTI_NOEXCEPT
		: m_r(NULL)
	{
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
		m_r = NULL;
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
		rhs.m_r = NULL;

		InitOuterPtr();
	}

	bool IsValid() const { return m_r->isValid; }
	bool IsParentVal() const { return m_r->isParentVal; }
	const MetaClassInfo<Ch>& GetMetaInfo() const { return *(m_r->pMetaClass); }

	/*
	 * Field accessor.
	 */

	//! Check if pField is a filed in this object.
	bool HasField(const void *pField) const
	{
		size_t offset = GetFieldOffset(pField);
		return (m_r->pMetaClass->FindIndex(offset) != -1);
	}

	//! Mark status of field to FStatus::kValid.
	void MarkValid(const void* pField) { MarkFieldStatus(pField, FStatus::kValid); }
	//! Mark status of field to FStatus::kNull.
	void MarkNull(const void* pField) { MarkFieldStatus(pField, FStatus::kNull); }
	//! Mark status of field to FStatus::kMissing.
	void MarkMissing(const void* pField) { MarkFieldStatus(pField, FStatus::kMissing); }

	//! Get status of field.
	EFStatus GetStatus(const void *pField) const
	{
		const size_t offset = GetFieldOffset(pField);
		const int index = m_r->pMetaClass->FindIndex(offset);
		return index == -1 ? FStatus::kNotAField : m_r->fieldStatus[index];
	}

	//! Get unknwon fields
	rapidjson::GenericValue<Encoding> &GetUnknown() { return m_r->unknown; }
	const rapidjson::GenericValue<Encoding> &GetUnknown() const { return m_r->unknown; }

	/**
	 * @brief Get allocator used in object.
	 *
	 * The inner allocator is own allocator when init,
	 * but may change to other allocator when calling SetMembersAllocator() or Deserialize().
	 */
	rapidjson::MemoryPoolAllocator<> &GetAllocator() { return *(m_r->pAllocator); }
	const rapidjson::MemoryPoolAllocator<> &GetAllocator() const { return *(m_r->pAllocator); }

	/**
	 * @brief Get own allocator that used to manager resource.
	 *
	 * User could use the returned value to check if this object use outer allocator.
	 */
	rapidjson::MemoryPoolAllocator<> &GetOwnAllocator() { return m_r->ownDoc.GetAllocator(); }
	const rapidjson::MemoryPoolAllocator<> &GetOwnAllocator() const { return m_r->ownDoc.GetAllocator(); }

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
		if (m_r->isValid && GetStatus(&field) == ijst::FStatus::kValid) {
			return ::ijst::Optional<const T>(&field);
		}
		else {
			return ::ijst::Optional<const T>(NULL);
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
		if (m_r->isValid && GetStatus(&field) == ijst::FStatus::kValid) {
			return ::ijst::Optional<T>(&field);
		}
		else {
			return ::ijst::Optional<T>(NULL);
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
	 * @tparam TargetEncoding	encoding of output
	 *
	 * @param strOutput 		The output of result
	 * @param serFlag 			Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return					Error code
	 *
	 * @note use Serialize(HandlerBase<Ch>, SerFlag::Flag) to serialize to string with specify char_traits and allocator
	 */
	template<typename TargetEncoding>
	int Serialize(IJST_OUT std::basic_string<typename TargetEncoding::Ch> &strOutput, SerFlag::Flag serFlag = SerFlag::kNoneFlag) const
	{
		typedef rapidjson::GenericStringBuffer<TargetEncoding> TStringBuffer;
		typedef rapidjson::Writer<TStringBuffer, Encoding, TargetEncoding> TWriter;

		TStringBuffer buffer;
		TWriter writer(buffer);
		HandlerWrapper<TWriter> writerWrapper(writer);
		IJSTI_RET_WHEN_NOT_ZERO(DoSerialize(writerWrapper, serFlag));

		strOutput = std::basic_string<typename TargetEncoding::Ch>(buffer.GetString(), buffer.GetSize() / sizeof(typename TStringBuffer::Ch));
		return 0;
	}

	/**
	 * @brief Serialize the structure to string.
	 *
	 * @param strOutput 	The output of result
	 * @param serFlag 		Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return				Error code
	 */
	int Serialize(IJST_OUT std::basic_string<Ch> &strOutput, SerFlag::Flag serFlag = SerFlag::kNoneFlag) const
	{
		return this->template Serialize<Encoding>(strOutput, serFlag);
	}

	/**
	 * @brief Deserialize from C-style string with encoding
	 *
	 * @tparam parseFlags		parseFlags of rapidjson parse method
	 * @tparam SourceEncoding	encoding of source string
	 *
	 * @param cstrInput			Input C string
	 * @param length			Length of string
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note It will free own allocator
	 * @note It may cause compile error when SourceEncoding::Ch is not char with rapidJSON v1.1.0.
	 * 		The bug is fixed in HEAD version of rapidJSON.
	 */
	template <unsigned parseFlags, typename SourceEncoding>
	int Deserialize(const typename SourceEncoding::Ch* cstrInput, std::size_t length,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::GenericDocument<Encoding> *pErrDocOut = NULL)
	{
		ResetAllocator();

		if (detail::Util::IsBitSet(deserFlag, DeserFlag::kMoveFromIntermediateDoc)) {
			TDocument doc(m_r->pAllocator);
			doc.template Parse<parseFlags, SourceEncoding>(cstrInput, length);
			IJSTI_RET_WHEN_PARSE_ERROR(doc, Encoding);
			return DoFromJsonWrap<TValue>(&Accessor::DoMoveFromJson, doc, deserFlag, pErrDocOut);
		}
		else {
			TDocument doc;
			doc.template Parse<parseFlags, SourceEncoding>(cstrInput, length);
			IJSTI_RET_WHEN_PARSE_ERROR(doc, Encoding);
			return DoFromJsonWrap<const TValue>(&Accessor::DoFromJson, doc, deserFlag, pErrDocOut);
		}
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
	 * @note It will free own allocator
	 */
	template <unsigned parseFlags>
	int Deserialize(const Ch* cstrInput, std::size_t length,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::GenericDocument<Encoding> *pErrDocOut = NULL)
	{
		return this->template Deserialize<parseFlags, Encoding> (
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
	 * @note It will free own allocator
	 */
	int Deserialize(const Ch* cstrInput, std::size_t length,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::GenericDocument<Encoding> *pErrDocOut = NULL)
	{
		return this->template Deserialize<IJST_PARSE_DEFAULT_FLAGS>(
				cstrInput, length, deserFlag, pErrDocOut);
	}

	/**
	 * @brief Deserialize from C-style string with encoding
	 *
	 * @tparam parseFlags		parseFlags of rapidjson parse method
	 * @tparam SourceEncoding	encoding of source string
	 *
	 * @param cstrInput			Input C string
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note It will free own allocator
	 */
	template <unsigned parseFlags, typename SourceEncoding>
	int Deserialize(const typename SourceEncoding::Ch* cstrInput,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::GenericDocument<Encoding> *pErrDocOut = NULL)
	{
		ResetAllocator();

		if (detail::Util::IsBitSet(deserFlag, DeserFlag::kMoveFromIntermediateDoc)) {
			TDocument doc(m_r->pAllocator);
			doc.template Parse<parseFlags, SourceEncoding>(cstrInput);
			IJSTI_RET_WHEN_PARSE_ERROR(doc, Encoding);
			return DoFromJsonWrap<TValue>(&Accessor::DoMoveFromJson, doc, deserFlag, pErrDocOut);
		}
		else {
			TDocument doc;
			doc.template Parse<parseFlags, SourceEncoding>(cstrInput);
			IJSTI_RET_WHEN_PARSE_ERROR(doc, Encoding);
			return DoFromJsonWrap<const TValue>(&Accessor::DoFromJson, doc, deserFlag, pErrDocOut);
		}
	}

	/**
	 * @brief Deserialize from C-style string.
	 *
	 * @tparam parseFlags		parseFlags of rapidjson parse method
	 *
	 * @param cstrInput			Input C string
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note It will free own allocator
	 */
	template <unsigned parseFlags>
	int Deserialize(const Ch *cstrInput,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::GenericDocument<Encoding> *pErrDocOut = NULL)
	{
		return this->template Deserialize<parseFlags, Encoding> (
				cstrInput, deserFlag, pErrDocOut);
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
	 * @note It will free own allocator
	 */
	int Deserialize(const Ch *cstrInput,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::GenericDocument<Encoding> *pErrDocOut = NULL)
	{
		return this->template Deserialize<IJST_PARSE_DEFAULT_FLAGS>(
				cstrInput, deserFlag, pErrDocOut);
	}

	/**
	 * @brief Deserialize from std::basic_string.
	 *
	 * @param strInput			Input string
	 * @param deserFlag	 		Deserialization options, options can be combined by bitwise OR operator (|)
	 * @param pErrDocOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note It will free own allocator
	 */
	int Deserialize(const std::basic_string<Ch> &strInput,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					rapidjson::GenericDocument<Encoding> *pErrDocOut = NULL)
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
	 *
	 * @note It will free own allocator
	 */
	int Deserialize(const std::basic_string<Ch> &strInput, IJST_OUT std::basic_string<Ch>& errMsgOut,
					DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag)
	{
		typedef rapidjson::GenericStringBuffer<Encoding> TStringBuffer;
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
	 * @note It will free own allocator
	 * @note It will not copy const string reference in source json. Be careful if handler such situation,
	 * 			e.g, json object is generated by ParseInsitu().
	 */
	int FromJson(const rapidjson::GenericValue<Encoding> &srcJson,
				 DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
				 rapidjson::GenericDocument<Encoding> *pErrDocOut = NULL)
	{
		ResetAllocator();
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
	 * @note	Handler allocator VERY CAREFULLY. see DeserFlag::kMoveFromIntermediateDoc.
	 * @note 	It will free own allocator
	 * @note 	The source document may be changed after deserialize
	 * @note 	Make sure srcDocStolen use own allocator, or use allocator in this object
	 * @note 	It will not copy const string reference in source json. Be careful if handler such situation,
	 * 			e.g, json object is generated by ParseInsitu().
	 *
	 * @see DeserFlag::kMoveFromIntermediateDoc
	 */
	int MoveFromJson(rapidjson::GenericDocument<Encoding> &srcDocStolen,
					 DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag,
					 rapidjson::GenericDocument<Encoding> *pErrDocOut = NULL)
	{
		// Store document to manager allocator
		m_r->ownDoc.GetAllocator().Clear();
		m_r->ownDoc.Swap(srcDocStolen);
		m_r->pAllocator = &m_r->ownDoc.GetAllocator();
		return DoFromJsonWrap<TValue>(&Accessor::DoMoveFromJson, m_r->ownDoc, deserFlag, pErrDocOut);
	}

	/**
	 * @brief Shrink allocator of each member by recopy unknown fields using own allocator
	 *
	 * @see DeserFlag::kMoveFromIntermediateDoc, MoveFromJson
	 */
	void ShrinkAllocator()
	{
		DoShrinkAllocator();
	}

private:
	typedef rapidjson::GenericDocument<Encoding> TDocument;
	typedef rapidjson::GenericValue<Encoding> TValue;
	typedef MetaFieldInfo<Ch> TMetaFieldInfo;
	typedef MetaClassInfo<Ch> TMetaClassInfo;

	// #region Implement SerializeInterface
	template <typename, typename, typename> friend class detail::FSerializer;
	typedef typename detail::SerializerInterface<Encoding>::SerializeReq SerializeReq;
	int ISerialize(const SerializeReq &req) const
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

	int IFromJson(const FromJsonReq &req, IJST_OUT FromJsonResp& resp)
	{
		assert(req.pFieldBuffer == this);

		FromJsonParam param(req.deserFlag, resp.errDoc);
		if (req.canMoveSrc) {
			m_r->pAllocator = &req.allocator;
			return DoMoveFromJson(req.stream, param);
		}
		else {
			ResetAllocator();
			return DoFromJson(req.stream, param);
		}
	}

	void IShrinkAllocator(void* pField)
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
		if (m_r->isParentVal) {
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
			for (typename TValue::ConstMemberIterator itMember = m_r->unknown.MemberBegin(), itEnd = m_r->unknown.MemberEnd();
				 itMember != itEnd; ++itMember)
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
		IJST_ASSERT(!m_r->isParentVal || m_r->pMetaClass->GetFieldsInfo().size() == 1);
		for (typename std::vector<TMetaFieldInfo>::const_iterator
					 itMetaField = m_r->pMetaClass->GetFieldsInfo().begin(), itEnd = m_r->pMetaClass->GetFieldsInfo().end();
			 itMetaField != itEnd; ++itMetaField)
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
					if (!m_r->isParentVal) {
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

					if (!m_r->isParentVal) {
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
		if (m_r->isParentVal) {
			// Set field by stream itself
			assert(m_r->pMetaClass->GetFieldsInfo().size() == 1);
			return DoFieldFromJson(
					&m_r->pMetaClass->GetFieldsInfo()[0], stream, /*canMoveSrc=*/true, p);
		}

		// Set fields by members of stream
		if (!stream.IsObject()) {
			p.errDoc.ElementTypeMismatch("object", stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		// For each member
		typename TValue::MemberIterator itNextRemain = stream.MemberBegin();
		for (typename TValue::MemberIterator itMember = stream.MemberBegin(), itEnd = stream.MemberEnd();
			 itMember != itEnd; ++itMember)
		{

			// Get related field info
			const TMetaFieldInfo *pMetaField =
					m_r->pMetaClass->FindFieldByJsonName(itMember->name.GetString(), itMember->name.GetStringLength());

			if (pMetaField == NULL) {
				// Not a field in struct
				if (detail::Util::IsBitSet(p.deserFlag, DeserFlag::kErrorWhenUnknown)) {
					p.errDoc.UnknownMember(detail::GetJsonStr(itMember->name));
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
		if (m_r->isParentVal) {
			// Serialize field by stream itself
			assert(m_r->pMetaClass->GetFieldsInfo().size() == 1);
			return DoFieldFromJson(
					&m_r->pMetaClass->GetFieldsInfo()[0], const_cast<TValue &>(stream), /*canMoveSrc=*/true, p);
		}

		// Serialize fields by members of stream
		if (!stream.IsObject()) {
			p.errDoc.ElementTypeMismatch("object", stream);
			return ErrorCode::kDeserializeValueTypeError;
		}

		m_r->unknown.SetObject();
		// For each member
		for (typename TValue::ConstMemberIterator itMember = stream.MemberBegin(), itEnd = stream.MemberEnd();
			 itMember != itEnd; ++itMember)
		{
			// Get related field info
			const TMetaFieldInfo *pMetaField =
					m_r->pMetaClass->FindFieldByJsonName(itMember->name.GetString(), itMember->name.GetStringLength());

			if (pMetaField == NULL) {
				// Not a field in struct
				if (detail::Util::IsBitSet(p.deserFlag, DeserFlag::kErrorWhenUnknown)) {
					p.errDoc.UnknownMember(detail::GetJsonStr(itMember->name));
					return ErrorCode::kDeserializeSomeUnknownMember;
				}
				if (!detail::Util::IsBitSet(p.deserFlag, DeserFlag::kIgnoreUnknown)) {
					m_r->unknown.AddMember(
							TValue().SetString(itMember->name.GetString(), itMember->name.GetStringLength(), *(m_r->pAllocator)),
							TValue().CopyFrom(itMember->value, *(m_r->pAllocator)),
							*(m_r->pAllocator)
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
		if (stream.IsNull() && detail::Util::IsBitSet(metaField->desc, FDesc::Nullable)) {
			m_r->fieldStatus[metaField->index] = FStatus::kNull;
		}
		else {
			void *pField = GetFieldByOffset(metaField->offset);
			FromJsonReq elemReq(stream, *(m_r->pAllocator), p.deserFlag, canMoveSrc, pField, metaField->desc);
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
		for (typename std::vector<TMetaFieldInfo>::const_iterator
					 itFieldInfo = m_r->pMetaClass->GetFieldsInfo().begin(), itEnd = m_r->pMetaClass->GetFieldsInfo().end();
			 itFieldInfo != itEnd; ++itFieldInfo)
		{
			void *pField = GetFieldByOffset(itFieldInfo->offset);
			detail::GetSerializerInterface<Encoding>(*itFieldInfo)->ShrinkAllocator(pField);
		}

		// Shrink self allocator
		// Some context in own allocator may be free after shrinking children's allocator, so shrink allocator always
		rapidjson::GenericDocument<Encoding> newDoc;
		newDoc.CopyFrom(m_r->unknown, newDoc.GetAllocator());
		m_r->ownDoc.Swap(newDoc);
		m_r->unknown = static_cast<rapidjson::GenericValue<Encoding>&>(m_r->ownDoc); // move
		m_r->pAllocator = &m_r->ownDoc.GetAllocator();
	}

	void ResetAllocator()
	{
		m_r->ownDoc.GetAllocator().Clear();
		m_r->pAllocator = &m_r->ownDoc.GetAllocator();
	}

	void InitOuterPtr()
	{
		m_r->pOuter = reinterpret_cast<const unsigned char *>(this - m_r->pMetaClass->GetAccessorOffset());
	}

	void MarkFieldStatus(const void* field, EFStatus fStatus)
	{
		const std::size_t offset = GetFieldOffset(field);
		const int index = m_r->pMetaClass->FindIndex(offset);
		IJST_ASSERT(index >= 0 && (unsigned int)index < m_r->pMetaClass->GetFieldsInfo().size());
		m_r->fieldStatus[index] = fStatus;
	}

	int CheckFieldState(detail::ErrorDocSetter<Encoding>& errDoc) const
	{
		// Check all required field status
		bool hasErr = false;

		for (typename std::vector<TMetaFieldInfo>::const_iterator
					 itFieldInfo = m_r->pMetaClass->GetFieldsInfo().begin(), itEnd = m_r->pMetaClass->GetFieldsInfo().end();
			 itFieldInfo != itEnd; ++itFieldInfo)
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

	std::size_t GetFieldOffset(const void *ptr) const
	{
		const unsigned char *filed_ptr = reinterpret_cast<const unsigned char *>(ptr);
		return filed_ptr - m_r->pOuter;
	}

	void *GetFieldByOffset(std::size_t offset) const
	{
		return (void *) (m_r->pOuter + offset);
	}


	// It need at least one new operation in Accessor, e.g, fieldStatus
	// So allocate all resource in heap to reduce the size of Accessor.
	struct Resource {
		TValue unknown;
		// Should use document instead of Allocator because document can swap allocator
		TDocument ownDoc;

		EFStatus* fieldStatus;
		const TMetaClassInfo* pMetaClass;
		detail::JsonAllocator* pAllocator;
		const unsigned char *pOuter;

		bool isValid;
		bool isParentVal;
	};
	Resource* m_r;

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
	IJSTI_STATIC_ASSERT((std::is_same<typename Handler::Ch, typename Encoding::Ch>::value),
	"Handler::Ch and Encoding::Ch must be same");
	typedef typename Handler::Ch Ch;
	explicit SAXGeneratorWrapper(const Accessor<Encoding>& accessor, SerFlag::Flag serFlag = SerFlag::kNoneFlag) :
			m_accessor(accessor), m_serFlag (serFlag), m_h(NULL) {}

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

} // namespace ijst

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
//! @param ...			fields define: [(fType, fName, ...)]*
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

#define IJSTI_IDL_FTYPE(fType, fName, ...)				fType
#define IJSTI_IDL_FNAME(fType, fName, ...)				fName
#define IJSTI_IDL_P3(fType, fName, p3, ...)				p3
#define IJSTI_IDL_P4(fType, fName, p3, p4, ...)			p4
#define IJSTI_IDL_P5(fType, fName, p3, p4, p5)			p5
#define IJSTI_IDL_FNAME_STR(fType, fName, ...)			#fName

#define IJSTI_OFFSETOF(base, member)	(size_t(&base->member) - size_t(base))

#define IJSTI_STRUCT_PUBLIC_DEFINE(encoding)												\
	typedef encoding _ijst_Encoding;														\
	typedef encoding::Ch _ijst_Ch;															\
	typedef ::ijst::Accessor<_ijst_Encoding> _ijst_AccessorType;							\
	_ijst_AccessorType _;

#define IJSTI_DEFINE_FIELD(fType, fName, ... )												\
	fType fName;

#define IJSTI_FIELD_GETTER(fType, fName, ... )												\
	::ijst::Optional<const fType > IJSTI_PP_CONCAT(IJST_GETTER_PREFIX, fName)() const 		\
	{ return static_cast<const _ijst_AccessorType&>(this->_).GetOptional(this->fName); }	\
	::ijst::Optional< fType > IJSTI_PP_CONCAT(IJST_GETTER_PREFIX, fName)()					\
	{ return this->_.GetOptional(this->fName); }

#define IJSTI_METAINFO_DEFINE_START(stName, N)												\
	typedef ::ijst::detail::MetaClassInfoTyped< stName > _ijst_MetaInfoT;					\
	typedef ::ijst::detail::Singleton<_ijst_MetaInfoT> _ijst_MetaInfoS;						\
	friend class ::ijst::detail::MetaClassInfoTyped< stName >;								\
	template<bool DummyTrue>																\
	static void _ijst_InitMetaInfo(_ijst_MetaInfoT* metaInfo)								\
	{																						\
		/* Do not call MetaInfoS::GetInstance() int this function */			 			\
		IJST_OFFSET_BUFFER_NEW(dummyBuffer, sizeof(stName));								\
		const stName* stPtr = reinterpret_cast<const stName*>(dummyBuffer);					\
		::ijst::detail::MetaClassInfoSetter<_ijst_Encoding>					 				\
					mSetter(metaInfo->metaClass);											\
		mSetter.InitBegin(#stName, N, IJSTI_OFFSETOF(stPtr, _));

// Add meta info of field
// e.g., IJSTI_METAINFO_ADD(MyStruct, (ijst::T_int, i, "i"))
#ifdef _MSC_VER
	#define IJSTI_METAINFO_ADD(stName, fDef)  													\
			IJSTI_EXPAND(IJSTI_PP_CONCAT(IJSTI_METAINFO_ADD_IMPL_, IJSTI_PP_NARGS fDef)(stName, fDef))
#else
	#define IJSTI_METAINFO_ADD(stName, fDef)  													\
			IJSTI_PP_CONCAT(IJSTI_METAINFO_ADD_IMPL_, IJSTI_PP_NARGS fDef)(stName, fDef)
#endif

// Impl of meta info adding with fDef format (type, field_name)
// The json name is same as field name
// The serialize_intf is always decided by field type in this case
#define IJSTI_METAINFO_ADD_IMPL_2(stName, fDef)  											\
		mSetter.PushMetaField_2(															\
			&(IJSTI_FSERIALIZER_INS(IJSTI_IDL_FTYPE fDef, _ijst_Encoding)),					\
			IJSTI_OFFSETOF(stPtr, IJSTI_IDL_FNAME fDef),									\
			IJSTI_IDL_FNAME_STR fDef,														\
			IJSTI_IDL_FNAME_STR fDef														\
		);

// Impl of meta info adding with fDef format (type, field_name, p3)
#define IJSTI_METAINFO_ADD_IMPL_3(stName, fDef)  											\
		mSetter.PushMetaField_3<IJSTI_IDL_FTYPE fDef>(										\
			IJSTI_OFFSETOF(stPtr, IJSTI_IDL_FNAME fDef),									\
			IJSTI_IDL_FNAME_STR fDef,														\
			IJSTI_IDL_FNAME_STR fDef,														\
			IJSTI_IDL_P3 fDef 																\
		);

// Impl of meta info adding with fDef format (type, field_name, p3, p4)
#define IJSTI_METAINFO_ADD_IMPL_4(stName, fDef)  											\
		mSetter.PushMetaField_4<IJSTI_IDL_FTYPE fDef>(										\
			IJSTI_OFFSETOF(stPtr, IJSTI_IDL_FNAME fDef),									\
			IJSTI_IDL_FNAME_STR fDef,														\
			IJSTI_IDL_FNAME_STR fDef,														\
			IJSTI_IDL_P3 fDef, 																\
			IJSTI_IDL_P4 fDef 																\
		);

// Impl of meta info adding with fDef format (type, field_name, p3, p4, p5)
// The serialize_intf is always declared by user in this case
// The json_name is always declared by user in this case
#define IJSTI_METAINFO_ADD_IMPL_5(stName, fDef)  											\
		mSetter.PushMetaField_5(															\
			IJSTI_OFFSETOF(stPtr, IJSTI_IDL_FNAME fDef),									\
			IJSTI_IDL_FNAME_STR fDef,														\
			IJSTI_IDL_P3 fDef, 																\
			IJSTI_IDL_P4 fDef, 																\
			IJSTI_IDL_P5 fDef 																\
		);

#define IJSTI_METAINFO_DEFINE_END()															\
		mSetter.InitEnd();																	\
		IJST_OFFSET_BUFFER_DELETE(dummyBuffer);												\
	}

#define IJSTI_DEFINE_CLASS_END(stName)														\
	};																						\
	IJSTI_STRUCT_EXTERN_TEMPLATE(stName)													\
	IJSTI_STRUCT_EXPLICIT_TEMPLATE(stName)


//! list of templates could been declared extern
#define IJSTI_EXTERNAL_TEMPLATE_XLIST														\
	IJSTX(struct rapidjson::UTF8<>)															\
	IJSTX(class ijst::Accessor<>)															\
	IJSTX(class rapidjson::GenericDocument<rapidjson::UTF8<> >)								\
	IJSTX(class rapidjson::GenericValue<rapidjson::UTF8<> >)								\
	IJSTX(class rapidjson::MemoryPoolAllocator<>)											\
	IJSTX(class rapidjson::GenericStringBuffer<rapidjson::UTF8<> >)							\
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
#endif //IJST_ACCESSOR_HPP_INCLUDE_
