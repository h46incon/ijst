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

#if IJST_ENABLE_TO_JSON_OBJECT

#define UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, fieldPushMode)						\
do {																				\
	std::string json;																\
	int toStrRet = st._.Serialize(json, (fieldPushMode));							\
	ASSERT_EQ(toStrRet, 0);															\
	doc.Parse(json.c_str(), json.length());											\
	ASSERT_FALSE(doc.HasParseError());												\
	rapidjson::Value _jVal;															\
	int serRet = st._.MoveToJson(_jVal, st._.GetAllocator(), (fieldPushMode));		\
	ASSERT_EQ(serRet, 0);															\
	ASSERT_EQ((rapidjson::Value&)doc, _jVal);										\
	if (((fieldPushMode) & ijst::FPush::kPushUnknown) != 0)							\
	{ ASSERT_EQ(st._.GetUnknown().MemberCount(), 0u); }								\
} while (false)

#else

#define UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, fieldPushMode)				\
do {																		\
	std::string json;														\
	int toStrRet = st._.Serialize(json, (fieldPushMode));					\
	ASSERT_EQ(toStrRet, 0);													\
	doc.Parse(json.c_str(), json.length());									\
	ASSERT_FALSE(doc.HasParseError());										\
} while (false)

#endif

inline void CheckMemberTypeMismatch(const std::string& errMsg, const char* fieldName, const char* expectedType, const char* value)
{
	rapidjson::Document doc;
	doc.Parse(errMsg.c_str(), errMsg.length());
	ASSERT_FALSE(doc.HasParseError());

	ASSERT_TRUE(doc.IsObject());
	ASSERT_STREQ(doc["type"].GetString(), "ErrInObject");
	ASSERT_STREQ(doc["member"].GetString(), fieldName);

	ASSERT_TRUE(doc["err"].IsObject());
	ASSERT_STREQ(doc["err"]["type"].GetString(), "TypeMismatch");
	ASSERT_STREQ(doc["err"]["expectedType"].GetString(), expectedType);

	// The test of HeadOStream is in detail_test.cpp
	// The capacity of ostream must be same as ijst/detail/detail.h
	ijst::detail::HeadOStream ostream(16);
	const char* pc = value;
	while (*pc != '\0')
	{
		ostream.Put(*pc);
		++pc;
	}
	ASSERT_STREQ(ostream.str.c_str(), doc["err"]["json"].GetString());
}

#endif //UNIT_TEST_IJST_UTIL_H
