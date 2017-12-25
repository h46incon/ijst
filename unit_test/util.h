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
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);								\
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

#endif //UNIT_TEST_IJST_UTIL_H
