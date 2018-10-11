//
// Created by h46incon on 2018/1/29.
//

#ifndef UNIT_TEST_IJST_EXTERN_TEMPLATE_TEST_H
#define UNIT_TEST_IJST_EXTERN_TEMPLATE_TEST_H

#if __cplusplus >= 201103L

#include <ijst/ijst.h>
#include <ijst/types_std.h>
#include <ijst/types_container.h>
#include <gtest/gtest.h>

using namespace ijst;

#ifndef UT_TEST_NAME
	#define UT_TEST_NAME	Default
#endif

// Expands to the concatenation of its two arguments.
#define UTEST_PP_CONCAT(x, y) 		UTEST_PP_CONCAT_I(x, y)
#define UTEST_PP_CONCAT_I(x, y) 	x ## y

IJST_DEFINE_STRUCT(
		Empty
)

IJST_DEFINE_STRUCT_WITH_GETTER(
		ExternTest
		, (IJST_TINT, i, "i", 0)
		, (IJST_TRAW, r, "r", 0)
		, (IJST_TVEC(IJST_TSTR), vs, "vs", 0)
		, (IJST_TMAP(IJST_TBOOL), mb, "mb", 0)
		, (IJST_TST(Empty), e, "e", 0)
)

TEST(Template, UTEST_PP_CONCAT(UT_TEST_NAME, Accessor))
{
	const std::string json = "{\"i\": 0, \"r\": null, "
			"\"vs\": [\"v\"], \"mb\": {\"k\": true}, "
			"\"e\": {}, \"unknown\": false}";

	ExternTest st;
	std::string errMsg;

	// Deserialize
	int iRet = st._.Deserialize(json, errMsg);
	ASSERT_EQ(iRet, 0);

	// Get Unknown
	ASSERT_EQ(st._.GetUnknown()["unknown"].GetBool(), false);

	// Set
	IJST_SET(st, i, -1);

	// Serialize
	std::string out;
	st._.Serialize(out);
	rapidjson::Document doc;
	doc.Parse(out.data(), out.length());
	ASSERT_FALSE(doc.HasParseError());
	ASSERT_EQ(doc["i"].GetInt(), -1);
	ASSERT_TRUE(doc["r"].IsNull());
}

TEST(Template, UTEST_PP_CONCAT(UT_TEST_NAME, Static))
{
	// rapidjson::Document
	rapidjson::Document doc(rapidjson::kNullType);
	ASSERT_TRUE(doc.IsNull());

	// rapidjson::Value
	rapidjson::Value jVal(rapidjson::kObjectType);
	ASSERT_TRUE(jVal.IsObject());

	// rapidjson::MemoryPoolAllocator<>
	rapidjson::MemoryPoolAllocator<> allocator;
	ASSERT_EQ(allocator.Size(), 0u);

	// rapidjson::StringBuffer
	rapidjson::StringBuffer sb;
	ASSERT_EQ(sb.GetSize(), 0u);

	// rapidjson::Writer<rapidjson::StringBuffer>
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	writer.StartObject();
	writer.EndObject();
	ASSERT_STREQ(sb.GetString(), "{}");
}

#endif	// _cplusplus >= 201103L

#endif //UNIT_TEST_IJST_EXTERN_TEMPLATE_TEST_H
