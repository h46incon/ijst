//
// Created by h46incon on 2017/12/27.
//

#include <gtest/gtest.h>
#include <ijst/detail/detail.h>

using namespace std;
using namespace ijst;

TEST(Detail, CancelableOStream)
{
	detail::HeadOStream ostream(8);
	ASSERT_TRUE(ostream.str.empty());
	ASSERT_TRUE(ostream.HeadOnly());
	const unsigned long oldCapacity = ostream.str.capacity();

	ostream.Put('0');
	ostream.Put('1');
	ostream.Put('2');
	ostream.Put('3');
	ostream.Put('4');
	ostream.Put('5');
	ostream.Put('6');
	ostream.Put('7');

	ASSERT_EQ(ostream.str, "01234567");
	ASSERT_TRUE(ostream.HeadOnly());

	ostream.Put('8');
	ASSERT_EQ(ostream.str, "01234567...");
	ASSERT_FALSE(ostream.HeadOnly());

	ostream.Put('9');
	ASSERT_EQ(ostream.str, "01234567...");
	ASSERT_FALSE(ostream.HeadOnly());

	ASSERT_EQ(oldCapacity, ostream.str.capacity());
}

#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <limits>

TEST(Detail, CancelableWriter)
{
	detail::HeadOStream ostream(4096);
	detail::HeadWriter writer(ostream);

	writer.StartObject();
	writer.Key("k1", 2);
		writer.StartArray();
			writer.Int(std::numeric_limits<int>::min());
			writer.Int64(std::numeric_limits<int64_t>::min());
			writer.Uint(std::numeric_limits<unsigned int>::max());
			writer.Uint64(std::numeric_limits<uint64_t>::max());
			writer.Double(3.14);
			writer.RawNumber("42.1", 4);
			writer.String("v1", 2);
			writer.Bool(true);
			writer.Bool(false);
			writer.Null();
		writer.EndArray();
	writer.EndObject();

	ASSERT_TRUE(ostream.HeadOnly());

	rapidjson::Document doc;
	doc.Parse(ostream.str.c_str(), ostream.str.length());
	ASSERT_FALSE(doc.HasParseError());

	// Accept
	{
		detail::HeadOStream ostream2(4096);
		detail::HeadWriter writer2(ostream2);
		doc.Accept(writer2);
		rapidjson::Document doc2;
		doc2.Parse(ostream2.str.c_str(), ostream2.str.length());
		ASSERT_FALSE(doc2.HasParseError());
		ASSERT_EQ((rapidjson::Value&)doc, (rapidjson::Value&)doc2);
	}

	ASSERT_TRUE(doc.IsObject());
	ASSERT_EQ(doc.MemberCount(), 1u);

	ASSERT_TRUE(doc.HasMember("k1"));
	ASSERT_TRUE(doc["k1"].IsArray());
	const rapidjson::Value::Array& val = doc["k1"].GetArray();
	ASSERT_EQ(val[0].GetInt(), std::numeric_limits<int>::min());
	ASSERT_EQ(val[1].GetInt64(), std::numeric_limits<int64_t>::min());
	ASSERT_EQ(val[2].GetUint(), std::numeric_limits<unsigned int>::max());
	ASSERT_EQ(val[3].GetUint64(), std::numeric_limits<uint64_t>::max());
	ASSERT_EQ(val[4].GetDouble(), 3.14);
//	ASSERT_STREQ(val[5].GetDouble(), 42.1);	// Seems bug
	ASSERT_STREQ(val[6].GetString(), "v1");
	ASSERT_EQ(val[7].GetBool(), true);
	ASSERT_EQ(val[8].GetBool(), false);
	ASSERT_TRUE(val[9].IsNull());
	ASSERT_EQ(val.Size(), 10u);
}

#include <ijst/ijst.h>

struct StructWithField_ {
	int _;
};

struct StructWithMethod_ {
	void _()  { }
};

TEST(Detail, AccessorSFINAE)
{
	ijst::Optional<StructWithField_> stf(NULL);
	ASSERT_EQ(NULL, stf.Ptr());
	// compile error:
	// stf->_;

	ijst::Optional<StructWithMethod_> stm(NULL);
	ASSERT_EQ(NULL, stm.Ptr());
	// compile error:
	// stm->_();
}