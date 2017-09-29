//
// Created by h46incon on 2017/9/29.
//

#include "gtest/gtest.h"
#include "ijst/ijst.h"
using std::vector;

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
		Complicate,
		(IJST_TVEC(IJST_TVEC(IJST_TPRI(Int))), vvi, "vvi_v", 0),
		(IJST_TVEC(IJST_TMAP(IJST_TPRI(String))), vms, "vms_v", 0),
		(IJST_TMAP(IJST_TVEC(IJST_TPRI(String))), mvs, "mvs_v", 0),
		(IJST_TMAP(IJST_TMAP(IJST_TOBJ(Inner))), mmo, "mmo_v", 0)
)

