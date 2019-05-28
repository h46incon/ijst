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


class OverrideSt: private SimpleSt {
public:
	typedef SimpleSt _ijst_BaseClass;

	OverrideSt()
	{
		_.UpdateShadowMetaClass(&(::ijst::detail::IjstStructMeta<OverrideSt>::Ins()));
		_.SetOverrideMetaInfo(::ijst::detail::IjstStructOvrMeta<OverrideSt>::Ins());
	}

	_ijst_BaseClass& _ijst_Base() {return *this;}
	const _ijst_BaseClass& _ijst_Base() const {return *this;}

	using _ijst_BaseClass::_;
	using _ijst_BaseClass::_ijst_Encoding;
	using _ijst_BaseClass::_ijst_Ch;
	using _ijst_BaseClass::_ijst_AccessorType;

	using _ijst_BaseClass::int_1;
	using _ijst_BaseClass::int_2;

private:
	friend class ::ijst::detail::IjstStructMeta< OverrideSt >;
	friend class ::ijst::detail::IjstStructOvrMeta< OverrideSt >;

	template<bool DummyTrue>
	static void _ijst_InitMetaInfo(::ijst::MetaClassInfo<_ijst_Ch>& metaInfo, const OverrideSt* stPtr)
	{
		(void)stPtr;
		::ijst::detail::MetaClassInfoSetter<_ijst_Encoding> mSetter(metaInfo);
		mSetter.ShadowFrom(::ijst::detail::IjstStructMeta<_ijst_BaseClass>::Ins(), "OverrideSt");
	}

	static ::ijst::OverrideMetaInfos* _ijst_NewOvrMetaInfo(const OverrideSt* stPtr)
	{
		const MetaClassInfo<char> &metaInfo = ::ijst::detail::IjstStructMeta<OverrideSt>::Ins();
		::ijst::OverrideMetaInfos* pOverrideStOvrMeta = ijst::OverrideMetaInfos::NewFromSrcOrEmpty(
				::ijst::detail::IjstStructOvrMeta<_ijst_BaseClass>::Ins(), metaInfo.GetFieldSize());

		pOverrideStOvrMeta->metaInfos[metaInfo.FindIndex(IJSTI_OFFSETOF(stPtr, int_1))].SetFieldDesc(FDesc::Optional);  // relax
		pOverrideStOvrMeta->metaInfos[metaInfo.FindIndex(IJSTI_OFFSETOF(stPtr, int_2))].SetFieldDesc(FDesc::NoneFlag);  // strict
		pOverrideStOvrMeta->metaInfos[metaInfo.FindIndex(IJSTI_OFFSETOF(stPtr, int_3))].ijstFieldMetaInfo = NULL;  // just test

		return pOverrideStOvrMeta;
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

//IJST_DEFINE_STRUCT(
//		OutSt
//		, (IJST_TST(SimpleSt), in)
//		, (IJST_TST(SimpleSt), in2)
//		, (IJST_TST(OverrideSt), ost, ijst::FDesc::Optional)  // TODO: add test
//		, (IJST_TVEC(IJST_TST(SimpleSt)), vec)
//)
//
//class OverrideOutSt: private OutSt {
//public:
//	typedef OutSt _ijst_BaseClass;
//	_ijst_BaseClass& _ijst_Base() {return *this;}
//	const _ijst_BaseClass& _ijst_Base() const {return *this;}
//
//	using _ijst_BaseClass::_;
//	using _ijst_BaseClass::_ijst_Encoding;
//	using _ijst_BaseClass::_ijst_Ch;
//	using _ijst_BaseClass::_ijst_AccessorType;
//
//	using _ijst_BaseClass::in;
//	using _ijst_BaseClass::in2;
//	using _ijst_BaseClass::vec;
//
//	OverrideOutSt()
//	{
//		const MetaClassInfo<char> &metaInfo = _.GetMetaInfo();
//		ijst::OverrideMetaInfos* ovrMeta = new ijst::OverrideMetaInfos(metaInfo.GetFieldSize());
//
//		ovrMeta->metaInfos[metaInfo.FindIndex((char*)&in - (char*)this)].ijstFieldMetaInfo = pOverrideStOvrMeta;
//		ovrMeta->metaInfos[metaInfo.FindIndex((char*)&vec - (char*)this)].ijstFieldMetaInfo = pOverrideStOvrMeta;
//
//		_.SetOverrideMetaInfo(ovrMeta);
//	}
//};
//
//TEST(Override, Out)
//{
//	{
//		OverrideOutSt st;
//
//		string json = "{"
//					  "\"in\": {\"int_2\": 1},"
//					  "\"in2\": {\"int_1\": 1},"
//					  "\"vec\": [{\"int_2\": 1}]"
//					  "}";
//
//		string strErrMsg;
//		int iRet = st._.Deserialize(json, strErrMsg);
//		ASSERT_EQ(iRet, 0);
//	}
//
//	// in failed
//	{
//		OverrideOutSt st;
//		string json = "{"
//					  "\"in\": {\"int_1\": 1},"
//					  "\"in2\": {\"int_1\": 1},"
//					  "\"vec\": [{\"int_2\": 1}]"
//					  "}";
//		int iRet = st._.Deserialize(json);
//		ASSERT_EQ(iRet, ijst::ErrorCode::kDeserializeSomeFieldsInvalid);
//	}
//
//	// vec failed
//	{
//		OverrideOutSt st;
//		string json = "{"
//					  "\"in\": {\"int_2\": 1},"
//					  "\"in2\": {\"int_1\": 1},"
//					  "\"vec\": [{\"int_1\": 1}]"
//					  "}";
//		int iRet = st._.Deserialize(json);
//		ASSERT_EQ(iRet, ijst::ErrorCode::kDeserializeSomeFieldsInvalid);
//	}
//}
//
}
