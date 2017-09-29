//
// Created by h46incon on 2017/9/29.
//

#include "ijst/ijst.h"
#include "gtest/gtest.h"

namespace dummy_ns {
//IJST_DEFINE_STRUCT(
IJSTI_DEFINE_STRUCT_IMPL_4(
		SimpleSt,
		(IJST_TPRI(Int), int_1, "int_val_1", 0),
		(IJST_TPRI(Int), int_2, "int_val_2", 0),
		(IJST_TPRI(String), str_1, "str_val_1", 0),
		(IJST_TPRI(String), str_2, "str_val_2", 0)
)

// TODO: Copy constructor
// TODO: Assigment

TEST(BasicAPI, FieldStatus)
{
	SimpleSt simpleSt;

	// Accessor
	ASSERT_EQ(simpleSt._.GetStatus(simpleSt.int_1), ijst::FStatus::Null);
	simpleSt._.SetStrict(simpleSt.int_1, 0x5A5A);
	ASSERT_EQ(simpleSt.int_1, 0x5A5A);
	ASSERT_EQ(simpleSt._.GetStatus(simpleSt.int_1), ijst::FStatus::Valid);

	// IJST_* macro
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, str_1), ijst::FStatus::Null);
	IJST_SET_STRICT(simpleSt, str_1, std::string("str1"));
	ASSERT_STREQ(simpleSt.str_1.c_str(), "str1");
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, str_1), ijst::FStatus::Valid);

	// Make valid
	simpleSt.int_2 = 0xA5A5;
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, int_2), ijst::FStatus::Null);
	IJST_MAKE_VALID(simpleSt, int_2);
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, int_2), ijst::FStatus::Valid);
}

TEST(BaseAPI, FieldValue)
{
	SimpleSt simpleSt;

	// Init
	simpleSt._.SetStrict(simpleSt.int_1, 0x5A5A);
	IJST_SET(simpleSt, int_2, 0xA5A5);
	simpleSt._.Set(simpleSt.str_1, "str1");
	IJST_SET_STRICT(simpleSt, str_2, std::string("str2"));

	// Check
	ASSERT_EQ(simpleSt.int_1, 0x5A5A);
	ASSERT_EQ(simpleSt.int_2, 0xA5A5);
	ASSERT_STREQ(simpleSt.str_1.c_str(), "str1");
	ASSERT_STREQ(simpleSt.str_2.c_str(), "str2");
}
}
