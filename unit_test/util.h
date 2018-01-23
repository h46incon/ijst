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

inline void CheckTypeMismatch(const rapidjson::Value& errDoc, const char* expectedType, const char* value)
{
	ASSERT_TRUE(errDoc.IsObject());
	ASSERT_STREQ(errDoc["type"].GetString(), "TypeMismatch");
	ASSERT_STREQ(errDoc["expectedType"].GetString(), expectedType);

	// The test of HeadOStream is in detail_test.cpp
	// The capacity of ostream must be same as ijst/detail/detail.h
	ijst::detail::HeadOStream ostream(16);
	const char* pc = value;
	while (*pc != '\0')
	{
		ostream.Put(*pc);
		++pc;
	}
	ASSERT_STREQ(ostream.str.c_str(), errDoc["json"].GetString());
}

inline void CheckMemberTypeMismatch(const std::string& errMsg, const char* fieldName, const char* expectedType, const char* value)
{
	UTEST_PARSE_STR_TO_JSON(errMsg, doc)

	ASSERT_TRUE(doc.IsObject());
	ASSERT_STREQ(doc["type"].GetString(), "ErrInObject");
	ASSERT_STREQ(doc["member"].GetString(), fieldName);

	CheckTypeMismatch(doc["err"], expectedType, value);
}

#endif //UNIT_TEST_IJST_UTIL_H
