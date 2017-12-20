/**************************************************************************************************
 *		Serialization implementation of Primitive types
 *		Created by h46incon on 2017/11/2.
 **************************************************************************************************/

#ifndef _IJST_TYPES_STD_HPP_INCLUDE_
#define	_IJST_TYPES_STD_HPP_INCLUDE_

#include "ijst.h"

// Use int types declared in rapidjson
#include <rapidjson/rapidjson.h>

//! Declare a primitive type field. _T is a value in ijst::FType
#define IJST_TPRI(_T)	::ijst::detail::TypeClassPrim< ::ijst::FType::_T>

namespace ijst{
	//! Primitive field types
	struct FType {
		enum _E {
			//! bool -> uint8_t
			Bool,
			//! bool -> bool. @note: Could not declare IJST_TVEC(IJST_TPRI(RBool)) now
			RBool,
			//! bool -> a wrapper of bool
			WBool,
			//! number -> int
			Int,
			//! number -> uint64_t
			Int64,
			//! number -> unsigned int
			UInt,
			//! number -> uint64_t
			UInt64,
			//! number -> double
			Double,
			//! string -> std::string
			Str,
			//! anything -> a wrapper of rapidjson::Value
			Raw,
		};
	};
	typedef FType::_E EFType;

	namespace detail {
		template<EFType _T>
		struct TypeClassPrim {
			// nothing
		};
	}	// namespace detail

	typedef uint8_t 		FStoreBool;
	typedef bool 			FStoreRBool;
	typedef int 			FStoreInt;
	typedef int64_t 		FStoreInt64;
	typedef unsigned int 	FStoreUInt;
	typedef uint64_t 		FStoreUInt64;
	typedef double 			FStoreDouble;

#if IJST_USE_SL_WRAPPER
	typedef ijst::SLWrapper<std::string> FStoreString;
#else
	typedef std::string FStoreString;
#endif

	class FStoreWBool {
	public:
		FStoreWBool() : m_val(false) {}
		FStoreWBool(bool _val) : m_val(_val) {}
		operator bool() const { return m_val; }

	private:
		template <typename T> FStoreWBool(T);	// deleted
		bool m_val;
	};

	class FStoreRaw {
	public:
		FStoreRaw()
		{
			m_pOwnDoc = new rapidjson::Document();
			m_pAllocator = &m_pOwnDoc->GetAllocator();
		}

		FStoreRaw(const FStoreRaw &rhs)
		{
			m_pOwnDoc = new rapidjson::Document();
			m_pAllocator = &m_pOwnDoc->GetAllocator();
			v.CopyFrom(rhs.v, *m_pAllocator, true);
		}

		#if __cplusplus >= 201103L
		FStoreRaw(FStoreRaw &&rhs) IJSTI_NOEXCEPT
		{
			m_pOwnDoc = IJST_NULL;
			m_pAllocator = IJST_NULL;
			Steal(rhs);
		}
		#endif

		FStoreRaw &operator=(FStoreRaw rhs)
		{
			Steal(rhs);
			return *this;
		}

		void Steal(FStoreRaw& rhs) IJSTI_NOEXCEPT
		{
			if (this == &rhs) {
				return;
			}

			delete m_pOwnDoc;
			m_pOwnDoc = rhs.m_pOwnDoc;
			rhs.m_pOwnDoc = IJST_NULL;

			m_pAllocator = rhs.m_pAllocator;
			rhs.m_pAllocator = IJST_NULL;
			v = rhs.v;
		}

		~FStoreRaw()
		{
			delete m_pOwnDoc;
			m_pOwnDoc = IJST_NULL;
		}

		//! Get actually value in object
		JsonValue& V() {return v;}
		const JsonValue& V() const {return v;}
		//! See ijst::Accessor::GetAllocator
		JsonAllocator& GetAllocator() {return *m_pAllocator;}
		const JsonAllocator& GetAllocator() const {return *m_pAllocator;}
		//! See ijst::Accessor::GetOwnAllocator
		JsonAllocator& GetOwnAllocator() {return m_pOwnDoc->GetAllocator();}
		const JsonAllocator& GetOwnAllocator() const {return m_pOwnDoc->GetAllocator();}

	private:
		friend class detail::FSerializer<detail::TypeClassPrim<FType::Raw> >;
		JsonValue v;
		JsonAllocator* m_pAllocator;
		rapidjson::Document* m_pOwnDoc;		// use pointer to make FStoreRaw be a standard-layout type
	};

	namespace detail {
		template<>
		class FSerializer<TypeClassPrim<FType::Bool> > : public SerializerInterface {
		public:
			typedef ijst::FStoreBool VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				return (req.writer.Bool((*pField) != 0) ? 0 : Err::kWriteFailed);
			}

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
			{
				if (!req.stream.IsBool()) {
					resp.fStatus = FStatus::kParseFailed;
					resp.SetErrMsg("Value is not Bool");
					return Err::kDeserializeValueTypeError;
				}

				VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
				*pField = static_cast<VarType >(req.stream.GetBool() ? 1 : 0);
				return 0;
			}

#if IJST_ENABLE_TO_JSON_OBJECT
			virtual int ToJson(const ToJsonReq &req, ToJsonResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.SetBool((*pField) != 0);
				return 0;
			}
#endif
		};

		#define IJSTI_SERIALIZER_BOOL()																					\
			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE							\
			{																											\
				const VarType *pField = static_cast<const VarType *>(req.pField);										\
				return (req.writer.Bool(*pField) ? 0 : Err::kWriteFailed);												\
			}																											\
																														\
			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE			\
			{																											\
				if (!req.stream.IsBool()) {																				\
					resp.fStatus = FStatus::kParseFailed;																\
					resp.SetErrMsg("Value is not Bool");																\
					return Err::kDeserializeValueTypeError;																\
				}																										\
				VarType *pField = static_cast<VarType *>(req.pFieldBuffer);												\
				*pField = req.stream.GetBool();																			\
				return 0;																								\
			}																											\

#if IJST_ENABLE_TO_JSON_OBJECT
		#define IJSTI_SERIALIZER_BOOL_TO_JSON()																			\
			virtual int ToJson(const ToJsonReq &req, ToJsonResp &resp) IJSTI_OVERRIDE									\
			{																											\
				const VarType *pField = static_cast<const VarType *>(req.pField);										\
				req.buffer.SetBool(*pField);																			\
				return 0;																								\
			}
#else
	#define IJSTI_SERIALIZER_BOOL_TO_JSON()				// empty
#endif
		template<>
		class FSerializer<TypeClassPrim<FType::RBool> > : public SerializerInterface {
		public:
			typedef ijst::FStoreRBool VarType;

			IJSTI_SERIALIZER_BOOL()
			IJSTI_SERIALIZER_BOOL_TO_JSON()
		};

		template<>
		class FSerializer<TypeClassPrim<FType::WBool> > : public SerializerInterface {
		public:
			typedef ijst::FStoreWBool VarType;

			IJSTI_SERIALIZER_BOOL()
			IJSTI_SERIALIZER_BOOL_TO_JSON()
		};

		template<>
		class FSerializer<TypeClassPrim<FType::Int> > : public SerializerInterface {
		public:
			typedef ijst::FStoreInt VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				return (req.writer.Int(*pField) ? 0 : Err::kWriteFailed);
			}

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
			{
				if (!req.stream.IsInt()) {
					resp.fStatus = FStatus::kParseFailed;
					resp.SetErrMsg("Value is not Int");
					return Err::kDeserializeValueTypeError;
				}

				VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
				*pField = req.stream.GetInt();
				return 0;
			}

#if IJST_ENABLE_TO_JSON_OBJECT
			virtual int ToJson(const ToJsonReq &req, ToJsonResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.SetInt(*pField);
				return 0;
			}
#endif
		};

		template<>
		class FSerializer<TypeClassPrim<FType::Int64> > : public SerializerInterface {
		public:
			typedef ijst::FStoreInt64 VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				return (req.writer.Int64(*pField) ? 0 : Err::kWriteFailed);
			}

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
			{
				if (!req.stream.IsInt64()) {
					resp.fStatus = FStatus::kParseFailed;
					resp.SetErrMsg("Value is not Int64");
					return Err::kDeserializeValueTypeError;
				}

				VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
				*pField = req.stream.GetInt64();
				return 0;
			}

#if IJST_ENABLE_TO_JSON_OBJECT
			virtual int ToJson(const ToJsonReq &req, ToJsonResp &resp) IJSTI_OVERRIDE
				{
					const VarType *pField = static_cast<const VarType *>(req.pField);
					req.buffer.SetInt64(*pField);
					return 0;
				}
#endif
		};

	template<>
		class FSerializer<TypeClassPrim<FType::UInt> > : public SerializerInterface {
		public:
			typedef ijst::FStoreUInt VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				return (req.writer.Uint(*pField) ? 0 : Err::kWriteFailed);
			}

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
			{
				if (!req.stream.IsUint()) {
					resp.fStatus = FStatus::kParseFailed;
					resp.SetErrMsg("Value is not Uint");
					return Err::kDeserializeValueTypeError;
				}

				VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
				*pField = req.stream.GetUint();
				return 0;
			}

#if IJST_ENABLE_TO_JSON_OBJECT
			virtual int ToJson(const ToJsonReq &req, ToJsonResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.SetUint(*pField);
				return 0;
			}
#endif
		};

		template<>
		class FSerializer<TypeClassPrim<FType::UInt64> > : public SerializerInterface {
		public:
			typedef ijst::FStoreUInt64 VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				return (req.writer.Uint64(*pField) ? 0 : Err::kWriteFailed);
			}

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
			{
				if (!req.stream.IsUint64()) {
					resp.fStatus = FStatus::kParseFailed;
					resp.SetErrMsg("Value is not Uint64");
					return Err::kDeserializeValueTypeError;
				}

				VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
				*pField = req.stream.GetUint64();
				return 0;
			}

#if IJST_ENABLE_TO_JSON_OBJECT
			virtual int ToJson(const ToJsonReq &req, ToJsonResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.SetUint64(*pField);
				return 0;
			}
#endif

		};

		template<>
		class FSerializer<TypeClassPrim<FType::Double> > : public SerializerInterface {
		public:
			typedef ijst::FStoreDouble VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				return (req.writer.Double(*pField) ? 0 : Err::kWriteFailed);
			}

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
			{
				if (!req.stream.IsDouble()) {
					resp.fStatus = FStatus::kParseFailed;
					resp.SetErrMsg("Value is not Int");
					return Err::kDeserializeValueTypeError;
				}

				VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
				*pField = req.stream.GetDouble();
				return 0;
			}

#if IJST_ENABLE_TO_JSON_OBJECT
			virtual int ToJson(const ToJsonReq &req, ToJsonResp &resp) IJSTI_OVERRIDE
				{
					const VarType *pField = static_cast<const VarType *>(req.pField);
					req.buffer.SetDouble(*pField);
					return 0;
				}
#endif
	};


		template<>
		class FSerializer<TypeClassPrim<FType::Str> > : public SerializerInterface {
		public:
			typedef ijst::FStoreString VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				const std::string &field = IJST_CONT_VAL(*pField);
				return (req.writer.String(field.c_str(), static_cast<rapidjson::SizeType>(field.length())) ? 0 : Err::kWriteFailed);
			}

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
			{
				if (!req.stream.IsString()) {
					resp.fStatus = FStatus::kParseFailed;
					resp.SetErrMsg("Value is not String");
					return Err::kDeserializeValueTypeError;
				}

				VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
				*pField = std::string(req.stream.GetString(), req.stream.GetStringLength());
				return 0;
			}

#if IJST_ENABLE_TO_JSON_OBJECT
			virtual int ToJson(const ToJsonReq &req, ToJsonResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.SetString(IJST_CONT_VAL(*pField).c_str(), IJST_CONT_VAL(*pField).length(), req.allocator);
				return 0;
			}
#endif
		};

		template<>
		class FSerializer<TypeClassPrim<FType::Raw> > : public SerializerInterface {
		public:
			typedef ijst::FStoreRaw VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				return (pField->V().Accept(req.writer) ? 0 : Err::kWriteFailed);
			}

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
			{
				VarType *pField = static_cast<VarType *>(req.pFieldBuffer);

				if (req.canMoveSrc) {
					pField->v.Swap(req.stream);
					pField->m_pAllocator = &req.allocator;
				}
				else {
					pField->v.CopyFrom(req.stream, *pField->m_pAllocator, true);
				}
				return 0;
			}

#if IJST_ENABLE_TO_JSON_OBJECT
			virtual int ToJson(const ToJsonReq &req, ToJsonResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.CopyFrom(pField->V(), req.allocator, true);
				return 0;
			}

			virtual int SetAllocator(void *pField, JsonAllocator &allocator) IJSTI_OVERRIDE
			{
				VarType *pFieldT = static_cast<VarType *>(pField);
				if (pFieldT->m_pAllocator == &allocator) {
					return 0;
				}
				JsonValue temp;
				temp = pFieldT->v;
				pFieldT->v.CopyFrom(temp, allocator, false);
				pFieldT->m_pAllocator = &allocator;
				return 0;
			}
#endif
		};

	}	//namespace detail
}	//namespace ijst
#endif //_IJST_TYPES_STD_HPP_INCLUDE_
