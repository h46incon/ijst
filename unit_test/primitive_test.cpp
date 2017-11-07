//
// Created by h46incon on 2017/10/11.
//

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <gtest/gtest.h>
#include <ijst/ijst.h>
#include <ijst/types_std.h>
#include <ijst/types_time.h>
#include <limits>
using std::vector;
using std::map;
using std::string;

IJST_DEFINE_STRUCT(
		StInt
		, (IJST_TPRI(Int), v, "f_v", 0)
		, (IJST_TVEC(IJST_TPRI(Int)), vec_v, "f_vec", 0)
		, (IJST_TMAP(IJST_TPRI(Int)), map_v, "f_map", 0)
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
		StUInt32
		, (IJST_TPRI(UInt32), v, "f_v", 0)
		, (IJST_TVEC(IJST_TPRI(UInt32)), vec_v, "f_vec", 0)
		, (IJST_TMAP(IJST_TPRI(UInt32)), map_v, "f_map", 0)
)

TEST(Primitive, UInt32)
{
	int ret;

	// Deserialize error
	{
		const int retExpected = ijst::Err::kDeserializeValueTypeError;
		StUInt32 stErr;

		string errorJson = "{\"f_v\": -1}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"1\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": 4294967296}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);
	}

	StUInt32 st;

	// Default value
	{
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);
		ASSERT_EQ(st.v, 0u);
	}

	// Deserialize
	{
		string json = "{\"f_v\": 0, \"f_vec\": [4294967295, 1], \"f_map\": {\"v1\": 2, \"v2\": 4}}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.v, 0u);
		ASSERT_EQ(st.vec_v[0], 4294967295u);
		ASSERT_EQ(st.vec_v[1], 1u);
		ASSERT_EQ(st.map_v["v1"], 2u);
		ASSERT_EQ(st.map_v["v2"], 4u);
	}

	// Serialize
	{
		IJST_SET(st, v, 100u);
		st.vec_v[1] = 100u;
		st.vec_v.push_back(200u);
		st.map_v["v1"] = 200u;
		st.map_v["v3"] = 400u;

		rapidjson::Value jVal;
		ret = st._.SerializeInInnerAlloc(false, jVal);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(jVal["f_v"].GetUint(), 100u);
		ASSERT_EQ(jVal["f_vec"][0].GetUint(), 4294967295u);
		ASSERT_EQ(jVal["f_vec"][1].GetUint(), 100u);
		ASSERT_EQ(jVal["f_vec"][2].GetUint(), 200u);
		ASSERT_EQ(jVal["f_map"]["v1"].GetUint(), 200u);
		ASSERT_EQ(jVal["f_map"]["v2"].GetUint(), 4u);
		ASSERT_EQ(jVal["f_map"]["v3"].GetUint(), 400u);
	}
}

IJST_DEFINE_STRUCT(
		StUInt64
		, (IJST_TPRI(UInt64), v, "f_v", 0)
		, (IJST_TVEC(IJST_TPRI(UInt64)), vec_v, "f_vec", 0)
		, (IJST_TMAP(IJST_TPRI(UInt64)), map_v, "f_map", 0)
)

TEST(Primitive, UInt64)
{
	int ret;

	// Deserialize error
	{
		const int retExpected = ijst::Err::kDeserializeValueTypeError;
		StUInt64 stErr;

		string errorJson = "{\"f_v\": -1}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"1\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": 18446744073709551616}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);
	}

	StUInt64 st;

	// Default value
	{
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);
		ASSERT_EQ(st.v, 0u);
	}

	// Deserialize
	{
		string json = "{\"f_v\": 0, \"f_vec\": [18446744073709551615, 1], \"f_map\": {\"v1\": 2, \"v2\": 4}}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.v, 0u);
		ASSERT_EQ(st.vec_v[0], 18446744073709551615ul);
		ASSERT_EQ(st.vec_v[1], 1u);
		ASSERT_EQ(st.map_v["v1"], 2u);
		ASSERT_EQ(st.map_v["v2"], 4u);
	}

	// Serialize
	{
		IJST_SET(st, v, 100u);
		st.vec_v[1] = 100u;
		st.vec_v.push_back(200u);
		st.map_v["v1"] = 200u;
		st.map_v["v3"] = 400u;

		rapidjson::Value jVal;
		ret = st._.SerializeInInnerAlloc(false, jVal);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(jVal["f_v"].GetUint64(), 100u);
		ASSERT_EQ(jVal["f_vec"][0].GetUint64(), 18446744073709551615ul);
		ASSERT_EQ(jVal["f_vec"][1].GetUint64(), 100u);
		ASSERT_EQ(jVal["f_vec"][2].GetUint64(), 200u);
		ASSERT_EQ(jVal["f_map"]["v1"].GetUint64(), 200u);
		ASSERT_EQ(jVal["f_map"]["v2"].GetUint64(), 4u);
		ASSERT_EQ(jVal["f_map"]["v3"].GetUint64(), 400u);
	}
}

IJST_DEFINE_STRUCT(
		StInt32
		, (IJST_TPRI(Int32), v, "f_v", 0)
		, (IJST_TVEC(IJST_TPRI(Int32)), vec_v, "f_vec", 0)
		, (IJST_TMAP(IJST_TPRI(Int32)), map_v, "f_map", 0)
)

TEST(Primitive, Int32)
{
	int ret;

	// Deserialize error
	{
		const int retExpected = ijst::Err::kDeserializeValueTypeError;
		StInt32 stErr;

		string errorJson = "{\"f_v\": -2147483649}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"1\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": 2147483648}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);
	}

	StInt32 st;

	// Default value
	{
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);
		ASSERT_EQ(st.v, 0);
	}

	// Deserialize
	{
		string json = "{\"f_v\": 0, \"f_vec\": [-2147483648, 2147483647], \"f_map\": {\"v1\": 2, \"v2\": 4}}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.v, 0);
		ASSERT_EQ(st.vec_v[0], -2147483648);
		ASSERT_EQ(st.vec_v[1], 2147483647);
		ASSERT_EQ(st.map_v["v1"], 2);
		ASSERT_EQ(st.map_v["v2"], 4);
	}

	// Serialize
	{
		IJST_SET(st, v, 100);
		st.vec_v[0] = 2147483647;
		st.vec_v[1] = -2147483648;
		st.vec_v.push_back(200);
		st.map_v["v1"] = 200;
		st.map_v["v3"] = 400;

		rapidjson::Value jVal;
		ret = st._.SerializeInInnerAlloc(false, jVal);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(jVal["f_v"].GetInt(), 100);
		ASSERT_EQ(jVal["f_vec"][0].GetInt(), 2147483647);
		ASSERT_EQ(jVal["f_vec"][1].GetInt(), -2147483648);
		ASSERT_EQ(jVal["f_vec"][2].GetInt(), 200);
		ASSERT_EQ(jVal["f_map"]["v1"].GetInt(), 200);
		ASSERT_EQ(jVal["f_map"]["v2"].GetInt(), 4);
		ASSERT_EQ(jVal["f_map"]["v3"].GetInt(), 400);
	}
}

IJST_DEFINE_STRUCT(
		StInt64
		, (IJST_TPRI(Int64), v, "f_v", 0)
		, (IJST_TVEC(IJST_TPRI(Int64)), vec_v, "f_vec", 0)
		, (IJST_TMAP(IJST_TPRI(Int64)), map_v, "f_map", 0)
)

TEST(Primitive, Int64)
{
	int ret;

	// Deserialize error
	{
		const int retExpected = ijst::Err::kDeserializeValueTypeError;
		StInt64 stErr;

		string errorJson = "{\"f_v\": -9223372036854775809}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"1\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": 9223372036854775808}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);
	}

	StInt64 st;
	const int64_t i64Min = std::numeric_limits<int64_t>::min();

	// Default value
	{
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);
		ASSERT_EQ(st.v, 0);
	}

	// Deserialize
	{
		string json = "{\"f_v\": 0, \"f_vec\": [-9223372036854775808, 9223372036854775807], \"f_map\": {\"v1\": 2, \"v2\": 4}}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.v, 0);
		ASSERT_EQ(st.vec_v[0], i64Min);
		ASSERT_EQ(st.vec_v[1], 9223372036854775807);
		ASSERT_EQ(st.map_v["v1"], 2);
		ASSERT_EQ(st.map_v["v2"], 4);
	}

	// Serialize
	{
		IJST_SET(st, v, 100);
		st.vec_v[0] = 9223372036854775807;
		st.vec_v[1] = i64Min;
		st.vec_v.push_back(200);
		st.map_v["v1"] = 200;
		st.map_v["v3"] = 400;

		rapidjson::Value jVal;
		ret = st._.SerializeInInnerAlloc(false, jVal);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(jVal["f_v"].GetInt64(), 100);
		ASSERT_EQ(jVal["f_vec"][0].GetInt64(), 9223372036854775807);
		ASSERT_EQ(jVal["f_vec"][1].GetInt64(), i64Min);
		ASSERT_EQ(jVal["f_vec"][2].GetInt64(), 200);
		ASSERT_EQ(jVal["f_map"]["v1"].GetInt64(), 200);
		ASSERT_EQ(jVal["f_map"]["v2"].GetInt64(), 4);
		ASSERT_EQ(jVal["f_map"]["v3"].GetInt64(), 400);
	}
}

IJST_DEFINE_STRUCT(
		StBool
		, (IJST_TPRI(Bool), v, "f_v", 0)
		, (IJST_TVEC(IJST_TPRI(Bool)), vec_v, "f_vec", 0)
		, (IJST_TMAP(IJST_TPRI(Bool)), map_v, "f_map", 0)
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
		StString
		, (IJST_TPRI(Str), v, "f_v", 0)
		, (IJST_TVEC(IJST_TPRI(Str)), vec_v, "f_vec", 0)
		, (IJST_TMAP(IJST_TPRI(Str)), map_v, "f_map", 0)
)

TEST(Primitive, Str)
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
		StRaw
		, (IJST_TPRI(Raw), v, "f_v", 0)
		, (IJST_TVEC(IJST_TPRI(Raw)), vec_v, "f_vec", 0)
		, (IJST_TMAP(IJST_TPRI(Raw)), map_v, "f_map", 0)
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

IJST_DEFINE_STRUCT(
		StTime
		, (IJST_TTIME(), v, "f_v", 0)
		, (IJST_TVEC(IJST_TTIME()), vec_v, "f_vec", 0)
		, (IJST_TMAP(IJST_TTIME()), map_v, "f_map", 0)
)

TEST(Primitive, Time)
{
	//! NOTE: This test is only avaliable in UTC+8
	int ret;

	// Deserialize error
	{
		const int retExpected = ijst::Err::kDeserializeValueTypeError;
		StTime stErr;
		string errorJson;

		errorJson = "{\"f_v\": 0}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-10 00:00: \"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-10 00:00:a\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-1000:00:00\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-10 00:00:00a\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-10 00:00:00 a\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-10 00:00:00 a b \"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);
	}

	StTime st;
	// Default value
	{
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);
		ASSERT_EQ(st.v, 0);
	}

	// Deserialize
	{
		string json = "{\"f_v\": \"-9999-1-1 0:0:0\", "									// very small time stamp
				"\"f_vec\": [\"  2000-01-01   00:00:00   \""							// time string with spare spaces
							", \"9999-12-31 23:59:59\""									// very large time stamp
							", \"2015-06-30 23:59:60\", \"2015-07-01 0:0:0\" ], "		// case of 61 sec in a minute
				"\"f_map\": {\"v1\": \"2038-01-19 03:14:07\""
							", \"v2\": \"2038-1-19 3:14:8 \""							// time stamp need 64bit to store
							", \"vn1\": \"1970-1-1 07:59:59\""							// -1
							", \"vzero\": \"1970-1-1 08:00:00\"}}";						// 0
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.v, -377705145600);
		ASSERT_EQ(st.vec_v[0], 946656000);
		ASSERT_EQ(st.vec_v[1], 253402271999);
		ASSERT_EQ(st.vec_v[2], 1435680000);
		ASSERT_EQ(st.vec_v[3], 1435680000);
		ASSERT_EQ(st.map_v["v1"], 2147454847);
		ASSERT_EQ(st.map_v["v2"], 2147454848);
		ASSERT_EQ(st.map_v["vn1"], -1);
		ASSERT_EQ(st.map_v["vzero"], 0);
	}

	// Serialize
	{
		IJST_SET(st, v, -377705145600 + 1);
		st.vec_v[0] = 0;
		st.vec_v.push_back(2147454848);
		st.map_v["v1"] = 1;
		st.map_v["v3"] = 2147454849;

		rapidjson::Value jVal;
		ret = st._.SerializeInInnerAlloc(false, jVal);
		ASSERT_EQ(ret, 0);
		ASSERT_STREQ(jVal["f_v"].GetString(), "-9999-01-01 00:00:01");
		ASSERT_STREQ(jVal["f_vec"][0].GetString(), "1970-01-01 08:00:00");
		ASSERT_STREQ(jVal["f_vec"][1].GetString(), "9999-12-31 23:59:59");
		ASSERT_STREQ(jVal["f_vec"][2].GetString(), "2015-07-01 00:00:00");
		ASSERT_STREQ(jVal["f_vec"][3].GetString(), "2015-07-01 00:00:00");
		ASSERT_STREQ(jVal["f_vec"][4].GetString(), "2038-01-19 03:14:08");
		ASSERT_STREQ(jVal["f_map"]["vzero"].GetString(), "1970-01-01 08:00:00");
		ASSERT_STREQ(jVal["f_map"]["vn1"].GetString(), "1970-01-01 07:59:59");
		ASSERT_STREQ(jVal["f_map"]["v1"].GetString(), "1970-01-01 08:00:01");
		ASSERT_STREQ(jVal["f_map"]["v2"].GetString(), "2038-01-19 03:14:08");
		ASSERT_STREQ(jVal["f_map"]["v3"].GetString(), "2038-01-19 03:14:09");
	}
}

IJST_DEFINE_STRUCT(
		StFTime
		, (IJST_TFTIME(8), v, "f_v", 0)
		, (IJST_TVEC(IJST_TFTIME(8)), vec_v, "f_vec", 0)
		, (IJST_TMAP(IJST_TFTIME(8)), map_v, "f_map", 0)
)

TEST(Primitive, FastTime)
{
	int ret;

	// Deserialize error
	{
		const int retExpected = ijst::Err::kDeserializeValueTypeError;
		StFTime stErr;
		string errorJson;

		errorJson = "{\"f_v\": 0}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-10 00:00: \"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-10 00:00:a\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-1000:00:00\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-10 00:00:00a\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-10 00:00:00 a\"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);

		errorJson = "{\"f_v\": \"2000-10-10 00:00:00 a b \"}";
		ret = stErr._.Deserialize(errorJson, 0);
		ASSERT_EQ(ret, retExpected);
	}

	StFTime st;
	// Default value
	{
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);
		ASSERT_EQ(st.v, 0);
	}

	// Deserialize
	{
		string json = "{\"f_v\": \"-9999-1-1 0:0:0\", "									// very small time stamp
				"\"f_vec\": [\"  2000-01-01   00:00:00   \""							// time string with spare spaces
				", \"9999-12-31 23:59:59\""									// very large time stamp
				", \"2015-06-30 23:59:60\", \"2015-07-01 0:0:0\" ], "		// case of 61 sec in a minute
				"\"f_map\": {\"v1\": \"2038-01-19 03:14:07\""
				", \"v2\": \"2038-1-19 3:14:8 \""							// time stamp need 64bit to store
				", \"vn1\": \"1970-1-1 07:59:59\""							// -1
				", \"vzero\": \"1970-1-1 08:00:00\"}}";						// 0
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.v, -377705145600);
		ASSERT_EQ(st.vec_v[0], 946656000);
		ASSERT_EQ(st.vec_v[1], 253402271999);
		ASSERT_EQ(st.vec_v[2], 1435680000);
		ASSERT_EQ(st.vec_v[3], 1435680000);
		ASSERT_EQ(st.map_v["v1"], 2147454847);
		ASSERT_EQ(st.map_v["v2"], 2147454848);
		ASSERT_EQ(st.map_v["vn1"], -1);
		ASSERT_EQ(st.map_v["vzero"], 0);
	}

	// Serialize
	{
		IJST_SET(st, v, -377705145600 + 1);
		st.vec_v[0] = 0;
		st.vec_v.push_back(2147454848);
		st.map_v["v1"] = 1;
		st.map_v["v3"] = 2147454849;

		rapidjson::Value jVal;
		ret = st._.SerializeInInnerAlloc(false, jVal);
		ASSERT_EQ(ret, 0);
		ASSERT_STREQ(jVal["f_v"].GetString(), "-9999-01-01 00:00:01");
		ASSERT_STREQ(jVal["f_vec"][0].GetString(), "1970-01-01 08:00:00");
		ASSERT_STREQ(jVal["f_vec"][1].GetString(), "9999-12-31 23:59:59");
		ASSERT_STREQ(jVal["f_vec"][2].GetString(), "2015-07-01 00:00:00");
		ASSERT_STREQ(jVal["f_vec"][3].GetString(), "2015-07-01 00:00:00");
		ASSERT_STREQ(jVal["f_vec"][4].GetString(), "2038-01-19 03:14:08");
		ASSERT_STREQ(jVal["f_map"]["vzero"].GetString(), "1970-01-01 08:00:00");
		ASSERT_STREQ(jVal["f_map"]["vn1"].GetString(), "1970-01-01 07:59:59");
		ASSERT_STREQ(jVal["f_map"]["v1"].GetString(), "1970-01-01 08:00:01");
		ASSERT_STREQ(jVal["f_map"]["v2"].GetString(), "2038-01-19 03:14:08");
		ASSERT_STREQ(jVal["f_map"]["v3"].GetString(), "2038-01-19 03:14:09");
	}
}
