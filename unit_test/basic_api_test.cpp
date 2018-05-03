//
// Created by h46incon on 2017/9/29.
//

#include "util.h"
using namespace ijst;

namespace dummy_ns {

IJST_DEFINE_VALUE(
		ValVec, IJST_TVEC(T_int), v, 0
)

TEST(BasicAPI, DefineValueStVec)
{
	int ret;
	ValVec st;

	ASSERT_TRUE(st._.IsParentVal());

	// Deserialize
	const std::string json = "[0, 1, 2]";
	ret = st._.Deserialize(json);
	ASSERT_EQ(ret, 0);
	std::vector<int>& vRef = st.v;
	ASSERT_EQ(vRef.size(), 3u);
	ASSERT_EQ(vRef[0], 0);
	ASSERT_EQ(vRef[1], 1);
	ASSERT_EQ(vRef[2], 2);

	// Serialize
	vRef.push_back(3);
	rapidjson::Document doc;
	UTEST_SERIALIZE_AND_CHECK(st, doc, SerFlag::kNoneFlag);
	ASSERT_TRUE(doc.IsArray());
	ASSERT_EQ(doc.Size(), 4u);
	ASSERT_EQ(doc[0].GetInt(), 0);
	ASSERT_EQ(doc[1].GetInt(), 1);
	ASSERT_EQ(doc[2].GetInt(), 2);
	ASSERT_EQ(doc[3].GetInt(), 3);
}

IJST_DEFINE_VALUE_WITH_GETTER(
		ValMap, IJST_TMAP(T_int), v, 0
)

TEST(BasicAPI, DefineValueStMap)
{
	int ret;
	ValMap st;

	// Deserialize
	const std::string json = "{\"v1\": 1, \"v2\": 2}";
	ret = st._.Deserialize(json);
	ASSERT_EQ(ret, 0);
	std::map<std::string, int>& vRef = *st.get_v().Ptr();
	ASSERT_EQ(vRef.size(), 2u);
	ASSERT_EQ(vRef["v1"], 1);
	ASSERT_EQ(vRef["v2"], 2);

	// Serialize
	vRef["v3"] = 3;
	rapidjson::Document doc;
	UTEST_SERIALIZE_AND_CHECK(st, doc, SerFlag::kNoneFlag);
	ASSERT_TRUE(doc.IsObject());
	ASSERT_EQ(doc.MemberCount(), 3u);
	ASSERT_EQ(doc["v1"].GetInt(), 1);
	ASSERT_EQ(doc["v2"].GetInt(), 2);
	ASSERT_EQ(doc["v3"].GetInt(), 3);
}

IJST_DEFINE_STRUCT(
		SimpleSt
		, (T_int, int_1, "int_val_1", 0)
		, (T_int, int_2, "int_val_2", FDesc::Optional)
		, (T_string, str_1, "str_val_1", FDesc::Nullable)
		, (T_string, str_2, "str_val_2", FDesc::Optional | FDesc::Nullable)
)

#define DEFINE_TEST_STRUCT(encoding, stName, PF) \
	IJST_DEFINE_GENERIC_STRUCT_WITH_GETTER( \
		encoding, stName ## Inner \
		, (T_int, int_v, PF ## "int_val", 0) \
	) \
	IJST_DEFINE_GENERIC_STRUCT_WITH_GETTER( \
		encoding, stName \
		, (T_int, int_v, PF ## "int_val", 0) \
		, (IJST_TST(stName ## Inner), st_v, PF ## "st_val", 0) \
		, (IJST_TSTR, str_v, PF ## "str_val", 0) \
		, (IJST_TRAW, raw_v, PF ## "raw_val", 0) \
		, (IJST_TMAP(T_int), map_v, PF ## "map_val", 0) \
		, (IJST_TOBJ(T_int), obj_v, PF ## "obj_val", 0) \
	)

DEFINE_TEST_STRUCT(rapidjson::UTF8<>, U8TestSt, )
DEFINE_TEST_STRUCT(rapidjson::UTF16<>, U16TestSt, L)
#if __cplusplus >= 201103L
DEFINE_TEST_STRUCT(rapidjson::UTF32<char32_t>, U32TestSt, U)
#endif

template<typename Encoding>
void CheckFieldInfo(const MetaClassInfo<typename Encoding::Ch>& metaInfo,
					const std::string& fieldName, const std::string& jsonName, size_t offset, FDesc::Mode desc)
{
	std::basic_string<typename Encoding::Ch> encodedJsonName = Transcode<rapidjson::UTF8<>, Encoding>(jsonName.c_str());
	const MetaFieldInfo<typename Encoding::Ch> *fieldInfo = metaInfo.FindFieldByJsonName(encodedJsonName);
	ASSERT_FALSE(fieldInfo == NULL);
	ASSERT_EQ(fieldInfo->fieldName, fieldName);
	ASSERT_EQ(fieldInfo->offset, offset);
	ASSERT_EQ(fieldInfo->desc, desc);
	ASSERT_EQ(fieldInfo->jsonName, encodedJsonName);
}

template<typename Struct>
void TestStructAPI(const char *className)
{
	typedef typename Struct::_ijst_Encoding Encoding;
	typedef typename Encoding::Ch Ch;
	Struct st;
	ASSERT_FALSE(st._.IsParentVal());

	//--- MetaInfo
	const MetaClassInfo<Ch>& metaInfo = MetaClassInfo<Ch>::template GetMetaInfo<Struct>();
	ASSERT_EQ(&st._.GetMetaInfo(), &metaInfo);
	ASSERT_STREQ(metaInfo.GetClassName().c_str(), className);
	ASSERT_EQ(metaInfo.GetFieldsInfo().size(), 6u);
	ASSERT_EQ((ptrdiff_t)metaInfo.GetAccessorOffset(), (char*)&st._ - (char*)&st);
	CheckFieldInfo<Encoding>(metaInfo, "int_v", "int_val", (char*)&st.int_v - (char*)&st, 0);
	CheckFieldInfo<Encoding>(metaInfo, "st_v", "st_val", (char*)&st.st_v - (char*)&st, 0);
	CheckFieldInfo<Encoding>(metaInfo, "str_v", "str_val", (char*)&st.str_v - (char*)&st, 0);
	CheckFieldInfo<Encoding>(metaInfo, "raw_v", "raw_val", (char*)&st.raw_v - (char*)&st, 0);
	CheckFieldInfo<Encoding>(metaInfo, "map_v", "map_val", (char*)&st.map_v - (char*)&st, 0);
	CheckFieldInfo<Encoding>(metaInfo, "obj_v", "obj_val", (char*)&st.obj_v - (char*)&st, 0);
	// invalid json name search
	ASSERT_EQ(NULL, metaInfo.FindFieldByJsonName(Transcode<rapidjson::UTF8<>, Encoding >("NotAField")));

	//--- Optional
	ASSERT_EQ(IJST_NULL, st.get_st_v()->get_int_v().Ptr());
	ASSERT_EQ(IJST_NULL, st.get_int_v().Ptr());
	ASSERT_EQ(IJST_NULL, st.get_str_v().Ptr());
	ASSERT_EQ(IJST_NULL, st.get_raw_v().Ptr());
	ASSERT_EQ(IJST_NULL, st.get_map_v()[std::basic_string<Ch>()].Ptr());
	ASSERT_EQ(IJST_NULL, st.get_obj_v()[0].Ptr());
}

TEST(BasicAPI, MetaInfo)
{
	TestStructAPI<U8TestSt>("U8TestSt");
	TestStructAPI<U16TestSt>("U16TestSt");
#if __cplusplus >= 201103L
	TestStructAPI<U32TestSt>("U32TestSt");
#endif
}

TEST(BasicAPI, FieldStatus)
{
	SimpleSt simpleSt;

	// Accessor
	ASSERT_EQ(simpleSt._.GetStatus(&simpleSt.int_1), FStatus::kMissing);
	simpleSt._.SetStrict(simpleSt.int_1, 0x5A5A);
	ASSERT_EQ(simpleSt.int_1, 0x5A5A);
	ASSERT_EQ(simpleSt._.GetStatus(&simpleSt.int_1), FStatus::kValid);

	// IJST_* macro
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, str_1), FStatus::kMissing);
	IJST_SET_STRICT(simpleSt, str_1, T_string(std::string("str1")));
	ASSERT_STREQ(simpleSt.str_1.c_str(), "str1");
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, str_1), FStatus::kValid);

	// Mark valid
	simpleSt.int_2 = 0xA5A5;
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, int_2), FStatus::kMissing);
	IJST_MARK_VALID(simpleSt, int_2);
	ASSERT_EQ(IJST_GET_STATUS(simpleSt, int_2), FStatus::kValid);

	// IsField
	ASSERT_TRUE(simpleSt._.HasField(&simpleSt.int_1));
	ASSERT_FALSE(simpleSt._.HasField(&simpleSt.str_2 + 1));
}

TEST(BasicAPI, FieldValue)
{
	SimpleSt simpleSt;

	// Init
	simpleSt._.SetStrict(simpleSt.int_1, 0x5A5A);
	IJST_SET(simpleSt, int_2, 0xA5A5);
	simpleSt._.Set(simpleSt.str_1, "str1");
	IJST_SET_STRICT(simpleSt, str_2, T_string(std::string("str2")));

	// Check
	ASSERT_EQ(simpleSt.int_1, 0x5A5A);
	ASSERT_EQ(simpleSt.int_2, 0xA5A5);
	ASSERT_STREQ(simpleSt.str_1.c_str(), "str1");
	ASSERT_STREQ(simpleSt.str_2.c_str(), "str2");
}

TEST(BasicAPI, Constructor4LValue)
{
	// copy
	{
		SimpleSt temp1;
		IJST_SET(temp1, int_1, 0x5A5A);
		temp1._.GetUnknown().AddMember("k", rapidjson::Value().SetInt(0xA5A5).Move(), temp1._.GetAllocator());

		// copy
		SimpleSt st1(temp1);
		// copy value
		ASSERT_EQ(IJST_GET_STATUS(st1, int_1), FStatus::kValid);
		ASSERT_EQ(st1.int_1, 0x5A5A);
		// copy inner stream
		ASSERT_EQ(st1._.GetUnknown()["k"].GetInt(), 0xA5A5);
		// new inner stream and allocator
		ASSERT_NE(&st1._.GetAllocator(), &temp1._.GetAllocator());
		ASSERT_NE(&st1._.GetOwnAllocator(), &temp1._.GetOwnAllocator());
		ASSERT_NE(&st1._.GetUnknown(), &temp1._.GetUnknown());
		// new metaField
		IJST_SET(temp1, int_2, 0xA5A5);
		ASSERT_EQ(IJST_GET_STATUS(st1, int_2), FStatus::kMissing);
		// Avoid make temp1 become rvalue before
		temp1._.MarkValid(&temp1.int_1);
	}


	// Assign
	{
		SimpleSt temp2;
		IJST_SET(temp2, int_1, 0x5A5A);
		temp2._.GetUnknown().AddMember("k", rapidjson::Value().SetInt(0xA5A5).Move(), temp2._.GetAllocator());

		SimpleSt st2;
		IJST_SET(st2, int_2, 0x5A5A);
		st2 = temp2;
		// copy value
		ASSERT_EQ(IJST_GET_STATUS(st2, int_1), FStatus::kValid);
		ASSERT_EQ(st2.int_1, 0x5A5A);
		ASSERT_EQ(IJST_GET_STATUS(st2, int_2), FStatus::kMissing);
		// copy inner stream
		ASSERT_EQ(st2._.GetUnknown()["k"].GetInt(), 0xA5A5);
		// new inner stream and allocator
		ASSERT_NE(&st2._.GetAllocator(), &temp2._.GetAllocator());
		ASSERT_NE(&st2._.GetOwnAllocator(), &temp2._.GetOwnAllocator());
		ASSERT_NE(&st2._.GetUnknown(), &temp2._.GetUnknown());
		// new metaField
		IJST_SET(temp2, int_2, 0xA5A5);
		ASSERT_EQ(IJST_GET_STATUS(st2, int_2), FStatus::kMissing);
		// Avoid make temp2 become rvalue
		temp2._.MarkValid(&temp2.int_1);
	}
}

#if __cplusplus >= 201103L
TEST(BasicAPI, Constructor4RValue)
{
	// copy
	{
		SimpleSt temp1;
		IJST_SET(temp1, int_1, 0x5A5A);
		void* streamTemp1 = &temp1._.GetUnknown();
		void* allocatorTemp1 = &temp1._.GetAllocator();
		void* ownAllocatorTemp1 = &temp1._.GetOwnAllocator();

		SimpleSt st1(std::move(temp1));
		// value
		ASSERT_EQ(IJST_GET_STATUS(st1, int_1), FStatus::kValid);
		ASSERT_EQ(st1.int_1, 0x5A5A);
		// inner stream
		ASSERT_EQ(&st1._.GetUnknown(), streamTemp1);
		ASSERT_EQ(&st1._.GetAllocator(), allocatorTemp1);
		ASSERT_EQ(&st1._.GetOwnAllocator(), ownAllocatorTemp1);
		//	ASSERT_ANY_THROW(temp3._.GetUnknown());
	}

	// assign
	{
		SimpleSt temp2;
		IJST_SET(temp2, int_1, 0x5A5A);
		void* streamTemp2 = &temp2._.GetUnknown();
		void* allocatorTemp2 = &temp2._.GetAllocator();
		void* ownAllocatorTemp2 = &temp2._.GetOwnAllocator();

		SimpleSt st2;
		st2 = std::move(temp2);
		// value
		ASSERT_EQ(IJST_GET_STATUS(st2, int_1), FStatus::kValid);
		ASSERT_EQ(st2.int_1, 0x5A5A);
		// inner stream
		ASSERT_EQ(&st2._.GetUnknown(), streamTemp2);
		ASSERT_EQ(&st2._.GetAllocator(), allocatorTemp2);
		ASSERT_EQ(&st2._.GetOwnAllocator(), ownAllocatorTemp2);
		//	ASSERT_ANY_THROW(temp3._.GetUnknown());
	}
}

// TODO: Constructor for out buffer

#endif

TEST(BasicAPI, GetOptional)
{
	// Only test GetOptional simply, other case is tested in ChainedOptional with get_* method
	SimpleSt st;
	{
		ASSERT_EQ(NULL, st._.GetOptional(st.int_1).Ptr());
		IJST_MARK_VALID(st, int_1);
		ASSERT_EQ(&st.int_1, st._.GetOptional(st.int_1).Ptr());
	}

	{
		ASSERT_EQ(NULL, st._.GetOptional(st.str_1).Ptr());
		IJST_MARK_VALID(st, str_1);
		ASSERT_EQ(&st.str_1, st._.GetOptional(st.str_1).Ptr());
	}
}

IJST_DEFINE_STRUCT(
		Complicate
		, (IJST_TST(SimpleSt), st, "st_v", 0)
		, (IJST_TVEC(IJST_TST(SimpleSt)), vec, "vec_v", 0)
		, (IJST_TMAP(IJST_TST(SimpleSt)), map, "map_v", 0)
)

IJST_DEFINE_STRUCT_WITH_GETTER(
		SWGetter
		, (T_int, int_1, "int_val_1", 0)
		, (T_int, int_2, "int_val_2", 0)
		, (T_string, str_1, "str_val_1", 0)
		, (T_string, str_2, "str_val_2", 0)
)

IJST_DEFINE_STRUCT_WITH_GETTER(
		CWGetter
		, (IJST_TST(SimpleSt), sim, "sim_v", 0)
		, (IJST_TST(SWGetter), st, "st_v", 0)
		, (IJST_TVEC(IJST_TST(SWGetter)), vec, "vec_v", 0)
		, (IJST_TDEQUE(IJST_TST(SWGetter)), deq, "deq_v", 0)
		, (IJST_TMAP(IJST_TST(SWGetter)), map, "map_v", 0)
)

IJST_DEFINE_STRUCT_WITH_GETTER(
		CWGetter2
		, (IJST_TST(CWGetter), v, "v", 0)
)

TEST(BasicAPI, ChainedOptional)
{
	CWGetter2 st;
	const CWGetter2& cref = st;

	// fields are missing:

	// get_* null
	ASSERT_EQ(IJST_NULL, st.get_v().Ptr());
	// get_* null chained
	ASSERT_EQ(IJST_NULL, st.get_v()->get_vec().Ptr());
	// Long null chained
	ASSERT_EQ(IJST_NULL, st.get_v()->get_sim().Ptr());
	ASSERT_EQ(IJST_NULL, st.get_v()->get_vec()[0]->get_int_1().Ptr());
	ASSERT_EQ(IJST_NULL, st.get_v()->get_deq()[0]->get_int_1().Ptr());
	ASSERT_EQ(IJST_NULL, st.get_v()->get_map()[""]->get_int_1().Ptr());

	// get_ valid
	IJST_MARK_VALID(st, v);
	ASSERT_EQ(st.get_v().Ptr(), &(st.v));
	IJST_MARK_VALID(st.v, sim);
	ASSERT_EQ(st.get_v()->get_sim().Ptr(), &(st.v.sim));

	// vector null
	ASSERT_EQ(IJST_NULL, st.v.get_vec()[0].Ptr());
	// vector elem out of range
	IJST_MARK_VALID(st.v, vec);
	ASSERT_EQ(st.v.get_vec().Ptr(), &(st.v.vec));
	ASSERT_EQ(IJST_NULL, st.v.get_vec()[0].Ptr());
	// vector valid
	st.v.vec.resize(1);
	ASSERT_EQ(st.v.get_vec()[0].Ptr(), &(st.v.vec[0]));
	ASSERT_EQ(IJST_NULL, st.v.get_vec()[0]->get_int_1().Ptr());

	// deq null
	ASSERT_EQ(IJST_NULL, st.v.get_deq()[0].Ptr());
	// deq elem out of range
	IJST_MARK_VALID(st.v, deq);
	ASSERT_EQ(st.v.get_deq().Ptr(), &(st.v.deq));
	ASSERT_EQ(IJST_NULL, st.v.get_deq()[0].Ptr());
	// deq valid
	st.v.deq.resize(1);
	ASSERT_EQ(st.v.get_deq()[0].Ptr(), &(st.v.deq[0]));
	ASSERT_EQ(IJST_NULL, st.v.get_deq()[0]->get_int_1().Ptr());

	// map null
	ASSERT_EQ(IJST_NULL, st.v.get_map()[""].Ptr());
	// map key not exist
	IJST_MARK_VALID(st.v, map);
	ASSERT_EQ(st.v.get_map().Ptr(), &(st.v.map));
	ASSERT_EQ(IJST_NULL, st.v.get_map()[""].Ptr());
	// map valid
	st.v.map[""];
	ASSERT_EQ(st.v.get_map()[""].Ptr(), &(st.v.map[""]));
	ASSERT_EQ(IJST_NULL, st.v.get_map()[""]->get_int_1().Ptr());

	// Long valid chained
	IJST_MARK_VALID(st.v.vec[0], int_1);
	ASSERT_EQ(st.get_v()->get_vec()[0]->get_int_1().Ptr(), &(st.v.vec[0].int_1));
	IJST_MARK_VALID(st.v.deq[0], int_1);
	ASSERT_EQ(st.get_v()->get_deq()[0]->get_int_1().Ptr(), &(st.v.deq[0].int_1));
	IJST_MARK_VALID(st.v.map[""], int_2);
	ASSERT_EQ(st.get_v()->get_map()[""]->get_int_2().Ptr(), &(st.v.map[""].int_2));

	// Long valid chained of const
	ASSERT_EQ(cref.get_v()->get_vec()[0]->get_int_1().Ptr(), &(cref.v.vec[0].int_1));
	ASSERT_EQ(cref.get_v()->get_deq()[0]->get_int_1().Ptr(), &(cref.v.deq[0].int_1));
	ASSERT_EQ(cref.get_v()->get_map()[""]->get_int_2().Ptr(), &(st.v.map[""].int_2));
}


struct DummySt {
	IJST_DEFINE_STRUCT(
			SimpleSt
			, (T_int, int_1, "int_val_1", 0)
			, (T_int, int_2, "int_val_2", 0)
			, (T_string, str_1, "str_val_1", 0)
			, (T_string, str_2, "str_val_2", 0)
	)
};

TEST(BasicAPI, DefineInStruct)
{
	DummySt::SimpleSt st;
	IJST_SET(st, int_1, 1);
	ASSERT_EQ(st.int_1, 1);
}

}

