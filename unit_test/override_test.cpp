#include "util.h"

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
		EmptyOvrSt, override_test_ns::SimpleSt,
		(
		),
		(
		)
)

#if __cplusplus >= 201103L
void TestOvrStructInsideFunction()
{
	IJST_OVR_DEFINE_STRUCT(
			OvrStInFunction, override_test_ns::SimpleSt,
			(
					using _ijst_BaseClass::int_1;
			),
			(
			)
	)

	OvrStInFunction st;
	ASSERT_EQ(st.int_1, 0);
}

TEST(Override, OvrStructInsideFunction)
{
	TestOvrStructInsideFunction();
}
#endif

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


IJST_DEFINE_STRUCT(
		OutSt
		, (IJST_TST(SimpleSt), in)
		, (IJST_TST(SimpleSt), in2)
		, (IJST_TST(OverrideSt), ost)
		, (IJST_TVEC(IJST_TST(SimpleSt)), vec)
		, (IJST_TLIST(IJST_TST(SimpleSt)), list)
		, (IJST_TDEQUE(IJST_TST(SimpleSt)), deq)
		, (IJST_TMAP(IJST_TST(SimpleSt)), map)
		, (IJST_TVEC(IJST_TDEQUE(IJST_TST(SimpleSt))), vec_deq)
)

IJST_OVR_DEFINE_STRUCT(
		OverrideOutSt, OutSt,
		(
				using _ijst_BaseClass::in2;
				using _ijst_BaseClass::list;
				using _ijst_BaseClass::in;
				using _ijst_BaseClass::vec;
				using _ijst_BaseClass::deq;
				using _ijst_BaseClass::vec_deq;
				using _ijst_BaseClass::map;
		),
		(
				IJST_OVR_SET_FIELD_OVR_TYPE(in, OverrideSt);
				// keep in2 SimpleSt
				IJST_OVR_SET_FIELD_OVR_TYPE(vec, OverrideSt);
				IJST_OVR_SET_FIELD_OVR_TYPE(list, OverrideSt);
				IJST_OVR_SET_FIELD_OVR_TYPE(deq, OverrideSt);
				IJST_OVR_SET_FIELD_OVR_TYPE(map, OverrideSt);
				IJST_OVR_SET_FIELD_OVR_TYPE(vec_deq, OverrideSt);
		)
)


TEST(Override, SetFieldOvrType)
{
	OverrideOutSt st;

	// {"int_2": 1} is valid for OverrideSt, but not for SimpleSt
	// use this string to check if the override meta info is effective
	string json = "{"
				  "\"in\": {\"int_2\": 1},"		// OverrideSt
				  "\"in2\": {\"int_1\": 2},"	// SimpleSt
				  "\"ost\": {\"int_2\": 3},"	// OverrideSt member declared directly
				  "\"vec\": [{\"int_2\": 4}],"	// OverrideSt
				  "\"list\": [{\"int_2\": 5}],"	// OverrideSt
				  "\"deq\": [{\"int_2\": 6}],"	// OverrideSt
				  "\"map\": {\"key\": {\"int_2\": 7}},"	// OverrideSt
				  "\"vec_deq\": [[{\"int_2\": 8}]]"	// OverrideSt
				  "}";

	string strErrMsg;
	int iRet = st._.Deserialize(json, strErrMsg);
	ASSERT_EQ(iRet, 0);

	ASSERT_EQ(st.in.int_2, 1);
	ASSERT_EQ(st.vec[0].int_2, 4);
	ASSERT_EQ(st.deq[0].int_2, 6);
	ASSERT_EQ(st.vec_deq[0][0].int_2, 8);
}

IJST_OVR_DEFINE_STRUCT(
		OvrOvrSt, OverrideOutSt,
		(
				using _ijst_BaseClass::in;
		),
		(
				// set in back to SimpleSt
				IJST_OVR_SET_FIELD_OVR_TYPE(in, SimpleSt);
		)
)

TEST(Override, OvrOvrSt)
{

	OvrOvrSt st;

	// {"int_2": 1} is valid for OverrideSt, but not for SimpleSt
	// use this string to check if the override meta info is effective
	string json = "{"
				  "\"in\": {\"int_1\": 1},"		// SimpleSt
				  "\"in2\": {\"int_1\": 2},"	// SimpleSt
				  "\"ost\": {\"int_2\": 3},"	// OverrideSt member declared directly
				  "\"vec\": [{\"int_2\": 4}],"	// OverrideSt
				  "\"list\": [{\"int_2\": 5}],"	// OverrideSt
				  "\"deq\": [{\"int_2\": 6}],"	// OverrideSt
				  "\"map\": {\"key\": {\"int_2\": 7}},"	// OverrideSt
				  "\"vec_deq\": [[{\"int_2\": 8}]]"	// OverrideSt
				  "}";

	string strErrMsg;
	int iRet = st._.Deserialize(json, strErrMsg);
	ASSERT_EQ(iRet, 0);
	ASSERT_EQ(st.in.int_1, 1);
}


#if __cplusplus >= 201103L

IJST_OVR_DEFINE_STRUCT(
		OvrFieldSt, OverrideOutSt,
		(
				IJST_OVR_WRAP_FIELD(OverrideSt, wrap_, in)
				IJST_OVR_WRAP_FIELD(OverrideSt, wrap_, vec)
				IJST_OVR_WRAP_FIELD(OverrideSt, wrap_, deq)
				IJST_OVR_WRAP_FIELD(OverrideSt, wrap_, vec_deq)
		),
		(
		)
)


TEST(Override, OvrFieldWrapper)
{
	OvrFieldSt st;

	// {"int_2": 1} is valid for OverrideSt, but not for SimpleSt
	// use this string to check if the override meta info is effective
	string json = "{"
				  "\"in\": {\"int_2\": 1},"		// OverrideSt
				  "\"in2\": {\"int_1\": 1},"	// SimpleSt
				  "\"ost\": {\"int_2\": 3},"	// OverrideSt member declared directly
				  "\"vec\": [{\"int_2\": 4}],"	// OverrideSt
				  "\"list\": [{\"int_2\": 5}],"	// OverrideSt
				  "\"deq\": [{\"int_2\": 6}],"	// OverrideSt
				  "\"map\": {\"key\": {\"int_2\": 7}},"	// OverrideSt
				  "\"vec_deq\": [[{\"int_2\": 8}]]"	// OverrideSt
				  "}";

	string strErrMsg;
	int iRet = st._.Deserialize(json, strErrMsg);
	ASSERT_EQ(iRet, 0);

	const OvrFieldSt& stRef = st;

	ASSERT_EQ(st.wrap_in()->int_2, 1);
	ASSERT_EQ(stRef.wrap_in()->int_2, 1);

	ASSERT_EQ(st.wrap_vec()[0]->int_2, 4);
	ASSERT_EQ(stRef.wrap_vec()[0]->int_2, 4);

	ASSERT_EQ(st.wrap_deq()[0]->int_2, 6);
	ASSERT_EQ(stRef.wrap_deq()[0]->int_2, 6);

	ASSERT_EQ(st.wrap_vec_deq()[0][0]->int_2, 8);
	ASSERT_EQ(stRef.wrap_vec_deq()[0][0]->int_2, 8);

}

#endif
}
