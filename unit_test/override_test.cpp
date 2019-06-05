#include "util.h"
#include <typeinfo>

using namespace std;

// TODO: test when ijst struct with same name
namespace override_test_ns {

IJST_DEFINE_STRUCT(
		SimpleSt
		, (ijst::T_int, int_1)
		, (ijst::T_int, int_2, ijst::FDesc::Optional)
		, (ijst::T_int, int_3, ijst::FDesc::Optional)
		, (ijst::T_int, int_4, ijst::FDesc::Optional)
)

IJST_OVR_DEFINE_STRUCT(
		OverrideSt, override_test_ns::SimpleSt,
		(
				using _ijst_BaseClass::int_1;
				using _ijst_BaseClass::int_2;
		),
		(
				IJST_OVR_SET_FIELD_DESC(int_1, ijst::FDesc::Optional); // relax
				IJST_OVR_SET_FIELD_DESC(int_2, ijst::FDesc::NoneFlag); // strict
				IJST_OVR_SET_FIELD_DESC(int_3, ijst::FDesc::NotDefault | ijst::FDesc::Optional); // change
				IJST_OVR_SET_FIELD_DESC(int_4, ijst::FDesc::Nullable | ijst::FDesc::Optional); // change
		)
)


TEST(Override, SetFieldDesc)
{
	{
		OverrideSt st;

		// int_2: NoneFlag
		string json = "{\"int_1\": 1}";
		int iRet = st._.Deserialize(json);
		ASSERT_EQ(iRet, ijst::ErrorCode::kDeserializeSomeFieldsInvalid);
	}

	{
		OverrideSt st;

		// int_3: NotDefault
		string json = "{\"int_2\": 1, \"int_3\": 0}";
		int iRet = st._.Deserialize(json);
		ASSERT_EQ(iRet, ijst::ErrorCode::kDeserializeValueIsDefault);
	}


	{
		OverrideSt st;

		// valid
		// int_1 Optional, int_4: Nullable
		string json = "{\"int_2\": 1, \"int_3\": 1, \"int_4\": null}";
		int iRet = st._.Deserialize(json);
		ASSERT_EQ(iRet, 0);
		ASSERT_EQ(st.int_1, 0);
		SimpleSt& bst = st._ijst_Base();
		ASSERT_EQ(bst.int_3, 1);
	}
}

// TODO: override of overrided

IJST_DEFINE_STRUCT(
		OutSt
		, (IJST_TST(SimpleSt), in)
		, (IJST_TST(SimpleSt), in2)
		, (IJST_TST(OverrideSt), ost)
		, (IJST_TVEC(IJST_TST(SimpleSt)), vec)
		, (IJST_TLIST(IJST_TST(SimpleSt)), list)
		, (IJST_TDEQUE(IJST_TST(SimpleSt)), deq)
		, (IJST_TMAP(IJST_TST(SimpleSt)), map)
)

IJST_OVR_DEFINE_STRUCT(
		OverrideOutSt, OutSt,
		(
				using _ijst_BaseClass::in;
				using _ijst_BaseClass::in2;
				using _ijst_BaseClass::vec;
				using _ijst_BaseClass::list;
				using _ijst_BaseClass::deq;
				using _ijst_BaseClass::map;
		),
		(
				IJST_OVR_SET_FIELD_OVR_TYPE(in, OverrideSt);
				// keep in2 SimpleSt
				IJST_OVR_SET_FIELD_OVR_TYPE(vec, OverrideSt);
				IJST_OVR_SET_FIELD_OVR_TYPE(list, OverrideSt);
				IJST_OVR_SET_FIELD_OVR_TYPE(deq, OverrideSt);
				IJST_OVR_SET_FIELD_OVR_TYPE(map, OverrideSt);
		)
)

TEST(Override, SetFieldOvrType)
{
	{
		OverrideOutSt st;

		// {"int_2": 1} is valid for OverrideSt, but not for SimpleSt
		// use this string to check if the override meta info is effective
		string json = "{"
					  "\"in\": {\"int_2\": 1},"		// OverrideSt
					  "\"in2\": {\"int_1\": 1},"	// SimpleSt
					  "\"ost\": {\"int_2\": 1},"	// OverrideSt member declared directly
					  "\"vec\": [{\"int_2\": 1}],"	// OverrideSt
					  "\"list\": [{\"int_2\": 1}],"	// OverrideSt
					  "\"deq\": [{\"int_2\": 1}],"	// OverrideSt
					  "\"map\": {\"key\": {\"int_2\": 1}}"	// OverrideSt
					  "}";

		string strErrMsg;
		int iRet = st._.Deserialize(json, strErrMsg);
		ASSERT_EQ(iRet, 0);
	}

}

}
