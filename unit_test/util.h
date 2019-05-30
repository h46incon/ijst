//
// Created by h46incon on 2017/12/14.
//

#ifndef UNIT_TEST_IJST_UTIL_H
#define UNIT_TEST_IJST_UTIL_H

#include <gtest/gtest.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <ijst/ijst.h>
#include <ijst/types_std.h>
#include <ijst/types_container.h>
#include <ijst/detail/detail.h>
#define UTEST_SERIALIZE_AND_CHECK(st, doc, serFlag)							\
do {																		\
	std::string json;														\
	int toStrRet = st._.Serialize(json, (serFlag));							\
	ASSERT_EQ(toStrRet, 0);													\
	doc.Parse(json.c_str(), json.length());									\
	ASSERT_FALSE(doc.HasParseError());										\
	{																		\
		ijst::SAXGeneratorWrapper<rapidjson::Document> 						\
			generator(st._, serFlag);										\
		rapidjson::Document docTemp;										\
		docTemp.Populate(generator);										\
		ASSERT_FALSE(docTemp.HasParseError());								\
		ASSERT_EQ((rapidjson::Value&)docTemp, (rapidjson::Value&)doc);		\
	}																		\
} while (false)

#define UTEST_PARSE_STR_TO_JSON(str, jsonOutput)							\
	rapidjson::Document jsonOutput;											\
	jsonOutput.Parse(str.c_str(), str.length());							\
	ASSERT_FALSE(jsonOutput.HasParseError());

// Test for encoding
#define DEFINE_ENCODING_TEST_STRUCT(encoding, stName, PF) \
	IJST_DEFINE_GENERIC_STRUCT( \
		encoding, stName \
		, (ijst::T_int, int_v, PF ## "int_val")\
		, (IJST_TSTR, str_v, PF ## "str_val")\
		, (IJST_TMAP(ijst::T_int), map_v, PF ## "map_val")\
	)

DEFINE_ENCODING_TEST_STRUCT(rapidjson::UTF8<>, U8TestEncoding, )
#if __cplusplus >= 201103L
DEFINE_ENCODING_TEST_STRUCT(rapidjson::UTF16<char16_t>, U16TestEncoding, u)
DEFINE_ENCODING_TEST_STRUCT(rapidjson::UTF32<char32_t>, U32TestEncoding, U)
#endif

template<typename CharType>
void AssertStrEq(const CharType* s1, const CharType* s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++, s2++;
	}

	ASSERT_EQ(*s1, *s2);
}

template<>
inline void AssertStrEq<char>(const char* s1, const char* s2)
{
	ASSERT_STREQ(s1, s2);
}

template<>
inline void AssertStrEq<wchar_t>(const wchar_t* s1, const wchar_t* s2)
{
	ASSERT_STREQ(s1, s2);
}

template<typename SourceEncoding, typename TargetEncoding>
std::basic_string<typename TargetEncoding::Ch>Transcode(const typename SourceEncoding::Ch* src)
{
	rapidjson::GenericStringStream<SourceEncoding> source(src);
	rapidjson::GenericStringBuffer<TargetEncoding> target;
	while (source.Peek() != '\0') {
		rapidjson::Transcoder<SourceEncoding, TargetEncoding>::Transcode(source, target);
	}

	return std::basic_string<typename TargetEncoding::Ch>(target.GetString());
};

inline void CheckTypeMismatch(const rapidjson::Value& errDoc, const char* expectedType, const char* value)
{
	ASSERT_TRUE(errDoc.IsObject());
	ASSERT_STREQ(errDoc["type"].GetString(), "TypeMismatch");
	ASSERT_STREQ(errDoc["expectedType"].GetString(), expectedType);

	// The test of HeadOStream is in detail_test.cpp
	// The capacity of ostream must be same as ijst/detail/detail.h
	ijst::detail::HeadOStream<rapidjson::UTF8<> > ostream(16);
	const char* pc = value;
	while (*pc != '\0')
	{
		ostream.Put(*pc);
		++pc;
	}
	ASSERT_STREQ(ostream.str.c_str(), errDoc["json"].GetString());
}

inline void CheckMemberValueIsDefault(const char* memberName, const char* jsonKey, int retCode, const std::string& errMsg)
{
	ASSERT_EQ(ijst::ErrorCode::kDeserializeValueIsDefault, retCode);
	rapidjson::Document errDoc;
	errDoc.Parse(errMsg.data(), errMsg.length());
	ASSERT_FALSE(errDoc.HasParseError());
	ASSERT_TRUE(errDoc.IsObject());
	ASSERT_STREQ(errDoc["type"].GetString(), "ErrInObject");
	ASSERT_STREQ(errDoc["member"].GetString(), memberName);
	ASSERT_STREQ(errDoc["jsonKey"].GetString(), jsonKey);
	ASSERT_STREQ(errDoc["err"]["type"].GetString(), "ValueIsDefault");
}

inline void CheckMemberTypeMismatch(const std::string& errMsg, const char* memberName, const char* jsonKey, const char* expectedType, const char* value)
{
	UTEST_PARSE_STR_TO_JSON(errMsg, doc)

	ASSERT_TRUE(doc.IsObject());
	ASSERT_STREQ(doc["type"].GetString(), "ErrInObject");
	ASSERT_STREQ(doc["member"].GetString(), memberName);
	ASSERT_STREQ(doc["jsonKey"].GetString(), jsonKey);

	CheckTypeMismatch(doc["err"], expectedType, value);
}

#endif //UNIT_TEST_IJST_UTIL_H
