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
	ASSERT_EQ(ret, ijst::Err::kDeserializeSomeFiledsInvalid);
}

TEST(Deserialize, ParseError)
{
	string errJson = "{a:1}";
	SimpleSt st;
	int ret = st._.Deserialize(errJson, 0);
	ASSERT_EQ(ret, ijst::Err::kParseFaild);
}

