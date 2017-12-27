//
// Created by h46incon on 2017/12/27.
//

#include <gtest/gtest.h>
#include <ijst/detail/detail.h>

using namespace std;
using namespace ijst;

TEST(Detail, CancelableOStream)
{
	detail::CancelableOStream ostream(8);
	ASSERT_TRUE(ostream.str.empty());
	ASSERT_FALSE(ostream.IsDone());

	ostream.Put('0');
	ostream.Put('1');
	ostream.Put('2');
	ostream.Put('3');
	ostream.Put('4');
	ostream.Put('5');
	ostream.Put('6');
	ostream.Put('7');

	ASSERT_EQ(ostream.str, "01234567");
	ASSERT_FALSE(ostream.IsDone());

	ostream.Put('8');
	ASSERT_EQ(ostream.str, "01234567");
	ASSERT_TRUE(ostream.IsDone());

	ostream.Put('9');
	ASSERT_EQ(ostream.str, "01234567");
	ASSERT_TRUE(ostream.IsDone());
}

#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <limits>

TEST(Detail, CancelableWriter)
{
	detail::CancelableOStream ostream(4096);
	typedef detail::CancelableWriter<detail::CancelableOStream, rapidjson::Writer<detail::CancelableOStream> > Writer;
	Writer writer(ostream);

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
		writer.EndArray();
	writer.EndObject();

	ASSERT_FALSE(ostream.IsDone());

	rapidjson::Document doc;
	doc.Parse(ostream.str.c_str(), ostream.str.length());
	ASSERT_FALSE(doc.HasParseError());

	// Accept
	{
		detail::CancelableOStream ostream2(4096);
		Writer writer2(ostream2);
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
	ASSERT_STREQ(val[5].GetString(), "42.1");	// rapidjson::Writer.RawNumber will writer string
	ASSERT_STREQ(val[6].GetString(), "v1");
	ASSERT_EQ(val[7].GetBool(), true);
	ASSERT_EQ(val[8].GetBool(), false);
	ASSERT_EQ(val.Size(), 9u);
}
