//
// Created by h46incon on 2017/9/29.
//

#include "util.h"

#include <rapidjson/prettywriter.h>
#include <sstream>

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

TEST(Serialize, SerFlag)
{
	//--- Init
	Inner innerSt;
	// int_1 valid
	IJST_SET(innerSt, int_1, 1);
	ASSERT_EQ(IJST_GET_STATUS(innerSt, int_1), FStatus::kValid);
	// int_2 null but with value
	IJST_SET(innerSt, int_2, 2);
	IJST_MARK_NULL(innerSt, int_2);
	ASSERT_EQ(IJST_GET_STATUS(innerSt, int_2), FStatus::kNull);
	// str_1 missing but with value
	IJST_SET(innerSt, str_1, "str1");
	IJST_MARK_MISSING(innerSt, str_1);
	ASSERT_EQ(IJST_GET_STATUS(innerSt, str_1), FStatus::kMissing);
	// str_2 missing in default
	ASSERT_EQ(IJST_GET_STATUS(innerSt, str_2), FStatus::kMissing);


	//--- kNoneFlag
	{
		rapidjson::Document doc;
		UTEST_SERIALIZE_AND_CHECK(innerSt, doc, SerFlag::kNoneFlag);

		ASSERT_EQ(doc["int_val_1"].GetInt(), 1);
		ASSERT_TRUE(doc["int_val_2"].IsNull());	// null
		ASSERT_STREQ(doc["str_val_1"].GetString(), "str1");
		ASSERT_STREQ(doc["str_val_2"].GetString(), "");
	}

	//--- kIgnoreMissing
	{
		rapidjson::Document doc;
		UTEST_SERIALIZE_AND_CHECK(innerSt, doc, SerFlag::kIgnoreMissing);

		ASSERT_EQ(doc["int_val_1"].GetInt(), 1);
		ASSERT_TRUE(doc["int_val_2"].IsNull());	// null
		ASSERT_FALSE(doc.HasMember("str_val_1"));
		ASSERT_FALSE(doc.HasMember("str_val_2"));
	}

	//--- kIgnoreNull
	{
		rapidjson::Document doc;
		UTEST_SERIALIZE_AND_CHECK(innerSt, doc, SerFlag::kIgnoreNull);

		ASSERT_FALSE(doc.HasMember("int_val_2"));

		ASSERT_EQ(doc["int_val_1"].GetInt(), 1);
		ASSERT_STREQ(doc["str_val_1"].GetString(), "str1");
		ASSERT_STREQ(doc["str_val_2"].GetString(), "");
	}

	//--- kIgnoreNull | kIgnoreMissing
	{
		rapidjson::Document doc;
		UTEST_SERIALIZE_AND_CHECK(innerSt, doc, SerFlag::kIgnoreMissing | SerFlag::kIgnoreNull);

		ASSERT_EQ(doc["int_val_1"].GetInt(), 1);
		ASSERT_FALSE(doc.HasMember("int_val_2"));
		ASSERT_FALSE(doc.HasMember("str_val_1"));
		ASSERT_FALSE(doc.HasMember("str_val_2"));
	}

	//--- kIgnoreUnknown will be tested in Serialize.Unknown
}

IJST_DEFINE_STRUCT(
		NestSt
		, (IJST_TVEC(T_int), vec_1, "vec_val_1", 0)
		, (IJST_TST(Inner), inner_1, "inner_val_1", 0)
		, (IJST_TMAP(T_string), map_1, "map_val_1", 0)
		, (IJST_TVEC(IJST_TST(Inner)), vec_2, "vec_val_2", 0)
)

TEST(Serialize, NestedOnlyValidField)
{
	NestSt nestSt;

	// Empty struct
	rapidjson::Document doc;
	// All fields are kMissing status
	UTEST_SERIALIZE_AND_CHECK(nestSt, doc, SerFlag::kIgnoreMissing);
	ASSERT_TRUE(doc.IsObject());
	ASSERT_TRUE(doc.MemberCount() == 0);
}

TEST(Serialize, NestedPushAllField)
{
	NestSt nestSt;
	// Empty struct

	rapidjson::Document doc;
	UTEST_SERIALIZE_AND_CHECK(nestSt, doc, SerFlag::kNoneFlag);

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


IJST_DEFINE_STRUCT(
		ObjRefSt
		, (IJST_TST(Inner), inner, "inner_val", 0)
		, (IJST_TVEC(IJST_TST(Inner)), inner_v, "inner_v_val", 0)
		, (IJST_TMAP(IJST_TST(Inner)), inner_m, "inner_m_val", 0)
		, (IJST_TMAP(IJST_TVEC(IJST_TST(Inner))), inner_mv, "inner_mv_val", 0)
)

TEST(Serialize, Unknown)
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
		int ret = st._.Serialize(json, SerFlag::kIgnoreUnknown | SerFlag::kIgnoreMissing);
		ASSERT_EQ(ret, 0);
		doc.Parse(json.c_str(), json.length());
		ASSERT_FALSE(doc.HasParseError());

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

IJST_DEFINE_STRUCT(
		Complicate1
		, (IJST_TST(Inner), i1, "i1_v", 0)
		, (IJST_TST(Inner), i2, "i2_v", 0)
		, (IJST_TST(Inner), i3, "i3_v", 0)
		, (IJST_TST(Inner), i4, "i4_v", 0)
)

IJST_DEFINE_STRUCT(
		Complicate2
		, (IJST_TST(Complicate1), c1, "c1_v", 0)
		, (IJST_TVEC(IJST_TMAP(T_string)), vms, "vms_v", 0)
		, (IJST_TMAP(IJST_TVEC(T_string)), mvs, "mvs_v", 0)
		, (IJST_TMAP(IJST_TMAP(IJST_TST(Inner))), mmo, "mmo_v", 0)
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
	UTEST_SERIALIZE_AND_CHECK(st, doc, SerFlag::kNoneFlag);
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
	UTEST_SERIALIZE_AND_CHECK(st, doc, SerFlag::kNoneFlag);

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
	UTEST_SERIALIZE_AND_CHECK(st, doc, SerFlag::kNoneFlag);
	ASSERT_EQ(doc.MemberCount(), 0u);
}

IJST_DEFINE_STRUCT(
		Complicate3
		, (T_string, i1, "i1_k", 0)
		, (T_string, i2, "i2_k", 0)
		, (T_string, i3, "i3_k", 0)
		, (T_string, i4, "i4_k", 0)
		, (T_string, i5, "i5_k", 0)
		, (T_string, i6, "i6_k", 0)
		, (T_string, i7, "i7_k", 0)
		, (T_string, i8, "i8_k", 0)
		, (T_string, i9, "i9_k", 0)
		, (T_string, i10, "i10_k", 0)
		, (T_string, i11, "i11_k", 0)
		, (T_string, i12, "i12_k", 0)
		, (T_string, i13, "i13_k", 0)
		, (T_string, i14, "i14_k", 0)
		, (T_string, i15, "i15_k", 0)
		, (T_string, i16, "i16_k", 0)
		, (T_string, i17, "i17_k", 0)
		, (T_string, i18, "i18_k", 0)
		, (T_string, i19, "i19_k", 0)
		, (T_string, i20, "i20_k", 0)
		, (T_string, i21, "i21_k", 0)
		, (T_string, i22, "i22_k", 0)
		, (T_string, i23, "i23_k", 0)
		, (T_string, i24, "i24_k", 0)
		, (T_string, i25, "i25_k", 0)
		, (T_string, i26, "i26_k", 0)
		, (T_string, i27, "i27_k", 0)
		, (T_string, i28, "i28_k", 0)
		, (T_string, i29, "i29_k", 0)
		, (T_string, i30, "i30_k", 0)
		, (T_string, i31, "i31_k", 0)
		, (T_string, i32, "i32_k", 0)
		, (T_string, i33, "i33_k", 0)
		, (T_string, i34, "i34_k", 0)
		, (T_string, i35, "i35_k", 0)
		, (T_string, i36, "i36_k", 0)
		, (T_string, i37, "i37_k", 0)
		, (T_string, i38, "i38_k", 0)
		, (T_string, i39, "i39_k", 0)
		, (T_string, i40, "i40_k", 0)
		, (T_string, i41, "i41_k", 0)
		, (T_string, i42, "i42_k", 0)
		, (T_string, i43, "i43_k", 0)
		, (T_string, i44, "i44_k", 0)
		, (T_string, i45, "i45_k", 0)
		, (T_string, i46, "i46_k", 0)
		, (T_string, i47, "i47_k", 0)
		, (T_string, i48, "i48_k", 0)
		, (T_string, i49, "i49_k", 0)
		, (T_string, i50, "i50_k", 0)
		, (T_string, i51, "i51_k", 0)
		, (T_string, i52, "i52_k", 0)
		, (T_string, i53, "i53_k", 0)
		, (T_string, i54, "i54_k", 0)
		, (T_string, i55, "i55_k", 0)
		, (T_string, i56, "i56_k", 0)
		, (T_string, i57, "i57_k", 0)
		, (T_string, i58, "i58_k", 0)
		, (T_string, i59, "i59_k", 0)
		, (T_string, i60, "i60_k", 0)
		, (T_string, i61, "i61_k", 0)
		, (T_string, i62, "i62_k", 0)
		, (T_string, i63, "i63_k", 0)
		, (T_string, i64, "i64_k", 0)
)

void InitComplicate3(Complicate3& st)
{
	const MetaClassInfo &metaInfo = st._.GetMetaInfo();
	for (int i = 1; i <= 64; ++i)
	{
		// Init key and value
		std::stringstream ssFieldName;
		ssFieldName << "i" << i << "_k";
		const string fieldName = ssFieldName.str();
		std::stringstream ssFieldValue;
		ssFieldValue << "v_" << i;
		const string fieldValue = ssFieldValue.str();

		// Set
		const MetaFieldInfo *fieldInfo = metaInfo.FindFieldByJsonName(fieldName);
		ASSERT_TRUE(fieldInfo != NULL);
		string* v = (string*)(void*)((char*)&st + fieldInfo->offset);
		st._.SetStrict(*v, fieldValue);
	}
}

template <typename Encoding>
void CheckComplicate3Serialized(const rapidjson::GenericDocument<Encoding>& doc)
{
	ASSERT_FALSE(doc.HasParseError());
	for (int i = 1; i <= 64; ++i)
	{
		// Init key and value
		std::basic_stringstream<typename Encoding::Ch> ssFieldName;
		ssFieldName << "i" << i << "_k";
		const std::basic_string<typename Encoding::Ch> fieldName = ssFieldName.str();
		std::basic_stringstream<typename Encoding::Ch> ssFieldValue;
		ssFieldValue << "v_" << i;
		const std::basic_string<typename Encoding::Ch> fieldValue = ssFieldValue.str();

		// Check
		ASSERT_STREQ(doc[fieldName.c_str()].GetString(), fieldValue.c_str());
	}
}

TEST(Serialize, BigStruct)
{
	Complicate3 st;
	InitComplicate3(st);
	rapidjson::Document doc;
	UTEST_SERIALIZE_AND_CHECK(st, doc, SerFlag::kNoneFlag);
	CheckComplicate3Serialized(doc);
}

TEST(Serialize, SerializeHandler)
{
	Complicate3 st;
	InitComplicate3(st);
	rapidjson::StringBuffer buf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
	HandlerWrapper<rapidjson::PrettyWriter<rapidjson::StringBuffer> > writerWrapper(writer);
	st._.Serialize(writerWrapper);
	rapidjson::Document doc;
	doc.Parse(buf.GetString(), buf.GetSize() / sizeof(rapidjson::StringBuffer::Ch));
	CheckComplicate3Serialized(doc);
}

TEST(Serialize, WriteUTF16)
{
	Complicate3 st;
	InitComplicate3(st);

	// Write
	typedef rapidjson::GenericStringBuffer<rapidjson::UTF16LE<> > SBuffer;
	SBuffer buf;
	rapidjson::Writer<SBuffer> writer(buf);
	HandlerWrapper<rapidjson::Writer<SBuffer> > writerWrapper(writer);
	st._.Serialize(writerWrapper);

	// Check
	rapidjson::GenericDocument<rapidjson::UTF16LE<> > doc;
	doc.Parse(buf.GetString());
	CheckComplicate3Serialized(doc);
}

TEST(Serialize, GeneratorWrapper)
{
	// only valid fields
	{
		Complicate3 st;
		SAXGeneratorWrapper<rapidjson::Document> generator(st._, SerFlag::kIgnoreMissing);
		rapidjson::Document doc;
		doc.Populate(generator);
		ASSERT_FALSE(doc.HasParseError());
		ASSERT_TRUE(doc.IsObject());
		ASSERT_EQ(doc.MemberCount(), 0u);
	}
	// all fields
	{
		Complicate3 st;
		InitComplicate3(st);
		SAXGeneratorWrapper<rapidjson::Document> generator(st._);
		rapidjson::Document doc;
		doc.Populate(generator);
		CheckComplicate3Serialized(doc);
	}
}
