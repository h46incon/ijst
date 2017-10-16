//
// Created by h46incon on 2017/9/29.
//

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "gtest/gtest.h"
#include "ijst/ijst.h"
using std::vector;
using std::map;
using std::string;

IJST_DEFINE_STRUCT(
		Inner,
		(IJST_TPRI(Int), int_1, "int_val_1", 0),
		(IJST_TPRI(Int), int_2, "int_val_2", 0),
		(IJST_TPRI(String), str_1, "str_val_1", 0),
		(IJST_TPRI(String), str_2, "str_val_2", 0)
)

IJST_DEFINE_STRUCT(
		NestSt,
		(IJST_TVEC(IJST_TPRI(Int)), vec_1, "vec_val_1", 0),
		(IJST_TOBJ(Inner), inner_1, "inner_val_1", 0),
		(IJST_TMAP(IJST_TPRI(String)), map_1, "map_val_1", 0),
		(IJST_TVEC(IJST_TOBJ(Inner)), vec_2, "vec_val_2", 0)
)

TEST(Serialize, EmptyStruct)
{
	NestSt nestSt;

	// Empty struct
	int ret = nestSt._.SerializeInplace(false);
	ASSERT_EQ(ret, 0);
	ijst::StoreType &jVal = *nestSt._.FindUsedBuffer().pBuffer;
	ASSERT_TRUE(jVal.IsObject());
	ASSERT_TRUE(jVal.MemberCount() == 0);
}

TEST(Serialize, EmptyStruct_PushAllField)
{
	NestSt nestSt;
	// Empty struct

	int ret = nestSt._.SerializeInplace(true);
	ASSERT_EQ(ret, 0);
	ijst::StoreType &jVal = *nestSt._.FindUsedBuffer().pBuffer;

	// Check
	ASSERT_TRUE(jVal["vec_val_1"].IsArray());
	ASSERT_TRUE(jVal["vec_val_1"].Empty());

	ASSERT_TRUE(jVal["inner_val_1"].IsObject());
	ASSERT_EQ(jVal["inner_val_1"]["int_val_1"].GetInt(), 0);
	ASSERT_EQ(jVal["inner_val_1"]["int_val_2"].GetInt(), 0);
	ASSERT_STREQ(jVal["inner_val_1"]["str_val_1"].GetString(), "");
	ASSERT_STREQ(jVal["inner_val_1"]["str_val_2"].GetString(), "");

	ASSERT_TRUE(jVal["map_val_1"].IsObject());
	ASSERT_TRUE(jVal["map_val_1"].MemberCount() == 0);

	ASSERT_TRUE(jVal["vec_val_2"].IsArray());
	ASSERT_TRUE(jVal["vec_val_2"].Empty());
}

TEST(Serialize, UseOuterBuffer)
{
	NestSt nestSt;
	rapidjson::Document doc;
	int ret = nestSt._.Serialize(true, doc);
	ASSERT_EQ(ret, 0);
	ASSERT_GT(doc.MemberCount(), 0);

	ASSERT_EQ(nestSt._.FindUsedBuffer().pBuffer->MemberCount(), 0);
	ASSERT_EQ(nestSt.inner_1._.FindUsedBuffer().pBuffer->MemberCount(), 0);
}

TEST(Serialize, NullValue)
{
	Inner innerSt;
	IJST_SET(innerSt, int_1, 1);
	IJST_SET(innerSt, int_2, 2);
	IJST_MARK_NULL(innerSt, int_2);

	int ret = innerSt._.SerializeInplace(true);
	ASSERT_EQ(ret, 0);
	ijst::StoreType &jVal = *innerSt._.FindUsedBuffer().pBuffer;

	ASSERT_EQ(IJST_GET_STATUS(innerSt, int_2), ijst::FStatus::kNull);
	ASSERT_EQ(jVal["int_val_1"].GetInt(), 1);
	ASSERT_TRUE(jVal["int_val_2"].IsNull());
}

TEST(Serialize, RemovedField)
{
	NestSt st;
	// Add field
	ijst::BufferInfo bufferInfo = st._.FindUsedBuffer();
	bufferInfo.pBuffer->AddMember("vec_val_2", rapidjson::Value().SetInt(1).Move(), *bufferInfo.pAllocator);
	st.vec_1.push_back(1);
	st.map_1["k"] = "v";
	IJST_MARK_VALID(st, vec_1);
	IJST_MARK_VALID(st, map_1);

	IJST_MARK_REMOVED(st, vec_1);
	IJST_MARK_REMOVED(st, vec_2);
	IJST_MARK_REMOVED(st, map_1);

	int ret = st._.SerializeInplace(true);
	ASSERT_EQ(ret, 0);
	// Check field
	ASSERT_TRUE(st.vec_1.empty());
	ASSERT_TRUE(st.map_1.empty());
	// Check stream
	rapidjson::Value &jVal = *st._.FindUsedBuffer().pBuffer;
	ASSERT_FALSE(jVal.HasMember("vec_val_1"));
	ASSERT_FALSE(jVal.HasMember("vec_val_2"));
	ASSERT_FALSE(jVal.HasMember("map_val_1"));
	ASSERT_TRUE(jVal.HasMember("inner_val_1"));

	// TODO: Failed now
//	ASSERT_EQ(&jVal["inner_val_1"], &st.inner_1._.InnerBuffer());
//
//	// Serialize again to check the behaviour of inner
//	IJST_SET(st.inner_1, int_1, 2);
//	IJST_MARK_REMOVED(st, inner_1);
//
//	ret = st._.SerializeInplace(true);
//	ASSERT_EQ(ret, 0);
//	rapidjson::Value& jVal2 = st._.InnerBuffer();
//	ASSERT_FALSE(jVal2.HasMember("inner_val_1"));
//	ASSERT_EQ(IJST_GET_STATUS(st.inner_1, int_1), ijst::FStatus::kMissing);

}

IJST_DEFINE_STRUCT(
		ObjRefSt,
		(IJST_TOBJ(Inner), inner, "inner_val", 0),
		(IJST_TVEC(IJST_TOBJ(Inner)), inner_v, "inner_v_val", 0),
		(IJST_TMAP(IJST_TOBJ(Inner)), inner_m, "inner_m_val", 0),
		(IJST_TMAP(IJST_TVEC(IJST_TOBJ(Inner))), inner_mv, "inner_mv_val", 0)
)

TEST(Serialize, ObjRef)
{
	ObjRefSt st;
	ijst::StoreType &innerStream = *st._.FindUsedBuffer().pBuffer;
	ijst::AllocatorType &innerAllocate = *st._.FindUsedBuffer().pAllocator;

	// inner
	innerStream.AddMember("inner_val", rapidjson::Value().SetNull().Move(), innerAllocate);
	// inner_v
	innerStream.AddMember("inner_v_val", rapidjson::Value().SetArray().Move(), innerAllocate);
	innerStream["inner_v_val"].PushBack(rapidjson::Value().SetNull().Move(), innerAllocate);
	st.inner_v.push_back(Inner());
	// inner_m
	innerStream.AddMember("inner_m_val", rapidjson::Value().SetObject().Move(), innerAllocate);
	innerStream["inner_m_val"].AddMember("addi_k", rapidjson::Value().SetInt(0x5A5A).Move(), innerAllocate);
	innerStream["inner_m_val"].AddMember("k", rapidjson::Value().SetNull().Move(), innerAllocate);
	st.inner_m["k"] = Inner();
	st.inner_m["k2"] = Inner();
	// inner_mv
	st.inner_mv["k"] = vector<Inner>(1, Inner());

	// Serialize
	int ret = st._.SerializeInplace(true);
	ASSERT_EQ(ret, 0);
	ijst::StoreType &jVal = *st._.FindUsedBuffer().pBuffer;

	// Check value
	// inner
	ASSERT_EQ(&jVal["inner_val"], st.inner._.FindUsedBuffer().pBuffer);
	// inner_v
	ASSERT_EQ(jVal["inner_v_val"].Size(), 1);		// array type will reinit when serialize
	ASSERT_EQ(&jVal["inner_v_val"][0], st.inner_v.front()._.InnerBuffer());
	// inner_m
	ASSERT_EQ(jVal["inner_m_val"].MemberCount(), 3);	// object type will keep old element when serialize
	ASSERT_EQ(jVal["inner_m_val"]["addi_k"].GetInt(), 0x5A5A);			// old element
	ASSERT_TRUE(jVal["inner_m_val"]["k"].IsObject());					// old element will be overwrite by field
	ASSERT_EQ(&jVal["inner_m_val"]["k"], &st.inner_m["k"]._.InnerBuffer());
	ASSERT_EQ(&jVal["inner_m_val"]["k2"], &st.inner_m["k2"]._.InnerBuffer());
	// inner_mv
	ASSERT_EQ(&jVal["inner_mv_val"]["k"][0], &st.inner_mv["k"][0]._.InnerBuffer());
}

TEST(Serialize, AdditionalJsonField)
{
	ObjRefSt st;

	st._.InnerBuffer().AddMember("addi_o1", rapidjson::Value().SetString("str_o1").Move(), st._.InnerAllocator());
	st.inner._.InnerBuffer().AddMember("addi_i1", rapidjson::Value().SetString("str_i1").Move(), st.inner._.InnerAllocator());
	IJST_SET(st.inner, int_2, 11);

	int ret;
	// Serialize in outer buffer
	rapidjson::Document doc;
	ret = st._.Serialize(true, doc);
	ASSERT_EQ(ret, 0);
	ijst::StoreType &jVal = doc;
	ASSERT_EQ(jVal["inner_val"]["int_val_2"].GetInt(), 11);
	ASSERT_FALSE(jVal.HasMember("addi_o1"));
	ASSERT_FALSE(jVal["inner_val"].HasMember("addi_o1"));

	// Serialize in place
	ret = st._.SerializeInplace(true);
	ASSERT_EQ(ret, 0);
	ijst::StoreType &jVal2 = st._.InnerBuffer();
	ASSERT_STREQ(jVal2["addi_o1"].GetString(), "str_o1");
	ASSERT_STREQ(jVal2["inner_val"]["addi_i1"].GetString(), "str_i1");
	ASSERT_EQ(jVal2["inner_val"]["int_val_2"].GetInt(), 11);

	// Serialize again to check the behavior of holding outer stream in inner object
	jVal2.AddMember("addi_o2", rapidjson::Value().SetString("str_o2").Move(), st._.InnerAllocator());
	jVal2["inner_val"].AddMember("addi_i2", rapidjson::Value().SetString("str_i2").Move(), st._.InnerAllocator());
	ret = st._.SerializeInplace(true);
	ASSERT_EQ(ret, 0);
	ijst::StoreType &jVal3 = st._.InnerBuffer();
	ASSERT_STREQ(jVal2["addi_o1"].GetString(), "str_o1");
	ASSERT_STREQ(jVal2["inner_val"]["addi_i1"].GetString(), "str_i1");
	ASSERT_EQ(jVal2["inner_val"]["int_val_2"].GetInt(), 11);
	// Check new value
	ASSERT_STREQ(jVal2["addi_o2"].GetString(), "str_o2");
	ASSERT_STREQ(jVal2["inner_val"]["addi_i2"].GetString(), "str_i2");
}

IJST_DEFINE_STRUCT(
		Complicate1,
		(IJST_TOBJ(Inner), i1, "i1_v", 0),
		(IJST_TOBJ(Inner), i2, "i2_v", 0),
		(IJST_TOBJ(Inner), i3, "i3_v", 0),
		(IJST_TOBJ(Inner), i4, "i4_v", 0)
)

IJST_DEFINE_STRUCT(
		Complicate2,
		(IJST_TOBJ(Complicate1), c1, "c1_v", 0),
		(IJST_TVEC(IJST_TMAP(IJST_TPRI(String))), vms, "vms_v", 0),
		(IJST_TMAP(IJST_TVEC(IJST_TPRI(String))), mvs, "mvs_v", 0),
		(IJST_TMAP(IJST_TMAP(IJST_TOBJ(Inner))), mmo, "mmo_v", 0)
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
	map<string, string> ms1;
	ms1["k1"] = "v1";
	ms1["k2"] = "v2";
	map<string, string> ms2;
	ms2["k1"] = "v3";
	ms2["k3"] = "v4";
	st.vms.push_back(ms1);
	st.vms.push_back(ms2);
	// mvs
	vector<string> vs1;
	vs1.push_back("s1");
	vs1.push_back("s2");
	st.mvs["mk1"] = vs1;
	st.mvs["mk2"].push_back("s3");
	st.mvs["mk2"].push_back("s4");
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
	int ret = st._.SerializeInplace(true);
	ASSERT_EQ(ret, 0);
	rapidjson::Value &jVal = st._.InnerBuffer();
//	string ser;
//	st._.WriteUsedBuffer(ser);
//	std::cout << ser << std::endl;
	// c1
	ASSERT_EQ(jVal["c1_v"]["i1_v"]["int_val_1"].GetInt(), 1);
	ASSERT_EQ(jVal["c1_v"]["i2_v"]["int_val_2"].GetInt(), 2);
	ASSERT_STREQ(jVal["c1_v"]["i3_v"]["str_val_1"].GetString(), "str1");
	ASSERT_STREQ(jVal["c1_v"]["i4_v"]["str_val_2"].GetString(), "str2");
	//vms
	ASSERT_STREQ(jVal["vms_v"][0]["k1"].GetString(), "v1");
	ASSERT_STREQ(jVal["vms_v"][0]["k2"].GetString(), "v2");
	ASSERT_STREQ(jVal["vms_v"][1]["k1"].GetString(), "v3");
	ASSERT_STREQ(jVal["vms_v"][1]["k3"].GetString(), "v4");
	//mvs
	ASSERT_STREQ(jVal["mvs_v"]["mk1"][0].GetString(), "s1");
	ASSERT_STREQ(jVal["mvs_v"]["mk1"][1].GetString(), "s2");
	ASSERT_STREQ(jVal["mvs_v"]["mk2"][0].GetString(), "s3");
	ASSERT_STREQ(jVal["mvs_v"]["mk2"][1].GetString(), "s4");
	// mmo
	ASSERT_EQ(jVal["mmo_v"]["om1"]["im1"]["int_val_1"].GetInt(), 11);
	ASSERT_EQ(jVal["mmo_v"]["om1"]["im2"]["int_val_2"].GetInt(), 12);
	ASSERT_STREQ(jVal["mmo_v"]["om2"]["im1"]["str_val_1"].GetString(), "lstr1");
	ASSERT_STREQ(jVal["mmo_v"]["om2"]["im3"]["str_val_2"].GetString(), "lstr2");

	// Reference Check
	// c1
	ASSERT_EQ(&jVal["c1_v"], &st.c1._.InnerBuffer());
	ASSERT_EQ(&jVal["c1_v"]["i1_v"], &st.c1.i1._.InnerBuffer());
	ASSERT_EQ(&jVal["c1_v"]["i2_v"], &st.c1.i2._.InnerBuffer());
	ASSERT_EQ(&jVal["c1_v"]["i3_v"], &st.c1.i3._.InnerBuffer());
	ASSERT_EQ(&jVal["c1_v"]["i4_v"], &st.c1.i4._.InnerBuffer());
	// mmo
	ASSERT_EQ(&jVal["mmo_v"]["om1"]["im1"], &st.mmo["om1"]["im1"]._.InnerBuffer());
	ASSERT_EQ(&jVal["mmo_v"]["om1"]["im2"], &st.mmo["om1"]["im2"]._.InnerBuffer());
	ASSERT_EQ(&jVal["mmo_v"]["om2"]["im1"], &st.mmo["om2"]["im1"]._.InnerBuffer());
	ASSERT_EQ(&jVal["mmo_v"]["om2"]["im3"], &st.mmo["om2"]["im3"]._.InnerBuffer());
}

IJST_DEFINE_STRUCT(
		Complicate3,
		(IJST_TPRI(Int), i1, "i1_v", 0),
		(IJST_TPRI(Int), i2, "i2_v", 0),
		(IJST_TPRI(Int), i3, "i3_v", 0),
		(IJST_TPRI(Int), i4, "i4_v", 0),
		(IJST_TPRI(Int), i5, "i5_v", 0),
		(IJST_TPRI(Int), i6, "i6_v", 0),
		(IJST_TPRI(Int), i7, "i7_v", 0),
		(IJST_TPRI(Int), i8, "i8_v", 0),
		(IJST_TPRI(Int), i9, "i9_v", 0),
		(IJST_TPRI(Int), i10, "i10_v", 0),
		(IJST_TPRI(Int), i11, "i11_v", 0),
		(IJST_TPRI(Int), i12, "i12_v", 0),
		(IJST_TPRI(Int), i13, "i13_v", 0),
		(IJST_TPRI(Int), i14, "i14_v", 0),
		(IJST_TPRI(Int), i15, "i15_v", 0),
		(IJST_TPRI(Int), i16, "i16_v", 0),
		(IJST_TPRI(Int), i17, "i17_v", 0),
		(IJST_TPRI(Int), i18, "i18_v", 0),
		(IJST_TPRI(Int), i19, "i19_v", 0),
		(IJST_TPRI(Int), i20, "i20_v", 0),
		(IJST_TPRI(Int), i21, "i21_v", 0),
		(IJST_TPRI(Int), i22, "i22_v", 0),
		(IJST_TPRI(Int), i23, "i23_v", 0),
		(IJST_TPRI(Int), i24, "i24_v", 0),
		(IJST_TPRI(Int), i25, "i25_v", 0),
		(IJST_TPRI(Int), i26, "i26_v", 0),
		(IJST_TPRI(Int), i27, "i27_v", 0),
		(IJST_TPRI(Int), i28, "i28_v", 0),
		(IJST_TPRI(Int), i29, "i29_v", 0),
		(IJST_TPRI(Int), i30, "i30_v", 0),
		(IJST_TPRI(Int), i31, "i31_v", 0),
		(IJST_TPRI(Int), i32, "i32_v", 0),
		(IJST_TPRI(Int), i33, "i33_v", 0),
		(IJST_TPRI(Int), i34, "i34_v", 0),
		(IJST_TPRI(Int), i35, "i35_v", 0),
		(IJST_TPRI(Int), i36, "i36_v", 0),
		(IJST_TPRI(Int), i37, "i37_v", 0),
		(IJST_TPRI(Int), i38, "i38_v", 0),
		(IJST_TPRI(Int), i39, "i39_v", 0),
		(IJST_TPRI(Int), i40, "i40_v", 0),
		(IJST_TPRI(Int), i41, "i41_v", 0),
		(IJST_TPRI(Int), i42, "i42_v", 0),
		(IJST_TPRI(Int), i43, "i43_v", 0),
		(IJST_TPRI(Int), i44, "i44_v", 0),
		(IJST_TPRI(Int), i45, "i45_v", 0),
		(IJST_TPRI(Int), i46, "i46_v", 0),
		(IJST_TPRI(Int), i47, "i47_v", 0),
		(IJST_TPRI(Int), i48, "i48_v", 0),
		(IJST_TPRI(Int), i49, "i49_v", 0),
		(IJST_TPRI(Int), i50, "i50_v", 0),
		(IJST_TPRI(Int), i51, "i51_v", 0),
		(IJST_TPRI(Int), i52, "i52_v", 0),
		(IJST_TPRI(Int), i53, "i53_v", 0),
		(IJST_TPRI(Int), i54, "i54_v", 0),
		(IJST_TPRI(Int), i55, "i55_v", 0),
		(IJST_TPRI(Int), i56, "i56_v", 0),
		(IJST_TPRI(Int), i57, "i57_v", 0),
		(IJST_TPRI(Int), i58, "i58_v", 0),
		(IJST_TPRI(Int), i59, "i59_v", 0),
		(IJST_TPRI(Int), i60, "i60_v", 0),
		(IJST_TPRI(Int), i61, "i61_v", 0),
		(IJST_TPRI(Int), i62, "i62_v", 0),
		(IJST_TPRI(Int), i63, "i63_v", 0),
		(IJST_TPRI(Int), i64, "i64_v", 0)
)

TEST(Serialize, BigStruct)
{
	Complicate3 st;
	int ret = st._.SerializeInplace(true);
	ASSERT_EQ(ret, 0);
	rapidjson::Value &jVal = st._.InnerBuffer();
	ASSERT_EQ(jVal["i1_v"].GetInt(), 0);
	ASSERT_EQ(jVal["i64_v"].GetInt(), 0);
}

