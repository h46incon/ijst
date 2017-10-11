/**
 *				Serialization implementation of Primitive types
 */

// Including "ijst.h" is not necessary of compiler, but it will make life easier when coding in IDE
#include "ijst.h"

namespace ijst{
namespace detail {

template<>
class FSerializer<TypeClassPrim<FType::Int> > : public SerializerInterface {
public:
	typedef int VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp)
	{
		const VarType *fieldI = static_cast<const VarType *>(req.pField);
		req.buffer.SetInt(*fieldI);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp)
	{
		if (!req.stream.IsInt()) {
			resp.fStatus = FStatus::ParseFailed;
			resp.SetErrMsg("Value is not a Int");
			return Err::kDeserializeValueTypeError;
		}

		VarType *pBuffer = static_cast<VarType *>(req.pFieldBuffer);
		*pBuffer = req.stream.GetInt();
		return 0;
	}

};

template<>
class FSerializer<TypeClassPrim<FType::String> > : public SerializerInterface {
public:
	typedef std::string VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp)
	{
		const VarType *filedV = static_cast<const VarType *>(req.pField);
		req.buffer.SetString(filedV->c_str(), filedV->length(), req.allocator);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp)
	{
		if (!req.stream.IsString()) {
			resp.fStatus = FStatus::ParseFailed;
			resp.SetErrMsg("Value is not a String");
			return Err::kDeserializeValueTypeError;
		}

		VarType *pBuffer = static_cast<VarType *>(req.pFieldBuffer);
		*pBuffer = std::string(req.stream.GetString(), req.stream.GetStringLength());
		return 0;
	}
};

}	//namespace detail
}	//namespace ijst
