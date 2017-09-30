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
	ijst::StoreType &jVal = nestSt._.InnerStream();
	ASSERT_TRUE(jVal.IsObject());
	ASSERT_TRUE(jVal.MemberCount() == 0);
}

TEST(Serialize, EmptyStruct_PushAllField)
{
	NestSt nestSt;

	// Empty struct
	int ret = nestSt._.SerializeInplace(true);
	ASSERT_EQ(ret, 0);
	ijst::StoreType &jVal = nestSt._.InnerStream();

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

TEST(Serialize, UseOutterBuffer)
{
	NestSt nestSt;
	rapidjson::Document doc;
	int ret = nestSt._.Serialize(true, doc);
	ASSERT_EQ(ret, 0);
	ASSERT_GT(doc.MemberCount(), 0);

	ASSERT_EQ(nestSt._.InnerStream().MemberCount(), 0);
	ASSERT_EQ(nestSt.inner_1._.InnerStream().MemberCount(), 0);
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

	st.inner_v.push_back(Inner());
	st.inner_m["k"] = Inner();
	st.inner_m["k2"] = Inner();
	st.inner_mv["k"] = vector<Inner>(1, Inner());

	int ret = st._.SerializeInplace(true);
	ASSERT_EQ(ret, 0);
	ijst::StoreType &jVal = st._.InnerStream();

	ASSERT_EQ(&jVal["inner_val"], &st.inner._.InnerStream());
	ASSERT_EQ(&jVal["inner_v_val"][0], &st.inner_v.front()._.InnerStream());
	ASSERT_EQ(&jVal["inner_m_val"]["k"], &st.inner_m["k"]._.InnerStream());
	ASSERT_EQ(&jVal["inner_m_val"]["k2"], &st.inner_m["k2"]._.InnerStream());
	ASSERT_EQ(&jVal["inner_mv_val"]["k"][0], &st.inner_mv["k"][0]._.InnerStream());
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
	rapidjson::Value &jVal = st._.InnerStream();
//	string ser;
//	st._.WriteInnerStream(ser);
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
	ASSERT_EQ(&jVal["c1_v"], &st.c1._.InnerStream());
	ASSERT_EQ(&jVal["c1_v"]["i1_v"], &st.c1.i1._.InnerStream());
	ASSERT_EQ(&jVal["c1_v"]["i2_v"], &st.c1.i2._.InnerStream());
	ASSERT_EQ(&jVal["c1_v"]["i3_v"], &st.c1.i3._.InnerStream());
	ASSERT_EQ(&jVal["c1_v"]["i4_v"], &st.c1.i4._.InnerStream());
	// mmo
	ASSERT_EQ(&jVal["mmo_v"]["om1"]["im1"], &st.mmo["om1"]["im1"]._.InnerStream());
	ASSERT_EQ(&jVal["mmo_v"]["om1"]["im2"], &st.mmo["om1"]["im2"]._.InnerStream());
	ASSERT_EQ(&jVal["mmo_v"]["om2"]["im1"], &st.mmo["om2"]["im1"]._.InnerStream());
	ASSERT_EQ(&jVal["mmo_v"]["om2"]["im3"], &st.mmo["om2"]["im3"]._.InnerStream());
}

