//
// Created by h46incon on 2017/9/30.
//
#include "util.h"

using std::vector;
using std::map;
using std::string;
using namespace ijst;

IJST_DEFINE_STRUCT(
		SimpleSt
		, (T_int, int_1, "int_val_1", FDesc::Optional)
		, (T_int, int_2, "int_val_2", 0)
		, (T_string, str_1, "str_val_1", FDesc::Optional)
		, (T_string, str_2, "str_val_2", 0)
)

TEST(Deserialize, Empty)
{
	string emptyJson = "{}";
	SimpleSt st;
	int ret = st._.Deserialize(emptyJson);
	int retExpected = ErrorCode::kDeserializeSomeFieldsInvalid;
	ASSERT_EQ(ret, retExpected);
}

TEST(Deserialize, IgnoreFieldStatus)
{
	string emptyJson = "{}";
	// Deserialize
	{
		SimpleSt st;
		int ret = st._.Deserialize(emptyJson, DeserFlag::kNotCheckFieldStatus);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(IJST_GET_STATUS(st, int_2), (EFStatus)FStatus::kMissing);
	}

	// From Json
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		SimpleSt st;
		int ret = st._.FromJson(doc, DeserFlag::kNotCheckFieldStatus);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(IJST_GET_STATUS(st, int_2), (EFStatus)FStatus::kMissing);
	}
	// Move From Json
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		SimpleSt st;
		int ret = st._.MoveFromJson(doc, DeserFlag::kNotCheckFieldStatus);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(IJST_GET_STATUS(st, int_2), (EFStatus)FStatus::kMissing);
	}
}


TEST(Deserialize, ParseError)
{
	string errJson = "{withoutQuote:1}";
	SimpleSt st;
	int ret = st._.Deserialize(errJson);
	int retExpected = ErrorCode::kDeserializeParseFailed;
	ASSERT_EQ(ret, retExpected);
}

TEST(Deserialize, TypeError)
{
	string errJson = "{\"int_val_2\":\"str\"}";
	SimpleSt st;
	int ret = st._.Deserialize(errJson);
	int retExpected = ErrorCode::kDeserializeValueTypeError;
	ASSERT_EQ(ret, retExpected);
}

TEST(Deserialize, RequiredFields)
{
	string validJson = "{\"int_val_2\":1, \"str_val_2\":\"str2\"}";
	SimpleSt st;
	int ret = st._.Deserialize(validJson);
	ASSERT_EQ(ret, 0);

	ASSERT_EQ(IJST_GET_STATUS(st, int_1), (EFStatus)FStatus::kMissing);
	ASSERT_EQ(IJST_GET_STATUS(st, str_1), (EFStatus)FStatus::kMissing);
	ASSERT_EQ(st.int_2, 1);
	ASSERT_STREQ(st.str_2.c_str(), "str2");
}

TEST(Deserialize, AdditionalFields)
{
	string validJson = "{\"addi_field1\": \"a_field1\","
			"\"int_val_1\":1, \"str_val_2\":\"str2\", "
			"\"addi_field2\":\"a_field2\","
			"\"int_val_2\":2, \"addi_field3\": \"a_field3\"}";
	SimpleSt st;
	// keep unknown
	{
		int ret = st._.Deserialize(validJson);
		ASSERT_EQ(ret, 0);

		ASSERT_EQ(IJST_GET_STATUS(st, int_1), (EFStatus)FStatus::kValid);
		ASSERT_EQ(IJST_GET_STATUS(st, str_1), (EFStatus)FStatus::kMissing);
		ASSERT_EQ(st.int_1, 1);
		ASSERT_EQ(st.int_2, 2);
		ASSERT_STREQ(st.str_2.c_str(), "str2");
		ASSERT_EQ(st._.GetUnknown().MemberCount(), 3u);
		ASSERT_STREQ(st._.GetUnknown()["addi_field1"].GetString(), "a_field1");
		ASSERT_STREQ(st._.GetUnknown()["addi_field2"].GetString(), "a_field2");
		ASSERT_STREQ(st._.GetUnknown()["addi_field3"].GetString(), "a_field3");
	}

	// ignore unknown
	{
		int ret = st._.Deserialize(validJson, DeserFlag::kIgnoreUnknown, 0);
		ASSERT_EQ(ret, 0);

		ASSERT_EQ(IJST_GET_STATUS(st, int_1), (EFStatus)FStatus::kValid);
		ASSERT_EQ(IJST_GET_STATUS(st, str_1), (EFStatus)FStatus::kMissing);
		ASSERT_EQ(st.int_1, 1);
		ASSERT_EQ(st.int_2, 2);
		ASSERT_STREQ(st.str_2.c_str(), "str2");
		ASSERT_EQ(st._.GetUnknown().MemberCount(), 0u);
	}

	// error when unknown
	{
		int ret = st._.Deserialize(validJson, DeserFlag::kErrorWhenUnknown, 0);
		const int retExpect = ErrorCode::kDeserializeSomeUnknownMember;
		ASSERT_EQ(ret, retExpect);
	}
}

const string jsonSimpleTest = "{\"int_val_1\": 1, \"int_val_2\": 2, "
	"\"str_val_1\":\"s1\", \"str_val_2\":\"s2\", \"addi_field\": \"a_field\"}";
void CheckStruct(int deserRet, const SimpleSt& st)
{
	ASSERT_EQ(deserRet, 0);
	ASSERT_EQ(st.int_1, 1);
	ASSERT_EQ(st.int_2, 2);
	ASSERT_STREQ(st.str_1.c_str(), "s1");
	ASSERT_STREQ(st.str_2.c_str(), "s2");
	ASSERT_EQ(st._.GetUnknown().MemberCount(), 1u);
	ASSERT_STREQ(st._.GetUnknown()["addi_field"].GetString(), "a_field");
}

TEST(Deserialize, DeserializeAPI)
{
	SimpleSt st;
	int ret;

	//--- Deserialize with cstr
	ret = st._.Deserialize(jsonSimpleTest.c_str());
	CheckStruct(ret, st);

	ret = st._.Deserialize<rapidjson::kParseDefaultFlags>(jsonSimpleTest.c_str());
	CheckStruct(ret, st);

	ret = st._.Deserialize<rapidjson::kParseDefaultFlags, rapidjson::UTF8<> >(jsonSimpleTest.c_str());
	CheckStruct(ret, st);

	//--- Deserialize with cstr and length
	ret = st._.Deserialize(jsonSimpleTest.data(), jsonSimpleTest.length());
	CheckStruct(ret, st);

	ret = st._.Deserialize<rapidjson::kParseDefaultFlags>(jsonSimpleTest.data(), jsonSimpleTest.length());
	CheckStruct(ret, st);

	ret = st._.Deserialize<rapidjson::kParseDefaultFlags, rapidjson::UTF8<> >(jsonSimpleTest.data(), jsonSimpleTest.length());
	CheckStruct(ret, st);

	//--- Deserialize with std::string
	ret = st._.Deserialize(jsonSimpleTest);
	CheckStruct(ret, st);

	string errMsg;
	ret = st._.Deserialize(jsonSimpleTest, errMsg);
	CheckStruct(ret, st);

	//--- Some test of other encoding in done in test Deserialize::Encoding
}

const string encodingTestJson = "{\"int_val\": 1, \"str_val\": \"sv\", \"map_val\": {\"k\": 2} }";
template<typename TestStruct>
void CheckEncodingTestResult(int retCode, TestStruct& st)
{
	typedef typename TestStruct::_ijst_Encoding Encoding;
	std::basic_string<typename Encoding::Ch> (*FuncTrans)(const char*) = Transcode<rapidjson::UTF8<>, Encoding>;
	ASSERT_EQ(retCode, 0);
	ASSERT_EQ(st.int_v, 1);
	AssertStrEq(st.str_v.c_str(), FuncTrans("sv").c_str());
	ASSERT_EQ(st.map_v.size(), 1u);
	ASSERT_EQ(st.map_v[FuncTrans("k")], 2);
}

template<typename TestStruct, typename SourceEncoding>
void DoTestEncodingDeserialize()
{
	// encoding string
	std::basic_string<typename SourceEncoding::Ch> source = Transcode<rapidjson::UTF8<>, SourceEncoding>(encodingTestJson.c_str());

	TestStruct st;
	int ret = st._.template Deserialize<rapidjson::kParseDefaultFlags, SourceEncoding>(source.c_str());
	CheckEncodingTestResult(ret, st);
};

template<typename TestStruct>
void DoTestEncodingDeserializeWithSameEncoding()
{
	// encoding string
	typedef typename TestStruct::_ijst_Encoding Encoding;
	std::basic_string<typename Encoding::Ch> source = Transcode<rapidjson::UTF8<>, Encoding>(encodingTestJson.c_str());

	TestStruct st;
	int ret = st._.Deserialize(source.c_str());
	CheckEncodingTestResult(ret, st);
};

template<typename TestStruct>
void TestDeserializeEncoding()
{
	DoTestEncodingDeserializeWithSameEncoding<TestStruct>();

	DoTestEncodingDeserialize<TestStruct, rapidjson::UTF8<> >();
	DoTestEncodingDeserialize<TestStruct, rapidjson::UTF16<> >();
	DoTestEncodingDeserialize<TestStruct, rapidjson::UTF16LE<> >();
	DoTestEncodingDeserialize<TestStruct, rapidjson::UTF16BE<> >();
	DoTestEncodingDeserialize<TestStruct, rapidjson::UTF32<> >();
	DoTestEncodingDeserialize<TestStruct, rapidjson::UTF32LE<> >();
	DoTestEncodingDeserialize<TestStruct, rapidjson::UTF32BE<> >();
};

TEST(Deserialize, Encoding)
{
	TestDeserializeEncoding<U8TestEncoding>();
#if __cplusplus >= 201103L
	TestDeserializeEncoding<U16TestEncoding>();
	TestDeserializeEncoding<U32TestEncoding>();
#endif

	// special test of Deserialize with string length
	// this test is only valuable if source encoding is UTF8<> due to the bug of rapidJSON
	{
		U8TestEncoding st;
		int ret = st._.Deserialize<rapidjson::kParseDefaultFlags>(encodingTestJson.c_str(), encodingTestJson.length());
		CheckEncodingTestResult(ret, st);
	}
#if __cplusplus >= 201103L
	{
		U16TestEncoding st;
		int ret = st._.Deserialize<rapidjson::kParseDefaultFlags, rapidjson::UTF8<> >(encodingTestJson.c_str(), encodingTestJson.length());
		CheckEncodingTestResult(ret, st);
	}
	{
		U32TestEncoding st;
		int ret = st._.template Deserialize<rapidjson::kParseDefaultFlags, rapidjson::UTF8<> >(encodingTestJson.c_str(), encodingTestJson.length());
		CheckEncodingTestResult(ret, st);
	}
#endif
}

TEST(Deserialize, FromJson)
{
	SimpleSt st;
	{
		rapidjson::Document doc;
		{
			string validJson = "{\"int_val_2\":1, \"str_val_2\":\"str2\", \"addi_field\":\"a_field\"}";
			doc.Parse(validJson.c_str(), validJson.length());
			ASSERT_FALSE(doc.HasParseError());
		}
		int ret = st._.FromJson(doc);
		ASSERT_EQ(ret, 0);
		// Check src
		ASSERT_EQ(doc["int_val_2"].GetInt(), 1);
		ASSERT_STREQ(doc["str_val_2"].GetString(), "str2");
	}

	// Check st
	ASSERT_EQ(st.int_2, 1);
	ASSERT_STREQ(st.str_2.c_str(), "str2");
}

TEST(Deserialize, MoveFromJson)
{
	SimpleSt st;
	rapidjson::Document doc;
	{
		string validJson = "{\"int_val_2\":1, \"str_val_2\":\"str2\", \"addi_field\":\"a_field\"}";
		doc.Parse(validJson.c_str(), validJson.length());
		ASSERT_FALSE(doc.HasParseError());
	}
	int ret = st._.MoveFromJson(doc);
	ASSERT_EQ(ret, 0);

	// Check src
	ASSERT_TRUE(doc.IsNull());

	// Check st
	ASSERT_EQ(st.int_2, 1);
	ASSERT_STREQ(st.str_2.c_str(), "str2");
}

IJST_DEFINE_STRUCT(
		NullableSt
		, (T_int, int_1, "int_val_1", FDesc::Optional)
		, (T_int, int_2, "int_val_2", FDesc::Nullable)
		, (T_int, int_3, "int_val_3", FDesc::Nullable | FDesc::Optional)
)

TEST(Deserialize, NullValue)
{
	NullableSt st;
	int ret;
	int retExpected;

	// Empty
	{
		string json = "{}";
		ret = st._.Deserialize(json);
		retExpected = ErrorCode::kDeserializeSomeFieldsInvalid;
		ASSERT_EQ(ret, retExpected);
	}

	// require filed is null
	{
		string json = "{\"int_val_2\": null}";
		ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(IJST_GET_STATUS(st, int_2), (EFStatus)FStatus::kNull);
	}

	// require filed is valid
	{
		string json = "{\"int_val_2\": 2}";
		ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.int_2, 2);
	}

	// optional filed is null
	{
		string json = "{\"int_val_2\": 2, \"int_val_1\": null}";
		ret = st._.Deserialize(json);
		retExpected = ErrorCode::kDeserializeValueTypeError;
		ASSERT_EQ(ret, retExpected);
	}

	// optional | nullable filed is null
	{
		string json = "{\"int_val_2\": 2, \"int_val_3\": null}";
		ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.int_2, 2);
		ASSERT_EQ(IJST_GET_STATUS(st, int_3), (EFStatus)FStatus::kNull);
	}

	// optional | nullable filed is valid
	{
		string json = "{\"int_val_2\": 2, \"int_val_3\": 3}";
		ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.int_2, 2);
		ASSERT_EQ(st.int_3, 3);
	}
}


IJST_DEFINE_STRUCT(
		NotEmptySt
		, (IJST_TVEC(T_int), vec_v, "vec", FDesc::NotDefault | FDesc::Optional)
		, (IJST_TDEQUE(T_int), deq_v, "deq", FDesc::NotDefault | FDesc::Optional)
		, (IJST_TLIST(T_int), list_v, "list", FDesc::NotDefault | FDesc::Optional)
		, (IJST_TMAP(T_int), map_v, "map", FDesc::NotDefault | FDesc::Optional)
		, (IJST_TOBJ(T_int), obj_v, "obj", FDesc::NotDefault | FDesc::Optional)
)

void CheckNotEmpty(const string& validJson, const string& fieldEmptyJson, const char* emptyMemberName, const char* emptyJsonKey)
{
	// valid
	{
		NotEmptySt st;
		int ret = st._.Deserialize(validJson);
		ASSERT_EQ(ret, 0);
	}
	// empty
	{
		NotEmptySt st;
		string errMsg;
		int ret = st._.Deserialize(fieldEmptyJson, errMsg);
		CheckMemberValueIsDefault(emptyMemberName, emptyJsonKey, ret, errMsg);
	}
}
TEST(Deserialize, NotEmpty)
{

	// Field empty
	{
		NotEmptySt st;
		int ret;
		string json = "{}";
		ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
	}

	{
		CheckNotEmpty("{\"vec\": [0]}",
					  "{\"vec\": []}",
					  "vec_v", "vec");
		CheckNotEmpty("{\"deq\": [0]}",
					  "{\"deq\": []}",
					  "deq_v", "deq");
		CheckNotEmpty("{\"list\": [0]}",
					  "{\"list\": []}",
					  "list_v", "list");
		CheckNotEmpty("{\"map\": {\"v\": 0}}",
					  "{\"map\": {}}",
					  "map_v", "map");
		CheckNotEmpty("{\"obj\": {\"v\": 0}}",
					  "{\"obj\": {}}",
					  "obj_v", "obj");
	}
}

TEST(Deserialize, ParseFlags)
{
	// comment
	{
		string json = "{/*this is a comment*/}";
		NotEmptySt st;
		int ret = st._.Deserialize<rapidjson::kParseCommentsFlag>(json.c_str(), json.length());
		ASSERT_EQ(ret, 0);
	}
	// trailing comma
	{
		string json = "{\"v\": 0, }";
		NotEmptySt st;
		int ret = st._.Deserialize<rapidjson::kParseTrailingCommasFlag>(json.c_str(), json.length());
		ASSERT_EQ(ret, 0);
	}
}

IJST_DEFINE_STRUCT(
		StEmpty
)

IJST_DEFINE_STRUCT(
		StAllocShrink
		, (IJST_TST(StEmpty), empty, "val", 0)
		, (IJST_TVEC(StEmpty), vecEmpty, "vec", 0)
		, (IJST_TDEQUE(StEmpty), deqEmpty, "deq", 0)
		, (IJST_TLIST(StEmpty), listEmpty, "list", 0)
		, (IJST_TMAP(StEmpty), mapEmpty, "map", 0)
		, (IJST_TOBJ(StEmpty), objEmpty, "obj", 0)
		, (T_raw, raw, "raw", 0)
)

#define UTEST_ASSERT_USER_OWN_ALLOCATOR(st)		\
	ASSERT_EQ(&(st)._.GetAllocator(), &(st)._.GetOwnAllocator())

void CheckUseOwnAllocator(StAllocShrink& st)
{
	UTEST_ASSERT_USER_OWN_ALLOCATOR(st.empty);
	UTEST_ASSERT_USER_OWN_ALLOCATOR(st.vecEmpty[0]);
	UTEST_ASSERT_USER_OWN_ALLOCATOR(st.vecEmpty[1]);
	UTEST_ASSERT_USER_OWN_ALLOCATOR(st.deqEmpty[0]);
	UTEST_ASSERT_USER_OWN_ALLOCATOR(st.deqEmpty[1]);
	UTEST_ASSERT_USER_OWN_ALLOCATOR(st.listEmpty.front());
	UTEST_ASSERT_USER_OWN_ALLOCATOR(st.listEmpty.back());
	UTEST_ASSERT_USER_OWN_ALLOCATOR(st.mapEmpty["k1"]);
	UTEST_ASSERT_USER_OWN_ALLOCATOR(st.mapEmpty["k11"]);
	UTEST_ASSERT_USER_OWN_ALLOCATOR(st.objEmpty[0].value);
	UTEST_ASSERT_USER_OWN_ALLOCATOR(st.objEmpty[1].value);
	ASSERT_EQ(&st.raw.GetOwnAllocator(), &st.raw.GetAllocator());
}

void CheckUseParentAllocatorAndShrink(StAllocShrink &st, const rapidjson::Value &srcJson)
{
	//check use parend allocator
	rapidjson::MemoryPoolAllocator<>* parentAllocator = &st._.GetAllocator();
	ASSERT_EQ(parentAllocator, &st.empty._.GetAllocator());
	ASSERT_EQ(parentAllocator, &st.vecEmpty[0]._.GetAllocator());
	ASSERT_EQ(parentAllocator, &st.vecEmpty[1]._.GetAllocator());
	ASSERT_EQ(parentAllocator, &st.deqEmpty[0]._.GetAllocator());
	ASSERT_EQ(parentAllocator, &st.deqEmpty[1]._.GetAllocator());
	ASSERT_EQ(parentAllocator, &st.listEmpty.front()._.GetAllocator());
	ASSERT_EQ(parentAllocator, &st.listEmpty.back()._.GetAllocator());
	ASSERT_EQ(parentAllocator, &st.mapEmpty["k1"]._.GetAllocator());
	ASSERT_EQ(parentAllocator, &st.mapEmpty["k11"]._.GetAllocator());
	ASSERT_EQ(parentAllocator, &st.objEmpty[0].value._.GetAllocator());
	ASSERT_EQ(parentAllocator, &st.objEmpty[1].value._.GetAllocator());
	ASSERT_EQ(parentAllocator, &st.raw.GetAllocator());

	// Shrink and check
	st._.ShrinkAllocator();
	ASSERT_EQ(st._.GetAllocator().Size(), 0u);
	st._.GetAllocator().Clear();
	st._.GetOwnAllocator().Clear();
	CheckUseOwnAllocator(st);

	// Serialize and check
	std::string destJson;
	st._.Serialize(destJson);

	rapidjson::Document destDoc;
	destDoc.Parse(destJson.c_str());
	ASSERT_FALSE(destDoc.HasParseError());
	ASSERT_EQ(srcJson, (rapidjson::Value&)destDoc);
}

TEST(Deserialize, Allocator)
{
	const std::string srcJson =
			"{"
				"\"val\": {\"unk1\": \"v1\"}, "
				"\"vec\": [{\"unk2\": \"v2\"}, {\"unk22\": \"v22\"}], "
				"\"deq\": [{\"unk3\": \"v3\"}, {\"unk33\": \"v33\"}], "
				"\"list\": [{\"unk4\": \"v4\"}, {\"unk44\": \"v44\"}], "
				"\"map\": {\"k1\": {\"unk5\": \"v5\"}, \"k11\": {\"unk55\": \"v55\"}}, "
				"\"obj\": {\"k2\": {\"unk6\": \"v6\"}, \"k22\": {\"unk66\": \"v66\"}}, "
				"\"raw\": \"v7\""
			"}";

	rapidjson::Document srcDoc;
	srcDoc.Parse(srcJson.c_str());
	ASSERT_FALSE(srcDoc.HasParseError());

	//*** Deserialize by copy unknown
	{
		StAllocShrink st;
		int ret = st._.Deserialize(srcJson);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st._.GetAllocator().Size(), 0u);

		CheckUseOwnAllocator(st);
	}

	//*** Deserialize by move unknown
	{
		StAllocShrink st;
		int ret = st._.Deserialize(srcJson, DeserFlag::kMoveFromIntermediateDoc);
		ASSERT_EQ(ret, 0);
		ASSERT_GT(st._.GetAllocator().Size(), 0u);

		CheckUseParentAllocatorAndShrink(st, srcDoc);
	}

	//*** FromJson
	{
		StAllocShrink st;
		int ret = st._.FromJson(srcDoc);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st._.GetAllocator().Size(), 0u);

		CheckUseOwnAllocator(st);
	}

	//*** MoveFromJson
	{
		StAllocShrink st;
		{
			rapidjson::Document tempDoc;
			tempDoc.CopyFrom(srcDoc, tempDoc.GetAllocator());
			int ret = st._.MoveFromJson(tempDoc);
			ASSERT_EQ(ret, 0);
			// tempDoc is destroyed here
		}
		ASSERT_GT(st._.GetAllocator().Size(), 0u);

		CheckUseParentAllocatorAndShrink(st, srcDoc);
	}
}

TEST(Deserialize, ErrDoc_MemberMissing)
{
	string json = "{}";
	const int retExpected = ErrorCode::kDeserializeSomeFieldsInvalid;
	NullableSt st;
	string errMsg;
	int ret = st._.Deserialize(json, errMsg);
	ASSERT_EQ(ret, retExpected);
	rapidjson::Document errDoc;
	errDoc.Parse(errMsg.c_str(), errMsg.length());
	ASSERT_TRUE(errDoc.IsObject());
	ASSERT_STREQ(errDoc["type"].GetString(), "MissingMember");
	ASSERT_EQ(errDoc["members"].Size(), 1u);
	ASSERT_STREQ(errDoc["members"][0].GetString(), "int_val_2");
}

IJST_DEFINE_STRUCT(
		StErrCheck
		, (T_ubool, v, "f_v", FDesc::Optional)
		, (IJST_TMAP(T_ubool), map_v, "f_map", FDesc::Optional)
		, (IJST_TOBJ(T_ubool), obj_v, "f_obj", FDesc::Optional)
		, (IJST_TVEC(T_ubool), vec_v, "f_vec", FDesc::Optional)
		, (IJST_TDEQUE(T_ubool), deq_v, "f_deq", FDesc::Optional)
		, (IJST_TLIST(T_ubool), list_v, "f_list", FDesc::Optional)
)

void TestErrCheckTypeError(const std::string& errJson, const char* memberName, const char* jsonKey, const char* expectedType, const char* jsonVal)
{
	const int kErrTypeError = ErrorCode::kDeserializeValueTypeError;
	StErrCheck st;
	string errMsg;
	int ret = st._.Deserialize(errJson, errMsg);
	ASSERT_EQ(ret, kErrTypeError);
	CheckMemberTypeMismatch(errMsg, memberName, jsonKey, expectedType, jsonVal);
}

TEST(Deserialize, ErrorDoc_ContainerTypeError)
{
	// Type error object
	const int kErrTypeError = ErrorCode::kDeserializeValueTypeError;
	{
		const string json = "[1]";
		StErrCheck st;
		rapidjson::Document errDoc;
		int ret = st._.Deserialize(json, DeserFlag::kNoneFlag, &errDoc);
		ASSERT_EQ(ret, kErrTypeError);
		CheckTypeMismatch(errDoc, "object", "[1]");
	}

	// Type error bool
	{
		const string json = "{\"f_v\": \"ERROR_HERE\"}";
		TestErrCheckTypeError(json, "v", "f_v", "bool", "\"ERROR_HERE\"");
	}

	// Type error map
	{
		const string json = "{\"f_map\": false}";
		TestErrCheckTypeError(json, "map_v", "f_map", "object", "false");
	}

	// Type error object
	{
		const string json = "{\"f_obj\": null}";
		TestErrCheckTypeError(json, "obj_v", "f_obj", "object", "null");
	}

	// Type error vec
	{
		const string json = "{\"f_vec\": true}";
		TestErrCheckTypeError(json, "vec_v", "f_vec", "array", "true");
	}

	// Type error deq
	{
		const string json = "{\"f_deq\": false}";
		TestErrCheckTypeError(json, "deq_v", "f_deq", "array", "false");
	}

	// Type error list
	{
		const string json = "{\"f_list\": true}";
		TestErrCheckTypeError(json, "list_v", "f_list", "array", "true");
	}
}

void TestErrCheckCommonError(const string& json, int retExpected, rapidjson::Document& errDoc, DeserFlag::Flag deserFlag = DeserFlag::kNoneFlag)
{
	StErrCheck st;
	string errMsg;
	int ret = st._.Deserialize(json, errMsg, deserFlag);
	ASSERT_EQ(ret, retExpected);
	errDoc.Parse(errMsg.c_str(), errMsg.length());
	ASSERT_TRUE(errDoc.IsObject());
}

TEST(Deserialize, ErrDoc)
{
	// Parse Error
	{
		const string json = "ThisIsAErrJson";
		rapidjson::Document errDoc;
		TestErrCheckCommonError(json, ErrorCode::kDeserializeParseFailed, errDoc);
		ASSERT_STREQ(errDoc["type"].GetString(), "ParseError");
		ASSERT_EQ(errDoc["errCode"].GetInt(), (int)rapidjson::kParseErrorValueInvalid);

		// Parse insitu
		char *jsonBuf = new char[json.length()];
		memcpy(jsonBuf, json.c_str(), json.length());
		StErrCheck st;
		string errMsg;
		int ret = st._.Deserialize(json, errMsg);
		const int retExpected = ErrorCode::kDeserializeParseFailed;
		ASSERT_EQ(ret, retExpected);
		errDoc.Parse(errMsg.c_str(), errMsg.length());
		ASSERT_TRUE(errDoc.IsObject());
		ASSERT_STREQ(errDoc["type"].GetString(), "ParseError");
		ASSERT_EQ(errDoc["errCode"].GetInt(), (int)rapidjson::kParseErrorValueInvalid);
	}

	// Member Unknown
	{
		// Copy unknown
		const string json = "{\"UNKNOWN\": true }";
		rapidjson::Document errDoc;
		TestErrCheckCommonError(json, ErrorCode::kDeserializeSomeUnknownMember, errDoc, DeserFlag::kErrorWhenUnknown);
		ASSERT_STREQ(errDoc["type"].GetString(), "UnknownMember");
		ASSERT_STREQ(errDoc["jsonKey"].GetString(), "UNKNOWN");

		// Move unknown
		TestErrCheckCommonError(json, ErrorCode::kDeserializeSomeUnknownMember, errDoc,
								DeserFlag::kErrorWhenUnknown | DeserFlag::kMoveFromIntermediateDoc);
		ASSERT_STREQ(errDoc["type"].GetString(), "UnknownMember");
		ASSERT_STREQ(errDoc["jsonKey"].GetString(), "UNKNOWN");
	}

	// Error in object has been checked in many places

	// Error in map
	{
		const string json = "{\"f_map\": {\"v1\": \"ERROR_HERE\"} }";
		rapidjson::Document errDoc;
		TestErrCheckCommonError(json, ErrorCode::kDeserializeValueTypeError, errDoc);
		ASSERT_STREQ(errDoc["type"].GetString(), "ErrInObject");
		ASSERT_STREQ(errDoc["member"].GetString(), "map_v");
		ASSERT_STREQ(errDoc["jsonKey"].GetString(), "f_map");
		ASSERT_STREQ(errDoc["err"]["type"].GetString(), "ErrInMap");
		ASSERT_STREQ(errDoc["err"]["member"].GetString(), "v1");
		CheckTypeMismatch(errDoc["err"]["err"], "bool", "\"ERROR_HERE\"");
	}

	// Map key duplicated
	{
		const string json = "{\"f_map\": {\"v2\": true, \"v2\": true} }";
		rapidjson::Document errDoc;
		TestErrCheckCommonError(json, ErrorCode::kDeserializeMapKeyDuplicated, errDoc);
		ASSERT_STREQ(errDoc["type"].GetString(), "ErrInObject");
		ASSERT_STREQ(errDoc["member"].GetString(), "map_v");
		ASSERT_STREQ(errDoc["jsonKey"].GetString(), "f_map");
		ASSERT_STREQ(errDoc["err"]["type"].GetString(), "MapKeyDuplicated");
		ASSERT_STREQ(errDoc["err"]["key"].GetString(), "v2");
	}

	// Error in object
	{
		const string json = "{\"f_obj\": {\"o0\": null} }";
		rapidjson::Document errDoc;
		TestErrCheckCommonError(json, ErrorCode::kDeserializeValueTypeError, errDoc);
		ASSERT_STREQ(errDoc["type"].GetString(), "ErrInObject");
		ASSERT_STREQ(errDoc["member"].GetString(), "obj_v");
		ASSERT_STREQ(errDoc["jsonKey"].GetString(), "f_obj");
		ASSERT_STREQ(errDoc["err"]["type"].GetString(), "ErrInMap");
		ASSERT_STREQ(errDoc["err"]["member"].GetString(), "o0");
		CheckTypeMismatch(errDoc["err"]["err"], "bool", "null");
	}

	// Error in array
	{
		const string json = "{\"f_vec\": [1024]}";
		rapidjson::Document errDoc;
		TestErrCheckCommonError(json, ErrorCode::kDeserializeValueTypeError, errDoc);
		ASSERT_STREQ(errDoc["type"].GetString(), "ErrInObject");
		ASSERT_STREQ(errDoc["member"].GetString(), "vec_v");
		ASSERT_STREQ(errDoc["jsonKey"].GetString(), "f_vec");
		ASSERT_STREQ(errDoc["err"]["type"].GetString(), "ErrInArray");
		ASSERT_EQ(errDoc["err"]["index"].GetInt(), 0);
		CheckTypeMismatch(errDoc["err"]["err"], "bool", "1024");
	}

	// Error in deq
	{
		const string json = "{\"f_deq\": [2048]}";
		rapidjson::Document errDoc;
		TestErrCheckCommonError(json, ErrorCode::kDeserializeValueTypeError, errDoc);
		ASSERT_STREQ(errDoc["type"].GetString(), "ErrInObject");
		ASSERT_STREQ(errDoc["member"].GetString(), "deq_v");
		ASSERT_STREQ(errDoc["jsonKey"].GetString(), "f_deq");
		ASSERT_STREQ(errDoc["err"]["type"].GetString(), "ErrInArray");
		ASSERT_EQ(errDoc["err"]["index"].GetInt(), 0);
		CheckTypeMismatch(errDoc["err"]["err"], "bool", "2048");
	}

	// Error in list
	{
		const string json = "{\"f_list\": [4096]}";
		rapidjson::Document errDoc;
		TestErrCheckCommonError(json, ErrorCode::kDeserializeValueTypeError, errDoc);
		ASSERT_STREQ(errDoc["type"].GetString(), "ErrInObject");
		ASSERT_STREQ(errDoc["member"].GetString(), "list_v");
		ASSERT_STREQ(errDoc["jsonKey"].GetString(), "f_list");
		ASSERT_STREQ(errDoc["err"]["type"].GetString(), "ErrInArray");
		ASSERT_EQ(errDoc["err"]["index"].GetInt(), 0);
		CheckTypeMismatch(errDoc["err"]["err"], "bool", "4096");
	}

}
