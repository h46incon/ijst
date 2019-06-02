#include "util.h"
#include <typeinfo>

using namespace ijst;
using namespace std;

// TODO: test when ijst struct with same name
namespace override_test_ns {

IJST_DEFINE_STRUCT(
		SimpleSt
		, (T_int, int_1)
		, (T_int, int_2, FDesc::Optional)
		, (T_int, int_3, FDesc::Optional)
		, (T_int, int_4, FDesc::Optional)
)


// class stName: private stBase {
#define IJST_OVR_DEFINE_P1(stName, stBase) \
	typedef stBase _ijst_BaseClass; \
	typedef stName _ijst_ThisClass; \
	template<bool DummyTrue> \
	static void _ijst_InitMetaInfo(::ijst::MetaClassInfo<_ijst_Ch>& metaInfo, const _ijst_ThisClass * stPtr) \
	{ \
		(void)stPtr; \
		::ijst::detail::MetaClassInfoSetter<_ijst_Encoding> mSetter(metaInfo); \
		mSetter.ShadowFrom(::ijst::detail::IjstStructMeta<_ijst_BaseClass>::Ins(), #stName); \
	} \
public: \
	stName() \
	{ \
		_.UpdateShadowMetaClass(&(::ijst::detail::IjstStructMeta<_ijst_ThisClass>::Ins())); \
		_.SetOverrideMetaInfo(::ijst::detail::IjstStructOvrMeta<_ijst_ThisClass>::Ins()); \
	} \
	_ijst_BaseClass& _ijst_Base() {return *this;} \
	const _ijst_BaseClass& _ijst_Base() const {return *this;} \
	using _ijst_BaseClass::_; \
	using _ijst_BaseClass::_ijst_Encoding; \
	using _ijst_BaseClass::_ijst_Ch; \
	using _ijst_BaseClass::_ijst_AccessorType;

#define IJST_OVR_DEFINE_P2() \
private: \
	friend class ::ijst::detail::IjstStructMeta< _ijst_ThisClass >; \
	friend class ::ijst::detail::IjstStructOvrMeta< _ijst_ThisClass >; \
	static ::ijst::OverrideMetaInfos* _ijst_NewOvrMetaInfo(const _ijst_ThisClass* stPtr) \
	{ \
		const MetaClassInfo<char> &metaInfo = ::ijst::detail::IjstStructMeta<_ijst_ThisClass>::Ins(); \
		::ijst::OverrideMetaInfos* pOverrideStOvrMeta = ijst::OverrideMetaInfos::NewFromSrcOrEmpty( \
				::ijst::detail::IjstStructOvrMeta<_ijst_BaseClass>::Ins(), metaInfo.GetFieldSize()); \

#define IJST_OVR_DEFINE_P3() \
		return pOverrideStOvrMeta; \
	}

//};

#define IJST_OVR_SET_FIELD_DESC(field, desc) \
	do { \
		int idx = metaInfo.FindIndex(IJSTI_OFFSETOF(stPtr, field)); \
		assert(idx != -1); \
		pOverrideStOvrMeta->metaInfos[idx].SetFieldDesc((desc)); \
	} while (false)

#define IJST_OVR_SET_FIELD_OVR_TYPE(field, type) \
	do { \
		int idx = metaInfo.FindIndex(IJSTI_OFFSETOF(stPtr, field)); \
		assert(idx != -1); \
		pOverrideStOvrMeta->metaInfos[idx].ijstFieldMetaInfo = ::ijst::detail::IjstStructOvrMeta< type >::Ins(); \
	} while (false)

class OverrideSt: private SimpleSt {
IJST_OVR_DEFINE_P1(OverrideSt, SimpleSt)

	using _ijst_BaseClass::int_1;
	using _ijst_BaseClass::int_2;

IJST_OVR_DEFINE_P2()

	IJST_OVR_SET_FIELD_DESC(int_1, FDesc::Optional); // relax
	IJST_OVR_SET_FIELD_DESC(int_2, FDesc::NoneFlag); // strict

IJST_OVR_DEFINE_P3()
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
IJST_OVR_DEFINE_P1(OverrideOutSt, OutSt)

	using _ijst_BaseClass::in;
	using _ijst_BaseClass::in2;
	using _ijst_BaseClass::vec;

IJST_OVR_DEFINE_P2()

	IJST_OVR_SET_FIELD_OVR_TYPE(in, OverrideSt);
	IJST_OVR_SET_FIELD_OVR_TYPE(vec, OverrideSt);

IJST_OVR_DEFINE_P3()
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
