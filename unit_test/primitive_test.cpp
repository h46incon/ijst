//
// Created by h46incon on 2017/10/11.
//

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <gtest/gtest.h>
#include <ijst/ijst.h>
using std::vector;
using std::map;
using std::string;

IJST_DEFINE_STRUCT(
		StInt,
		(IJST_TPRI(Int), v, "f_v", 0),
		(IJST_TVEC(IJST_TPRI(Int)), vec_v, "f_vec", 0),
		(IJST_TMAP(IJST_TPRI(Int)), map_v, "f_map", 0)
)

TEST(Primitive, Int)
{
	int ret;

	// Deserialize error
	{
		string errorJson = "{\"f_v\": \"1\"}";
		StInt stErr;
		ret = stErr._.Deserialize(errorJson, 0);
		int retExpected = ijst::Err::kDeserializeValueTypeError;
		ASSERT_EQ(ret, retExpected);
	}

	StInt st;

	// Default value
	{
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);
		ASSERT_EQ(st.v, 0);
	}

	// Deserialize
	{
		string json = "{\"f_v\": 0, \"f_vec\": [-1, 1], \"f_map\": {\"v1\": -2, \"v2\": 2}}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.v, 0);
		ASSERT_EQ(st.vec_v[0], -1);
		ASSERT_EQ(st.vec_v[1], 1);
		ASSERT_EQ(st.map_v["v1"], -2);
		ASSERT_EQ(st.map_v["v2"], 2);
	}

	// Serialize
	{
		IJST_SET(st, v, 100);
		st.vec_v[0] = -100;
		st.vec_v.push_back(100);
		st.map_v["v1"] = -200;
		st.map_v["v3"] = 200;

		rapidjson::Value jVal;
		ret = st._.SerializeInInnerAlloc(false, jVal);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(jVal["f_v"].GetInt(), 100);
		ASSERT_EQ(jVal["f_vec"][0].GetInt(), -100);
		ASSERT_EQ(jVal["f_vec"][1].GetInt(), 1);
		ASSERT_EQ(jVal["f_vec"][2].GetInt(), 100);
		ASSERT_EQ(jVal["f_map"]["v1"].GetInt(), -200);
		ASSERT_EQ(jVal["f_map"]["v2"].GetInt(), 2);
		ASSERT_EQ(jVal["f_map"]["v3"].GetInt(), 200);
	}

}

IJST_DEFINE_STRUCT(
		StBool,
		(IJST_TPRI(Bool), v, "f_v", 0),
		(IJST_TVEC(IJST_TPRI(Bool)), vec_v, "f_vec", 0),
		(IJST_TMAP(IJST_TPRI(Bool)), map_v, "f_map", 0)
)

TEST(Primitive, Bool)
{
	int ret;

	// Deserialize error
	{
		string errorJson = "{\"f_v\": \"1\"}";
		StBool stErr;
		ret = stErr._.Deserialize(errorJson, 0);
		int retExpected = ijst::Err::kDeserializeValueTypeError;
		ASSERT_EQ(ret, retExpected);
	}

	StBool st;

	// Default value
	{
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);
		ASSERT_EQ(st.v, 0);
	}

	// Deserialize
	{
		string json = "{\"f_v\": true, \"f_vec\": [true, false], \"f_map\": {\"v1\": true, \"v2\": false}}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.v, 1);
		ASSERT_EQ(st.vec_v[0], 1);
		ASSERT_EQ(st.vec_v[1], 0);
		ASSERT_EQ(st.map_v["v1"], 1);
		ASSERT_EQ(st.map_v["v2"], 0);
	}

	// Serialize
	{
		IJST_SET(st, v, 0);
		st.vec_v[0] = 0;
		st.vec_v.push_back(1);
		st.map_v["v1"] = 0;
		st.map_v["v3"] = 1;

		rapidjson::Value jVal;
		ret = st._.SerializeInInnerAlloc(false, jVal);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(jVal["f_v"].GetBool(), false);
		ASSERT_EQ(jVal["f_vec"][0].GetBool(), false);
		ASSERT_EQ(jVal["f_vec"][1].GetBool(), false);
		ASSERT_EQ(jVal["f_vec"][2].GetBool(), true);
		ASSERT_EQ(jVal["f_map"]["v1"].GetBool(), false);
		ASSERT_EQ(jVal["f_map"]["v2"].GetBool(), false);
		ASSERT_EQ(jVal["f_map"]["v3"].GetBool(), true);
	}

}

IJST_DEFINE_STRUCT(
		StString,
		(IJST_TPRI(String), v, "f_v", 0),
		(IJST_TVEC(IJST_TPRI(String)), vec_v, "f_vec", 0),
		(IJST_TMAP(IJST_TPRI(String)), map_v, "f_map", 0)
)

TEST(Primitive, String)
{
	int ret;

	// Deserialize error
	{
		string errorJson = "{\"f_v\": 0}";
		StString stErr;
		ret = stErr._.Deserialize(errorJson, 0);
		int retExpected = ijst::Err::kDeserializeValueTypeError;
		ASSERT_EQ(ret, retExpected);
	}

	StString st;
	// Default value
	{
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);
		ASSERT_TRUE(st.v.empty());
	}

	// Deserialize
	{
		string json = "{\"f_v\": \"v1\", \"f_vec\": [\"v1\", \"v2\"], \"f_map\": {\"v1\": \"v11\", \"v2\": \"v22\"}}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_STREQ(st.v.c_str(), "v1");
		ASSERT_STREQ(st.vec_v[0].c_str(), "v1");
		ASSERT_STREQ(st.vec_v[1].c_str(),"v2");
		ASSERT_STREQ(st.map_v["v1"].c_str(), "v11");
		ASSERT_STREQ(st.map_v["v2"].c_str(), "v22");
	}

	// Serialize
	{
		IJST_SET(st, v, "v0");
		st.vec_v[0] = "v01";
		st.vec_v.push_back("v03");
		st.map_v["v1"] = "v011";
		st.map_v["v3"] = "v033";

		rapidjson::Value jVal;
		ret = st._.SerializeInInnerAlloc(false, jVal);
		ASSERT_EQ(ret, 0);
		ASSERT_STREQ(jVal["f_v"].GetString(), "v0");
		ASSERT_STREQ(jVal["f_vec"][0].GetString(), "v01");
		ASSERT_STREQ(jVal["f_vec"][1].GetString(), "v2");
		ASSERT_STREQ(jVal["f_vec"][2].GetString(), "v03");
		ASSERT_STREQ(jVal["f_map"]["v1"].GetString(), "v011");
		ASSERT_STREQ(jVal["f_map"]["v2"].GetString(), "v22");
		ASSERT_STREQ(jVal["f_map"]["v3"].GetString(), "v033");
	}

}

IJST_DEFINE_STRUCT(
		StRaw,
		(IJST_TPRI(Raw), v, "f_v", 0),
		(IJST_TVEC(IJST_TPRI(Raw)), vec_v, "f_vec", 0),
		(IJST_TMAP(IJST_TPRI(Raw)), map_v, "f_map", 0)
)

TEST(Primitive, Raw)
{
	int ret;

	// Deserialize error
	{
		// Raw could store any json type
	}

	StRaw st;
	// Default value
	{
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);
		ASSERT_TRUE(st.v.V().IsNull());
	}

	// Deserialize
	{
		string json = "{\"f_v\": \"v1\", \"f_vec\": [\"v1\", 2], \"f_map\": {\"v1\": null, \"v2\": {\"v21\": false }}}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_STREQ(st.v.V().GetString(), "v1");
		ASSERT_STREQ(st.vec_v[0].V().GetString(), "v1");
		ASSERT_EQ(st.vec_v[1].V().GetInt(), 2);
		ASSERT_TRUE(st.map_v["v1"].V().IsNull());
		ASSERT_EQ(st.map_v["v2"].V()["v21"].GetBool(), false);
	}

	// Serialize
	{
		st.v.V().SetInt(0);
		IJST_MARK_VALID(st, v);

		ijst::FStoreRaw raw;
		raw.V().SetNull();
		st.vec_v[0] = raw;
		raw.V().SetString("v03");
		st.vec_v.push_back(raw);

		raw.V().SetString("v2");
		st.map_v["v1"] = raw;
		raw.V().SetObject();
		raw.V().AddMember("v31", rapidjson::Value().SetInt(-1).Move(), raw.GetAllocator());
		st.map_v["v3"] = raw;

		rapidjson::Value jVal;
		ret = st._.SerializeInInnerAlloc(false, jVal);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(jVal["f_v"].GetInt(), 0);
		ASSERT_TRUE(jVal["f_vec"][0].IsNull());
		ASSERT_EQ(jVal["f_vec"][1].GetInt(), 2);
		ASSERT_STREQ(jVal["f_vec"][2].GetString(), "v03");
		ASSERT_STREQ(jVal["f_map"]["v1"].GetString(), "v2");
		ASSERT_EQ(jVal["f_map"]["v2"]["v21"].GetBool(), false);
		ASSERT_EQ(jVal["f_map"]["v3"]["v31"].GetInt(), -1);
	}
}

TEST(Primitive, Raw_BasicAPI)
{
	{
		// Copy constructor
		ijst::FStoreRaw src;
		src.V().SetString("src_v", src.GetAllocator());

		ijst::FStoreRaw dst(src);
		ASSERT_STREQ(src.V().GetString(), "src_v");
		ASSERT_STREQ(dst.V().GetString(), "src_v");
		ASSERT_NE(&src.GetAllocator(), &dst.GetAllocator());
	}

	{
		// assignment
		ijst::FStoreRaw src;
		src.V().SetString("src_v", src.GetAllocator());

		ijst::FStoreRaw dst;
		dst = src;
		ASSERT_STREQ(src.V().GetString(), "src_v");
		ASSERT_STREQ(dst.V().GetString(), "src_v");
		ASSERT_NE(&src.GetAllocator(), &dst.GetAllocator());
	}

#if __cplusplus >= 201103L
	{
		// Rvalue Copy constructor
		ijst::FStoreRaw src;
		src.V().SetString("src_v", src.GetAllocator());
		ijst::AllocatorType* pSrcAlloc = &src.GetAllocator();

		ijst::FStoreRaw dst(std::move(src));
		ASSERT_STREQ(dst.V().GetString(), "src_v");
		ASSERT_EQ(&dst.GetAllocator(), pSrcAlloc);
		ASSERT_EQ(&src.GetAllocator(), nullptr);
	}

	{
		// Rvalue assignment
		ijst::FStoreRaw src;
		src.V().SetString("src_v", src.GetAllocator());
		ijst::AllocatorType* pSrcAlloc = &src.GetAllocator();

		ijst::FStoreRaw dst;
		dst = std::move(src);
		ASSERT_STREQ(dst.V().GetString(), "src_v");
		ASSERT_EQ(&dst.GetAllocator(), pSrcAlloc);
		ASSERT_EQ(&src.GetAllocator(), nullptr);

	}
#endif
}
