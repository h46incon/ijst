//
// Created by h46incon on 2017/12/26.
//

#ifndef IJST_DETAIL_HPP_INCLUDE_
#define IJST_DETAIL_HPP_INCLUDE_

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

#include <vector>
#include <string>

namespace ijst{
namespace detail{
#if __cplusplus >= 201103L
	#define IJSTI_NULL 							nullptr
	#define IJSTI_MOVE(val) 					std::move((val))
	#define IJSTI_OVERRIDE						override
	#define IJSTI_NOEXCEPT						noexcept
	#define IJSTI_DELETED						= delete
	#define IJSTI_STATIC_ASSERT(cond, msg) 		static_assert((cond), msg)
#else
	#define IJSTI_NULL 							NULL
	#define IJSTI_MOVE(val) 					(val)
	#define IJSTI_OVERRIDE
	#define IJSTI_NOEXCEPT
	#define IJSTI_DELETED
	#define IJSTI_STATIC_ASSERT(cond, msg)
#endif

// Expands to the concatenation of its two arguments.
#define IJSTI_PP_CONCAT(x, y) 		IJSTI_PP_CONCAT_I(x, y)
#define IJSTI_PP_CONCAT_I(x, y) 	x ## y

// Expand __VA_ARGS__ for msvc preprocessor
#define IJSTI_EXPAND(...)			__VA_ARGS__

// forward declaration
template<typename Encoding> class SerializerInterface;
template<typename CharType> class MetaClassInfoSetter;
template<typename T> class MetaClassInfoTyped;

typedef rapidjson::MemoryPoolAllocator<> JsonAllocator;
/**
 * Singleton interface
 * @tparam T type
 */
template<typename T>
class Singleton {
public:
	inline static T& GetInstance()
	{
		static T instance;
		return instance;
	}

	inline static void InitInstanceInGlobal()
	{
		// When accessing gInstance in code, the GetInstance() function will be called in
		// global scope (before main in many compilers)
		volatile T* dummy = gInstance;
		(void)dummy;
	}

private:
	static T* gInstance;
};
// static member of template class could declare in header
template<typename T> T* Singleton<T>::gInstance = &Singleton<T>::GetInstance();

template <typename T>
struct HasType {
	typedef void Void;
};

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
rapidjson::GenericStringRef<typename Encoding::Ch> EncodeString(const char* pSrc, JsonAllocator& allocator)
{
	rapidjson::GenericStringBuffer<Encoding, JsonAllocator> sb(&allocator);
	const char* ptr = pSrc;
	while ((*ptr) != '\0') {
		Encoding::Encode(sb, static_cast<unsigned>(*ptr));
		++ptr;
	}

	return rapidjson::GenericStringRef<typename Encoding::Ch>
	        (sb.GetString(), static_cast<rapidjson::SizeType>(sb.GetSize() / sizeof(typename Encoding::Ch)));
}

template<typename Encoding>
struct ErrorDocSetter {
	typedef rapidjson::GenericDocument<Encoding> TDocument;
	typedef rapidjson::GenericValue<Encoding> TValue;
	typedef typename Encoding::Ch Ch;
	//! Constructor
	//! @param _pErrDoc		Error message output. set to nullptr if do not need to enable error message
	explicit ErrorDocSetter(TDocument* _pErrDoc):
			pAllocator(_pErrDoc == IJSTI_NULL ? IJSTI_NULL : &_pErrDoc->GetAllocator()),
			pErrMsg(_pErrDoc) {}

	void ParseFailed(rapidjson::ParseErrorCode errCode)
	{
		if (pAllocator == IJSTI_NULL) { return; }
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
		if (pAllocator == IJSTI_NULL) { return; }

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
		if (pAllocator == IJSTI_NULL) { return; }

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
		if (pAllocator == IJSTI_NULL) { return; }

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
		if (pAllocator == IJSTI_NULL) { return; }
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
		if (pAllocator == IJSTI_NULL) { return; }

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
		if (pAllocator == IJSTI_NULL) { return; }

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
		if (pAllocator == IJSTI_NULL) { return; }

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
		if (pAllocator == IJSTI_NULL) { return; }

		pErrMsg->SetObject();
		pErrMsg->AddMember(
				EncodeString<Encoding>("type", *pAllocator),
				EncodeString<Encoding>("ValueIsDefault", *pAllocator),
				*pAllocator);
	}

	void ElementAddMemberName(const std::basic_string<Ch>& memberName)
	{
		if (pAllocator == IJSTI_NULL) { return; }
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

struct Util {
	/**
	 * Custom swap() to avoid dependency on C++ <algorithm> header
	 * @tparam T 	Type of the arguments to swap, should be instantiated with primitive C++ types only.
	 * @note This has the same semantics as std::swap().
	 */
	template <typename T>
	static inline void Swap(T& a, T& b) RAPIDJSON_NOEXCEPT {
		T tmp = IJSTI_MOVE(a);
		a = IJSTI_MOVE(b);
		b = IJSTI_MOVE(tmp);
	}

	struct VectorBinarySearchResult {
		bool isFind;
		size_t index;
	};

	struct CompResult {
		enum E_ {
			GT,
			EQ,
			LE
		};
	};
	typedef CompResult::E_ ECompResult;

	template<typename VType>
	static VectorBinarySearchResult VectorBinarySearch(const std::vector<VType>& vec, const VType& target, ECompResult (*comp)(const VType&, const VType&))
	{
		size_t beg = 0;
		size_t end = vec.size();

		while (beg < end) {
			size_t mid = beg + (end - beg) / 2;
			ECompResult c = comp(vec[mid], target);
			switch (c)
			{
				case CompResult::GT:
					end = mid;
					break;
				case CompResult::LE:
					beg = mid + 1;
					break;
				case CompResult::EQ:
					return VectorBinarySearchResult{true, mid};
			}
		}
		return VectorBinarySearchResult{false, end};
	};

	template<typename Encoding>
	static void ShrinkAllocatorWithOwnDoc(rapidjson::GenericDocument<Encoding>& ownDoc, rapidjson::GenericValue<Encoding>& val, JsonAllocator*& pAllocatorOut)
	{
		if (pAllocatorOut == &ownDoc.GetAllocator()) {
			if (pAllocatorOut->Capacity() == pAllocatorOut->Size()) {
				// The capacity will not shrink
				return;
			}
		}
		rapidjson::GenericDocument<Encoding> newDoc;
		newDoc.CopyFrom(val, newDoc.GetAllocator());
		ownDoc.Swap(newDoc);
		val = static_cast<rapidjson::GenericValue<Encoding>&>(ownDoc);
		pAllocatorOut = &ownDoc.GetAllocator();
	}

	static bool IsBitSet(unsigned int val, unsigned int bit)
	{
		return (val & bit) != 0;
	}

};

}	// namespace detail
}	// namespace ijst

#endif //IJST_DETAIL_HPP_INCLUDE_
