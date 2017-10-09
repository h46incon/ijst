//
// Created by h46incon on 2017/9/30.
//
#include "gtest/gtest.h"
#include "ijst/ijst.h"
using std::vector;
using std::map;
using std::string;


IJST_DEFINE_STRUCT(
		SimpleSt,
		(IJST_TPRI(Int), int_1, "int_val_1", ijst::FDesc::Optional),
		(IJST_TPRI(Int), int_2, "int_val_2", 0),
		(IJST_TPRI(String), str_1, "str_val_1", ijst::FDesc::Optional),
		(IJST_TPRI(String), str_2, "str_val_2", 0)
)

TEST(Deserialize, Empty)
{
	string emptyJson = "{}";
	SimpleSt st;
	int ret = st._.Deserialize(emptyJson, 0);
	int retExpected = ijst::Err::kDeserializeSomeFiledsInvalid;
	ASSERT_EQ(ret, retExpected);
}

TEST(Deserialize, ParseError)
{
	string errJson = "{withoutQuote:1}";
	SimpleSt st;
	int ret = st._.Deserialize(errJson, 0);
	int retExpected = ijst::Err::kParseFaild;
	ASSERT_EQ(ret, retExpected);
}

TEST(Deserialize, TypeError)
{
	string errJson = "{\"int_val_2\":\"str\"}";
	SimpleSt st;
	int ret = st._.Deserialize(errJson, 0);
	int retExpected = ijst::Err::kDeserializeValueTypeError;
	ASSERT_EQ(ret, retExpected);
}

TEST(Deserialize, RequiredFields)
{
	string validJson = "{\"int_val_2\":1, \"str_val_2\":\"str2\"}";
	SimpleSt st;
	int ret = st._.Deserialize(validJson, 0);
	ASSERT_EQ(ret, 0);

	ASSERT_EQ(IJST_GET_STATUS(st, int_1), ijst::FStatus::Null);
	ASSERT_EQ(IJST_GET_STATUS(st, str_1), ijst::FStatus::Null);
	ASSERT_EQ(st.int_2, 1);
	ASSERT_STREQ(st.str_2.c_str(), "str2");
}

TEST(Deserialize, AdditionalFields)
{
	string validJson = "{\"int_val_1\":1, \"int_val_2\":2, \"str_val_2\":\"str2\", \"additional_field\": \"a_field\"}";
	SimpleSt st;
	int ret = st._.Deserialize(validJson, 0);
	ASSERT_EQ(ret, 0);

	ASSERT_EQ(IJST_GET_STATUS(st, int_1), ijst::FStatus::Valid);
	ASSERT_EQ(IJST_GET_STATUS(st, str_1), ijst::FStatus::Null);
	ASSERT_EQ(st.int_1, 1);
	ASSERT_EQ(st.int_2, 2);
	ASSERT_STREQ(st.str_2.c_str(), "str2");

	ASSERT_STREQ(st._.InnerStream()["additional_field"].GetString(), "a_field");
}
