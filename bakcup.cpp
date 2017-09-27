//
// Created by h46incon on 2017/9/20.
//
#if IJST_JSON_LIB == "JSON_CPP"
	#include "json/json.h"
	typedef Json::Value StoreType;
	#define IJSTI_STORE_LIST 		Json::Value(Json::arrayValue)
	#define IJSTI_STORE_MAP 		Json::Value(Json::objectValue)
	#define IJSTI_STORE_NULL		Json::Value(Json::nullValue)
	#define IJSTI_STORE_MOVE(dest, src)			dest.swap(src)
#elif IJST_JSON_LIB == "RAPID_JSON"

class SerializerDispatcher {
private:
	typedef IJMapType<FType::_E, SerializerInterface*> InterfaceMap;
public:
	SerializerInterface *Get(FType::_E ftype)
	{
		InterfaceMap::iterator itera = mapInterface.find(ftype);
		if (itera == mapInterface.end()) {
			throw std::runtime_error("Unimplement ftype: " + ftype);
		}
		return itera->second;
	}

private:
	template <FType::_E ftype> friend class SerializerRegister;
	InterfaceMap mapInterface;
};

typedef Singleton<SerializerDispatcher> SerializerDispatcherS;

template<class ftype, unsigned int fdecorate>
struct FVarType {
	typedef void Type;
};
#define GetVarType(ftype, fdesc) FVarType<(ftype), (desc) & FDesc::_MaskDecorate>::Type


template<class _T>
struct FVarType<_T, 0> {
	typedef typename FSerializer<_T>::VarType Type;
};

template<class _T>
struct FVarType<_T, FDesc::Repeat> {
	typedef typename FSerializer<_T>::VarType ElemType;
	typedef typename std::vector<ElemType> Type;
};

template<class _T>
struct FVarType<_T, FDesc::Repeat_GT1> {
	typedef typename FSerializer<_T>::VarType ElemType;
	typedef typename std::vector<ElemType> Type;
};

template<class _T>
struct FVarType<_T, FDesc::Map> {
	typedef typename FSerializer<_T>::VarType ElemType;
	typedef typename IJMapType<std::string, ElemType> Type;
};

template <FType::_E ftype>
class SerializerRegister{
public:
	SerializerRegister()
	{
		std::cout << "SerializerRegister: " << ftype << std::endl;
		SerializerDispatcherS::GetInstance()->mapInterface[ftype] = FSerializerS(ftype)::GetInstance();
	}
};

SerializerRegister<FType::Int> _reg_Int;
SerializerRegister<FType::String> _reg_String;
