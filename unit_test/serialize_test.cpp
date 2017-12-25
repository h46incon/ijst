//
// Created by h46incon on 2017/9/29.
//

#include "util.h"

#include <ijst/ijst.h>
#include <ijst/types_std.h>

using std::vector;
using std::map;
using std::string;
using namespace ijst;

IJST_DEFINE_STRUCT(
		Inner
		, (IJST_TPRI(Int), int_1, "int_val_1", 0)
		, (IJST_TPRI(Int), int_2, "int_val_2", 0)
		, (IJST_TPRI(Str), str_1, "str_val_1", 0)
		, (IJST_TPRI(Str), str_2, "str_val_2", 0)
)

IJST_DEFINE_STRUCT(
		NestSt
		, (IJST_TVEC(IJST_TPRI(Int)), vec_1, "vec_val_1", 0)
		, (IJST_TOBJ(Inner), inner_1, "inner_val_1", 0)
		, (IJST_TMAP(IJST_TPRI(Str)), map_1, "map_val_1", 0)
		, (IJST_TVEC(IJST_TOBJ(Inner)), vec_2, "vec_val_2", 0)
)

TEST(Serialize, EmptyValue)
{
	NestSt nestSt;

	// Empty struct
	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(nestSt, doc, 0);
	ASSERT_TRUE(doc.IsObject());
	ASSERT_TRUE(doc.MemberCount() == 0);
}

TEST(Serialize, EmptyValue_PushAllField)
{
	NestSt nestSt;
	// Empty struct

	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(nestSt, doc, FPush::kPushAllFields | FPush::kPushUnknown);

	// Check
	ASSERT_TRUE(doc["vec_val_1"].IsArray());
	ASSERT_TRUE(doc["vec_val_1"].Empty());

	ASSERT_TRUE(doc["inner_val_1"].IsObject());
	ASSERT_EQ(doc["inner_val_1"]["int_val_1"].GetInt(), 0);
	ASSERT_EQ(doc["inner_val_1"]["int_val_2"].GetInt(), 0);
	ASSERT_STREQ(doc["inner_val_1"]["str_val_1"].GetString(), "");
	ASSERT_STREQ(doc["inner_val_1"]["str_val_2"].GetString(), "");

	ASSERT_TRUE(doc["map_val_1"].IsObject());
	ASSERT_EQ(doc["map_val_1"].MemberCount(), 0u);

	ASSERT_TRUE(doc["vec_val_2"].IsArray());
	ASSERT_TRUE(doc["vec_val_2"].Empty());
}

TEST(Serialize, AllocatorLifeCycle)
{
	// TODO: Use inner and out allocator
	// TODO: reinit after serialize
}

TEST(Serialize, NullValue)
{
	Inner innerSt;
	IJST_SET(innerSt, int_1, 1);
	IJST_SET(innerSt, int_2, 2);
	IJST_MARK_NULL(innerSt, int_2);

	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(innerSt, doc, FPush::kPushAllFields | FPush::kPushUnknown);

	ASSERT_EQ(IJST_GET_STATUS(innerSt, int_2), FStatus::kNull);
	ASSERT_EQ(doc["int_val_1"].GetInt(), 1);
	ASSERT_TRUE(doc["int_val_2"].IsNull());
}

TEST(Serialize, MarkMissing)
{
	Inner innerSt;
	IJST_SET(innerSt, int_1, 1);
	IJST_SET(innerSt, int_2, 2);
	IJST_MARK_MISSING(innerSt, int_2);

	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(innerSt, doc, 0);

	ASSERT_EQ(doc["int_val_1"].GetInt(), 1);
	ASSERT_FALSE(doc.HasMember("int_val_2"));
}

IJST_DEFINE_STRUCT(
		ObjRefSt
		, (IJST_TOBJ(Inner), inner, "inner_val", 0)
		, (IJST_TVEC(IJST_TOBJ(Inner)), inner_v, "inner_v_val", 0)
		, (IJST_TMAP(IJST_TOBJ(Inner)), inner_m, "inner_m_val", 0)
		, (IJST_TMAP(IJST_TVEC(IJST_TOBJ(Inner))), inner_mv, "inner_mv_val", 0)
)

TEST(Serialize, AdditionalJsonField)
{
	ObjRefSt st;

	st._.GetUnknown().AddMember("addi_o1", rapidjson::Value().SetString("str_o1").Move(), st._.GetAllocator());
	st.inner._.GetUnknown().AddMember("addi_i1", rapidjson::Value().SetString("str_i1").Move(), st.inner._.GetAllocator());
	IJST_MARK_VALID(st, inner);
	IJST_SET(st.inner, int_2, 11);

	// Serialize without additional field
	{
		rapidjson::Document doc;
		string json;
		int ret = st._.Serialize(json, 0);
		ASSERT_EQ(ret, 0);
		doc.Parse(json.c_str(), json.length());
		ASSERT_FALSE(doc.HasParseError());

#if IJST_ENABLE_TO_JSON_OBJECT
		rapidjson::Value jVal;
		JsonAllocator allocator;
		ret = st._.ToJson(jVal, allocator, FPush::kZero);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ((rapidjson::Value&)doc, jVal);
#endif

		ASSERT_EQ(doc.MemberCount(), 1u);
		ASSERT_TRUE(doc.HasMember("inner_val"));
		ASSERT_FALSE(doc.HasMember("addi_o1"));
		ASSERT_FALSE(doc["inner_val"].GetObject().HasMember("inner_val"));
	}

	// Serialize with additional field
	{
		rapidjson::Document doc;
		string json;
		int ret = st._.Serialize(json);
		ASSERT_EQ(ret, 0);
		doc.Parse(json.c_str(), json.length());
		ASSERT_FALSE(doc.HasParseError());

#if IJST_ENABLE_TO_JSON_OBJECT
		rapidjson::Value jVal;
		JsonAllocator allocator;
		ret = st._.ToJson(jVal, allocator);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ((rapidjson::Value&)doc, jVal);
#endif

		// Check output
		ASSERT_EQ(ret, 0);
		ASSERT_STREQ(doc["addi_o1"].GetString(), "str_o1");
		ASSERT_STREQ(doc["inner_val"]["addi_i1"].GetString(), "str_i1");
		ASSERT_EQ(doc["inner_val"]["int_val_2"].GetInt(), 11);

		// Check src
		ASSERT_STREQ(st._.GetUnknown()["addi_o1"].GetString(), "str_o1");
		ASSERT_STREQ(st.inner._.GetUnknown()["addi_i1"].GetString(), "str_i1");
	}
}

#if IJST_ENABLE_TO_JSON_OBJECT
TEST(Serialize, AdditionalJsonFieldMoved)
{
	ObjRefSt st;

	st._.GetUnknown().AddMember("addi_o1", rapidjson::Value().SetString("str_o1").Move(), st._.GetAllocator());
	st.inner._.GetUnknown().AddMember("addi_i1", rapidjson::Value().SetString("str_i1").Move(), st.inner._.GetAllocator());
	IJST_MARK_VALID(st, inner);
	IJST_SET(st.inner, int_2, 11);

	// Move Serialize without additional field
	{
		ObjRefSt st2 = st;
		// Serialize
		rapidjson::Document doc;
		UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kZero);

		// Check output
		ASSERT_EQ(doc.MemberCount(), 1u);
		ASSERT_TRUE(doc.HasMember("inner_val"));
		ASSERT_FALSE(doc.HasMember("addi_o1"));
		ASSERT_FALSE(doc["inner_val"].GetObject().HasMember("inner_val"));
	}
	// Serialize
	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kPushAllFields | FPush::kPushUnknown);

	// Check src
	ASSERT_EQ(st._.GetUnknown().MemberCount(), 0u);
	ASSERT_EQ(st.inner._.GetUnknown().MemberCount(), 0u);

	// Check output
	ASSERT_STREQ(doc["addi_o1"].GetString(), "str_o1");
	ASSERT_STREQ(doc["inner_val"]["addi_i1"].GetString(), "str_i1");
	ASSERT_EQ(doc["inner_val"]["int_val_2"].GetInt(), 11);
}
#endif

IJST_DEFINE_STRUCT(
		Complicate1
		, (IJST_TOBJ(Inner), i1, "i1_v", 0)
		, (IJST_TOBJ(Inner), i2, "i2_v", 0)
		, (IJST_TOBJ(Inner), i3, "i3_v", 0)
		, (IJST_TOBJ(Inner), i4, "i4_v", 0)
)

IJST_DEFINE_STRUCT(
		Complicate2
		, (IJST_TOBJ(Complicate1), c1, "c1_v", 0)
		, (IJST_TVEC(IJST_TMAP(IJST_TPRI(Str))), vms, "vms_v", 0)
		, (IJST_TMAP(IJST_TVEC(IJST_TPRI(Str))), mvs, "mvs_v", 0)
		, (IJST_TMAP(IJST_TMAP(IJST_TOBJ(Inner))), mmo, "mmo_v", 0)
)

TEST(Serialize, Complicate)
{
	Complicate2 st;
	// Init
	// c1
	st.c1.i1.int_1 = 1;
	st.c1.i2.int_2 = 2;
	st.c1.i3.str_1 = "str1";
	st.c1.i4.str_2 = "str2";
	// vms
	map<string, FStoreString> ms1;
	ms1["k1"] = "v1";
	ms1["k2"] = "v2";
	map<string, FStoreString> ms2;
	ms2["k1"] = "v3";
	ms2["k3"] = "v4";
	IJST_CONT_VAL(st.vms).push_back(ms1);
	IJST_CONT_VAL(st.vms).push_back(ms2);
	// mvs
	vector<FStoreString> vs1;
	vs1.push_back(string("s1"));
	vs1.push_back(string("s2"));
	st.mvs["mk1"] = vs1;
	IJST_CONT_VAL(st.mvs["mk2"]).push_back(string("s3"));
	IJST_CONT_VAL(st.mvs["mk2"]).push_back(string("s4"));
	// mmo
	Inner in1;
	in1.int_1 = 11;
	map<string, Inner> mi1;
	mi1["im1"] = in1;
	mi1["im2"].int_2 = 12;
	st.mmo["om1"] = mi1;
	st.mmo["om2"]["im1"].str_1 = "lstr1";
	st.mmo["om2"]["im3"].str_2 = "lstr2";

	// Value Check
	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kPushAllFields | FPush::kPushUnknown);
	// c1
	ASSERT_EQ(doc["c1_v"]["i1_v"]["int_val_1"].GetInt(), 1);
	ASSERT_EQ(doc["c1_v"]["i2_v"]["int_val_2"].GetInt(), 2);
	ASSERT_STREQ(doc["c1_v"]["i3_v"]["str_val_1"].GetString(), "str1");
	ASSERT_STREQ(doc["c1_v"]["i4_v"]["str_val_2"].GetString(), "str2");
	//vms
	ASSERT_STREQ(doc["vms_v"][0]["k1"].GetString(), "v1");
	ASSERT_STREQ(doc["vms_v"][0]["k2"].GetString(), "v2");
	ASSERT_STREQ(doc["vms_v"][1]["k1"].GetString(), "v3");
	ASSERT_STREQ(doc["vms_v"][1]["k3"].GetString(), "v4");
	//mvs
	ASSERT_STREQ(doc["mvs_v"]["mk1"][0].GetString(), "s1");
	ASSERT_STREQ(doc["mvs_v"]["mk1"][1].GetString(), "s2");
	ASSERT_STREQ(doc["mvs_v"]["mk2"][0].GetString(), "s3");
	ASSERT_STREQ(doc["mvs_v"]["mk2"][1].GetString(), "s4");
	// mmo
	ASSERT_EQ(doc["mmo_v"]["om1"]["im1"]["int_val_1"].GetInt(), 11);
	ASSERT_EQ(doc["mmo_v"]["om1"]["im2"]["int_val_2"].GetInt(), 12);
	ASSERT_STREQ(doc["mmo_v"]["om2"]["im1"]["str_val_1"].GetString(), "lstr1");
	ASSERT_STREQ(doc["mmo_v"]["om2"]["im3"]["str_val_2"].GetString(), "lstr2");
}

IJST_DEFINE_STRUCT(
	Container
	, (IJST_TVEC(IJST_TPRI(Int)), dVector, "vec", 0)
	, (IJST_TDEQUE(IJST_TPRI(Int)), dDeque, "deque", 0)
	, (IJST_TLIST(IJST_TPRI(Int)), dList, "list", 0)
)

TEST(Serialize, Container)
{
	Container st;
	IJST_CONT_VAL(st.dVector).push_back(1);
	IJST_CONT_VAL(st.dDeque).push_back(1);
	IJST_CONT_VAL(st.dDeque).push_front(0);
	IJST_CONT_VAL(st.dList).push_back(-1);
	IJST_CONT_VAL(st.dList).push_front(-2);

	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kPushAllFields | FPush::kPushUnknown);

	ASSERT_EQ(doc["vec"].Size(), 1u);
	ASSERT_EQ(doc["vec"][0].GetInt(), 1);
	ASSERT_EQ(doc["deque"].Size(), 2u);
	ASSERT_EQ(doc["deque"][0].GetInt(), 0);
	ASSERT_EQ(doc["deque"][1].GetInt(), 1);
	ASSERT_EQ(doc["list"].Size(), 2u);
	ASSERT_EQ(doc["list"][0].GetInt(), -2);
	ASSERT_EQ(doc["list"][1].GetInt(), -1);
}

IJST_DEFINE_STRUCT(
	EmptySt
);

TEST(Serialize, EmptyStruct)
{
	EmptySt st;
	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kPushAllFields | FPush::kPushUnknown);
	ASSERT_EQ(doc.MemberCount(), 0u);
}

IJST_DEFINE_STRUCT(
		Complicate3
		, (IJST_TPRI(Int), i1, "i1_v", 0)
		, (IJST_TPRI(Int), i2, "i2_v", 0)
		, (IJST_TPRI(Int), i3, "i3_v", 0)
		, (IJST_TPRI(Int), i4, "i4_v", 0)
		, (IJST_TPRI(Int), i5, "i5_v", 0)
		, (IJST_TPRI(Int), i6, "i6_v", 0)
		, (IJST_TPRI(Int), i7, "i7_v", 0)
		, (IJST_TPRI(Int), i8, "i8_v", 0)
		, (IJST_TPRI(Int), i9, "i9_v", 0)
		, (IJST_TPRI(Int), i10, "i10_v", 0)
		, (IJST_TPRI(Int), i11, "i11_v", 0)
		, (IJST_TPRI(Int), i12, "i12_v", 0)
		, (IJST_TPRI(Int), i13, "i13_v", 0)
		, (IJST_TPRI(Int), i14, "i14_v", 0)
		, (IJST_TPRI(Int), i15, "i15_v", 0)
		, (IJST_TPRI(Int), i16, "i16_v", 0)
		, (IJST_TPRI(Int), i17, "i17_v", 0)
		, (IJST_TPRI(Int), i18, "i18_v", 0)
		, (IJST_TPRI(Int), i19, "i19_v", 0)
		, (IJST_TPRI(Int), i20, "i20_v", 0)
		, (IJST_TPRI(Int), i21, "i21_v", 0)
		, (IJST_TPRI(Int), i22, "i22_v", 0)
		, (IJST_TPRI(Int), i23, "i23_v", 0)
		, (IJST_TPRI(Int), i24, "i24_v", 0)
		, (IJST_TPRI(Int), i25, "i25_v", 0)
		, (IJST_TPRI(Int), i26, "i26_v", 0)
		, (IJST_TPRI(Int), i27, "i27_v", 0)
		, (IJST_TPRI(Int), i28, "i28_v", 0)
		, (IJST_TPRI(Int), i29, "i29_v", 0)
		, (IJST_TPRI(Int), i30, "i30_v", 0)
		, (IJST_TPRI(Int), i31, "i31_v", 0)
		, (IJST_TPRI(Int), i32, "i32_v", 0)
		, (IJST_TPRI(Int), i33, "i33_v", 0)
		, (IJST_TPRI(Int), i34, "i34_v", 0)
		, (IJST_TPRI(Int), i35, "i35_v", 0)
		, (IJST_TPRI(Int), i36, "i36_v", 0)
		, (IJST_TPRI(Int), i37, "i37_v", 0)
		, (IJST_TPRI(Int), i38, "i38_v", 0)
		, (IJST_TPRI(Int), i39, "i39_v", 0)
		, (IJST_TPRI(Int), i40, "i40_v", 0)
		, (IJST_TPRI(Int), i41, "i41_v", 0)
		, (IJST_TPRI(Int), i42, "i42_v", 0)
		, (IJST_TPRI(Int), i43, "i43_v", 0)
		, (IJST_TPRI(Int), i44, "i44_v", 0)
		, (IJST_TPRI(Int), i45, "i45_v", 0)
		, (IJST_TPRI(Int), i46, "i46_v", 0)
		, (IJST_TPRI(Int), i47, "i47_v", 0)
		, (IJST_TPRI(Int), i48, "i48_v", 0)
		, (IJST_TPRI(Int), i49, "i49_v", 0)
		, (IJST_TPRI(Int), i50, "i50_v", 0)
		, (IJST_TPRI(Int), i51, "i51_v", 0)
		, (IJST_TPRI(Int), i52, "i52_v", 0)
		, (IJST_TPRI(Int), i53, "i53_v", 0)
		, (IJST_TPRI(Int), i54, "i54_v", 0)
		, (IJST_TPRI(Int), i55, "i55_v", 0)
		, (IJST_TPRI(Int), i56, "i56_v", 0)
		, (IJST_TPRI(Int), i57, "i57_v", 0)
		, (IJST_TPRI(Int), i58, "i58_v", 0)
		, (IJST_TPRI(Int), i59, "i59_v", 0)
		, (IJST_TPRI(Int), i60, "i60_v", 0)
		, (IJST_TPRI(Int), i61, "i61_v", 0)
		, (IJST_TPRI(Int), i62, "i62_v", 0)
		, (IJST_TPRI(Int), i63, "i63_v", 0)
		, (IJST_TPRI(Int), i64, "i64_v", 0)
)

TEST(Serialize, BigStruct)
{
	Complicate3 st;
	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kPushAllFields | FPush::kPushUnknown);
	ASSERT_EQ(doc["i1_v"].GetInt(), 0);
	ASSERT_EQ(doc["i64_v"].GetInt(), 0);
}

