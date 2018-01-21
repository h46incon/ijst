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
	int retExpected = ErrorCode::kDeserializeSomeFiledsInvalid;
	ASSERT_EQ(ret, retExpected);
}

TEST(Deserialize, IgnoeFieldStatus)
{
	string emptyJson = "{}";
	// Deserialize
	{
		SimpleSt st;
		int ret = st._.Deserialize(emptyJson, UnknownMode::kKeep, false);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(IJST_GET_STATUS(st, int_2), FStatus::kMissing);
	}
	// DeserializeInsitu
	{
		char * buf = new char [emptyJson.size() + 1];
		memcpy(buf, emptyJson.c_str(), emptyJson.size());
		buf[emptyJson.size()] = '\0';
		SimpleSt st;
		int ret = st._.DeserializeInsitu(buf, UnknownMode::kKeep, false);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(IJST_GET_STATUS(st, int_2), FStatus::kMissing);
	}
#if IJST_ENABLE_FROM_JSON_OBJECT
	// From Json
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		SimpleSt st;
		int ret = st._.FromJson(doc, UnknownMode::kKeep, false);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(IJST_GET_STATUS(st, int_2), FStatus::kMissing);
	}
	// Move From Json
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		SimpleSt st;
		int ret = st._.MoveFromJson(doc, UnknownMode::kKeep, false);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(IJST_GET_STATUS(st, int_2), FStatus::kMissing);
	}
#endif
}


TEST(Deserialize, ParseError)
{
	string errJson = "{withoutQuote:1}";
	SimpleSt st;
	int ret = st._.Deserialize(errJson);
	int retExpected = ErrorCode::kDeserializeParseFaild;
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

	ASSERT_EQ(IJST_GET_STATUS(st, int_1), FStatus::kMissing);
	ASSERT_EQ(IJST_GET_STATUS(st, str_1), FStatus::kMissing);
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

		ASSERT_EQ(IJST_GET_STATUS(st, int_1), FStatus::kValid);
		ASSERT_EQ(IJST_GET_STATUS(st, str_1), FStatus::kMissing);
		ASSERT_EQ(st.int_1, 1);
		ASSERT_EQ(st.int_2, 2);
		ASSERT_STREQ(st.str_2.c_str(), "str2");
		ASSERT_EQ(st._.GetUnknown().MemberCount(), 3u);
		ASSERT_STREQ(st._.GetUnknown()["addi_field1"].GetString(), "a_field1");
		ASSERT_STREQ(st._.GetUnknown()["addi_field2"].GetString(), "a_field2");
		ASSERT_STREQ(st._.GetUnknown()["addi_field3"].GetString(), "a_field3");
	}

	// throw unknown
	{
		int ret = st._.Deserialize(validJson, UnknownMode::kIgnore, 0);
		ASSERT_EQ(ret, 0);

		ASSERT_EQ(IJST_GET_STATUS(st, int_1), FStatus::kValid);
		ASSERT_EQ(IJST_GET_STATUS(st, str_1), FStatus::kMissing);
		ASSERT_EQ(st.int_1, 1);
		ASSERT_EQ(st.int_2, 2);
		ASSERT_STREQ(st.str_2.c_str(), "str2");
		ASSERT_EQ(st._.GetUnknown().MemberCount(), 0u);
	}

	// error when unknown
	{
		int ret = st._.Deserialize(validJson, UnknownMode::kError, 0);
		const int retExpect = ErrorCode::kDeserializeSomeUnknownMember;
		ASSERT_EQ(ret, retExpect);
	}
}

#if IJST_ENABLE_FROM_JSON_OBJECT
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
#endif

TEST(Deserialize, Insitu)
{
	SimpleSt st;
	string validJson = "{\"int_val_2\":1, \"str_val_2\":\"str2\"}";
	char* buf = new char[validJson.length() + 1];
	strncpy(buf, validJson.c_str(), validJson.length() + 1);
	int ret = st._.DeserializeInsitu(buf);
	ASSERT_EQ(ret, 0);

	// Check st
	ASSERT_EQ(st.int_2, 1);
	ASSERT_STREQ(st.str_2.c_str(), "str2");
	delete[] buf;
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
		retExpected = ErrorCode::kDeserializeSomeFiledsInvalid;
		ASSERT_EQ(ret, retExpected);
	}

	// require filed is null
	{
		string json = "{\"int_val_2\": null}";
		ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(IJST_GET_STATUS(st, int_2), FStatus::kNull);
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
		ASSERT_EQ(IJST_GET_STATUS(st, int_3), FStatus::kNull);
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
		, (IJST_TVEC(T_int), vec_v, "vec", FDesc::ElemNotEmpty | FDesc::Optional)
		, (IJST_TMAP(T_int), map_v, "map", FDesc::ElemNotEmpty | FDesc::Optional)
)

TEST(Deserialize, NotEmpty)
{
	NotEmptySt st;
	int ret;
	int retExpected;

	// Field empty
	{
		string json = "{}";
		ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
	}

	// vec elem empty
	{
		string json = "{\"vec\": [], \"map\": {\"v\": 1}}";
		ret = st._.Deserialize(json);
		retExpected = ErrorCode::kDeserializeElemEmpty;
		ASSERT_EQ(ret, retExpected);
	}

	// map elem empty
	{
		string json = "{\"vec\": [1], \"map\": {}}";
		ret = st._.Deserialize(json);
		retExpected = ErrorCode::kDeserializeElemEmpty;
		ASSERT_EQ(ret, retExpected);
	}

	// vec valid
	{
		string json = "{\"vec\": [1]}";
		ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
	}

	// map valid
	{
		string json = "{\"map\": {\"v\": 1}}";
		ret = st._.Deserialize(json);
		ASSERT_EQ(ret, 0);
	}
}

TEST(Deserialize, ParseFlags)
{
	string json = "{/*this is a comment*/}";
	// Deserialize
	{
		NotEmptySt st;
		int ret = st._.Deserialize<rapidjson::kParseCommentsFlag>(json.c_str(), json.length());
		ASSERT_EQ(ret, 0);
	}

	// DeserializeInsitu
	{
		NotEmptySt st;
		char* buf = new char[json.length() + 1];
		strncpy(buf, json.c_str(), json.length() + 1);
		int ret = st._.DeserializeInsitu<rapidjson::kParseCommentsFlag>(buf);
		ASSERT_EQ(ret, 0);
		delete[] buf;
	}
}

TEST(Deserialize, ErrDoc_MemberMissing)
{
	string json = "{}";
	const int retExpected = ErrorCode::kDeserializeSomeFiledsInvalid;
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

void TestErrCheckTypeError(const std::string& errJson, const char* memberName, const char* expectedType, const char* jsonVal)
{
	const int kErrTypeError = ErrorCode::kDeserializeValueTypeError;
	StErrCheck st;
	string errMsg;
	int ret = st._.Deserialize(errJson, errMsg);
	ASSERT_EQ(ret, kErrTypeError);
	CheckMemberTypeMismatch(errMsg, memberName, expectedType, jsonVal);
}

TEST(Deserialize, ErrorDoc_ContainerTypeError)
{
	// Type error object
	const int kErrTypeError = ErrorCode::kDeserializeValueTypeError;
	{
		const string json = "[1]";
		StErrCheck st;
		rapidjson::Document errDoc;
		int ret = st._.Deserialize(json, UnknownMode::kKeep, true, &errDoc);
		ASSERT_EQ(ret, kErrTypeError);
		CheckTypeMismatch(errDoc, "object", "[1]");
	}

	// Type error bool
	{
		const string json = "{\"f_v\": \"ERROR_HERE\"}";
		TestErrCheckTypeError(json, "f_v", "bool", "\"ERROR_HERE\"");
	}

	// Type error map
	{
		const string json = "{\"f_map\": false}";
		TestErrCheckTypeError(json, "f_map", "object", "false");
	}

	// Type error object
	{
		const string json = "{\"f_obj\": null}";
		TestErrCheckTypeError(json, "f_obj", "object", "null");
	}

	// Type error vec
	{
		const string json = "{\"f_vec\": true}";
		TestErrCheckTypeError(json, "f_vec", "array", "true");
	}

	// Type error deq
	{
		const string json = "{\"f_deq\": false}";
		TestErrCheckTypeError(json, "f_deq", "array", "false");
	}

	// Type error list
	{
		const string json = "{\"f_list\": true}";
		TestErrCheckTypeError(json, "f_list", "array", "true");
	}
}

void TestErrCheckCommonError(const string& json, int retExpected, rapidjson::Document& errDoc, EUnknownMode unknownMode = UnknownMode::kKeep)
{
	StErrCheck st;
	string errMsg;
	int ret = st._.Deserialize(json, errMsg, unknownMode);
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
		TestErrCheckCommonError(json, ErrorCode::kDeserializeParseFaild, errDoc, UnknownMode::kKeep);
		ASSERT_STREQ(errDoc["type"].GetString(), "ParseError");
		ASSERT_EQ(errDoc["errCode"].GetInt(), (int)rapidjson::kParseErrorValueInvalid);

		// Parse insitu
		char *jsonBuf = new char[json.length()];
		memcpy(jsonBuf, json.c_str(), json.length());
		StErrCheck st;
		string errMsg;
		int ret = st._.Deserialize(json, errMsg);
		const int retExpected = ErrorCode::kDeserializeParseFaild;
		ASSERT_EQ(ret, retExpected);
		errDoc.Parse(errMsg.c_str(), errMsg.length());
		ASSERT_TRUE(errDoc.IsObject());
		ASSERT_STREQ(errDoc["type"].GetString(), "ParseError");
		ASSERT_EQ(errDoc["errCode"].GetInt(), (int)rapidjson::kParseErrorValueInvalid);
	}

	// Member Unknown
	{
		const string json = "{\"UNKNOWN\": true }";
		rapidjson::Document errDoc;
		TestErrCheckCommonError(json, ErrorCode::kDeserializeSomeUnknownMember, errDoc, UnknownMode::kError);
		ASSERT_STREQ(errDoc["type"].GetString(), "UnknownMember");
		ASSERT_STREQ(errDoc["member"].GetString(), "UNKNOWN");
	}

	// Error in object has been checked in many places

	// Error in map
	{
		const string json = "{\"f_map\": {\"v1\": \"ERROR_HERE\"} }";
		rapidjson::Document errDoc;
		TestErrCheckCommonError(json, ErrorCode::kDeserializeValueTypeError, errDoc);
		ASSERT_STREQ(errDoc["type"].GetString(), "ErrInObject");
		ASSERT_STREQ(errDoc["member"].GetString(), "f_map");
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
		ASSERT_STREQ(errDoc["member"].GetString(), "f_map");
		ASSERT_STREQ(errDoc["err"]["type"].GetString(), "MapKeyDuplicated");
		ASSERT_STREQ(errDoc["err"]["key"].GetString(), "v2");
	}

	// Error in object
	{
		const string json = "{\"f_obj\": {\"o0\": null} }";
		rapidjson::Document errDoc;
		TestErrCheckCommonError(json, ErrorCode::kDeserializeValueTypeError, errDoc);
		ASSERT_STREQ(errDoc["type"].GetString(), "ErrInObject");
		ASSERT_STREQ(errDoc["member"].GetString(), "f_obj");
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
		ASSERT_STREQ(errDoc["member"].GetString(), "f_vec");
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
		ASSERT_STREQ(errDoc["member"].GetString(), "f_deq");
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
		ASSERT_STREQ(errDoc["member"].GetString(), "f_list");
		ASSERT_STREQ(errDoc["err"]["type"].GetString(), "ErrInArray");
		ASSERT_EQ(errDoc["err"]["index"].GetInt(), 0);
		CheckTypeMismatch(errDoc["err"]["err"], "bool", "4096");
	}

}
