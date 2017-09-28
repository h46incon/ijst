#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cstddef>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "ijst/ijst.h"


using namespace std;

/**	========================================================================================
 *				Test code
 */
namespace nstest {

class StTestI {
private:
	typedef ::ijst::detail::MetaInfo<StTestI> MetaInfoT;
	typedef ::ijst::detail::Singleton<MetaInfoT> MetaInfoS;
	friend MetaInfoT;
public:
	StTestI() :
			_(&(MetaInfoS::GetInstance()->metaClass)), name(), age(), int_vec(), int_map()
	{
	}

	ijst::detail::Accessor _;
	ijst::detail::FSerializer<IJST_TPRI(String) >::VarType name;
	ijst::detail::FSerializer<IJST_TPRI(Int) >::VarType age;
	ijst::detail::FSerializer<IJST_TVEC(IJST_TPRI(Int))>::VarType int_vec;
	ijst::detail::FSerializer<IJST_TMAP(IJST_TPRI(Int))>::VarType int_map;

private:
	static void InitMetaInfo(MetaInfoT* metaInfo)
	{
		MetaInfoS::InitInstanceBeforeMain();
		cout << "InitMetaInfo StTestI" << endl;
		metaInfo->metaClass.tag = "StTest";
		metaInfo->metaClass.metaFields.reserve(4);
		metaInfo->metaClass.accessorOffset = offsetof(StTestI, _);
		metaInfo->metaClass.PushMetaField("name", offsetof(StTestI, name), 0, IJSTI_FSERIALIZER_INS(IJST_TPRI(String)));
		metaInfo->metaClass.PushMetaField("age", offsetof(StTestI, age), 0, IJSTI_FSERIALIZER_INS(IJST_TPRI(Int)));
		metaInfo->metaClass.PushMetaField("int_vec", offsetof(StTestI, int_vec), 0, IJSTI_FSERIALIZER_INS(IJST_TVEC(IJST_TPRI(Int))));
		metaInfo->metaClass.PushMetaField("int_map", offsetof(StTestI, int_map), 0, IJSTI_FSERIALIZER_INS(IJST_TMAP(IJST_TPRI(Int))));
		metaInfo->metaClass.InitMap();
	}
};

} //namespace nstest

class StTest{
private:
	typedef ijst::detail::Singleton<ijst::detail::MetaInfo<StTest> > MetaInfoS;
public:
	StTest(): _(&(MetaInfoS::GetInstance()->metaClass)), name(), age(), st_test(), int_vec(), int_map() { }

	ijst::detail::Accessor _;
	ijst::detail::FSerializer<IJST_TPRI(String)>::VarType name;
	ijst::detail::FSerializer<IJST_TPRI(Int)>::VarType age;
	ijst::detail::FSerializer<IJST_TVEC(IJST_TOBJ(nstest::StTestI))>::VarType st_test;
	ijst::detail::FSerializer<IJST_TVEC(IJST_TPRI(Int))>::VarType int_vec;
	ijst::detail::FSerializer<IJST_TMAP(IJST_TPRI(Int))>::VarType int_map;
};

namespace ijst{
namespace detail{
template <>
MetaInfo<StTest>::MetaInfo()
{
	cout << "private init" << endl;
	this->metaClass.tag = "StTest";
	this->metaClass.metaFields.reserve(4);
	this->metaClass.accessorOffset = offsetof(FieldType, _);
	this->metaClass.PushMetaField("name", offsetof(FieldType, name), 0, IJSTI_FSERIALIZER_INS(IJST_TPRI(String)));
	this->metaClass.PushMetaField("age", offsetof(FieldType, age), 0, IJSTI_FSERIALIZER_INS(IJST_TPRI(Int)));
	this->metaClass.PushMetaField("st_test", offsetof(FieldType, st_test), 0, IJSTI_FSERIALIZER_INS(IJST_TVEC(IJST_TOBJ(nstest::StTestI))));
	this->metaClass.PushMetaField("int_vec", offsetof(FieldType, int_vec), 0, IJSTI_FSERIALIZER_INS(IJST_TVEC(IJST_TPRI(Int))));
	this->metaClass.PushMetaField("int_map", offsetof(FieldType, int_map), 0, IJSTI_FSERIALIZER_INS(IJST_TMAP(IJST_TPRI(Int))));
//	this->m_metaClass.PushMetaField("int_map", offsetof(FieldType, int_map), ijst::FDesc::Map, IJSTI_FSERIALIZER_INS(int));
	this->metaClass.InitMap();
}
}
}

namespace dummyNS {
//IJST_DEFINE_STRUCT(
IJSTI_DEFINE_STRUCT_IMPL_4(
		Person,
		(IJST_TPRI(Int), age, "jAge", 0),
		(IJST_TPRI(String), name, "jName", ijst::FDesc::Optional),
		(IJST_TVEC(IJST_TPRI(Int)), avec, "jVec", 0),
		(IJST_TMAP(IJST_TPRI(Int)), amap, "jMap", 0)
)
}

//int main(int argc, char **argv)
//{
//	dummyNS::Person person;
//
//	// Set field in person
//	// Set API 1
//	person._.Set(person.name, "newName");
//	// Set API 2
//	IJST_SET(person, age, 10);
//	// Access container
//	person.avec.push_back(1);
//	person.amap["value"] = 2;
//	// MakeValid API 1
//	person._.MakeValid(person.avec);
//	// MakeValid API 2
//	IJST_MAKE_VALID(person, amap);
//
//	// Serialize
//	Json::Value stream;
//	person._.Serialize(stream, true);
//	cout << stream.toStyledString();
//}

int main()
{
	std::cout << "main()" << endl;
	StTest stTest;
	stTest._.SetStrict(stTest.name, string("NewName"));
//	cout << "name status: " << stTest._.GetStatus(stTest.name) << endl;
	stTest._.SetStrict(stTest.age, 1);
//	cout << "age status: " << stTest._.GetStatus(stTest.age) << endl;

//	cout << "ptrSt: " << &stTest << endl;
//	cout << "ptrName: " << offsetof(StTest, name) << endl;
//	cout << "ptrAge: " << offsetof(StTest, age) << endl;
//	cout << stTest.name << endl;

	stTest.int_vec.push_back(1);
	stTest.int_vec.push_back(2);
	stTest._.MakeValid(stTest.int_vec);

	stTest.int_map["3"] = 3;
	stTest.int_map["4"] = 4;
	stTest.int_map["5"] = 5;
	stTest._.MakeValid(stTest.int_map);

	nstest::StTestI stTestI;
	stTestI._.Set(stTestI.name, "StTestI");
	stTest.st_test.push_back(stTestI);
	stTest._.MakeValid(stTest.st_test);

//	stTest._.ShowTag();

	int ret;
	ret = stTest._.SerializeInplace(true);
	rapidjson::StringBuffer buffer;

	buffer.Clear();

	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	stTest._.InnerStream().Accept(writer);

	cout << "Serializer: " << endl << buffer.GetString() << endl;
}


