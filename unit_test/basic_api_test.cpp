//
// Created by h46incon on 2017/9/29.
//

#include <ijst/ijst.h>
#include <ijst/types_std.h>
#include <gtest/gtest.h>

namespace dummy_ns {

TEST(BasicAPI, WrapperCommon)
{
	// default constructor
	ijst::SLWrapper<std::string> val;
	ASSERT_TRUE(val.Val().empty());

	val.Val() = "Val";
	{
		const ijst::SLWrapper<std::string>& valRef = val;
		ASSERT_EQ(valRef.Val(), "Val");
	}

	// Copy constructor
	{
		ijst::SLWrapper<std::string> val2 (val);
		ASSERT_EQ(val2.Val(), "Val");
	}

	// Copy constructor for TVal
	{
		std::string innerV = val.Val();
		ijst::SLWrapper<std::string> val2(innerV);
		ASSERT_EQ(val2.Val(), "Val");
	}

	// Assignment
	{
		ijst::SLWrapper<std::string> val2;
		val2 = val;
		ASSERT_EQ(val2.Val(), "Val");
	}

	// Assignment for TVal
	{
		std::string innerV = val.Val();
		ijst::SLWrapper<std::string> val2;
		val2 = innerV;
		ASSERT_EQ(val2.Val(), "Val");
	}

	// Check source
	ASSERT_EQ(val.Val(), "Val");

#if __cplusplus >= 201103L
	// Copy constructor for RValue
	{
		ijst::SLWrapper<std::string> valTemp (val);
		ijst::SLWrapper<std::string> val2 (std::move(valTemp));
		ASSERT_EQ(val2.Val(), "Val");
	}

	// Copy constructor for TVal RValue
	{
		std::string innerV = val.Val();
		ijst::SLWrapper<std::string> val2(std::move(innerV));
		ASSERT_EQ(val2.Val(), "Val");
	}

	// Assignment for RValue
	{
		ijst::SLWrapper<std::string> valTemp (val);
		ijst::SLWrapper<std::string> val2;
		val2 = std::move(valTemp);
		ASSERT_EQ(val2.Val(), "Val");
	}

	// Assignment for TVal
	{
		std::string innerV = val.Val();
		ijst::SLWrapper<std::string> val2;
		val2 = std::move(innerV);
		ASSERT_EQ(val2.Val(), "Val");
	}
#endif
}
TEST(BasicAPI, WrapperVector)
{
	// default constructor
	ijst::Vector<int> val;
	ASSERT_TRUE(val->empty());

	// operator []
	val->push_back(1);
	val[0] = 2;
	const ijst::Vector<int>& vecRef = val;
	ASSERT_EQ(vecRef[0], 2);

	// Copy constructor
	{
		ijst::Vector<int> val2 (val);
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2[0], 2);
	}

	// Copy constructor for TVal
	{
		std::vector<int> innerV = val.Val();
		ijst::Vector<int> val2(innerV);
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2[0], 2);
	}

	// Assignment
	{
		ijst::Vector<int> val2;
		val2 = val;
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2[0], 2);
	}

	// Assignment for TVal
	{
		std::vector<int> innerV = val.Val();
		ijst::Vector<int> val2;
		val2 = innerV;
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2[0], 2);
	}

	// Check source
	ASSERT_EQ(val->size(), 1u);
	ASSERT_EQ(val[0], 2);

#if __cplusplus >= 201103L
	// Copy constructor for RValue
	{
		ijst::Vector<int> valTemp (val);
		ijst::Vector<int> val2 (std::move(valTemp));
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2[0], 2);
	}

	// Copy constructor for TVal RValue
	{
		std::vector<int> innerV = val.Val();
		ijst::Vector<int> val2(std::move(innerV));
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2[0], 2);
	}

	// Assignment for RValue
	{
		ijst::Vector<int> valTemp (val);
		ijst::Vector<int> val2;
		val2 = std::move(valTemp);
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2[0], 2);
	}

	// Assignment for TVal
	{
		std::vector<int> innerV = val.Val();
		ijst::Vector<int> val2;
		val2 = std::move(innerV);
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2[0], 2);
	}
#endif
}


TEST(BasicAPI, WrapperMap)
{
	// default constructor
	ijst::Map<std::string, int> val;
	ASSERT_TRUE(val->empty());

	// operator []
	val["k"] = 2;
	ASSERT_EQ(val["k"], 2);

	// Copy constructor
	{
		ijst::Map<std::string, int> val2 (val);
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2["k"], 2);
	}

	// Copy constructor for TVal
	{
		std::map<std::string, int> innerV = val.Val();
		ijst::Map<std::string, int> val2(innerV);
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2["k"], 2);
	}

	// Assignment
	{
		ijst::Map<std::string, int> val2;
		val2 = val;
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2["k"], 2);
	}

	// Assignment for TVal
	{
		std::map<std::string, int> innerV = val.Val();
		ijst::Map<std::string, int> val2;
		val2 = innerV;
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2["k"], 2);
	}

	// Check source
	ASSERT_EQ(val->size(), 1u);
	ASSERT_EQ(val["k"], 2);

#if __cplusplus >= 201103L
	// Copy constructor for RValue
	{
		ijst::Map<std::string, int> valTemp (val);
		ijst::Map<std::string, int> val2 (std::move(valTemp));
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2["k"], 2);
	}

	// Copy constructor for TVal RValue
	{
		std::map<std::string, int> innerV = val.Val();
		ijst::Map<std::string, int> val2(std::move(innerV));
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2["k"], 2);
	}

	// Assignment for RValue
	{
		ijst::Map<std::string, int> valTemp (val);
		ijst::Map<std::string, int> val2;
		val2 = std::move(valTemp);
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2["k"], 2);
	}

	// Assignment for TVal
	{
		std::map<std::string, int> innerV = val.Val();
		ijst::Map<std::string, int> val2;
		val2 = std::move(innerV);
		ASSERT_EQ(val2->size(), 1u);
		ASSERT_EQ(val2["k"], 2);
	}
#endif
}

IJST_DEFINE_STRUCT(
		SimpleSt
		, (IJST_TPRI(Int), int_1, "int_val_1", 0)
		, (IJST_TPRI(Int), int_2, "int_val_2", 0)
		, (IJST_TPRI(Str), str_1, "str_val_1", 0)
		, (IJST_TPRI(Str), str_2, "str_val_2", 0)
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

	// Mark valid
	simpleSt.int_2 = 0xA5A5;
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, int_2), ijst::FStatus::kMissing);
	IJST_MARK_VALID(simpleSt, int_2);
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, int_2), ijst::FStatus::kValid);

	// IsField
	ASSERT_TRUE(simpleSt._.HasField(&simpleSt.int_1));
	ASSERT_FALSE(simpleSt._.HasField(&simpleSt.str_2 + 1));
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
		Complicate
		, (IJST_TOBJ(SimpleSt), st, "st_v", 0)
		, (IJST_TVEC(IJST_TOBJ(SimpleSt)), vec, "vec_v", 0)
		, (IJST_TMAP(IJST_TOBJ(SimpleSt)), map, "map_v", 0)
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
	IJST_CONT_VAL(cst.vec).push_back(SimpleSt());
	IJST_CONT_VAL(cst.vec).push_back(SimpleSt());
	cst.map["v1"] = SimpleSt();
	cst.map["v2"] = SimpleSt();

	cst._.InitMembersAllocator();
	// Allocator is same of fields inited
	ASSERT_EQ(&cst._.GetAllocator(), &cst.vec[0]._.GetAllocator());
	ASSERT_EQ(&cst._.GetAllocator(), &cst.vec[1]._.GetAllocator());
	ASSERT_EQ(&cst._.GetAllocator(), &cst.map["v1"]._.GetAllocator());
	ASSERT_EQ(&cst._.GetAllocator(), &cst.map["v2"]._.GetAllocator());
	// Allocator is not same of fields not inited
	ASSERT_NE(&cst._.GetAllocator(), &cst.map["v3"]._.GetAllocator());
}

IJST_DEFINE_STRUCT_WITH_GETTER(
		SWGetter
		, (IJST_TPRI(Int), int_1, "int_val_1", 0)
		, (IJST_TPRI(Int), int_2, "int_val_2", 0)
		, (IJST_TPRI(Str), str_1, "str_val_1", 0)
		, (IJST_TPRI(Str), str_2, "str_val_2", 0)
)

IJST_DEFINE_STRUCT_WITH_GETTER(
		CWGetter
		, (IJST_TOBJ(SimpleSt), sim, "sim_v", 0)
		, (IJST_TOBJ(SWGetter), st, "st_v", 0)
		, (IJST_TVEC(IJST_TOBJ(SWGetter)), vec, "vec_v", 0)
		, (IJST_TMAP(IJST_TOBJ(SWGetter)), map, "map_v", 0)
)

IJST_DEFINE_STRUCT_WITH_GETTER(
		CWGetter2
		, (IJST_TOBJ(CWGetter), v, "v", 0)
)

TEST(BasicAPI, ChainedOptional)
{
	CWGetter2 st;

	// fields are missing:

	// Get* null
	ASSERT_EQ(IJST_NULL, st.Getv().Ptr());
	// Get* null chained
	ASSERT_EQ(IJST_NULL, st.Getv()->Getvec().Ptr());
	// Long null chained
	ASSERT_EQ(IJST_NULL, st.Getv()->Getsim().Ptr());
	ASSERT_EQ(IJST_NULL, st.Getv()->Getvec()[0]->Getint_1().Ptr());
	ASSERT_EQ(IJST_NULL, st.Getv()->Getmap()[""]->Getint_1().Ptr());

	// Get valid
	IJST_MARK_VALID(st, v);
	ASSERT_EQ(st.Getv().Ptr(), &(st.v));
	IJST_MARK_VALID(st.v, sim);
	ASSERT_EQ(st.Getv()->Getsim().Ptr(), &(st.v.sim));

	// vector null
	ASSERT_EQ(IJST_NULL, st.v.Getvec()[0].Ptr());
	// vector elem out of range
	IJST_MARK_VALID(st.v, vec);
	ASSERT_EQ(st.v.Getvec().Ptr(), &(st.v.vec));
	ASSERT_EQ(IJST_NULL, st.v.Getvec()[0].Ptr());
	// vector valid
	IJST_CONT_VAL(st.v.vec).resize(1);
	ASSERT_EQ(st.v.Getvec()[0].Ptr(), &(st.v.vec[0]));
	ASSERT_EQ(IJST_NULL, st.v.Getvec()[0]->Getint_1().Ptr());

	// map null
	ASSERT_EQ(IJST_NULL, st.v.Getmap()[""].Ptr());
	// map key not exist
	IJST_MARK_VALID(st.v, map);
	ASSERT_EQ(st.v.Getmap().Ptr(), &(st.v.map));
	ASSERT_EQ(IJST_NULL, st.v.Getmap()[""].Ptr());
	// map valid
	st.v.map[""];
	ASSERT_EQ(st.v.Getmap()[""].Ptr(), &(st.v.map[""]));
	ASSERT_EQ(IJST_NULL, st.v.Getmap()[""]->Getint_1().Ptr());

	// Long valid chained
	IJST_MARK_VALID(st.v.vec[0], int_1);
	ASSERT_EQ(st.Getv()->Getvec()[0]->Getint_1().Ptr(), &(st.v.vec[0].int_1));
	IJST_MARK_VALID(st.v.map[""], int_2);
	ASSERT_EQ(st.Getv()->Getmap()[""]->Getint_2().Ptr(), &(st.v.map[""].int_2));
}

struct DummySt {
	IJST_DEFINE_STRUCT(
			SimpleSt
			, (IJST_TPRI(Int), int_1, "int_val_1", 0)
			, (IJST_TPRI(Int), int_2, "int_val_2", 0)
			, (IJST_TPRI(Str), str_1, "str_val_1", 0)
			, (IJST_TPRI(Str), str_2, "str_val_2", 0)
	)
};

TEST(BasicAPI, DefineInStruct)
{
	DummySt::SimpleSt st;
	IJST_SET(st, int_1, 1);
	ASSERT_EQ(st.int_1, 1);
}

#if __cplusplus >= 201103L
void TestDefineInFunction()
{
	IJST_DEFINE_STRUCT(
		StInFunc
		, (IJST_TPRI(Int), int_1, "int_val_1", 0)
	);

	StInFunc st;
	IJST_SET(st, int_1, 1);
	ASSERT_EQ(st.int_1, 1);
}

TEST(BasicAPI, DefineInFunction)
{
	TestDefineInFunction();
}
#endif
}

