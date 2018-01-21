//
// Created by h46incon on 2017/10/11.
//

#include "util.h"

#include <ijst/types_std.h>
#include <ijst/types_stdlayout_wrapper.h>
#include <limits>
using std::vector;
using std::map;
using std::string;
using namespace ijst;


template<typename jsonType>
jsonType GetJsonVal(rapidjson::Value& jVal);

template <>
bool GetJsonVal<bool>(rapidjson::Value& jVal)
{
	return jVal.GetBool();
}

template <>
int GetJsonVal<int>(rapidjson::Value& jVal)
{
	return jVal.GetInt();
}

template <>
int64_t GetJsonVal<int64_t>(rapidjson::Value& jVal)
{
	return jVal.GetInt64();
}

template <>
unsigned int GetJsonVal<unsigned int>(rapidjson::Value& jVal)
{
	return jVal.GetUint();
}

template <>
uint64_t GetJsonVal<uint64_t>(rapidjson::Value& jVal)
{
	return jVal.GetUint64();
}

template <>
double GetJsonVal<double>(rapidjson::Value& jVal)
{
	return jVal.GetDouble();
}

template <>
string GetJsonVal<string>(rapidjson::Value& jVal)
{
	return string(jVal.GetString(), jVal.GetStringLength());
}

#define UTEST_DEFINE_STRUCT(StName, PrimType) 				\
	IJST_DEFINE_STRUCT(										\
		StName												\
		, (PrimType, v, "f_v", 0)							\
		, (IJST_TMAP(PrimType), map_v, "f_map", 0)			\
		, (IJST_TVEC(PrimType), vec_v, "f_vec", 0)			\
		, (IJST_TDEQUE(PrimType), deq_v, "f_deq", 0)		\
		, (IJST_TLIST(PrimType), list_v, "f_list", 0)		\
		, (IJST_TOBJ(PrimType), obj_v, "f_obj", 0)			\
	)

/**
 * Struct test
 * @tparam Struct 	ijst struct type
 * @tparam VT 		VarType
 * @tparam JT 		JsonVarType
 */
template <typename Struct, typename VT, typename JT>
void DoTestSt(const string& json, VT vDefault
			, VT v0, VT map1, VT map2, VT vec0, VT vec1, VT deq0, VT deq1, VT lst0, VT lst1, VT obj0, VT obj1
			, VT nv0, VT nmap1, VT nmap3, VT nvec0, VT nvec1, VT nvec2, VT ndeq0, VT ndeq1, VT nlst0, VT nlst1, VT nobj0, VT nobj2
			, JT sv0, JT smap1, JT smap2, JT smap3, JT svec0, JT svec1, JT svec2, JT sdeq0, JT sdeq1, JT slst0, JT slst1, JT sobj0, JT sobj1, JT sobj2
)
{
	Struct st;
	// Default value
	{
		ASSERT_EQ(st._.GetUnknown().MemberCount(), 0u);
		ASSERT_EQ(st.v, vDefault);
	}

	// Deserialize
	{
		int ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
		// v
		ASSERT_EQ(st.v, v0);
		// map
		ASSERT_EQ(st.map_v.size(), 2u);
		ASSERT_EQ(st.map_v["v1"], map1);
		ASSERT_EQ(st.map_v["v2"], map2);
		// vec
		ASSERT_EQ(st.vec_v.size(), 2u);
		ASSERT_EQ(st.vec_v[0], vec0);
		ASSERT_EQ(st.vec_v[1], vec1);
		// deq
		ASSERT_EQ(st.deq_v.size(), 2u);
		ASSERT_EQ(st.deq_v[0], deq0);
		ASSERT_EQ(st.deq_v[1], deq1);
		// list
		ASSERT_EQ(st.list_v.size(), 2u);
		ASSERT_EQ(st.list_v.front(), lst0);
		ASSERT_EQ(st.list_v.back(), lst1);
		// obj
		ASSERT_EQ(st.obj_v.size(), 2u);
		ASSERT_EQ(st.obj_v[0].name, "o0");
		ASSERT_EQ(st.obj_v[0].value, obj0);
		ASSERT_EQ(st.obj_v[1].name, "o1");
		ASSERT_EQ(st.obj_v[1].value, obj1);
	}

	// Fields modify
	// v
	IJST_SET(st, v, nv0);
	// map
	st.map_v["v1"] = nmap1;
	st.map_v["v3"] = nmap3;
	// vec
	st.vec_v[0] = nvec0;
	st.vec_v.pop_back();
	st.vec_v.push_back(nvec1);
	st.vec_v.push_back(nvec2);
	// deque
	st.deq_v.push_back(ndeq1);
	st.deq_v.pop_front();
	st.deq_v[0] = ndeq0;
	// list
	st.list_v.push_back(nlst1);
	st.list_v.pop_front();
	st.list_v.front() = (nlst0);
	// obj
	st.obj_v[0].name = "no0";
	st.obj_v[0].value = nobj0;
	st.obj_v.push_back(T_Member<VT>(string("no2"), nobj2));


	// Serialize
	rapidjson::Document doc;
	UTEST_SERIALIZE_AND_CHECK(st, doc, SerFlag::kNoneFlag);
	JT (*pGetJsonVal)(rapidjson::Value &) = GetJsonVal<JT>;
	// v
	ASSERT_EQ(pGetJsonVal(doc["f_v"]), sv0);
	// map
	ASSERT_EQ(doc["f_map"].MemberCount(), 3u);
	ASSERT_EQ(pGetJsonVal(doc["f_map"]["v1"]), smap1);
	ASSERT_EQ(pGetJsonVal(doc["f_map"]["v2"]), smap2);
	ASSERT_EQ(pGetJsonVal(doc["f_map"]["v3"]), smap3);
	// vec
	ASSERT_EQ(doc["f_vec"].Size(), 3u);
	ASSERT_EQ(pGetJsonVal(doc["f_vec"][0]), svec0);
	ASSERT_EQ(pGetJsonVal(doc["f_vec"][1]), svec1);
	ASSERT_EQ(pGetJsonVal(doc["f_vec"][2]), svec2);
	// deq
	ASSERT_EQ(doc["f_deq"].Size(), 2u);
	ASSERT_EQ(pGetJsonVal(doc["f_deq"][0]), sdeq0);
	ASSERT_EQ(pGetJsonVal(doc["f_deq"][1]), sdeq1);
	// list
	ASSERT_EQ(doc["f_list"].Size(), 2u);
	ASSERT_EQ(pGetJsonVal(doc["f_list"][0]), slst0);
	ASSERT_EQ(pGetJsonVal(doc["f_list"][1]), slst1);
	// obj
	ASSERT_EQ(doc["f_vec"].Size(), 3u);
	ASSERT_EQ(pGetJsonVal(doc["f_obj"]["no0"]), sobj0);
	ASSERT_EQ(pGetJsonVal(doc["f_obj"]["o1"]), sobj1);
	ASSERT_EQ(pGetJsonVal(doc["f_obj"]["no2"]), sobj2);
};

/**
 * Struct test
 * @tparam Struct 	ijst struct type
 * @tparam VT 		VarType
 * @tparam JT 		JsonVarType
 */
template <typename Struct, typename VT, typename JT>
void TestSt(const string& json, VT vDefault
		, VT v0, VT map1, VT map2, VT vec0, VT vec1, VT deq0, VT deq1, VT lst0, VT lst1, VT obj0, VT obj1
		, VT nv0, VT nmap1, VT nmap3, VT nvec0, VT nvec1, VT nvec2, VT ndeq0, VT ndeq1, VT nlst0, VT nlst1, VT nobj0, VT nobj2
)
{
	DoTestSt<Struct, VT, JT>(
			json, vDefault
			, v0, map1, map2, vec0, vec1, deq0, deq1, lst0, lst1, obj0, obj1
			, nv0, nmap1, nmap3, nvec0, nvec1, nvec2, ndeq0, ndeq1, nlst0, nlst1, nobj0, nobj2
			, (JT)nv0, (JT)nmap1, (JT)map2, (JT)nmap3, (JT)nvec0, (JT)nvec1, (JT)nvec2, (JT)ndeq0, (JT)ndeq1, (JT)nlst0, (JT)nlst1, (JT)nobj0, (JT)obj1, (JT)nobj2
	);
};

/**
 * Struct test
 * @tparam Struct 	ijst struct type
 * @tparam VT 		VarType
 * @tparam JT 		JsonVarType
 */
template <typename Struct, typename VT>
void TestSt(const string& json, VT vDefault
		, VT v0, VT map1, VT map2, VT vec0, VT vec1, VT deq0, VT deq1, VT lst0, VT lst1, VT obj0, VT obj1
		, VT nv0, VT nmap1, VT nmap3, VT nvec0, VT nvec1, VT nvec2, VT ndeq0, VT ndeq1, VT nlst0, VT nlst1, VT nobj0, VT nobj2
)
{
	DoTestSt<Struct, VT, VT>(
			json, vDefault
			, v0, map1, map2, vec0, vec1, deq0, deq1, lst0, lst1, obj0, obj1
			, nv0, nmap1, nmap3, nvec0, nvec1, nvec2, ndeq0, ndeq1, nlst0, nlst1, nobj0, nobj2
			, nv0, nmap1, map2, nmap3, nvec0, nvec1, nvec2, ndeq0, ndeq1, nlst0, nlst1, nobj0, obj1, nobj2
	);
};

template<typename Struct>
void TestMemberTypeMismatch(const string& errJson, const char* type, const char* value)
{
	Struct stErr;
	string errMsg;
	int ret = stErr._.Deserialize(errJson, errMsg, ijst::UnknownMode::kKeep, true);
	int retExpected = ErrorCode::kDeserializeValueTypeError;
	ASSERT_EQ(ret, retExpected);
	CheckMemberTypeMismatch(errMsg, "f_v", type, value);
}

IJST_DEFINE_STRUCT(
		StBool
		, (T_bool, v, "f_v", 0)
		, (IJST_TMAP(T_bool), map_v, "f_map", 0)
		, (IJST_TDEQUE(T_bool), deq_v, "f_deq", 0)
		, (IJST_TLIST(T_bool), list_v, "f_list", 0)
		, (IJST_TOBJ(T_bool), obj_v, "f_obj", 0)
)

TEST(Primitive, Bool)
{
	// Deserialize error
	string errorJson = "{\"f_v\": \"1\"}";
	TestMemberTypeMismatch<StBool>(errorJson, "bool", "\"1\"");

	StBool st;
	// Default value
	{
		ASSERT_EQ(st._.GetUnknown().MemberCount(), 0u);
		ASSERT_EQ(st.v, false);
	}


	const string json = "{\"f_v\": true"
			", \"f_map\": {\"v1\": true, \"v2\": false}, \"f_obj\": {\"o1\": true, \"o2\": false} "
			", \"f_deq\": [false, true], \"f_list\": [true, false]}";
	// Deserialize
	{
		int ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.v, true);
		ASSERT_EQ(st.map_v.size(), 2u);
		ASSERT_EQ(st.map_v["v1"], true);
		ASSERT_EQ(st.map_v["v2"], false);
		ASSERT_EQ(st.obj_v.size(), 2u);
		ASSERT_EQ(st.obj_v[0].name, "o1");
		ASSERT_EQ(st.obj_v[0].value, true);
		ASSERT_EQ(st.obj_v[1].name, "o2");
		ASSERT_EQ(st.obj_v[1].value, false);
		ASSERT_EQ(st.deq_v.size(), 2u);
		ASSERT_EQ(st.deq_v[0], false);
		ASSERT_EQ(st.deq_v[1], true);
		ASSERT_EQ(st.list_v.size(), 2u);
		ASSERT_EQ(st.list_v.front(), true);
		ASSERT_EQ(st.list_v.back(), false);
	}

	// Serialize
	{
		IJST_SET(st, v, false);
		st.map_v["v1"] = true;
		st.map_v["v3"] = false;
		st.deq_v.clear();
		st.deq_v.push_front(true);
		st.deq_v.push_back(false);
		st.list_v.clear();
		st.list_v.push_front(true);
		st.list_v.push_back(false);

		rapidjson::Document doc;
		UTEST_SERIALIZE_AND_CHECK(st, doc, SerFlag::kNoneFlag);
		bool (*pGetJsonVal)(rapidjson::Value &) = GetJsonVal<bool>;
		ASSERT_EQ(pGetJsonVal(doc["f_v"]), false);
		ASSERT_EQ(doc["f_map"].MemberCount(), 3u);
		ASSERT_EQ(pGetJsonVal(doc["f_map"]["v1"]), true);
		ASSERT_EQ(pGetJsonVal(doc["f_map"]["v2"]), false);
		ASSERT_EQ(pGetJsonVal(doc["f_map"]["v3"]), false);
		ASSERT_EQ(doc["f_obj"].MemberCount(), 2u);
		ASSERT_EQ(pGetJsonVal(doc["f_obj"]["o1"]), true);
		ASSERT_EQ(pGetJsonVal(doc["f_obj"]["o2"]), false);
		ASSERT_EQ(doc["f_deq"].Size(), 2u);
		ASSERT_EQ(pGetJsonVal(doc["f_deq"][0]), true);
		ASSERT_EQ(pGetJsonVal(doc["f_deq"][1]), false);
		ASSERT_EQ(doc["f_list"].Size(), 2u);
		ASSERT_EQ(pGetJsonVal(doc["f_list"][0]), true);
		ASSERT_EQ(pGetJsonVal(doc["f_list"][1]), false);
	}
}

UTEST_DEFINE_STRUCT(StUBool, T_ubool)

TEST(Primitive, UBool)
{
	// Deserialize error
	string errorJson = "{\"f_v\": \"1\"}";
	TestMemberTypeMismatch<StUBool>(errorJson, "bool", "\"1\"");
	errorJson = "{\"f_v\": 1}";
	TestMemberTypeMismatch<StUBool>(errorJson, "bool", "1");

	const string json = "{\"f_v\": true"
			", \"f_map\": {\"v1\": true, \"v2\": false}, \"f_obj\": {\"o0\": false, \"o1\": true}"
			", \"f_vec\": [false, true], \"f_deq\": [true, false], \"f_list\": [false, true]}";
	DoTestSt<StUBool, uint8_t, bool> (
			json, 0
			, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1
			, 0, 0,	0, 1, 0, 1, 0, 1, 1, 0, 1, 0
			, false, false, false, false, true, false, true, false, true, true, false, true, true, false
	);
}

UTEST_DEFINE_STRUCT(StWBool, T_wbool)
TEST(Primitive, WBool)
{
	// Deserialize error
	string errorJson = "{\"f_v\": \"1\"}";
	TestMemberTypeMismatch<StWBool>(errorJson, "bool", "\"1\"");

	const string json = "{\"f_v\": true"
			", \"f_map\": {\"v1\": true, \"v2\": false}, \"f_obj\":{\"o0\": false, \"o1\": true}"
			", \"f_vec\": [false, true], \"f_deq\": [true, false], \"f_list\": [false, true]}";
	TestSt<StWBool, T_wbool, bool> (
			json, false
			, true, true, false, false, true, true, false, false, true, false, true
			, false, false, false, true, false, true, false, true, true, false, true, false
	);
}

UTEST_DEFINE_STRUCT(StInt, T_int)
TEST(Primitive, Int)
{
	// Deserialize error
	string errorJson = "{\"f_v\": \"1\"}";
	TestMemberTypeMismatch<StInt>(errorJson, "int", "\"1\"");

	StInt st;

	const string json = "{\"f_v\": 0"
			", \"f_map\": {\"v1\": -65537, \"v2\": 65536}, \"f_obj\": {\"o0\": 314, \"o1\": 628}"
			", \"f_vec\": [-2147483648, 2147483647], \"f_deq\": [-1, 1], \"f_list\": [-2, 2]}";

	TestSt<StInt, int> (
			json, 0
			, 0, -65537, 65536, -2147483647-1, 2147483647, -1, 1, -2, 2, 314, 628
			, -2147483647-1, 0, 65536, -65536, 2147483647, -2147483647-1, 10, -10, 20, -20, 628, 314
	);
}

UTEST_DEFINE_STRUCT(StInt64, T_int64)
TEST(Primitive, Int64)
{
	// Deserialize error
	{
		string errorJson = "{\"f_v\": -9223372036854775809}";
		TestMemberTypeMismatch<StInt64>(errorJson, "int64", "-9223372036854775809");

		errorJson = "{\"f_v\": \"1\"}";
		TestMemberTypeMismatch<StInt64>(errorJson, "int64", "\"1\"");

		errorJson = "{\"f_v\": 9223372036854775808}";
		TestMemberTypeMismatch<StInt64>(errorJson, "int64", "9223372036854775808");
	}

	const string json = "{\"f_v\": 0"
			", \"f_map\": {\"v1\": 2, \"v2\": 4}, \"f_obj\": {\"o0\": 6, \"o1\": 8}"
			", \"f_vec\": [-9223372036854775808, 9223372036854775807], \"f_deq\": [-1, 1], \"f_list\": [-2, 2]}";
	const int64_t i64Min = std::numeric_limits<int64_t>::min();
	const int64_t i64Max = std::numeric_limits<int64_t>::max();

	TestSt<StInt64, int64_t>(
			json, 0
			, 0, 2, 4, i64Min, 9223372036854775807, -1, 1, -2, 2, 6, 8
			, i64Min, i64Min, i64Max, 0, i64Min, 9223372036854775807, 10, -10, 20, -20, 30, -30
	);
}

UTEST_DEFINE_STRUCT(StUInt, T_uint)
TEST(Primitive, UInt)
{
	// Deserialize error
	{
		string errorJson = "{\"f_v\": -1}";
		TestMemberTypeMismatch<StUInt>(errorJson, "uint", "-1");

		errorJson = "{\"f_v\": \"1\"}";
		TestMemberTypeMismatch<StUInt>(errorJson, "uint", "\"1\"");

		errorJson = "{\"f_v\": 4294967296}";
		TestMemberTypeMismatch<StUInt>(errorJson, "uint", "4294967296");
	}

	const string json = "{\"f_v\": 0"
			", \"f_map\": {\"v1\": 2, \"v2\": 4}, \"f_obj\": {\"o0\": 6, \"o1\": 8}"
			", \"f_vec\": [4294967295, 1], \"f_deq\": [0, 1], \"f_list\": [2, 3]}";

	TestSt<StUInt, unsigned int> (
			json, 0
			, 0, 2, 4, 4294967295U, 1, 0, 1, 2, 3, 6, 8
			, 4294967295U, 200, 4294967295U, 4294967295U, 2, 4, 4, 5, 6, 7, 8, 9
	);
}

UTEST_DEFINE_STRUCT(StUInt64, T_uint64)
TEST(Primitive, UInt64)
{
	// Deserialize error
	{
		string errorJson = "{\"f_v\": -1}";
		TestMemberTypeMismatch<StUInt64>(errorJson, "uint64", "-1");

		errorJson = "{\"f_v\": \"1\"}";
		TestMemberTypeMismatch<StUInt64>(errorJson, "uint64", "\"1\"");

		errorJson = "{\"f_v\": 18446744073709551616}";
		TestMemberTypeMismatch<StUInt64>(errorJson, "uint64", "18446744073709551616");
	}

	const string json = "{\"f_v\": 0"
			", \"f_map\": {\"v1\": 2, \"v2\": 4}, \"f_obj\": {\"o0\": 6, \"o1\": 8}"
			", \"f_vec\": [18446744073709551615, 1], \"f_deq\": [0, 1], \"f_list\": [2, 3]}";

	TestSt<StUInt64, uint64_t> (
			json, 0
			, 0, 2, 4, 18446744073709551615ul, 1, 0, 1, 2, 3, 6, 8
			, 18446744073709551615ul, 18446744073709551615ul, 0, 0, 18446744073709551615ul, 200, 4, 5, 6, 7, 9, 10
	);
}

UTEST_DEFINE_STRUCT(StDouble, T_double)
TEST(Primitive, Double)
{
	// Deserialize error
	{
		string errorJson = "{\"f_v\": true}";
		TestMemberTypeMismatch<StDouble>(errorJson, "number", "true");

		errorJson = "{\"f_v\": \"1\"}";
		TestMemberTypeMismatch<StDouble>(errorJson, "number", "\"1\"");
	}

	const string json = "{\"f_v\": 0.0"
			", \"f_map\": {\"v1\": 2.2, \"v2\": 4.4},\"f_obj\": {\"o0\": 6.6, \"o1\": 8.8}"
			", \"f_vec\": [-0.1, 0.2], \"f_deq\": [1, -1], \"f_list\": [100, -100]}";

	TestSt<StDouble, double> (
			json, 0.0
			, 0.0, 2.2, 4.4, -0.1, 0.2, 1.0, -1.0, 100.0, -100.0, 6.6, 8.8
			, 1.1, 22.2, 44.4, 0.1, -100.2, 200.2, 2.0, -2.0, -200.0, 200.0, -300.0, 300.0

	);
}

UTEST_DEFINE_STRUCT(StString, T_string)
TEST(Primitive, Str)
{
	// Deserialize error
	{
		string errorJson = "{\"f_v\": 0}";
		TestMemberTypeMismatch<StString>(errorJson, "string", "0");
	}

	const string json = "{\"f_v\": \"true\""
			", \"f_map\": {\"v1\": \"false\", \"v2\": \"v22\"}, \"f_obj\": {\"o0\": \"o0\", \"o1\": \"o1\"}"
			", \"f_vec\": [\"0\", \"1\"], \"f_deq\": [\"\", \"null\"], \"f_list\": [\"0\", \"NaN\"]}";

	TestSt<StString, string>(
			json, ""
			, "true", "false", "v22", "0", "1", "", "null", "0", "NaN", "o0", "o1"
			, "false", "0.2", "0.4", "map1", "true", "map3", "", "0", "NaN", "null", "o00", "o11"
	);
}

UTEST_DEFINE_STRUCT(StWrapper, T_Wrapper<int>)
TEST(Primitive, Wrapper)
{
	// Deserialize error
	{
		string errorJson = "{\"f_v\": \"0\"}";
		TestMemberTypeMismatch<StWrapper>(errorJson, "int", "\"0\"");
	}
	const string json = "{\"f_v\": 0"
			", \"f_map\": {\"v1\": -65537, \"v2\": 65536}, \"f_obj\": {\"o0\": -3, \"o1\": 3} "
			", \"f_vec\": [-2147483648, 2147483647], \"f_deq\": [-1, 1], \"f_list\": [-2, 2]}";

	TestSt<StWrapper, T_Wrapper<int>, int> (
			json, 0
			, 0, -65537, 65536, -2147483647-1, 2147483647, -1, 1, -2, 2, -3, 3
			, -2147483647-1, 0, 65536, -65536, 2147483647, -2147483647-1, 10, -10, 20, -20, 30, -30
	);
}

UTEST_DEFINE_STRUCT(StRaw, T_raw)

void CheckStRawDeserialize(StRaw &st)
{
	// Check st
	ASSERT_STREQ(st.v.V().GetString(), "v1");

	ASSERT_EQ(st.map_v.size(), 2u);
	ASSERT_TRUE(st.map_v["v1"].V().IsNull());
	ASSERT_EQ(st.map_v["v2"].V()["v21"].GetBool(), false);

	ASSERT_EQ(st.obj_v.size(), 2u);
	ASSERT_EQ(st.obj_v[0].name, "o0");
	ASSERT_EQ(st.obj_v[0].value.V().GetInt(), 0);
	ASSERT_EQ(st.obj_v[1].name, "o1");
	ASSERT_EQ(st.obj_v[1].value.V().GetBool(), true);

	ASSERT_EQ(st.vec_v.size(), 2u);
	ASSERT_STREQ(st.vec_v[0].V().GetString(), "v1");
	ASSERT_EQ(st.vec_v[1].V().GetInt(), 2);

	ASSERT_TRUE(st.deq_v.empty());

	ASSERT_EQ(st.list_v.size(), 1u);
	ASSERT_TRUE(st.list_v.front().V().IsNull());
}

TEST(Primitive, Raw)
{
	int ret;
	(void) ret;

	// Deserialize error
	{
		// Raw could store any json type
	}

	StRaw st;
	// Default value
	{
		ASSERT_EQ(st._.GetUnknown().MemberCount(), 0u);
		ASSERT_TRUE(st.v.V().IsNull());
	}

	const string json = "{\"f_v\": \"v1\""
			", \"f_map\": {\"v1\": null, \"v2\": {\"v21\": false }}, \"f_obj\": {\"o0\": 0, \"o1\": true}"
			", \"f_vec\": [\"v1\", 2], \"f_deq\": [], \"f_list\": [null]}";
#if IJST_ENABLE_FROM_JSON_OBJECT
	// Deserialize copy form json
	{
		{
			rapidjson::Document doc;
			doc.Parse(json.c_str(), json.length());
			ASSERT_FALSE(doc.HasParseError());
			ret = st._.FromJson(doc);
			ASSERT_EQ(ret, 0);
			// Check source doc
			ASSERT_STREQ(doc["f_v"].GetString(), "v1");
			ASSERT_TRUE(doc["f_map"]["v1"].IsNull());
			ASSERT_EQ(doc["f_map"]["v2"]["v21"].GetBool(), false);
			ASSERT_EQ(doc["f_obj"]["o0"].GetInt(), 0);
			ASSERT_EQ(doc["f_obj"]["o1"].GetBool(), true);
			ASSERT_STREQ(doc["f_vec"][0].GetString(), "v1");
			ASSERT_EQ(doc["f_vec"][1].GetInt(), 2);
			ASSERT_EQ(doc["f_deq"].Size(), 0u);
			ASSERT_TRUE(doc["f_list"][0].IsNull());
			// doc is destory here
		}
		// Check st
		CheckStRawDeserialize(st);
	}

	// Deserialize Move from json
	{
		rapidjson::Document doc;
		doc.Parse(json.c_str(), json.length());
		ASSERT_FALSE(doc.HasParseError());
		ret = st._.MoveFromJson(doc);
		ASSERT_EQ(ret, 0);
		// Check st
		CheckStRawDeserialize(st);
		// Check source doc
		ASSERT_TRUE(doc.IsNull());
	}
#endif
	// Deserialize
	{
		ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
		// Check st
		CheckStRawDeserialize(st);
	}

	// Serialize
	{
		st.v.V().SetInt(0);
		IJST_MARK_VALID(st, v);

		T_raw raw;
		raw.V().SetNull();
		st.vec_v[0] = raw;
		raw.V().SetString("v03");
		st.vec_v.push_back(raw);

		raw.V().SetString("v2");
		st.map_v["v1"] = raw;
		raw.V().SetObject();
		raw.V().AddMember("v31", rapidjson::Value().SetInt(-1).Move(), raw.GetAllocator());
		st.map_v["v3"] = raw;

		rapidjson::Document doc;
		UTEST_SERIALIZE_AND_CHECK(st, doc, SerFlag::kNoneFlag);
		ASSERT_EQ(doc["f_v"].GetInt(), 0);
		ASSERT_TRUE(doc["f_vec"][0].IsNull());
		ASSERT_EQ(doc["f_vec"][1].GetInt(), 2);
		ASSERT_STREQ(doc["f_vec"][2].GetString(), "v03");
		ASSERT_STREQ(doc["f_map"]["v1"].GetString(), "v2");
		ASSERT_EQ(doc["f_map"]["v2"]["v21"].GetBool(), false);
		ASSERT_EQ(doc["f_map"]["v3"]["v31"].GetInt(), -1);
	}
}

TEST(Primitive, Raw_BasicAPI)
{
	{
		// Copy constructor
		T_raw src;
		src.V().SetString("src_v", src.GetAllocator());

		T_raw dst(src);
		ASSERT_STREQ(src.V().GetString(), "src_v");
		ASSERT_STREQ(dst.V().GetString(), "src_v");
		ASSERT_NE(&src.GetAllocator(), &dst.GetAllocator());
	}

	{
		// assignment
		T_raw src;
		src.V().SetString("src_v", src.GetAllocator());

		T_raw dst;
		dst = src;
		ASSERT_STREQ(src.V().GetString(), "src_v");
		ASSERT_STREQ(dst.V().GetString(), "src_v");
		ASSERT_NE(&src.GetAllocator(), &dst.GetAllocator());
	}

#if __cplusplus >= 201103L
	{
		// Rvalue Copy constructor
		T_raw src;
		src.V().SetString("src_v", src.GetAllocator());
		JsonAllocator* pSrcAlloc = &src.GetAllocator();

		T_raw dst(std::move(src));
		ASSERT_STREQ(dst.V().GetString(), "src_v");
		ASSERT_EQ(&dst.GetAllocator(), pSrcAlloc);
		ASSERT_EQ(&src.GetAllocator(), nullptr);
	}

	{
		// Rvalue assignment
		T_raw src;
		src.V().SetString("src_v", src.GetAllocator());
		JsonAllocator* pSrcAlloc = &src.GetAllocator();

		T_raw dst;
		dst = std::move(src);
		ASSERT_STREQ(dst.V().GetString(), "src_v");
		ASSERT_EQ(&dst.GetAllocator(), pSrcAlloc);
		ASSERT_EQ(&src.GetAllocator(), nullptr);

	}
#endif
}
