//
// Created by h46incon on 2017/9/29.
//

#include "util.h"

#include <rapidjson/prettywriter.h>

using std::vector;
using std::map;
using std::string;
using namespace ijst;

IJST_DEFINE_STRUCT(
		Inner
		, (T_int, int_1, "int_val_1", 0)
		, (T_int, int_2, "int_val_2", 0)
		, (T_string, str_1, "str_val_1", 0)
		, (T_string, str_2, "str_val_2", 0)
)

IJST_DEFINE_STRUCT(
		NestSt
		, (IJST_TVEC(T_int), vec_1, "vec_val_1", 0)
		, (IJST_TOBJ(Inner), inner_1, "inner_val_1", 0)
		, (IJST_TMAP(T_string), map_1, "map_val_1", 0)
		, (IJST_TVEC(IJST_TOBJ(Inner)), vec_2, "vec_val_2", 0)
)

TEST(Serialize, EmptyValue)
{
	NestSt nestSt;

	// Empty struct
	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(nestSt, doc, FPush::kOnlyValidField);
	ASSERT_TRUE(doc.IsObject());
	ASSERT_TRUE(doc.MemberCount() == 0);
}

TEST(Serialize, EmptyValue_PushAllField)
{
	NestSt nestSt;
	// Empty struct

	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(nestSt, doc, FPush::kNoneFlag);

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
	UTEST_MOVE_TO_STRING_AND_CHECK(innerSt, doc, FPush::kNoneFlag);

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
	UTEST_MOVE_TO_STRING_AND_CHECK(innerSt, doc, FPush::kOnlyValidField);

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
		int ret = st._.Serialize(json, FPush::kIgnoreUnknown | FPush::kOnlyValidField);
		ASSERT_EQ(ret, 0);
		doc.Parse(json.c_str(), json.length());
		ASSERT_FALSE(doc.HasParseError());

#if IJST_ENABLE_TO_JSON_OBJECT
		rapidjson::Value jVal;
		JsonAllocator allocator;
		ret = st._.ToJson(jVal, allocator, FPush::kIgnoreUnknown | FPush::kOnlyValidField);
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
		UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kIgnoreUnknown | FPush::kOnlyValidField);

		// Check output
		ASSERT_EQ(doc.MemberCount(), 1u);
		ASSERT_TRUE(doc.HasMember("inner_val"));
		ASSERT_FALSE(doc.HasMember("addi_o1"));
		ASSERT_FALSE(doc["inner_val"].GetObject().HasMember("inner_val"));
	}
	// Serialize
	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kNoneFlag);

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
		, (IJST_TVEC(IJST_TMAP(T_string)), vms, "vms_v", 0)
		, (IJST_TMAP(IJST_TVEC(T_string)), mvs, "mvs_v", 0)
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
	map<string, T_string> ms1;
	ms1["k1"] = "v1";
	ms1["k2"] = "v2";
	map<string, T_string> ms2;
	ms2["k1"] = "v3";
	ms2["k3"] = "v4";
	st.vms.push_back(ms1);
	st.vms.push_back(ms2);
	// mvs
	vector<T_string> vs1;
	vs1.push_back(string("s1"));
	vs1.push_back(string("s2"));
	st.mvs["mk1"] = vs1;
	st.mvs["mk2"].push_back(string("s3"));
	st.mvs["mk2"].push_back(string("s4"));
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
	UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kNoneFlag);
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
	, (IJST_TVEC(T_int), dVector, "vec", 0)
	, (IJST_TDEQUE(T_int), dDeque, "deque", 0)
	, (IJST_TLIST(T_int), dList, "list", 0)
)

TEST(Serialize, Container)
{
	Container st;
	st.dVector.push_back(1);
	st.dDeque.push_back(1);
	st.dDeque.push_front(0);
	st.dList.push_back(-1);
	st.dList.push_front(-2);

	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kNoneFlag);

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
	UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kNoneFlag);
	ASSERT_EQ(doc.MemberCount(), 0u);
}

IJST_DEFINE_STRUCT(
		Complicate3
		, (T_int, i1, "i1_v", 0)
		, (T_int, i2, "i2_v", 0)
		, (T_int, i3, "i3_v", 0)
		, (T_int, i4, "i4_v", 0)
		, (T_int, i5, "i5_v", 0)
		, (T_int, i6, "i6_v", 0)
		, (T_int, i7, "i7_v", 0)
		, (T_int, i8, "i8_v", 0)
		, (T_int, i9, "i9_v", 0)
		, (T_int, i10, "i10_v", 0)
		, (T_int, i11, "i11_v", 0)
		, (T_int, i12, "i12_v", 0)
		, (T_int, i13, "i13_v", 0)
		, (T_int, i14, "i14_v", 0)
		, (T_int, i15, "i15_v", 0)
		, (T_int, i16, "i16_v", 0)
		, (T_int, i17, "i17_v", 0)
		, (T_int, i18, "i18_v", 0)
		, (T_int, i19, "i19_v", 0)
		, (T_int, i20, "i20_v", 0)
		, (T_int, i21, "i21_v", 0)
		, (T_int, i22, "i22_v", 0)
		, (T_int, i23, "i23_v", 0)
		, (T_int, i24, "i24_v", 0)
		, (T_int, i25, "i25_v", 0)
		, (T_int, i26, "i26_v", 0)
		, (T_int, i27, "i27_v", 0)
		, (T_int, i28, "i28_v", 0)
		, (T_int, i29, "i29_v", 0)
		, (T_int, i30, "i30_v", 0)
		, (T_int, i31, "i31_v", 0)
		, (T_int, i32, "i32_v", 0)
		, (T_int, i33, "i33_v", 0)
		, (T_int, i34, "i34_v", 0)
		, (T_int, i35, "i35_v", 0)
		, (T_int, i36, "i36_v", 0)
		, (T_int, i37, "i37_v", 0)
		, (T_int, i38, "i38_v", 0)
		, (T_int, i39, "i39_v", 0)
		, (T_int, i40, "i40_v", 0)
		, (T_int, i41, "i41_v", 0)
		, (T_int, i42, "i42_v", 0)
		, (T_int, i43, "i43_v", 0)
		, (T_int, i44, "i44_v", 0)
		, (T_int, i45, "i45_v", 0)
		, (T_int, i46, "i46_v", 0)
		, (T_int, i47, "i47_v", 0)
		, (T_int, i48, "i48_v", 0)
		, (T_int, i49, "i49_v", 0)
		, (T_int, i50, "i50_v", 0)
		, (T_int, i51, "i51_v", 0)
		, (T_int, i52, "i52_v", 0)
		, (T_int, i53, "i53_v", 0)
		, (T_int, i54, "i54_v", 0)
		, (T_int, i55, "i55_v", 0)
		, (T_int, i56, "i56_v", 0)
		, (T_int, i57, "i57_v", 0)
		, (T_int, i58, "i58_v", 0)
		, (T_int, i59, "i59_v", 0)
		, (T_int, i60, "i60_v", 0)
		, (T_int, i61, "i61_v", 0)
		, (T_int, i62, "i62_v", 0)
		, (T_int, i63, "i63_v", 0)
		, (T_int, i64, "i64_v", 0)
)

TEST(Serialize, BigStruct)
{
	Complicate3 st;
	rapidjson::Document doc;
	UTEST_MOVE_TO_STRING_AND_CHECK(st, doc, FPush::kNoneFlag);
	ASSERT_EQ(doc["i1_v"].GetInt(), 0);
	ASSERT_EQ(doc["i64_v"].GetInt(), 0);
}

TEST(Serialize, SerializeHandler)
{
	Complicate3 st;
	rapidjson::StringBuffer buf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
	HandlerWrapper<rapidjson::PrettyWriter<rapidjson::StringBuffer> > writerWrapper(writer);
	st._.Serialize(writerWrapper);
	rapidjson::Document doc;
	doc.Parse(buf.GetString(), buf.GetLength());
	ASSERT_FALSE(doc.HasParseError());
	ASSERT_EQ(doc["i1_v"].GetInt(), 0);
	ASSERT_EQ(doc["i64_v"].GetInt(), 0);
}

