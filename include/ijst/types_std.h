/**************************************************************************************************
 *		Serialization implementation of Primitive types
 *		Created by h46incon on 2017/11/2.
 **************************************************************************************************/

#ifndef _IJST_TYPES_STD_HPP_INCLUDE_
#define	_IJST_TYPES_STD_HPP_INCLUDE_

#include "ijst.h"
#if __cplusplus >= 201103L
	#include <cstdint>
#else
	#include <stdint.h>
#endif

//! Declare a primitive type field. _T is a value in ijst::FType
#define IJST_TPRI(_T)	::ijst::detail::TypeClassPrim< ::ijst::FType::_T>

namespace ijst{
	//! Primitive field types
	struct FType {
		enum _E {
			Bool,
			Int,
			UInt32,
			UInt64,
			Int32,
			Int64,
			Str,
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

	typedef uint8_t FStoreBool; 		// Could not use bool type because std::vector<bool> is not a container!
	typedef int FStoreInt;
	typedef uint32_t FStoreUInt32;
	typedef uint64_t FStoreUInt64;
	typedef int32_t FStoreInt32;
	typedef int64_t FStoreInt64;
	typedef std::string FStoreString;

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
		FStoreRaw(FStoreRaw &&rhs)
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

		void Steal(FStoreRaw& rhs)
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
		BufferType& V() {return v;}
		const BufferType& V() const {return v;}
		//! See ijst::Accessor::GetAllocator
		AllocatorType& GetAllocator() {return *m_pAllocator;}
		const AllocatorType& GetAllocator() const {return *m_pAllocator;}
		//! See ijst::Accessor::GetOwnAllocator
		AllocatorType& GetOwnAllocator() {return m_pOwnDoc->GetAllocator();}
		const AllocatorType& GetOwnAllocator() const {return m_pOwnDoc->GetAllocator();}

	private:
		friend class detail::FSerializer<detail::TypeClassPrim<FType::Raw> >;
		BufferType v;
		AllocatorType* m_pAllocator;
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
				req.buffer.SetBool((*pField) != 0);
				return 0;
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
		};

		template<>
		class FSerializer<TypeClassPrim<FType::Int> > : public SerializerInterface {
		public:
			typedef ijst::FStoreInt VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.SetInt(*pField);
				return 0;
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
		};

		template<>
		class FSerializer<TypeClassPrim<FType::UInt32> > : public SerializerInterface {
		public:
			typedef ijst::FStoreUInt32 VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.SetUint(*pField);
				return 0;
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
		};

		template<>
		class FSerializer<TypeClassPrim<FType::UInt64> > : public SerializerInterface {
		public:
			typedef ijst::FStoreUInt64 VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.SetUint64(*pField);
				return 0;
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
		};

		template<>
		class FSerializer<TypeClassPrim<FType::Int32> > : public SerializerInterface {
		public:
			typedef ijst::FStoreInt32 VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.SetInt(*pField);
				return 0;
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
		};

		template<>
		class FSerializer<TypeClassPrim<FType::Int64> > : public SerializerInterface {
		public:
			typedef ijst::FStoreInt64 VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.SetInt64(*pField);
				return 0;
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
		};

		template<>
		class FSerializer<TypeClassPrim<FType::Str> > : public SerializerInterface {
		public:
			typedef ijst::FStoreString VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.SetString(pField->c_str(), pField->length(), req.allocator);
				return 0;
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
		};

		template<>
		class FSerializer<TypeClassPrim<FType::Raw> > : public SerializerInterface {
		public:
			typedef ijst::FStoreRaw VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				req.buffer.CopyFrom(pField->V(), req.allocator, true);
				return 0;
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

			virtual int SetAllocator(void *pField, AllocatorType &allocator) IJSTI_OVERRIDE
			{
				VarType *pFieldT = static_cast<VarType *>(pField);
				if (pFieldT->m_pAllocator == &allocator) {
					return 0;
				}
				BufferType temp;
				temp = pFieldT->v;
				pFieldT->v.CopyFrom(temp, allocator, false);
				pFieldT->m_pAllocator = &allocator;
				return 0;
			}
		};

	}	//namespace detail
}	//namespace ijst
#endif //_IJST_TYPES_STD_HPP_INCLUDE_
