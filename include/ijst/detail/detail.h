//
// Created by h46incon on 2017/12/26.
//

#ifndef _IJST_DETAIL_HPP_INCLUDE_
#define _IJST_DETAIL_HPP_INCLUDE_

#include <rapidjson/rapidjson.h>
#include <string>

namespace ijst{
namespace detail{

#if __cplusplus >= 201103L
	#define IJSTI_MOVE(val) 	std::move((val))
	#define IJSTI_OVERRIDE		override
	#define IJSTI_NOEXCEPT		noexcept
#else
	#define IJSTI_MOVE(val) 	(val)
	#define IJSTI_OVERRIDE
	#define IJSTI_NOEXCEPT
#endif

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


template<typename _Ch>
class GenericCancelableOStream {
public:
	explicit GenericCancelableOStream(size_t _capacity)
			: m_capacity (_capacity), m_IsDone(false) { str.reserve(m_capacity); }

	typedef _Ch Ch;
	//! Write a character.
	void Put(Ch c)
	{
		if (str.size() < m_capacity) {
			str.push_back(c);
		}
		else {
			m_IsDone = true;
		}
	}

	//! Flush the buffer.
	void Flush() {;}

	bool IsDone() const { return m_IsDone; }

	std::basic_string<Ch> str;
private:
	const size_t m_capacity;
	bool m_IsDone;
};

typedef GenericCancelableOStream<char> CancelableOStream;

/**
 * Cancelable Writer, implement rapidjson::Handler concept
 * It will return false when OutputStream.IsDone() return true
 *
 * @tparam OutputStream		Should implement rapidjson::OutputStream concept, and bool IsDone() interface
 * @tparam BaseWriter		Should implement rapidjson::Handler concept, and BaseWriter(OutputStream&) consturctor
 */
template<typename OutputStream, typename BaseWriter>
class CancelableWriter {
public:
	CancelableWriter(OutputStream& stream) : m_stream(stream), m_baseWriter(stream) {}

	typedef typename BaseWriter::Ch Ch;
	bool Null() { return m_baseWriter.Null() && !m_stream.IsDone(); }
	bool Bool(bool b) { return m_baseWriter.Bool(b) && !m_stream.IsDone(); }
	bool Int(int i) { return m_baseWriter.Int(i) && !m_stream.IsDone(); }
	bool Uint(unsigned i) { return m_baseWriter.Uint(i) && !m_stream.IsDone(); }
	bool Int64(int64_t i) { return m_baseWriter.Int64(i) && !m_stream.IsDone(); }
	bool Uint64(uint64_t i) { return m_baseWriter.Uint64(i) && !m_stream.IsDone(); }
	bool Double(double d) { return m_baseWriter.Double(d) && !m_stream.IsDone(); }
	/// enabled via kParseNumbersAsStringsFlag, string is not null-terminated (use length)
	bool RawNumber(const Ch* str, rapidjson::SizeType length, bool copy = false)
	{ return m_baseWriter.RawNumber(str, length, copy) && !m_stream.IsDone(); }
	bool String(const Ch* str, rapidjson::SizeType length, bool copy = false)
	{ return m_baseWriter.String(str, length, copy) && !m_stream.IsDone(); }
	bool StartObject() { return m_baseWriter.StartObject() && !m_stream.IsDone(); }
	bool Key(const Ch* str, rapidjson::SizeType length, bool copy = false)
	{ return m_baseWriter.Key(str, length, copy) && !m_stream.IsDone(); }
	bool EndObject(rapidjson::SizeType memberCount = 0)
	{ return m_baseWriter.EndObject(memberCount) && !m_stream.IsDone(); }
	bool StartArray() { return m_baseWriter.StartArray() && !m_stream.IsDone(); }
	bool EndArray(rapidjson::SizeType elementCount = 0)
	{ return m_baseWriter.EndArray(elementCount) && !m_stream.IsDone(); }

private:
	OutputStream& m_stream;
	BaseWriter m_baseWriter;
};

}	// namespace detail
}	// namespace ijst

#endif //_IJST_DETAIL_HPP_INCLUDE_
