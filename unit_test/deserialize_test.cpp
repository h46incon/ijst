//
// Created by h46incon on 2017/9/30.
//
#include <gtest/gtest.h>
#include <ijst/ijst.h>
#include <ijst/types_std.h>

using std::vector;
using std::map;
using std::string;

IJST_DEFINE_STRUCT(
		SimpleSt
		, (IJST_TPRI(Int), int_1, "int_val_1", ijst::FDesc::Optional)
		, (IJST_TPRI(Int), int_2, "int_val_2", 0)
		, (IJST_TPRI(Str), str_1, "str_val_1", ijst::FDesc::Optional)
		, (IJST_TPRI(Str), str_2, "str_val_2", 0)
)

TEST(Deserialize, Empty)
{
	string emptyJson = "{}";
	SimpleSt st;
	int ret = st._.Deserialize(emptyJson, 0);
	int retExpected = ijst::Err::kDeserializeSomeFiledsInvalid;
	ASSERT_EQ(ret, retExpected);
}


TEST(Deserialize, ParseError)
{
	string errJson = "{withoutQuote:1}";
	SimpleSt st;
	int ret = st._.Deserialize(errJson, 0);
	int retExpected = ijst::Err::kDeserializeParseFaild;
	ASSERT_EQ(ret, retExpected);
}

TEST(Deserialize, TypeError)
{
	string errJson = "{\"int_val_2\":\"str\"}";
	SimpleSt st;
	int ret = st._.Deserialize(errJson, 0);
	int retExpected = ijst::Err::kDeserializeValueTypeError;
	ASSERT_EQ(ret, retExpected);
}

TEST(Deserialize, RequiredFields)
{
	string validJson = "{\"int_val_2\":1, \"str_val_2\":\"str2\"}";
	SimpleSt st;
	int ret = st._.Deserialize(validJson, 0);
	ASSERT_EQ(ret, 0);

	ASSERT_EQ(IJST_GET_STATUS(st, int_1), ijst::FStatus::kMissing);
	ASSERT_EQ(IJST_GET_STATUS(st, str_1), ijst::FStatus::kMissing);
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
		int ret = st._.Deserialize(validJson, 0);
		ASSERT_EQ(ret, 0);

		ASSERT_EQ(IJST_GET_STATUS(st, int_1), ijst::FStatus::kValid);
		ASSERT_EQ(IJST_GET_STATUS(st, str_1), ijst::FStatus::kMissing);
		ASSERT_EQ(st.int_1, 1);
		ASSERT_EQ(st.int_2, 2);
		ASSERT_STREQ(st.str_2.c_str(), "str2");
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 3u);
		ASSERT_STREQ(st._.GetBuffer()["addi_field1"].GetString(), "a_field1");
		ASSERT_STREQ(st._.GetBuffer()["addi_field2"].GetString(), "a_field2");
		ASSERT_STREQ(st._.GetBuffer()["addi_field3"].GetString(), "a_field3");
	}

	// throw unknown
	{
		int ret = st._.Deserialize(validJson, 0, ijst::UnknownMode::kIgnore);
		ASSERT_EQ(ret, 0);

		ASSERT_EQ(IJST_GET_STATUS(st, int_1), ijst::FStatus::kValid);
		ASSERT_EQ(IJST_GET_STATUS(st, str_1), ijst::FStatus::kMissing);
		ASSERT_EQ(st.int_1, 1);
		ASSERT_EQ(st.int_2, 2);
		ASSERT_STREQ(st.str_2.c_str(), "str2");
		ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);
	}

	// error when unknown
	{
		int ret = st._.Deserialize(validJson, 0, ijst::UnknownMode::kError);
		const int retExpect = ijst::Err::kSomeUnknownMember;
		ASSERT_EQ(ret, retExpect);
	}
}

TEST(Deserialize, CopySrc)
{
	SimpleSt st;
	{
		rapidjson::Document doc;
		{
			string validJson = "{\"int_val_2\":1, \"str_val_2\":\"str2\", \"addi_field\":\"a_field\"}";
			doc.Parse(validJson.c_str(), validJson.length());
			ASSERT_FALSE(doc.HasParseError());
		}
		int ret = st._.Deserialize(doc, 0);
		ASSERT_EQ(ret, 0);
		// Check src
		ASSERT_EQ(doc["int_val_2"].GetInt(), 1);
		ASSERT_STREQ(doc["str_val_2"].GetString(), "str2");
	}

	// Check st
	ASSERT_EQ(st.int_2, 1);
	ASSERT_STREQ(st.str_2.c_str(), "str2");
}

TEST(Deserialize, MoveSrc)
{
	SimpleSt st;
	rapidjson::Document doc;
	{
		string validJson = "{\"int_val_2\":1, \"str_val_2\":\"str2\", \"addi_field\":\"a_field\"}";
		doc.Parse(validJson.c_str(), validJson.length());
		ASSERT_FALSE(doc.HasParseError());
	}
	int ret = st._.MoveDeserialize(doc, 0);
	ASSERT_EQ(ret, 0);

	// Check src
	ASSERT_TRUE(doc.IsNull());

	// Check st
	ASSERT_EQ(st.int_2, 1);
	ASSERT_STREQ(st.str_2.c_str(), "str2");
}

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
		, (IJST_TPRI(Int), int_1, "int_val_1", ijst::FDesc::Optional)
		, (IJST_TPRI(Int), int_2, "int_val_2", ijst::FDesc::Nullable)
		, (IJST_TPRI(Int), int_3, "int_val_3", ijst::FDesc::Nullable | ijst::FDesc::Optional)
)

TEST(Deserialize, NullValue)
{
	NullableSt st;
	int ret;
	int retExpected;

	// Empty
	{
		string json = "{}";
		ret = st._.Deserialize(json, 0);
		retExpected = ijst::Err::kDeserializeSomeFiledsInvalid;
		ASSERT_EQ(ret, retExpected);
	}

	// require filed is null
	{
		string json = "{\"int_val_2\": null}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(IJST_GET_STATUS(st, int_2), ijst::FStatus::kNull);
	}

	// require filed is valid
	{
		string json = "{\"int_val_2\": 2}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.int_2, 2);
	}

	// optional filed is null
	{
		string json = "{\"int_val_2\": 2, \"int_val_1\": null}";
		ret = st._.Deserialize(json, 0);
		retExpected = ijst::Err::kDeserializeValueTypeError;
		ASSERT_EQ(ret, retExpected);
	}

	// optional | nullable filed is null
	{
		string json = "{\"int_val_2\": 2, \"int_val_3\": null}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.int_2, 2);
		ASSERT_EQ(IJST_GET_STATUS(st, int_3), ijst::FStatus::kNull);
	}

	// optional | nullable filed is valid
	{
		string json = "{\"int_val_2\": 2, \"int_val_3\": 3}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(st.int_2, 2);
		ASSERT_EQ(st.int_3, 3);
	}
}


IJST_DEFINE_STRUCT(
		NotEmptySt
		, (IJST_TVEC(IJST_TPRI(Int)), vec_v, "vec", ijst::FDesc::ElemNotEmpty | ijst::FDesc::Optional)
		, (IJST_TMAP(IJST_TPRI(Int)), map_v, "map", ijst::FDesc::ElemNotEmpty | ijst::FDesc::Optional)
)

TEST(Deserialize, NotEmpty)
{
	NotEmptySt st;
	int ret;
	int retExpected;

	// Field empty
	{
		string json = "{}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
	}

	// vec elem empty
	{
		string json = "{\"vec\": [], \"map\": {\"v\": 1}}";
		ret = st._.Deserialize(json, 0);
		retExpected = ijst::Err::kDeserializeElemEmpty;
		ASSERT_EQ(ret, retExpected);
	}

	// map elem empty
	{
		string json = "{\"vec\": [1], \"map\": {}}";
		ret = st._.Deserialize(json, 0);
		retExpected = ijst::Err::kDeserializeElemEmpty;
		ASSERT_EQ(ret, retExpected);
	}

	// vec valid
	{
		string json = "{\"vec\": [1]}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
	}

	// map valid
	{
		string json = "{\"map\": {\"v\": 1}}";
		ret = st._.Deserialize(json, 0);
		ASSERT_EQ(ret, 0);
	}
}
