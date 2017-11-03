//
// Created by h46incon on 2017/9/29.
//

#include <ijst/ijst.h>
#include <ijst/types_std.h>
#include <gtest/gtest.h>

namespace dummy_ns {
IJST_DEFINE_STRUCT(
		SimpleSt,
		(IJST_TPRI(Int), int_1, "int_val_1", 0),
		(IJST_TPRI(Int), int_2, "int_val_2", 0),
		(IJST_TPRI(Str), str_1, "str_val_1", 0),
		(IJST_TPRI(Str), str_2, "str_val_2", 0)
)

TEST(BasicAPI, FieldStatus)
{
	SimpleSt simpleSt;

	// Accessor
	ASSERT_EQ(simpleSt._.GetStatus(&simpleSt.int_1), ijst::FStatus::kMissing);
	simpleSt._.SetStrict(simpleSt.int_1, 0x5A5A);
	ASSERT_EQ(simpleSt.int_1, 0x5A5A);
	ASSERT_EQ(simpleSt._.GetStatus(&simpleSt.int_1), ijst::FStatus::kValid);

	// IJST_* macro
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, str_1), ijst::FStatus::kMissing);
	IJST_SET_STRICT(simpleSt, str_1, std::string("str1"));
	ASSERT_STREQ(simpleSt.str_1.c_str(), "str1");
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, str_1), ijst::FStatus::kValid);

	// Make valid
	simpleSt.int_2 = 0xA5A5;
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, int_2), ijst::FStatus::kMissing);
	IJST_MARK_VALID(simpleSt, int_2);
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, int_2), ijst::FStatus::kValid);
}

TEST(BasicAPI, FieldValue)
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

TEST(BasicAPI, Constructor4LValue)
{
	// copy
	{
		SimpleSt temp1;
		IJST_SET(temp1, int_1, 0x5A5A);
		temp1._.GetBuffer().AddMember("k", rapidjson::Value().SetInt(0xA5A5).Move(), temp1._.GetAllocator());

		// copy
		SimpleSt st1(temp1);
		// copy value
		ASSERT_EQ(IJST_GET_STATUS(st1, int_1), ijst::FStatus::kValid);
		ASSERT_EQ(st1.int_1, 0x5A5A);
		// copy inner stream
		ASSERT_EQ(st1._.GetBuffer()["k"].GetInt(), 0xA5A5);
		// new inner stream and allocator
		ASSERT_NE(&st1._.GetAllocator(), &temp1._.GetAllocator());
		ASSERT_NE(&st1._.GetOwnAllocator(), &temp1._.GetOwnAllocator());
		ASSERT_NE(&st1._.GetBuffer(), &temp1._.GetBuffer());
		// new metaField
		IJST_SET(temp1, int_2, 0xA5A5);
		ASSERT_EQ(IJST_GET_STATUS(st1, int_2), ijst::FStatus::kMissing);
		// Avoid make temp1 become rvalue before
		temp1._.MarkValid(&temp1.int_1);
	}


	// Assign
	{
		SimpleSt temp2;
		IJST_SET(temp2, int_1, 0x5A5A);
		temp2._.GetBuffer().AddMember("k", rapidjson::Value().SetInt(0xA5A5).Move(), temp2._.GetAllocator());

		SimpleSt st2;
		IJST_SET(st2, int_2, 0x5A5A);
		st2 = temp2;
		// copy value
		ASSERT_EQ(IJST_GET_STATUS(st2, int_1), ijst::FStatus::kValid);
		ASSERT_EQ(st2.int_1, 0x5A5A);
		ASSERT_EQ(IJST_GET_STATUS(st2, int_2), ijst::FStatus::kMissing);
		// copy inner stream
		ASSERT_EQ(st2._.GetBuffer()["k"].GetInt(), 0xA5A5);
		// new inner stream and allocator
		ASSERT_NE(&st2._.GetAllocator(), &temp2._.GetAllocator());
		ASSERT_NE(&st2._.GetOwnAllocator(), &temp2._.GetOwnAllocator());
		ASSERT_NE(&st2._.GetBuffer(), &temp2._.GetBuffer());
		// new metaField
		IJST_SET(temp2, int_2, 0xA5A5);
		ASSERT_EQ(IJST_GET_STATUS(st2, int_2), ijst::FStatus::kMissing);
		// Avoid make temp2 become rvalue
		temp2._.MarkValid(&temp2.int_1);
	}



}

#if __cplusplus >= 201103L
TEST(BasicAPI, Constructor4RValue)
{
	// copy
	{
		SimpleSt temp1;
		IJST_SET(temp1, int_1, 0x5A5A);
		void* streamTemp1 = &temp1._.GetBuffer();
		void* allocatorTemp1 = &temp1._.GetAllocator();
		void* ownAllocatorTemp1 = &temp1._.GetOwnAllocator();

		SimpleSt st1(std::move(temp1));
		// value
		ASSERT_EQ(IJST_GET_STATUS(st1, int_1), ijst::FStatus::kValid);
		ASSERT_EQ(st1.int_1, 0x5A5A);
		// inner stream
		ASSERT_EQ(&st1._.GetBuffer(), streamTemp1);
		ASSERT_EQ(&st1._.GetAllocator(), allocatorTemp1);
		ASSERT_EQ(&st1._.GetOwnAllocator(), ownAllocatorTemp1);
		//	ASSERT_ANY_THROW(temp3._.GetBuffer());
	}

	// assign
	{
		SimpleSt temp2;
		IJST_SET(temp2, int_1, 0x5A5A);
		void* streamTemp2 = &temp2._.GetBuffer();
		void* allocatorTemp2 = &temp2._.GetAllocator();
		void* ownAllocatorTemp2 = &temp2._.GetOwnAllocator();

		SimpleSt st2;
		st2 = std::move(temp2);
		// value
		ASSERT_EQ(IJST_GET_STATUS(st2, int_1), ijst::FStatus::kValid);
		ASSERT_EQ(st2.int_1, 0x5A5A);
		// inner stream
		ASSERT_EQ(&st2._.GetBuffer(), streamTemp2);
		ASSERT_EQ(&st2._.GetAllocator(), allocatorTemp2);
		ASSERT_EQ(&st2._.GetOwnAllocator(), ownAllocatorTemp2);
		//	ASSERT_ANY_THROW(temp3._.GetBuffer());
	}
}

// TODO: Constructor for out buffer

#endif

IJST_DEFINE_STRUCT(
		Complicate,
		(IJST_TOBJ(SimpleSt), st, "st_v", 0),
		(IJST_TVEC(IJST_TOBJ(SimpleSt)), vec, "vec_v", 0),
		(IJST_TMAP(IJST_TOBJ(SimpleSt)), map, "map_v", 0)
)

TEST(BasicAPI, Allocator)
{
	Complicate cst;
	ASSERT_NE(&cst._.GetAllocator(), &cst.st._.GetAllocator());

	// SetMembersAllocator
	SimpleSt st;
	st._.SetMembersAllocator(cst._.GetAllocator());
	ASSERT_EQ(&cst._.GetAllocator(), &st._.GetAllocator());

	// Init
	cst.vec.push_back(SimpleSt());
	cst.vec.push_back(SimpleSt());
	cst.map["v1"] = SimpleSt();
	cst.map["v2"] = SimpleSt();

	cst._.Init();
	// Allocator is same of fields inited
	ASSERT_EQ(&cst._.GetAllocator(), &cst.vec[0]._.GetAllocator());
	ASSERT_EQ(&cst._.GetAllocator(), &cst.vec[1]._.GetAllocator());
	ASSERT_EQ(&cst._.GetAllocator(), &cst.map["v1"]._.GetAllocator());
	ASSERT_EQ(&cst._.GetAllocator(), &cst.map["v2"]._.GetAllocator());
	// Allocator is not same of fields not inited
	ASSERT_NE(&cst._.GetAllocator(), &cst.map["v3"]._.GetAllocator());
}

struct DummySt {
	IJST_DEFINE_STRUCT(
			SimpleSt,
			(IJST_TPRI(Int), int_1, "int_val_1", 0),
			(IJST_TPRI(Int), int_2, "int_val_2", 0),
			(IJST_TPRI(Str), str_1, "str_val_1", 0),
			(IJST_TPRI(Str), str_2, "str_val_2", 0)
	)
};

TEST(BasicAPI, DefineInStruct)
{
	DummySt::SimpleSt st;
	IJST_SET(st, int_1, 1);
	ASSERT_EQ(st.int_1, 1);
}
}
