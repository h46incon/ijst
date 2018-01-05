//
// Created by h46incon on 2017/12/26.
//

#ifndef _IJST_DETAIL_HPP_INCLUDE_
#define _IJST_DETAIL_HPP_INCLUDE_

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <string>

namespace ijst{
namespace detail{

#if __cplusplus >= 201103L
	#define IJSTI_MOVE(val) 	std::move((val))
	#define IJSTI_OVERRIDE		override
	#define IJSTI_NOEXCEPT		noexcept
	#define IJSTI_NULL 			nullptr
#else
	#define IJSTI_MOVE(val) 	(val)
	#define IJSTI_OVERRIDE
	#define IJSTI_NOEXCEPT
	#define IJSTI_NULL 			NULL
#endif

// Expands to the concatenation of its two arguments.
#define IJSTI_PP_CONCAT(x, y) 		IJSTI_PP_CONCAT_I(x, y)
#define IJSTI_PP_CONCAT_I(x, y) 	x ## y

#define IJSTI_NEW_ELEM_ERR_DOC(pErrDoc, pElemErrDoc)									\
	rapidjson::Document* pElemErrDoc = detail::ErrDoc::NewErrDoc(pErrDoc);				\
	detail::MemoryGuarder<rapidjson::Document> IJSTI_PP_CONCAT(ptrGuarder_, __LINE__) (pElemErrDoc)

/**
 * Singleton interface
 * @tparam _T type
 */
template<typename _T>
class Singleton {
public:
	inline static _T *GetInstance()
	{
		static _T instance;
		return &instance;
	}

	inline static void InitInstanceBeforeMain()
	{
		// When accessing initInstanceTag in code, the GetInstance() function will be called before main
		volatile void* dummy = initInstanceTag;
		(void)dummy;
	}

private:
	static void* initInstanceTag;
};
template<typename _T> void *Singleton<_T>::initInstanceTag = Singleton<_T>::GetInstance();

template<typename T>
class MemoryGuarder {
public:
	explicit MemoryGuarder(T* ptr) : m_Ptr (ptr) {}
	~MemoryGuarder() { delete m_Ptr; }

private:
	MemoryGuarder& operator= (const MemoryGuarder&);
	MemoryGuarder(const MemoryGuarder&);
	T* const m_Ptr;
};

/**
 * Custom swap() to avoid dependency on C++ <algorithm> header
 * @tparam _T 	Type of the arguments to swap, should be instantiated with primitive C++ types only.
 * @note This has the same semantics as std::swap().
 */
template <typename _T>
inline void Swap(_T& a, _T& b) RAPIDJSON_NOEXCEPT {
	_T tmp = IJSTI_MOVE(a);
	a = IJSTI_MOVE(b);
	b = IJSTI_MOVE(tmp);
}

template<typename _Ch>
class GenericHeadOStream {
public:
	explicit GenericHeadOStream(size_t _capacity)
			: m_capacity (_capacity), m_headOnly(true) { str.reserve(m_capacity + kEllipseSize); }

	typedef _Ch Ch;
	//! Write a character.
	void Put(Ch c)
	{
		if (str.size() < m_capacity) {
			str.push_back(c);
		}
		else {
			if (m_headOnly) {
				str.append("...");
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

typedef GenericHeadOStream<char> HeadOStream;

/**
 * Head Writer that only write heading string. Implement rapidjson::Handler concept
 * It will return false when OutputStream.IsDone() return true
 *
 * @tparam OutputStream		Should implement rapidjson::OutputStream concept, and bool HeadOnly() interface
 * @tparam BaseWriter		Should implement rapidjson::Handler concept, and BaseWriter(OutputStream&) constructor
 */
template<typename OutputStream, typename BaseWriter>
class GenericHeadWriter {
public:
	GenericHeadWriter(OutputStream& stream) : m_stream(stream), m_baseWriter(stream) {}

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

typedef GenericHeadWriter<HeadOStream, rapidjson::Writer<HeadOStream> > HeadWriter;

struct DeserializeErrDoc {
	//! New a DeserializeErrDoc
	//! @param _pAllocator		allocator. set to nullptr if do not need to enable error message
	explicit DeserializeErrDoc(rapidjson::MemoryPoolAllocator<>* _pAllocator):
			pAllocator(_pAllocator) {}

	void TypeMismatch(const char *expectedType, const rapidjson::Value &errVal)
	{
		if (pAllocator == IJSTI_NULL) { return; }

		HeadOStream ostream(16);
		HeadWriter writer(ostream);
		errVal.Accept(writer);

		errMsg.SetObject();
		errMsg.AddMember("type", rapidjson::StringRef("TypeMismatch"), *pAllocator);
		errMsg.AddMember("expectedType",
						 rapidjson::Value().SetString(expectedType, *pAllocator),
						 *pAllocator);
		errMsg.AddMember("json",
						 rapidjson::Value().SetString(ostream.str.c_str(), ostream.str.length(), *pAllocator),
						 *pAllocator);
	}

	//! Set error message about error of member in object
	void ErrorInObject(const char* type, const std::string& memberName, DeserializeErrDoc* errDetail = IJSTI_NULL)
	{
		if (pAllocator == IJSTI_NULL) { return; }

		errMsg.SetObject();
		errMsg.AddMember("type",
						 rapidjson::Value().SetString(type, *pAllocator),
						 *pAllocator);
		errMsg.AddMember("member",
						 rapidjson::Value().SetString(memberName.c_str(), memberName.length(), *pAllocator),
						 *pAllocator);
		if (errDetail != IJSTI_NULL) {
			assert(pAllocator == errDetail->pAllocator);
			errMsg.AddMember("err", errDetail->errMsg, *pAllocator);
		}
	}

	//! Set error message about error of member in array
	void ErrorInArray(const char* type, unsigned index, DeserializeErrDoc* errDetail)
	{
		if (pAllocator == IJSTI_NULL) { return; }
		assert(errDetail != IJSTI_NULL);
		assert(errDetail->pAllocator == pAllocator);

		errMsg.SetObject();
		errMsg.AddMember("type",
						 rapidjson::Value().SetString(type, *pAllocator),
						 *pAllocator);
		errMsg.AddMember("index",
						 rapidjson::Value().SetUint(index),
						 *pAllocator);
		errMsg.AddMember("err", errDetail->errMsg, *pAllocator);
	}

	void MissingMember(DeserializeErrDoc& members)
	{
		if (pAllocator == IJSTI_NULL) { return; }
		assert(pAllocator == members.pAllocator);

		errMsg.SetObject();
		errMsg.AddMember("type", rapidjson::StringRef("MissingMember"), *pAllocator);
		errMsg.AddMember("members", members.errMsg, *pAllocator);
	}

	void PushMemberName(const std::string &memberName)
	{
		if (pAllocator == IJSTI_NULL) { return; }
		assert(errMsg.IsArray());

		errMsg.PushBack(
				rapidjson::Value().SetString(memberName.c_str(), memberName.length(), *pAllocator),
				*pAllocator
		);
	}

	// Pointer to allocator that used to setting error message
	// Use nullptr if do not need error message
	rapidjson::MemoryPoolAllocator<>* const pAllocator;
	rapidjson::Value errMsg;
};

}	// namespace detail
}	// namespace ijst

#endif //_IJST_DETAIL_HPP_INCLUDE_
