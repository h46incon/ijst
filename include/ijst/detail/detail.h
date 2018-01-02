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
#define IJSTI_PP_CONCAT(x, y) IJSTI_PP_CONCAT_PRIMITIVE(x, y)
#define IJSTI_PP_CONCAT_PRIMITIVE(x, y) x ## y

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

struct ErrDoc {
	//! New a error document according to pErrDoc
	//!@note The caller should delete the return pointer
	static rapidjson::Document* NewErrDoc(rapidjson::Document* pErrDoc)
	{
		if (pErrDoc == IJSTI_NULL) { return IJSTI_NULL; }
		return new rapidjson::Document(&pErrDoc->GetAllocator());
	}

	static void TypeMismatch(rapidjson::Document *pErrDoc, const char *expectedType, const rapidjson::Value &errVal)
	{
		if (pErrDoc == IJSTI_NULL) { return; }

		HeadOStream ostream(16);
		HeadWriter writer(ostream);
		errVal.Accept(writer);

		pErrDoc->SetObject();
		pErrDoc->AddMember("type", rapidjson::StringRef("TypeMismatch"), pErrDoc->GetAllocator());
		pErrDoc->AddMember("expectedType",
						   rapidjson::Value().SetString(expectedType, pErrDoc->GetAllocator()),
						   pErrDoc->GetAllocator());
		pErrDoc->AddMember("json",
						   rapidjson::Value().SetString(ostream.str.c_str(), ostream.str.length(), pErrDoc->GetAllocator()),
						   pErrDoc->GetAllocator());
	}

	//! Set error message about error of member in object
	static void ErrorInObject(rapidjson::Document* pErrDoc, const char* type, const std::string& memberName, rapidjson::Value* errDetail = IJSTI_NULL)
	{
		if (pErrDoc == IJSTI_NULL) { return; }

		pErrDoc->SetObject();
		pErrDoc->AddMember("type",
						   rapidjson::Value().SetString(type, pErrDoc->GetAllocator()),
						   pErrDoc->GetAllocator());
		pErrDoc->AddMember("member",
						   rapidjson::Value().SetString(memberName.c_str(), memberName.length(), pErrDoc->GetAllocator()),
						   pErrDoc->GetAllocator());
		if (errDetail != IJSTI_NULL) {
			pErrDoc->AddMember("err", *errDetail, pErrDoc->GetAllocator());
		}
	}

	//! Set error message about error of member in array
	static void ErrorInArray(rapidjson::Document* pErrDoc, const char* type, unsigned index, rapidjson::Value* errDetail)
	{
		if (pErrDoc == IJSTI_NULL) { return; }
		assert(errDetail != IJSTI_NULL);

		pErrDoc->SetObject();
		pErrDoc->AddMember("type",
						   rapidjson::Value().SetString(type, pErrDoc->GetAllocator()),
						   pErrDoc->GetAllocator());
		pErrDoc->AddMember("index",
						   rapidjson::Value().SetUint(index),
						   pErrDoc->GetAllocator());
		pErrDoc->AddMember("err", *errDetail, pErrDoc->GetAllocator());
	}

	static void MissingMember(rapidjson::Document *pErrDoc, rapidjson::Document *pMembers)
	{
		if (pErrDoc == IJSTI_NULL) { return; }
		assert(pMembers != IJSTI_NULL);
		assert(&pErrDoc->GetAllocator() == &pMembers->GetAllocator());

		pErrDoc->SetObject();
		pErrDoc->AddMember("type", rapidjson::StringRef("MissingMember"), pErrDoc->GetAllocator());
		pErrDoc->AddMember("members", pMembers->Move(), pErrDoc->GetAllocator());
	}

	static void PushMemberName(rapidjson::Document *pErrDoc, const std::string &memberName)
	{
		if (pErrDoc == IJSTI_NULL) { return; }
		assert(pErrDoc->IsArray());

		pErrDoc->PushBack(
				rapidjson::Value().SetString(memberName.c_str(), memberName.length(), pErrDoc->GetAllocator()),
				pErrDoc->GetAllocator()
		);
	}

};

}	// namespace detail
}	// namespace ijst

#endif //_IJST_DETAIL_HPP_INCLUDE_
