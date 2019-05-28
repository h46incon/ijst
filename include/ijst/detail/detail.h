//
// Created by h46incon on 2017/12/26.
//

#ifndef IJST_DETAIL_DETAIL_HPP_INCLUDE_
#define IJST_DETAIL_DETAIL_HPP_INCLUDE_


#include "../meta_info.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

#include <string>

//! return Err::kWriteFailed if action return false
#define IJSTI_RET_WHEN_WRITE_FAILD(action) 						\
	do { if(!(action)) return ErrorCode::kWriteFailed; } while (false)
//! return if action return non-0
#define IJSTI_RET_WHEN_NOT_ZERO(action) 						\
	do { int ret = (action); if(ret != 0) return (ret); } while (false)
//! helper in Accessor::Deserialize()
#define IJSTI_RET_WHEN_PARSE_ERROR(doc, encoding)							\
	do {																	\
		if (doc.HasParseError()) {											\
			detail::ErrorDocSetter<encoding> errDocSetter(pErrDocOut);		\
			errDocSetter.ParseFailed(doc.GetParseError());					\
			return ErrorCode::kDeserializeParseFailed;						\
		}																	\
	} while (false)
//! return error and set error doc when type mismatch
#define IJSTI_RET_WHEN_TYPE_MISMATCH(checkCode, expType)			\
	if (!(checkCode)) {												\
		resp.errDoc.ElementTypeMismatch(expType, req.stream);		\
		return ErrorCode::kDeserializeValueTypeError;				\
	}

namespace ijst{

// forward declaration
template<typename Ch> class HandlerBase;

namespace detail{

typedef rapidjson::MemoryPoolAllocator<> JsonAllocator;

template<typename Encoding>
class HeadOStream {
public:
	typedef typename Encoding::Ch Ch;

	explicit HeadOStream(size_t _capacity)
			: m_capacity (_capacity), m_headOnly(true) { str.reserve(m_capacity + kEllipseSize); }

	//! Write a character.
	void Put(Ch c)
	{
		if (str.size() < m_capacity) {
			str.push_back(c);
		}
		else {
			if (m_headOnly) {
				rapidjson::GenericStringBuffer<Encoding> sb;
				for (size_t i = 0; i < kEllipseSize; ++i) {
					Encoding::Encode(sb, static_cast<unsigned>('.'));
				}
				str.append(sb.GetString());
				m_headOnly = false;
			}
		}
	}

	//! Flush the buffer.
	void Flush() {;}

	bool HeadOnly() const { return m_headOnly; }

	std::basic_string<Ch> str;
private:
	static const size_t kEllipseSize = 3;
	const size_t m_capacity;
	bool m_headOnly;
};

/**
 * Head Writer that only write heading string. Implement rapidjson::Handler concept
 * It will return false when OutputStream.IsDone() return true
 *
 * @tparam OutputStream		Should implement rapidjson::OutputStream concept, and bool HeadOnly() interface
 * @tparam BaseWriter		Should implement rapidjson::Handler concept, and BaseWriter(OutputStream&) constructor
 */
template<typename OutputStream, typename BaseWriter>
class HeadWriter {
public:
	explicit HeadWriter(OutputStream& stream) : m_stream(stream), m_baseWriter(stream) {}

	typedef typename BaseWriter::Ch Ch;
	bool Null() { return m_baseWriter.Null() && m_stream.HeadOnly(); }
	bool Bool(bool b) { return m_baseWriter.Bool(b) && m_stream.HeadOnly(); }
	bool Int(int i) { return m_baseWriter.Int(i) && m_stream.HeadOnly(); }
	bool Uint(unsigned i) { return m_baseWriter.Uint(i) && m_stream.HeadOnly(); }
	bool Int64(int64_t i) { return m_baseWriter.Int64(i) && m_stream.HeadOnly(); }
	bool Uint64(uint64_t i) { return m_baseWriter.Uint64(i) && m_stream.HeadOnly(); }
	bool Double(double d) { return m_baseWriter.Double(d) && m_stream.HeadOnly(); }
	/// enabled via kParseNumbersAsStringsFlag, string is not null-terminated (use length)
	bool RawNumber(const Ch* str, rapidjson::SizeType length, bool copy = false)
	{ return m_baseWriter.RawNumber(str, length, copy) && m_stream.HeadOnly(); }
	bool String(const Ch* str, rapidjson::SizeType length, bool copy = false)
	{ return m_baseWriter.String(str, length, copy) && m_stream.HeadOnly(); }
	bool StartObject() { return m_baseWriter.StartObject() && m_stream.HeadOnly(); }
	bool Key(const Ch* str, rapidjson::SizeType length, bool copy = false)
	{ return m_baseWriter.Key(str, length, copy) && m_stream.HeadOnly(); }
	bool EndObject(rapidjson::SizeType memberCount = 0)
	{ return m_baseWriter.EndObject(memberCount) && m_stream.HeadOnly(); }
	bool StartArray() { return m_baseWriter.StartArray() && m_stream.HeadOnly(); }
	bool EndArray(rapidjson::SizeType elementCount = 0)
	{ return m_baseWriter.EndArray(elementCount) && m_stream.HeadOnly(); }

private:
	OutputStream& m_stream;
	BaseWriter m_baseWriter;
};

template<typename Encoding>
void EncodeStringToBuffer(const char* pSrc, rapidjson::GenericStringBuffer<Encoding, JsonAllocator>& bufferOut)
{
	const char* ptr = pSrc;
	while ((*ptr) != '\0') {
		Encoding::Encode(bufferOut, static_cast<unsigned>(*ptr));
		++ptr;
	}
}

template<typename Encoding>
rapidjson::GenericStringRef<typename Encoding::Ch> EncodeString(const char* pSrc, JsonAllocator& allocator)
{
	rapidjson::GenericStringBuffer<Encoding, JsonAllocator> sb(&allocator);
	EncodeStringToBuffer(pSrc, sb);
	return rapidjson::GenericStringRef<typename Encoding::Ch>
	        (sb.GetString(), static_cast<rapidjson::SizeType>(sb.GetSize() / sizeof(typename Encoding::Ch)));
}

template<typename Encoding>
inline std::basic_string<typename Encoding::Ch> GetJsonStr(const rapidjson::GenericValue<Encoding>& jVal)
{
	return std::basic_string<typename Encoding::Ch>(jVal.GetString(), jVal.GetStringLength());
}

template<typename Encoding>
struct ErrorDocSetter {
	typedef rapidjson::GenericDocument<Encoding> TDocument;
	typedef rapidjson::GenericValue<Encoding> TValue;
	typedef typename Encoding::Ch Ch;
	//! Constructor
	//! @param _pErrDoc		Error message output. set to nullptr if do not need to enable error message
	explicit ErrorDocSetter(TDocument* _pErrDoc):
			pAllocator(_pErrDoc == NULL ? NULL : &_pErrDoc->GetAllocator()),
			pErrMsg(_pErrDoc) {}

	void ParseFailed(rapidjson::ParseErrorCode errCode)
	{
		if (pAllocator == NULL) { return; }
		pErrMsg->SetObject();

		pErrMsg->AddMember(
				EncodeString<Encoding>("type", *pAllocator),
				EncodeString<Encoding>("ParseError", *pAllocator),
				*pAllocator);
		pErrMsg->AddMember(
				EncodeString<Encoding>("errCode", *pAllocator),
				TValue().SetInt(static_cast<int>(errCode)),
				*pAllocator);
		pErrMsg->AddMember(
				EncodeString<Encoding>("err", *pAllocator),
				EncodeString<Encoding>(rapidjson::GetParseError_En(errCode), *pAllocator),
				*pAllocator);
	}

	//! Set error message about error of member in object
	void ErrorInObject(const std::string& memberName, const std::basic_string<Ch>& jsonKey)
	{
		if (pAllocator == NULL) { return; }

		// backup errMsg
		TValue errDetail;
		errDetail = *pErrMsg;	// move

		pErrMsg->SetObject();
		pErrMsg->AddMember(
				EncodeString<Encoding>("type", *pAllocator),
				EncodeString<Encoding>("ErrInObject", *pAllocator),
				*pAllocator);
		pErrMsg->AddMember(
				EncodeString<Encoding>("member", *pAllocator),
				EncodeString<Encoding>(memberName.c_str(), *pAllocator),
				*pAllocator);
		pErrMsg->AddMember(
				EncodeString<Encoding>("jsonKey", *pAllocator),
				TValue().SetString(jsonKey.data(), static_cast<rapidjson::SizeType>(jsonKey.size()), *pAllocator),
				*pAllocator);
		if (!errDetail.IsNull()) {
			pErrMsg->AddMember(
					EncodeString<Encoding>("err", *pAllocator),
					errDetail,
					*pAllocator);
		}
	}

	//! Set error message about error of member in object
	void ErrorInMap(const std::basic_string<Ch>& jsonKey)
	{
		if (pAllocator == NULL) { return; }

		// backup errMsg
		TValue errDetail;
		errDetail = *pErrMsg;	// move

		pErrMsg->SetObject();
		pErrMsg->AddMember(
				EncodeString<Encoding>("type", *pAllocator),
				EncodeString<Encoding>("ErrInMap", *pAllocator),
				*pAllocator);
		pErrMsg->AddMember(
				EncodeString<Encoding>("member", *pAllocator),
				TValue().SetString(jsonKey.data(), static_cast<rapidjson::SizeType>(jsonKey.size()), *pAllocator),
				*pAllocator);
		if (!errDetail.IsNull()) {
			pErrMsg->AddMember(
					EncodeString<Encoding>("err", *pAllocator),
					errDetail,
					*pAllocator);
		}
	}

	//! Set error message about error of member in array
	void ErrorInArray(unsigned index)
	{
		if (pAllocator == NULL) { return; }

		// backup errMsg
		TValue errDetail;
		errDetail = *pErrMsg;	// move

		pErrMsg->SetObject();
		pErrMsg->AddMember(
				EncodeString<Encoding>("type", *pAllocator),
				EncodeString<Encoding>("ErrInArray", *pAllocator),
				*pAllocator);
		pErrMsg->AddMember(
				EncodeString<Encoding>("index", *pAllocator),
				TValue().SetUint(index),
				*pAllocator);
		if (!errDetail.IsNull()) {
			pErrMsg->AddMember(
					EncodeString<Encoding>("err", *pAllocator),
					errDetail,
					*pAllocator);
		}
	}

	void MissingMember()
	{
		if (pAllocator == NULL) { return; }
		assert(pErrMsg->IsArray());

		// backup errMsg
		TValue errDetail;
		errDetail = *pErrMsg;	// move

		pErrMsg->SetObject();
		pErrMsg->AddMember(
				EncodeString<Encoding>("type", *pAllocator),
				EncodeString<Encoding>("MissingMember", *pAllocator),
				*pAllocator);
		pErrMsg->AddMember(
				EncodeString<Encoding>("members", *pAllocator),
				errDetail,
				*pAllocator);
	}

	void UnknownMember(const std::basic_string<Ch>& jsonKey)
	{
		if (pAllocator == NULL) { return; }

		// backup errMsg
		TValue errDetail;
		errDetail = *pErrMsg;	// move

		pErrMsg->SetObject();
		pErrMsg->AddMember(
				EncodeString<Encoding>("type", *pAllocator),
				EncodeString<Encoding>("UnknownMember", *pAllocator),
				*pAllocator);
		pErrMsg->AddMember(
				EncodeString<Encoding>("jsonKey", *pAllocator),
				TValue().SetString(jsonKey.data(), static_cast<rapidjson::SizeType>(jsonKey.size()), *pAllocator),
				*pAllocator);
	}

	void ElementMapKeyDuplicated(const std::basic_string<Ch>& keyName)
	{
		if (pAllocator == NULL) { return; }

		pErrMsg->SetObject();
		pErrMsg->AddMember(
				EncodeString<Encoding>("type", *pAllocator),
				EncodeString<Encoding>("MapKeyDuplicated", *pAllocator),
				*pAllocator);
		pErrMsg->AddMember(
				EncodeString<Encoding>("key", *pAllocator),
				TValue().SetString(keyName.data(), static_cast<rapidjson::SizeType>(keyName.size()), *pAllocator),
				*pAllocator);
	}

	void ElementTypeMismatch(const char *expectedType, const TValue &errVal)
	{
		if (pAllocator == NULL) { return; }

		typedef HeadOStream<Encoding> THeadOStream;
		THeadOStream ostream(16);
		HeadWriter<THeadOStream, rapidjson::Writer<THeadOStream, Encoding, Encoding> > writer(ostream);
		errVal.Accept(writer);

		pErrMsg->SetObject();
		pErrMsg->AddMember(
				EncodeString<Encoding>("type", *pAllocator),
				EncodeString<Encoding>("TypeMismatch", *pAllocator),
				*pAllocator);
		pErrMsg->AddMember(
				EncodeString<Encoding>("expectedType", *pAllocator),
				EncodeString<Encoding>(expectedType, *pAllocator),
				*pAllocator);
		pErrMsg->AddMember(
				EncodeString<Encoding>("json", *pAllocator),
				TValue().SetString(ostream.str.data(), static_cast<rapidjson::SizeType>(ostream.str.size()), *pAllocator),
				*pAllocator);
	}

	void ElementValueIsDefault()
	{
		if (pAllocator == NULL) { return; }

		pErrMsg->SetObject();
		pErrMsg->AddMember(
				EncodeString<Encoding>("type", *pAllocator),
				EncodeString<Encoding>("ValueIsDefault", *pAllocator),
				*pAllocator);
	}

	void ElementAddMemberName(const std::basic_string<Ch>& memberName)
	{
		if (pAllocator == NULL) { return; }
		if (pErrMsg->IsNull()) {
			pErrMsg->SetArray();
		}
		assert(pErrMsg->IsArray());

		pErrMsg->PushBack(
				TValue().SetString(memberName.data(), static_cast<rapidjson::SizeType>(memberName.size()), *pAllocator),
				*pAllocator
		);
	}

	// Pointer to allocator that used to setting error message
	// Use nullptr if do not need error message
	JsonAllocator* const pAllocator;
	TValue* const pErrMsg;
};


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

		// override meta info when deserialize
		const OverrideMetaInfos* pOvrMetaInfo;

		// true if move context in stream to avoid copy when possible
		bool canMoveSrc;

		DeserFlag::Flag deserFlag;

		// handler FDesc in the parent, so do not need it now
//		FDesc::Mode fDesc;

		FromJsonReq(rapidjson::GenericValue<Encoding>& _stream, JsonAllocator& _allocator,
					DeserFlag::Flag _deserFlag, bool _canMoveSrc,
					void* _pField, const OverrideMetaInfos* _pOvrMetaInfo)
				: pFieldBuffer(_pField)
				  , stream(_stream)
				  , allocator(_allocator)
				  , pOvrMetaInfo(_pOvrMetaInfo)
				  , canMoveSrc(_canMoveSrc)
				  , deserFlag(_deserFlag)
		{ }
	};

	struct FromJsonResp {
		ErrorDocSetter<Encoding>& errDoc;
		bool isValueDefault;

		explicit FromJsonResp(ErrorDocSetter<Encoding>& _errDoc) :
				errDoc(_errDoc), isValueDefault(false)
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
	IJSTI_STATIC_ASSERT(!(std::is_same<T, T>::value),	// always failed
						"This base template should not be instantiated. (Maybe use wrong param when define ijst struct)");
	typedef void VarType;
	IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);

	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE = 0;
	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE = 0;
	virtual void ShrinkAllocator(void * pField) IJSTI_OVERRIDE { (void)pField; }
};

#define IJSTI_FSERIALIZER_INS(T, Encoding) 		\
		::ijst::detail::Singleton< ::ijst::detail::FSerializer< T, Encoding > >()

/**
 * Get and cast serializerInterface in metaFieldInfo to specify type
 *
 * @tparam Encoding 		encoding of output
 *
 * @param metaFieldInfo 	metaFieldInfo
 * @return 					typed SerializerInterface
 */
template<typename Encoding>
inline SerializerInterface<Encoding>* GetSerializerInterface(const MetaFieldInfo<typename Encoding::Ch>& metaFieldInfo)
{
	return reinterpret_cast<SerializerInterface<Encoding>*>(metaFieldInfo.serializerInterface);
}

/**	========================================================================================
 *				Private
 */
/**
 * MetaClassInfo of ijst struct
 *
 * @tparam T 	class. Concept require
 * 					typedef T::_ijst_Ch
 * 					void T::_ijst_InitMetaInfo<bool>(MetaClassInfo&, const T*)
 */
template<typename T>
class IjstStructMeta {
public:
	static const MetaClassInfo<typename T::_ijst_Ch>& Ins()
	{
		static IjstStructMeta ins;
		return ins.metaClass;
	}

private:
	MetaClassInfo<typename T::_ijst_Ch> metaClass;

	IjstStructMeta()
	{
		IJST_OFFSET_BUFFER_NEW(dummyBuffer, sizeof(T));
		const T* stPtr = reinterpret_cast<T*>(dummyBuffer);

		T::template _ijst_InitMetaInfo<true>(metaClass, stPtr);

		IJST_OFFSET_BUFFER_DELETE(dummyBuffer);
	}
};

/**
 * OverrideMetaInfos of ijst struct
 *
 * @tparam T 	class. Concept require
 * 					const OverrideMetaInfos* T::_ijst_NewOvrMetaInfo(const T*)
 */
template<typename T>
class IjstStructOvrMeta {
public:
	static const OverrideMetaInfos* Ins()
	{
		static IjstStructOvrMeta ins;
		return ins.pOverMetas;
	}

private:
	const OverrideMetaInfos* pOverMetas;

	IjstStructOvrMeta() :
			pOverMetas(NULL)
	{
		IJST_OFFSET_BUFFER_NEW(dummyBuffer, sizeof(T));
		const T* stPtr = reinterpret_cast<T*>(dummyBuffer);

		pOverMetas = T::_ijst_NewOvrMetaInfo(stPtr);

		IJST_OFFSET_BUFFER_DELETE(dummyBuffer);
	}

	~IjstStructOvrMeta()
	{
		delete pOverMetas;
		pOverMetas = NULL;
	}
};

template<typename Encoding>
class MetaClassInfoSetter {
public:
	typedef typename Encoding::Ch Ch;
	explicit MetaClassInfoSetter(MetaClassInfo<Ch>& _d) : d(_d), m_fieldSize(0), m_maxSize(0) { }

	void InitBegin(const std::string& _tag, std::size_t _maxFieldCount, std::size_t _accessorOffset)
	{
		d.m_structName = _tag;
		d.m_accessorOffset = _accessorOffset;
		d.m_fieldsInfo = new MetaFieldInfo<Ch>[_maxFieldCount];
		d.m_isResourceOwner = true;

		m_maxSize = _maxFieldCount;
	}

	void ShadowFrom(const MetaClassInfo<Ch>& src, const std::string& _tag)
	{
		d.ShadowFrom(src, _tag);
	}

	/// The complete IDL of declaring a field is (type, name, json_name, desc, serialize_intf)
	/// type and name are required. But json_name, desc, serialize_intf are all optional.
	/// User can omit any one of them, but the order must not be broken
	/// i.e. (type, name, desc, json_name) is illegal because desc and json_name break the order.


	//! Call from IJSTI_METAINFO_ADD_IMPL_2 with IDL (type, name)
	//! The serialize interface is always decided by type, so it is generated by caller to reduce template
	void PushMetaField_2(SerializerInterface<Encoding>* pSerializeInterface, std::size_t offset, const char* fieldName, const char* jsonName)
	{
		DoPushMetaField(pSerializeInterface, offset, fieldName, jsonName, FDesc::NoneFlag);
	}

	//! Call from IJSTI_METAINFO_ADD_IMPL_3 with IDL (type, name, json_name)
	template<typename FieldType>
	void PushMetaField_3(std::size_t offset, const char* fieldName, const char* jsonNameIgnored, const std::basic_string<Ch>& trueJsonName)
	{
		(void)jsonNameIgnored;
		SerializerInterface<Encoding>* pSerializeInterface = &IJSTI_FSERIALIZER_INS(FieldType, Encoding);
		DoPushMetaField(pSerializeInterface, offset, fieldName, trueJsonName, FDesc::NoneFlag);
	}

	//! Call from IJSTI_METAINFO_ADD_IMPL_3 with IDL (type, name, desc)
	template<typename FieldType>
	void PushMetaField_3(std::size_t offset, const char* fieldName, const char* jsonName, FDesc::Mode desc)
	{
		SerializerInterface<Encoding>* pSerializeInterface = &IJSTI_FSERIALIZER_INS(FieldType, Encoding);
		DoPushMetaField(pSerializeInterface, offset, fieldName, jsonName, desc);
	}

	//! Call from IJSTI_METAINFO_ADD_IMPL_3 with IDL (type, name, serialize_intf)
	template<typename FieldTypeIgnored>
	void PushMetaField_3(std::size_t offset, const char* fieldName, const char* jsonName, SerializerInterface<Encoding>* pSerializeInterface)
	{
		DoPushMetaField(pSerializeInterface, offset, fieldName, jsonName, FDesc::NoneFlag);
	}

	//! Call from IJSTI_METAINFO_ADD_IMPL_4 with IDL (type, name, json_name, desc)
	//! Backward compatibility with FDesc declaration is "0"
	template<typename FieldType>
	void PushMetaField_4(std::size_t offset, const char* fieldName, const char* jsonNameIgnored, const std::basic_string<Ch>& trueJsonName, int desc)
	{
		(void)jsonNameIgnored;
		SerializerInterface<Encoding>* pSerializeInterface = &IJSTI_FSERIALIZER_INS(FieldType, Encoding);
		DoPushMetaField(pSerializeInterface, offset, fieldName, trueJsonName, static_cast<FDesc::Mode>(desc));
	}

	//! Call from IJSTI_METAINFO_ADD_IMPL_4 with IDL (type, name, json_name, desc)
	template<typename FieldType>
	void PushMetaField_4(std::size_t offset, const char* fieldName, const char* jsonNameIgnored, const std::basic_string<Ch>& trueJsonName, FDesc::Mode desc)
	{
		(void)jsonNameIgnored;
		SerializerInterface<Encoding>* pSerializeInterface = &IJSTI_FSERIALIZER_INS(FieldType, Encoding);
		DoPushMetaField(pSerializeInterface, offset, fieldName, trueJsonName, desc);
	}

	//! Call from IJSTI_METAINFO_ADD_IMPL_4 with IDL (type, name, json_name, serialize_intf)
	template<typename FieldTypeIgnored>
	void PushMetaField_4(std::size_t offset, const char* fieldName, const char* jsonNameIgnored, const std::basic_string<Ch>& trueJsonName, SerializerInterface<Encoding>* pSerializeInterface)
	{
		(void)jsonNameIgnored;
		DoPushMetaField(pSerializeInterface, offset, fieldName, trueJsonName, FDesc::NoneFlag);
	}

	//! Call from IJSTI_METAINFO_ADD_IMPL_4 with IDL (type, name, desc, serialize_intf)
	template<typename FieldTypeIgnored>
	void PushMetaField_4(std::size_t offset, const char* fieldName, const char* jsonName, FDesc::Mode desc, SerializerInterface<Encoding>* pSerializeInterface)
	{
		DoPushMetaField(pSerializeInterface, offset, fieldName, jsonName, desc);
	}

	//! Call from IJSTI_METAINFO_ADD_IMPL_5 with IDL (type, name, json_name, desc, serialize_intf)
	//! serialize_intf is always declared by the user, so delete the template param FieldType to reduce template
	void PushMetaField_5(std::size_t offset, const char* fieldName,  const std::basic_string<Ch>& jsonName, FDesc::Mode desc, SerializerInterface<Encoding>* pSerializeInterface)
	{
		DoPushMetaField(pSerializeInterface, offset, fieldName, jsonName, desc);
	}

	void InitEnd()
	{
		// assert MetaClassInfo's map has not inited before
		assert(!d.m_mapInited);

		//--- sort field info by offset
		// m_fieldsInfo is already sorted in most case, use insertion sort
		for (size_t i1 = 1; i1 < m_fieldSize; i1++) {
			for (size_t j = i1; j > 0 && d.m_fieldsInfo[j - 1].offset > d.m_fieldsInfo[j].offset; j--) {
				Util::Swap(d.m_fieldsInfo[j], d.m_fieldsInfo[j - 1]);
			}
		}

		// new resource
		d.m_fieldSize = m_fieldSize;
		d.m_nameHashVal = new uint32_t[m_fieldSize];
		d.m_hashedFieldPtr = new const MetaFieldInfo<Ch>*[m_fieldSize];
		d.m_offsets = new size_t[m_fieldSize];

		for (size_t i = 0; i < m_fieldSize; ++i)
		{
			MetaFieldInfo<Ch>& ptrMetaField = d.m_fieldsInfo[i];
			ptrMetaField.index = static_cast<int>(i);

			d.m_offsets[i] = ptrMetaField.offset;
			InsertMetaFieldToHash(ptrMetaField, i);
			// Assert field offset is sorted and not exist before
			assert(i == 0 || d.m_offsets[i]  > d.m_offsets[i-1]);
		}

		d.m_mapInited = true;
	}

private:
	void DoPushMetaField(SerializerInterface<Encoding>* pSerializeInterface, std::size_t offset, const char* fieldName, const char* jsonName, FDesc::Mode desc)
	{
		if (pSerializeInterface == NULL) {
			return;
		}

		// convert jsonName to correct encoding
		JsonAllocator allocator;
		rapidjson::GenericStringBuffer<Encoding, JsonAllocator> sb(&allocator);
		EncodeStringToBuffer(jsonName, sb);
		std::basic_string<Ch> jsonNameEncoded = std::basic_string<Ch>(sb.GetString(), static_cast<rapidjson::SizeType>(sb.GetSize() / sizeof(Ch)));

		DoPushMetaField(pSerializeInterface, offset, fieldName, jsonNameEncoded, desc);
	}

	void DoPushMetaField(SerializerInterface<Encoding>* pSerializeInterface, std::size_t offset, const char* fieldName, const std::basic_string<Ch>& jsonName, FDesc::Mode desc)
	{
		if (pSerializeInterface == NULL) {
			return;
		}

		assert(m_fieldSize < m_maxSize);
		MetaFieldInfo<Ch>& metaField = d.m_fieldsInfo[m_fieldSize];
		++m_fieldSize;

		metaField.jsonName = jsonName;
		metaField.fieldName = std::string(fieldName);
		metaField.offset = offset;
		metaField.desc = desc;
		metaField.serializerInterface = pSerializeInterface;
	}

	void InsertMetaFieldToHash(const MetaFieldInfo<Ch> &ptrMetaField, size_t insertedSize)
	{
		const uint32_t hash = MetaClassInfo<Ch>::StringHash(ptrMetaField.jsonName.data(), ptrMetaField.jsonName.length());

		const detail::Util::VectorBinarySearchResult searchRet =
				detail::Util::VectorBinarySearch(d.m_nameHashVal, insertedSize, hash);

		for (size_t i = insertedSize; i > searchRet.index; --i) {
			d.m_nameHashVal[i] = d.m_nameHashVal[i - 1];
			d.m_hashedFieldPtr[i] = IJSTI_MOVE(d.m_hashedFieldPtr[i - 1]);
		}
		d.m_nameHashVal[searchRet.index] = hash;
		d.m_hashedFieldPtr[searchRet.index] = &ptrMetaField;
	}

	MetaClassInfo<Ch>& d;
	size_t m_fieldSize;
	size_t m_maxSize;
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
	IJSTI_STATIC_ASSERT((std::is_same<Encoding, typename T::_ijst_Encoding>::value),
						"Inner ijst struct's encoding must be same as outer class's encoding");

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
}	// namespace ijst

#endif //IJST_DETAIL_DETAIL_HPP_INCLUDE_
