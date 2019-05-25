#include "util.h"
#include <typeinfo>

using namespace ijst;
using namespace std;

// TODO: test when ijst struct with same name
namespace override_test_ns {

IJST_DEFINE_STRUCT(
		SimpleSt
		, (T_int, int_1, 0)
		, (T_int, int_2, FDesc::Optional)
		, (T_int, int_3, FDesc::Optional)
		, (T_int, int_4, FDesc::Optional)
)



ijst::OverrideMetaInfos* pOverrideStOvrMeta = NULL;

class OverrideSt: private SimpleSt {
public:
	SimpleSt& _ijst_Base() {return *this;}
	const SimpleSt& _ijst_Base() const {return *this;}

	using SimpleSt::_;
	using SimpleSt::_ijst_Encoding;
	using SimpleSt::_ijst_Ch;
	using SimpleSt::_ijst_AccessorType;

	using SimpleSt::int_1;
	using SimpleSt::int_2;

	OverrideSt()
	{
		const MetaClassInfo<char> &metaInfo = _.GetMetaInfo();

		pOverrideStOvrMeta = new ijst::OverrideMetaInfos(metaInfo.GetFieldSize());
		pOverrideStOvrMeta->metaInfos[metaInfo.FindIndex((char*)&int_1 - (char*)this)].SetFieldDesc(FDesc::Optional);  // relax
		pOverrideStOvrMeta->metaInfos[metaInfo.FindIndex((char*)&int_2 - (char*)this)].SetFieldDesc(FDesc::NoneFlag);  // strict

		_.SetOverrideMetaInfo(pOverrideStOvrMeta);
	}
};

TEST(Override, Base)
{
	{
		OverrideSt st;

		string json = "{\"int_1\": 1}";
		int iRet = st._.Deserialize(json);
		ASSERT_EQ(iRet, ijst::ErrorCode::kDeserializeSomeFieldsInvalid);
	}

	{
		OverrideSt st;
		string json = "{\"int_2\": 1}";
		int iRet = st._.Deserialize(json);
		ASSERT_EQ(iRet, 0);
		ASSERT_EQ(st.int_1, 0);
		SimpleSt& bst = st._ijst_Base();
		ASSERT_EQ(bst.int_2, 1);
	}
}

IJST_DEFINE_STRUCT(
		OutSt
		, (IJST_TST(SimpleSt), in)
		, (IJST_TST(SimpleSt), in2)
		, (IJST_TST(OverrideSt), ost, ijst::FDesc::Optional)  // TODO: add test
		, (IJST_TVEC(IJST_TST(SimpleSt)), vec)
)

class OverrideOutSt: private OutSt {
public:
	typedef OutSt _ijst_BaseClass;
	_ijst_BaseClass& _ijst_Base() {return *this;}
	const _ijst_BaseClass& _ijst_Base() const {return *this;}

	using _ijst_BaseClass::_;
	using _ijst_BaseClass::_ijst_Encoding;
	using _ijst_BaseClass::_ijst_Ch;
	using _ijst_BaseClass::_ijst_AccessorType;

	using _ijst_BaseClass::in;
	using _ijst_BaseClass::in2;
	using _ijst_BaseClass::vec;

	OverrideOutSt()
	{
		const MetaClassInfo<char> &metaInfo = _.GetMetaInfo();
		ijst::OverrideMetaInfos* ovrMeta = new ijst::OverrideMetaInfos(metaInfo.GetFieldSize());

		ovrMeta->metaInfos[metaInfo.FindIndex((char*)&in - (char*)this)].ijstFieldMetaInfo = pOverrideStOvrMeta;
		ovrMeta->metaInfos[metaInfo.FindIndex((char*)&vec - (char*)this)].ijstFieldMetaInfo = pOverrideStOvrMeta;

		_.SetOverrideMetaInfo(ovrMeta);
	}
};

TEST(Override, Out)
{
	{
		OverrideOutSt st;

		string json = "{"
					  "\"in\": {\"int_2\": 1},"
					  "\"in2\": {\"int_1\": 1},"
					  "\"vec\": [{\"int_2\": 1}]"
					  "}";

		string strErrMsg;
		int iRet = st._.Deserialize(json, strErrMsg);
		ASSERT_EQ(iRet, 0);
	}

	// in failed
	{
		OverrideOutSt st;
		string json = "{"
					  "\"in\": {\"int_1\": 1},"
					  "\"in2\": {\"int_1\": 1},"
					  "\"vec\": [{\"int_2\": 1}]"
					  "}";
		int iRet = st._.Deserialize(json);
		ASSERT_EQ(iRet, ijst::ErrorCode::kDeserializeSomeFieldsInvalid);
	}

	// vec failed
	{
		OverrideOutSt st;
		string json = "{"
					  "\"in\": {\"int_2\": 1},"
					  "\"in2\": {\"int_1\": 1},"
					  "\"vec\": [{\"int_1\": 1}]"
					  "}";
		int iRet = st._.Deserialize(json);
		ASSERT_EQ(iRet, ijst::ErrorCode::kDeserializeSomeFieldsInvalid);
	}
}

}
