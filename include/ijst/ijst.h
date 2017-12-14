//
// Created by h46incon on 2017/9/19.
//

#ifndef _IJST_HPP_INCLUDE_
#define _IJST_HPP_INCLUDE_

#include "std_layout_wrapper.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

#include <cassert>
#include <cstddef>
#include <map>
#include <vector>
#include <string>
#include <sstream>

/**	========================================================================================
 *				Public Interface
 */

/*!
 * IJST_AUTO_META_INIT
 *	if define IJST_AUTO_META_INIT before include this header, the meta class information will init before main.
 *	That's will make it thread-safe to init meta class information before C++11.
 *	The feature is enable default before C++11. So set the value to 0 to force disable it.
*/
#ifndef IJST_AUTO_META_INIT
	#if __cplusplus < 201103L
		#define IJST_AUTO_META_INIT		1
	#else
		#define IJST_AUTO_META_INIT		0
	#endif
#endif

/*!
 * IJST_ASSERT
 *	By default, ijst uses assert() for errors that indicate a bug.
 *	User can override it by defining IJST_ASSERT(x) macro.
*/
#ifndef IJST_ASSERT
	#define IJST_ASSERT(x) assert(x)
#endif

/*!
 * IJST_USE_SL_WRAPPER
 *	By default, ijst uses std::vector, std::map, and std::string when defining struct, and the offset of each field is
 *	computed by "((Struct*)0)->field".
 *	User can specify IJST_USE_SL_WRAPPER to 1 to use ijst::Vector, ijst::Map, ijst::SLWrapper<std::string> and offsetof()
 *	macro. This option will make the behaviour more standards-compliant, but will bring a little inconvenience when using.
*/
#ifndef IJST_USE_SL_WRAPPER
	#define IJST_USE_SL_WRAPPER	0
#endif

#define IJST_OUT

//! Declare a ijst struct
#define IJST_DEFINE_STRUCT(stName, ...) \
    IJSTI_DEFINE_STRUCT_IMPL(IJSTI_PP_NFIELD(stName, ##__VA_ARGS__), stName, F, ##__VA_ARGS__)
//! Declare a ijst struct with getter
#define IJST_DEFINE_STRUCT_WITH_GETTER(stName, ...) \
    IJSTI_DEFINE_STRUCT_IMPL(IJSTI_PP_NFIELD(stName, ##__VA_ARGS__), stName, T, ##__VA_ARGS__)

//! Declare a vector<_T> field
#define IJST_TVEC(_T)	::ijst::detail::TypeClassVec< _T>
//! Declare a map<_T> field
#define IJST_TMAP(_T)	::ijst::detail::TypeClassMap< _T>
//! Declare a object field which _T is a ijst struct type
#define IJST_TOBJ(_T)	::ijst::detail::TypeClassObj< _T>

//! Get status of field in obj
#define IJST_GET_STATUS(obj, field)				obj._.GetStatus(& ((obj).field))
//! Mark status of field in obj to FStatus::kValid
#define IJST_MARK_VALID(obj, field)				obj._.MarkValid(& ((obj).field))
//! Mark status of field in obj to FStatus::kNull
#define IJST_MARK_NULL(obj, field)				obj._.MarkNull(& ((obj).field))
//! Mark status of field in obj to FStatus::kMissing
#define IJST_MARK_MISSING(obj, field)			obj._.MarkMissing(& ((obj).field))
//! Set field in obj to val and mark it valid.
#define IJST_SET(obj, field, val)				obj._.Set((obj).field, (val))
//! Set field in obj to val and mark it valid. Type of field and val must be same
#define IJST_SET_STRICT(obj, field, val)		obj._.SetStrict((obj).field, (val))

//! IJST_NULL
#if __cplusplus >= 201103L
	#define IJST_NULL 			nullptr
#else
	#define IJST_NULL 			NULL
#endif

//! Wrappers
#if IJST_USE_SL_WRAPPER
	#define IJST_CONT_VEC(...)			ijst::Vector<__VA_ARGS__>
	#define IJST_CONT_MAP(...)			ijst::Map<__VA_ARGS__>
	#define IJST_CONT_VAL(_v)			(_v).Val()
	#define IJST_OFFSETOF(_T, member)	offsetof(_T, member)
#else
	#define IJST_CONT_VEC(...)			std::vector<__VA_ARGS__>
	#define IJST_CONT_MAP(...)			std::map<__VA_ARGS__>
	#define IJST_CONT_VAL(_v)			(_v)
	#define IJST_OFFSETOF(_T, member)	((size_t)&(((_T*)0)->member))
#endif


namespace ijst {
	typedef rapidjson::Value JsonValue;
	typedef rapidjson::MemoryPoolAllocator<> 	JsonAllocator;

	//! Field description
	struct FDesc {
		static const unsigned int _MaskDesc 		= 0x000000FF;
		static const unsigned int Optional 			= 0x00000001;
		static const unsigned int Nullable 			= 0x00000002;
		static const unsigned int ElemNotEmpty 		= 0x00000004;
		// FDesc of Element (i.e. Nullable, ElemNotEmpty) inside container is hard to represent, has not plan to implement it
	};

	//! Field status
	struct FStatus {
	public:
		enum _E {
			kNotAField,
			kMissing,
			kNull,
			kParseFailed,
			kValid,
		};
	};
	typedef FStatus::_E EFStatus;

	//! Error code
	struct Err {
		static const int kSucc 							= 0x00000000;
		static const int kDeserializeValueTypeError 	= 0x00001001;
		static const int kDeserializeSomeFiledsInvalid 	= 0x00001002;
		static const int kDeserializeParseFaild 		= 0x00001003;
		static const int kDeserializeElemEmpty 			= 0x00001004;
		static const int kDeserializeSomeUnknownMember	= 0x00001005;
		static const int kInnerError 					= 0x00002001;
		static const int kWriteFailed					= 0x00003001;
	};

	//! Behaviour when meeting unknown member in json object
	struct UnknownMode {
	public:
		enum _E {
			kKeep,		// keep unknown fields
			kIgnore,	// ignore unknown fields
			kError		// error when unknown fields occurs
		};
	};
	typedef UnknownMode::_E EUnknownMode;

	#define IJSTI_OPTIONAL_BASE_DEFINE(_T)						\
		public:													\
			Optional(_T* _pVal) : m_pVal(_pVal) {}				\
			_T* Ptr() const { return m_pVal; }					\
		private:												\
			_T* const m_pVal;

	template <typename _T>
	class Optional
	{
		typedef _T ValType;
		IJSTI_OPTIONAL_BASE_DEFINE(ValType)
	public:
		_T* operator->() const
		{
			static _T empty(false);
			if (m_pVal == IJST_NULL) {
				return &empty;
			}
			else {
				return m_pVal;
			}
		}
	};

	template <typename _TElem>
	class Optional <IJST_CONT_VEC(_TElem)>
	{
		typedef IJST_CONT_VEC(_TElem) ValType;
		IJSTI_OPTIONAL_BASE_DEFINE(ValType)
	public:
		Optional<_TElem> operator[](typename std::vector<_TElem>::size_type i) const
		{
			if (m_pVal == IJST_NULL || IJST_CONT_VAL(*m_pVal).size() <= i) {
				return Optional<_TElem>(IJST_NULL);
			}
			return Optional<_TElem>(&(*m_pVal)[i]);
		}

	};

	template <typename _TElem>
	class Optional <const IJST_CONT_VEC(_TElem)>
	{
		typedef const IJST_CONT_VEC(_TElem) ValType;
		IJSTI_OPTIONAL_BASE_DEFINE(ValType)
	public:
		Optional<const _TElem> operator[](typename std::vector<_TElem>::size_type i) const
		{
			if (m_pVal == IJST_NULL || IJST_CONT_VAL(*m_pVal).size() <= i) {
				return Optional<const _TElem>(IJST_NULL);
			}
			return Optional<const _TElem>(&(*m_pVal)[i]);
		}

	};

	template <typename _TElem>
	class Optional <IJST_CONT_MAP(std::string, _TElem)>
	{
		typedef IJST_CONT_MAP(std::string, _TElem) ValType;
		IJSTI_OPTIONAL_BASE_DEFINE(ValType)
	public:
		Optional<_TElem> operator[](const std::string& key) const
		{
			if (m_pVal == IJST_NULL) {
				return Optional<_TElem>(IJST_NULL);
			}
			typename std::map<std::string, _TElem>::iterator it = IJST_CONT_VAL(*m_pVal).find(key);
			if (it == IJST_CONT_VAL(*m_pVal).end()){
				return Optional<_TElem>(IJST_NULL);
			}
			else {
				return Optional<_TElem>(&it->second);
			}
		}

	};

	template <typename _TElem>
	class Optional <const IJST_CONT_MAP(std::string, _TElem)>
	{
		typedef const IJST_CONT_MAP(std::string, _TElem) ValType;
		IJSTI_OPTIONAL_BASE_DEFINE(ValType)
	public:
		Optional<const _TElem> operator[](const std::string& key) const
		{
			if (m_pVal == IJST_NULL) {
				return Optional<const _TElem>(IJST_NULL);
			}
			typename std::map<std::string, _TElem>::const_iterator it = IJST_CONT_VAL(*m_pVal).find(key);
			if (it == IJST_CONT_VAL(*m_pVal).end()){
				return Optional<const _TElem>(IJST_NULL);
			}
			else {
				return Optional<const _TElem>(&it->second);
			}
		}

	};
	/**	========================================================================================
	 *				Inner Interface
	 */
	namespace detail {
		typedef rapidjson::Writer<rapidjson::StringBuffer> JsonWriter;
		//! Set errMsg to pErrMsgOut when not null.
		//! Use macro instead of function to avoid compute errMsg when pErrMsgOut is null.
		#define IJSTI_SET_ERRMSG(pErrMsgOut, errMsg)				\
			do {													\
				if ((pErrMsgOut) != IJST_NULL) {					\
					*(pErrMsgOut) = (errMsg);						\
				}													\
			} while (false)


		#if __cplusplus >= 201103L
			#define IJSTI_MOVE(val) 	std::move((val))
			#define IJSTI_OVERRIDE		override
			#define IJSTI_NOEXCEPT		noexcept
		#else
			#define IJSTI_MOVE(val) 	(val)
			#define IJSTI_OVERRIDE
			#define IJSTI_NOEXCEPT
		#endif

		#if IJST_AUTO_META_INIT
			#define IJSTI_TRY_INIT_META_BEFORE_MAIN(_T)			::ijst::detail::Singleton< _T>::InitInstanceBeforeMain();
		#else
			#define IJSTI_TRY_INIT_META_BEFORE_MAIN(_T)
		#endif

		template<class _T>
		struct TypeClassVec {
			// nothing
		};

		template<class _T>
		struct TypeClassMap {
			// nothing
		};

		template<class _T>
		struct TypeClassObj {
			// nothing
		};

		/**
		 * Singleton interface
		 * @tparam _T type
		 */
		template<typename _T>
		class Singleton {
		public:
			inline static _T *GetInstance()
			{
				static _T instance;
				return &instance;
			}

			inline static void InitInstanceBeforeMain()
			{
				// When accessing initInstanceTag in code, the GetInstance() function will be called before main
				volatile void* dummy = initInstanceTag;
				(void)dummy;
			}

		private:
			static void* initInstanceTag;
		};
		template<typename _T> void *Singleton<_T>::initInstanceTag = Singleton<_T>::GetInstance();


		class SerializerInterface {
		public:
			struct SerializeReq {
				// true if need serialize all field, false if serialize only valid field
				bool pushAllField;

				// Pointer of field to serialize.
				// The actual type of field should be decide in the derived class
				const void* pField;

				// true if move context in pField to avoid copy when possible
				bool canMoveSrc;

				// Output buffer info. The instance should serialize in this object and use allocator
				IJST_OUT JsonValue& buffer;
				JsonAllocator& allocator;

				SerializeReq(JsonValue &_buffer, JsonAllocator &_allocator,
							 const void *_pField, bool _canMoveSrc,
							 bool _pushAllField)
						: pushAllField(_pushAllField)
						  , pField(_pField)
						  , canMoveSrc(_canMoveSrc)
						  , buffer(_buffer)
						  , allocator(_allocator)
				{ }
			};

			struct SerializeResp {
			};

			struct WriteReq {
				// true if need serialize all field, false if serialize only valid field
				bool pushAllField;

				// Pointer of field to serialize.
				// The actual type of field should be decide in the derived class
				const void* pField;

				JsonWriter& writer;

				WriteReq(JsonWriter& _writer, const void *_pField, bool _pushAllField)
						: pushAllField(_pushAllField)
						, pField(_pField)
						, writer(_writer)
				{ }
			};

			struct WriteResp {
			};

			struct DeserializeReq {
				// Pointer of deserialize output.
				// The instance should deserialize in this object
				// The actual type of field should be decide in the derived class
				void* pFieldBuffer;

				// The input stream and allocator
				JsonValue& stream;
				JsonAllocator& allocator;

				// true if move context in stream to avoid copy when possible
				bool canMoveSrc;
				EUnknownMode unknownMode;

				DeserializeReq(JsonValue &_stream, JsonAllocator &_allocator,
							   EUnknownMode _unknownMode, bool _canMoveSrc,
							   void *_pField)
						: pFieldBuffer(_pField)
						  , stream(_stream)
						  , allocator(_allocator)
						  , canMoveSrc(_canMoveSrc)
						  , unknownMode(_unknownMode)
				{ }
			};

			struct DeserializeResp {
				EFStatus fStatus;
				size_t fieldCount;
				const bool needErrMsg;
				std::string errMsg;

				explicit DeserializeResp(bool _needErrMsg = false) :
						fStatus(FStatus::kMissing),
						fieldCount(0),
						needErrMsg(_needErrMsg)
				{ }

				template<typename _T>
				bool SetErrMsg(const _T &_errMsg)
				{
					if (!needErrMsg) {
						return false;
					}
					errMsg = _errMsg;
					return true;
				}

				template<typename _T>
				bool CombineErrMsg(const _T & _errMsg, const DeserializeResp& childResp)
				{
					if (!needErrMsg) {
						return false;
					}

					std::stringstream oss;
					oss << _errMsg << "[" << childResp.errMsg << "]";
					errMsg = oss.str();
					return true;
				}
			};

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp)= 0;

			virtual int Write(const WriteReq& req, WriteResp &resp) = 0;

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp)= 0;

			virtual int SetAllocator(void *pField, JsonAllocator &allocator) {return 0;}

			virtual ~SerializerInterface()
			{ }
		};

		/**
		 * Template interface of serialization class
		 * This template is unimplemented, and will throw a compile error when use it.
		 * @tparam _T class
		 */
		template<class _T>
		class FSerializer : public SerializerInterface {
		public:
			#if __cplusplus >= 201103L
			static_assert(!std::is_same<_T, _T>::value,
						  "This base template should not be instantiated. (Maybe use wrong param when define ijst struct)");
			#endif

			typedef void VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE = 0;

			virtual int Write(const WriteReq& req, WriteResp &resp) IJSTI_OVERRIDE = 0;

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE = 0;

			virtual int SetAllocator(void *pField, JsonAllocator &allocator) IJSTI_OVERRIDE;
		};

		#define IJSTI_FSERIALIZER_INS(_T) ::ijst::detail::Singleton< ::ijst::detail::FSerializer< _T> >::GetInstance()

		/**	========================================================================================
		 *				Private
		 */

		/**
		 * Serialization class of Object types
		 * @tparam _T class
		 */
		template<class _T>
		class FSerializer<TypeClassObj<_T> > : public SerializerInterface {
		public:
			typedef _T VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				_T *pField = (_T *) req.pField;
				return pField->_.ISerialize(req, resp);
			}

			virtual int Write(const WriteReq& req, WriteResp &resp) IJSTI_OVERRIDE
			{
				_T *pField = (_T *) req.pField;
				return pField->_.IWrite(req, resp);
			}

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
			{
				_T *pField = (_T *) req.pFieldBuffer;
				return pField->_.IDeserialize(req, resp);
			}

			virtual int SetAllocator(void* pField, JsonAllocator& allocator) IJSTI_OVERRIDE
			{
				_T *pFieldT = (_T *) pField;
				return pFieldT->_.ISetAllocator(pField, allocator);
			}
		};

		/**
		 * Serialization class of Vector types
		 * @tparam _T class
		 */
		template<class _T>
		class FSerializer<TypeClassVec<_T> > : public SerializerInterface {
		private:
			typedef typename FSerializer<_T>::VarType ElemVarType;
			typedef std::vector<ElemVarType> RealVarType;
		public:
			typedef IJST_CONT_VEC(ElemVarType) VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pFieldWrapper = static_cast<const VarType *>(req.pField);
				assert(pFieldWrapper != IJST_NULL);
				const RealVarType& field = IJST_CONT_VAL(*pFieldWrapper);
				SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);
				req.buffer.SetArray();
				req.buffer.Reserve(static_cast<rapidjson::SizeType>(field.size()), req.allocator);

				for (typename RealVarType::const_iterator itera = field.begin(); itera != field.end(); ++itera) {
					JsonValue newElem;
					SerializeReq elemReq(newElem, req.allocator, &(*itera), req.canMoveSrc, req.pushAllField);
					SerializeResp elemResp;
					int ret = interface->Serialize(elemReq, elemResp);

					if (ret != 0) {
						return ret;
					}
					req.buffer.PushBack(newElem, req.allocator);
				}
				return 0;
			}

			virtual int Write(const WriteReq &req, WriteResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pFieldWrapper = static_cast<const VarType *>(req.pField);
				assert(pFieldWrapper != IJST_NULL);
				const RealVarType& field = IJST_CONT_VAL(*pFieldWrapper);
				SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);

				if (!req.writer.StartArray()) {
					return Err::kWriteFailed;
				}

				for (typename RealVarType::const_iterator itera = field.begin(); itera != field.end(); ++itera) {
					WriteReq elemReq(req.writer, &(*itera), req.pushAllField);
					WriteResp elemResp;
					int ret = interface->Write(elemReq, elemResp);

					if (ret != 0) {
						return ret;
					}
				}

				if (!req.writer.EndArray()) {
					return Err::kWriteFailed;
				}
				return 0;
			}

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
			{
				if (!req.stream.IsArray()) {
					resp.fStatus = FStatus::kParseFailed;
					resp.SetErrMsg("Value is not a Array");
					return Err::kDeserializeValueTypeError;
				}

				VarType *pFieldWrapper = static_cast<VarType *>(req.pFieldBuffer);
				assert(pFieldWrapper != IJST_NULL);
				RealVarType& field = IJST_CONT_VAL(*pFieldWrapper);
				field.clear();
				// field.shrink_to_fit();
				field.reserve(req.stream.Size());
				SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(_T);

				for (rapidjson::Value::ValueIterator itVal = req.stream.Begin();
					 itVal != req.stream.End(); ++itVal)
				{
					// Alloc buffer
					// New a elem buffer in container first to avoid copy
					// Use resize() instead of push_back() to avoid copy constructor in C++11
					field.resize(field.size() + 1);
					DeserializeReq elemReq(*itVal, req.allocator,
										   req.unknownMode, req.canMoveSrc, &field.back());

					// Deserialize
					DeserializeResp elemResp(resp.needErrMsg);
					int ret = serializerInterface->Deserialize(elemReq, elemResp);
					if (ret != 0)
					{
						field.pop_back();
						if (resp.needErrMsg)
						{
							std::stringstream oss;
							oss << "Deserialize elem error. index: " << field.size() << ", err: ";
							resp.CombineErrMsg(oss.str(), elemResp);
						}
						return ret;
					}
					resp.fStatus = FStatus::kParseFailed;
					++resp.fieldCount;
				}
				resp.fStatus = FStatus::kValid;
				return 0;
			}

			virtual int SetAllocator(void *pField, JsonAllocator &allocator) IJSTI_OVERRIDE
			{
				VarType *pFieldWrapper = static_cast<VarType *>(pField);
				assert(pFieldWrapper != IJST_NULL);
				RealVarType& field = IJST_CONT_VAL(*pFieldWrapper);
				SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);

				// Loop
				for (typename RealVarType::iterator itera = field.begin(); itera != field.end(); ++itera)
				{
					int ret = interface->SetAllocator(&(*itera), allocator);
					if (ret != 0) {
						return ret;
					}
				}

				return 0;
			}
		};

		/**
		 * Serialization class of Map types
		 * @tparam _T class
		 */
		template<class _T>
		class FSerializer<TypeClassMap<_T> > : public SerializerInterface {
		private:
			typedef typename FSerializer<_T>::VarType ElemVarType;
			typedef std::map<std::string, ElemVarType> RealVarType;
		public:
			typedef IJST_CONT_MAP(std::string, ElemVarType) VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pFieldWrapper = static_cast<const VarType *>(req.pField);
				assert(pFieldWrapper != IJST_NULL);
				const RealVarType &field = IJST_CONT_VAL(*pFieldWrapper);
				SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);
				if (!req.buffer.IsObject()) {
					req.buffer.SetObject();
				}

				for (typename RealVarType::const_iterator itFieldMember = field.begin(); itFieldMember != field.end(); ++itFieldMember)
				{
					// Init
					const void* pFieldValue = &itFieldMember->second;
					JsonValue newElem;
					SerializeReq elemReq(newElem, req.allocator, pFieldValue, req.canMoveSrc, req.pushAllField);
					SerializeResp elemResp;
					int ret = interface->Serialize(elemReq, elemResp);

					// Check return
					if (ret != 0) {
						return ret;
					}

					// Add member by copy key name
					rapidjson::GenericStringRef<char> fieldNameRef =
							rapidjson::StringRef(itFieldMember->first.c_str(), itFieldMember->first.length());
					req.buffer.AddMember(
							rapidjson::Value().SetString(fieldNameRef, req.allocator),
							newElem,
							req.allocator
					);
				}
				return 0;
			}

			virtual int Write(const WriteReq &req, WriteResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pFieldWrapper = static_cast<const VarType *>(req.pField);
				assert(pFieldWrapper != IJST_NULL);
				const RealVarType& field = IJST_CONT_VAL(*pFieldWrapper);
				SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);

				if (!req.writer.StartObject()) {
					return Err::kWriteFailed;
				}

				for (typename RealVarType::const_iterator itFieldMember = field.begin(); itFieldMember != field.end(); ++itFieldMember) {
					const std::string& key = itFieldMember->first;
					if (!req.writer.Key(key.c_str(), static_cast<rapidjson::SizeType>(key.length()))) {
						return Err::kWriteFailed;
					}

					WriteReq elemReq(req.writer, &(itFieldMember->second), req.pushAllField);
					WriteResp elemResp;
					int ret = interface->Write(elemReq, elemResp);

					if (ret != 0) {
						return ret;
					}
				}

				if (!req.writer.EndObject()) {
					return Err::kWriteFailed;
				}
				return 0;
			}

			virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
			{
				if (!req.stream.IsObject()) {
					resp.fStatus = FStatus::kParseFailed;
					resp.SetErrMsg("Value is not a Object");
					return Err::kDeserializeValueTypeError;
				}

				VarType *pFieldWrapper = static_cast<VarType *>(req.pFieldBuffer);
				assert(pFieldWrapper != IJST_NULL);
				RealVarType&field = IJST_CONT_VAL(*pFieldWrapper);
				field.clear();
				// pField->shrink_to_fit();
				SerializerInterface *serializerInterface = IJSTI_FSERIALIZER_INS(_T);

				for (rapidjson::Value::MemberIterator itMember = req.stream.MemberBegin();
					 itMember != req.stream.MemberEnd(); ++itMember)
				{
					// Get information
					const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
					// New a elem buffer in container first to avoid copy
					bool hasAlloc = false;
					if (field.find(fieldName) == field.end()) {
						hasAlloc = true;
					}

					ElemVarType &elemBuffer = field[fieldName];
					DeserializeReq elemReq(itMember->value, req.allocator,
										   req.unknownMode, req.canMoveSrc, &elemBuffer);

					// Deserialize
					DeserializeResp elemResp(resp.needErrMsg);
					int ret = serializerInterface->Deserialize(elemReq, elemResp);
					if (ret != 0)
					{
						if (hasAlloc)
						{
							field.erase(fieldName);
						}
						resp.needErrMsg &&
							resp.CombineErrMsg("Deserialize elem error. key: " + fieldName + ", err: ",
											   elemResp
						);
						resp.fStatus = FStatus::kParseFailed;
						return ret;
					}
					++resp.fieldCount;
				}
				resp.fStatus = FStatus::kValid;
				return 0;
			}

			virtual int SetAllocator(void* pField, JsonAllocator& allocator) IJSTI_OVERRIDE
			{
				VarType *pFieldWrapper = static_cast<VarType *>(pField);
				assert(pFieldWrapper != IJST_NULL);
				RealVarType&field = IJST_CONT_VAL(*pFieldWrapper);
				SerializerInterface *interface = IJSTI_FSERIALIZER_INS(_T);

				// Reset member
				for (typename RealVarType::iterator itera = field.begin(); itera != field.end(); ++itera)
				{
					int ret = interface->SetAllocator(&(itera->second), allocator);
					if (ret != 0) {
						return ret;
					}
				}

				return 0;
			}
		};

		struct MetaField { // NOLINT
			std::string name;
			std::size_t offset;
			unsigned int desc;
			SerializerInterface *serializerInterface;
		};

		class MetaClass {
		public:
			MetaClass() : accessorOffset(0), mapInited(false) { }

			void PushMetaField(const std::string &name, std::size_t offset,
							   unsigned int desc, SerializerInterface *serializerInterface)
			{
				MetaField metaField;
				metaField.name = name;
				metaField.offset = offset;
				metaField.desc = desc;
				metaField.serializerInterface = serializerInterface;

				metaFields.push_back(IJSTI_MOVE(metaField));
			}

			void InitMap()
			{
				// assert MetaClass's map has not inited before
				assert(!mapInited);

				for (size_t i = 0; i < metaFields.size(); ++i) {
					const MetaField *ptrMetaField = &(metaFields[i]);
					// Assert field offset not exist before
					assert(mapOffset.find(ptrMetaField->offset) == mapOffset.end());
					// Make sure field json name not exist before
					IJST_ASSERT(mapName.find(ptrMetaField->name) == mapName.end());

					mapName[ptrMetaField->name] = ptrMetaField;
					mapOffset[ptrMetaField->offset] = ptrMetaField;
				}
				mapInited = true;
			}

			std::vector<MetaField> metaFields;
			std::map<std::string, const MetaField *> mapName;
			std::map<std::size_t, const MetaField *> mapOffset;
			std::string tag;
			std::size_t accessorOffset;
		private:
			bool mapInited;

		};

		/**
		 * Reflection info.
		 * Push meta class info of _T in specialized constructor MetaInfo<_T>().
		 * @tparam _T: class. Concept require _T::InitMetaInfo(MetaInfo*)
		 */
		template<class _T>
		class MetaInfo {
		public:
			MetaClass metaClass;

		private:
			friend class Singleton<MetaInfo<_T> >;

			MetaInfo()
			{
				_T::InitMetaInfo(this);
			}
		};

	}	// namespace detail

	/**
	 * Struct Accessor.
	 * User can access and modify fields in struct, serialize and deserialize a struct.
	 */
	class Accessor {
	public:
		//region constructors
		explicit Accessor(const detail::MetaClass *pMetaClass, bool isValid) :
				m_pMetaClass(pMetaClass), m_isValid(isValid)
		{
			m_pFieldStatus = new FieldStatusType();
			m_pBuffer = new rapidjson::Value(rapidjson::kObjectType);
			m_pOwnDoc = new rapidjson::Document();
			m_pAllocator = &m_pOwnDoc->GetAllocator();
			InitOuterPtr();
		}

		Accessor(const Accessor &rhs)
		{
			assert(this != &rhs);

			m_isValid = rhs.m_isValid;

			m_pFieldStatus = new FieldStatusType(*rhs.m_pFieldStatus);
			m_pBuffer = new rapidjson::Value(rapidjson::kObjectType);
			m_pOwnDoc = new rapidjson::Document();
			m_pAllocator = &m_pOwnDoc->GetAllocator();

			m_pMetaClass = rhs.m_pMetaClass;
			InitOuterPtr();

			m_pBuffer->CopyFrom(*rhs.m_pBuffer, *m_pAllocator, true);
		}

		#if __cplusplus >= 201103L
		Accessor(Accessor &&rhs) IJSTI_NOEXCEPT
		{
			m_pBuffer = IJST_NULL;
			m_pOwnDoc = IJST_NULL;
			m_pFieldStatus = IJST_NULL;
			Steal(rhs);
		}
		#endif

		Accessor &operator=(Accessor rhs)
		{
			Steal(rhs);
			return *this;
		}

		~Accessor() IJSTI_NOEXCEPT
		{
			delete m_pBuffer;
			m_pBuffer = IJST_NULL;
			delete m_pOwnDoc;
			m_pOwnDoc = IJST_NULL;
			delete m_pFieldStatus;
			m_pFieldStatus = IJST_NULL;
		}
		//endregion

		//! Steal other Accessor object.
		void Steal(Accessor &rhs)
		{
			if (this == &rhs) {
				return;
			}

			// Handler resource
			delete m_pBuffer;
			m_pBuffer = rhs.m_pBuffer;
			rhs.m_pBuffer = IJST_NULL;

			delete m_pOwnDoc;
			m_pOwnDoc = rhs.m_pOwnDoc;
			rhs.m_pOwnDoc = IJST_NULL;

			delete m_pFieldStatus;
			m_pFieldStatus = rhs.m_pFieldStatus;
			rhs.m_pFieldStatus = IJST_NULL;

			// other simple field
			m_pMetaClass = rhs.m_pMetaClass;
			rhs.m_pMetaClass = IJST_NULL;
			m_pAllocator = rhs.m_pAllocator;
			rhs.m_pAllocator = IJST_NULL;

			m_isValid = rhs.m_isValid;

			InitOuterPtr();
		}

		//! Init members allocator to m_pAllocator.
		int InitMembersAllocator()
		{
			return SetMembersAllocator(*m_pAllocator);
		}

		inline bool IsValid() const { return m_isValid; }

		/*
		 * Field accessor.
		 */
		//! Check if pField is a filed in this object
		inline bool HasField(const void *pField) const
		{
			size_t offset = GetFieldOffset(pField);
			return (m_pMetaClass->mapOffset.find(offset) != m_pMetaClass->mapOffset.end());
		}

		//! Set field to val and mark it valid.
		template<typename _T1, typename _T2>
		inline void Set(_T1 &field, const _T2 &value)
		{
			MarkValid(&field);
			field = value;
		}

		//! Set field to val and mark it valid. The type of field and value must be same
		template<typename _T>
		inline void SetStrict(_T &field, const _T &value)
		{
			Set(field, value);
		}

		//! Mark status of field to FStatus::kValid
		inline void MarkValid(const void* pField) { MarkFieldStatus(pField, FStatus::kValid); }
		//! Mark status of field to FStatus::kNull
		inline void MarkNull(const void* pField) { MarkFieldStatus(pField, FStatus::kNull); }
		//! Mark status of field to FStatus::kMissing
		inline void MarkMissing(const void* pField) { MarkFieldStatus(pField, FStatus::kMissing); }

		//! Get status of field
		inline EFStatus GetStatus(const void *pField) const
		{
			const size_t offset = GetFieldOffset(pField);
			return GetStatusByOffset(offset);
		}

		//! Get inner buffer.
		inline JsonValue &GetBuffer() { return *m_pBuffer; }
		inline const JsonValue &GetBuffer() const { return *m_pBuffer; }

		//! Get allocator used in object.
		//! The inner allocator is own allocator when init, but may change to other allocator
		//! when calling SetMembersAllocator() or Deserialize().
		inline JsonAllocator &GetAllocator() { return *m_pAllocator; }
		inline const JsonAllocator &GetAllocator() const { return *m_pAllocator; }

		//! Get own allocator that used to manager resource.
		//! User could use the returned value to check if this object use outer allocator.
		inline JsonAllocator &GetOwnAllocator() { return m_pOwnDoc->GetAllocator(); }
		inline const JsonAllocator &GetOwnAllocator() const { return m_pOwnDoc->GetAllocator(); }

		//! Set Inner allocator.
		//! @note Will not clear pervious allocator.
		int SetMembersAllocator(JsonAllocator &allocator)
		{
			if (m_pAllocator != &allocator) {
				// copy buffer when need
				JsonValue temp;
				temp = *m_pBuffer;
				// The life cycle of const string in temp should be same as this object
				m_pBuffer->CopyFrom(temp, allocator, false);
			}

			m_pAllocator = &allocator;

			// Set allocator in members
			for (std::vector<detail::MetaField>::const_iterator itMetaField = m_pMetaClass->metaFields.begin();
				 itMetaField != m_pMetaClass->metaFields.end(); ++itMetaField)
			{
				void *pField = GetFieldByOffset(itMetaField->offset);
				int ret = itMetaField->serializerInterface->SetAllocator(pField, allocator);
				if (ret != 0) {
					return ret;
				}
			}
			return 0;
		}


		/**
		 * Serialize the structure.
		 * @param pushAllField 		True if push all field, false if push only valid or null field
		 * @param output 			The output of result
		 * @param allocator	 		Allocator when adding members to output.
		 * @return					Error code.
		 *
		 * @note If using inner allocator, user should carefully handler the structure's life cycle.
		 */
		inline int Serialize(bool pushAllField, IJST_OUT JsonValue& output, JsonAllocator& allocator) const
		{
			return Accessor::template DoSerialize<false, const Accessor>
					(*this, pushAllField, output, allocator);
		}

		/**
		 * Serialize the structure to string
		 * @param pushAllField 		True if push all field, false if push only valid or null field
		 * @param strOutput 		The output of result
		 * @return					Error code
		 */
		int SerializeToString(bool pushAllField, IJST_OUT std::string &strOutput) const
		{
			rapidjson::StringBuffer buffer;
			detail::JsonWriter writer(buffer);
			int iRet = DoWrite(writer, pushAllField);
			if (iRet != 0) {
				return iRet;
			}

			strOutput = std::string(buffer.GetString(), buffer.GetLength());
			return 0;
		}

		/**
		 * Serialize the structure.
		 * @note The object may be invalid after serialization
		 * @param pushAllField 		True if push all field, false if push only valid or null field
		 * @param output 			The output of result
		 * @param allocator	 		Allocator when adding members to output.
		 * @return					Error code
		 *
		 * @note If using inner allocator, user should carefully handler the structure's life cycle.
		 */
		inline int MoveSerialize(bool pushAllField, IJST_OUT JsonValue& output, JsonAllocator& allocator)
		{
			return Accessor::template DoSerialize<true, Accessor>
					(*this, pushAllField, output, allocator);
		}

		/**
		 * Serialize the structure with inner allocator.
		 * @param pushAllField 		True if push all field, false if push only valid or null field
		 * @param output 			The output of result
		 * @return
		 *
		 * @note The object may be invalid after serialization.
		 * @note User should make sure the structure's life cycle is longer than output.
		 */
		inline int MoveSerializeInInnerAlloc(bool pushAllField, IJST_OUT JsonValue &output)
		{
			return Accessor::template DoSerialize<true, Accessor>
					(*this, pushAllField, output, *m_pAllocator);
		}

		/**
		 * Deserialize from json object
		 * @param stream			Input json object
		 * @param unknownMode		Behaviour when meet unknown member in json
		 * @param pErrMsgOut		Error message output. Null if do not need error message
		 * @return					Error code
		 */
		inline int Deserialize(const JsonValue &stream, EUnknownMode unknownMode = UnknownMode::kKeep,
							   std::string *pErrMsgOut = IJST_NULL)
		{
			size_t fieldCount;
			return DoDeserialize(stream, unknownMode, pErrMsgOut, fieldCount);
		}

		/**
		 * Deserialize from std::string
		 * @param strInput			Input string
		 * @param unknownMode		Behaviour when meet unknown member in json
		 * @param pErrMsgOut		Error message output. Null if do not need error message
		 * @return					Error code
		 */
		inline int Deserialize(const std::string &strInput, EUnknownMode unknownMode = UnknownMode::kKeep,
							   std::string *pErrMsgOut = IJST_NULL)
		{
			return Deserialize(strInput.c_str(), strInput.length(), unknownMode, pErrMsgOut);
		}

		/**
		 * Deserialize from C-style string
		 * @param cstrInput			Input C string
		 * @param length			Length of string
		 * @param unknownMode		Behaviour when meet unknown member in json
		 * @param pErrMsgOut		Error message output. Null if do not need error message
		 * @return					Error code
		 *
		 * @note The input string can contain '\0'
		 */
		int Deserialize(const char *cstrInput, std::size_t length, EUnknownMode unknownMode = UnknownMode::kKeep,
						std::string *pErrMsgOut = IJST_NULL)
		{
			// The new object will call Deserialize() interfaces soon in most situation
			// So clear own allocator will not bring much benefice
			m_pAllocator = &m_pOwnDoc->GetAllocator();
			rapidjson::Document doc(m_pAllocator);
			doc.Parse(cstrInput, length);
			if (doc.HasParseError())
			{
				if (pErrMsgOut != IJST_NULL)
				{
					*pErrMsgOut = IJSTI_MOVE(std::string(
							rapidjson::GetParseError_En(doc.GetParseError())
					));
				}
				return Err::kDeserializeParseFaild;
			}
			size_t fieldCount;
			return DoMoveDeserialize(doc, unknownMode, pErrMsgOut, fieldCount);
		}

		/**
		 * Deserialize from json document. The source object may be stolen after deserialize.
		 * Because the accessor need manager the input allocator, but the Allocator class has no Swap() interface,
		 * so use document object instead.
		 * @param srcDocStolen		Input document object.
		 * @param unknownMode		Behaviour when meet unknown member in json
		 * @param pErrMsgOut		Error message output. Null if do not need error message
		 * @return					Error code
		 *
		 * @note The source document may be changed after deserialize.
		 * @note Make sure srcDocStolen use own allocator, or use allocator in this object.
		 */
		inline int MoveDeserialize(rapidjson::Document &srcDocStolen, EUnknownMode unknownMode = UnknownMode::kKeep,
								   std::string *pErrMsgOut = IJST_NULL)
		{
			// Store document to manager allocator
			m_pOwnDoc->Swap(srcDocStolen);
			m_pAllocator = &m_pOwnDoc->GetAllocator();
			size_t fieldCount;
			return DoMoveDeserialize(*m_pOwnDoc, unknownMode, pErrMsgOut, fieldCount);
		}

		/**
		 * Deserialize insitu from str
		 * @param cstrInput			Input C string
		 * @param unknownMode		Behaviour when meet unknown member in json
		 * @param pErrMsgOut		Error message output. Null if do not need error message
		 * @return					Error code
		 *
		 * @note The context in str may be changed after deserialize
		 * @note Make sure the lifecycle of str is longer than this object
		 */
		int DeserializeInsitu(char *str, EUnknownMode unknownMode = UnknownMode::kKeep,
							  std::string *pErrMsgOut = IJST_NULL)
		{
			// The new object will call Deserialize() interfaces in most situation
			// So clear own allocator will not bring much benefice
			m_pAllocator = &m_pOwnDoc->GetAllocator();
			rapidjson::Document doc(m_pAllocator);
			doc.ParseInsitu(str);
			if (doc.HasParseError())
			{
				if (pErrMsgOut != IJST_NULL)
				{
					*pErrMsgOut = IJSTI_MOVE(std::string(
							rapidjson::GetParseError_En(doc.GetParseError())
					));
				}
				return Err::kDeserializeParseFaild;
			}
			size_t fieldCount;
			return DoMoveDeserialize(doc, unknownMode, pErrMsgOut, fieldCount);
		}


	private:
		// #region Implement SerializeInterface
		template <class _T> friend class detail::FSerializer;
		typedef detail::SerializerInterface::SerializeReq SerializeReq;
		typedef detail::SerializerInterface::SerializeResp SerializeResp;
		typedef detail::SerializerInterface::WriteReq WriteReq;
		typedef detail::SerializerInterface::WriteResp WriteResp;
		typedef detail::SerializerInterface::DeserializeReq DeserializeReq;
		typedef detail::SerializerInterface::DeserializeResp DeserializeResp;

		inline int ISerialize(const SerializeReq &req, SerializeResp &resp)
		{
			assert(req.pField == this);
			if (req.canMoveSrc) {
				return Accessor::template DoSerialize<true, Accessor>
						(*this, req.pushAllField, req.buffer, req.allocator);
			}
			else {
				return Accessor::template DoSerialize<false, const Accessor>
						(*this, req.pushAllField, req.buffer, req.allocator);
			}
		}

		inline int IWrite(const WriteReq &req, WriteResp &resp) const
		{
			assert(req.pField == this);
			return DoWrite(req.writer, req.pushAllField);
		}

		inline int IDeserialize(const DeserializeReq &req, IJST_OUT DeserializeResp& resp)
		{
			assert(req.pFieldBuffer == this);

			std::string *pErrMsg = resp.needErrMsg ? &resp.errMsg : IJST_NULL;
			m_pAllocator = &req.allocator;

			if (req.canMoveSrc) {
				return DoMoveDeserialize(req.stream, req.unknownMode, pErrMsg, resp.fieldCount);
			}
			else {
				return DoDeserialize(req.stream, req.unknownMode, pErrMsg, resp.fieldCount);
			}
		}

		inline int ISetAllocator(void* pField, JsonAllocator& allocator)
		{
			assert(pField == this);
			return SetMembersAllocator(allocator);
		}
		// #endregion

		inline void InitOuterPtr()
		{
			m_pOuter = reinterpret_cast<const unsigned char *>(this - m_pMetaClass->accessorOffset);
		}

		void MarkFieldStatus(const void* field, EFStatus fStatus)
		{
			const std::size_t offset = GetFieldOffset(field);
			(IJST_ASSERT(m_pMetaClass->mapOffset.find(offset) != m_pMetaClass->mapOffset.end()));

			(*m_pFieldStatus)[offset] = fStatus;
		}

		int DoSerializeAllFields(bool pushAllField, bool canMoveSrc,
								 IJST_OUT JsonValue &buffer, JsonAllocator &allocator) const
		{
			// Serialize fields to buffer
			buffer.SetObject();

			// Reserve space
			do {
				const size_t maxSize = m_pMetaClass->metaFields.size() + m_pBuffer->MemberCount();
				if (buffer.MemberCapacity() >= maxSize) {
					break;
				}

				size_t fieldSize;
				if (pushAllField) {
					fieldSize = maxSize;
				}
				else {
					fieldSize = m_pBuffer->MemberCount();
					for (std::vector<detail::MetaField>::const_iterator itMetaField = m_pMetaClass->metaFields.begin();
						 itMetaField != m_pMetaClass->metaFields.end(); ++itMetaField)
					{
						EFStatus fstatus = GetStatusByOffset(itMetaField->offset);
						if (fstatus == FStatus::kValid || fstatus == FStatus::kNull) {
							++fieldSize;
						}
					}

				}
				buffer.MemberReserve(fieldSize, allocator);
			} while (false);

			#ifndef NDEBUG
			const rapidjson::SizeType oldCapcity = buffer.MemberCapacity();
			#endif

			for (std::vector<detail::MetaField>::const_iterator itMetaField = m_pMetaClass->metaFields.begin();
				 itMetaField != m_pMetaClass->metaFields.end(); ++itMetaField)
			{
				// Check field state
				EFStatus fstatus = GetStatusByOffset(itMetaField->offset);
				switch (fstatus) {
					case FStatus::kValid:
					case FStatus::kMissing:
					case FStatus::kParseFailed:
					{
						if (fstatus != FStatus::kValid && !pushAllField) {
							continue;
						}
						int ret = DoSerializeField(itMetaField, canMoveSrc, pushAllField, buffer, allocator);
						if (ret != 0) {
							return ret;
						}
					}
						break;

					case FStatus::kNull:
					{
						int ret = DoSerializeNullField(itMetaField, buffer, allocator);
						if (ret != 0) {
							return ret;
						}
					}
						break;


					case FStatus::kNotAField:
					default:
						// Error occurs
						assert(false);
						return Err::kInnerError;
				}

			}
			// assert that buffer will not reallocate memory during serialization
			assert(buffer.MemberCapacity() == oldCapcity);
			return 0;
		}

		template <bool kCanMoveSrc, class _TAccessor>
		static int DoSerialize(_TAccessor& accessor, bool pushAllField, IJST_OUT JsonValue& buffer, JsonAllocator& allocator)
		{
			const Accessor& rThis = accessor;
			int iRet = rThis.DoSerializeAllFields(pushAllField, kCanMoveSrc, buffer, allocator);
			if (iRet != 0) {
				return iRet;
			}
			Accessor::template AppendInnerToBuffer<kCanMoveSrc, _TAccessor>(accessor, buffer, allocator);
			return 0;
		}

		template<bool kCanMoveSrc, class _TAccessor>
		static inline void AppendInnerToBuffer(_TAccessor &accessor, JsonValue &buffer, JsonAllocator &allocator);


		int DoSerializeField(std::vector<detail::MetaField>::const_iterator itMetaField,
									bool canMoveSrc, bool pushAllField,
									JsonValue &buffer, JsonAllocator &allocator) const
		{
			// Init
			const void *pFieldValue = GetFieldByOffset(itMetaField->offset);

			// Serialize field
			JsonValue elemOutput;
			SerializeReq elemSerializeReq(elemOutput, allocator, pFieldValue, canMoveSrc, pushAllField);

			SerializeResp elemSerializeResp;
			int ret = itMetaField->serializerInterface->Serialize(elemSerializeReq, elemSerializeResp);
			if (ret != 0) {
				return ret;
			}

			// Add member, copy field name because the fieldName store in Meta info maybe release when dynamical
			// library unload, and the memory pool should be fast to copy field name
			// Do not check existing key, that's a feature
			rapidjson::GenericStringRef<char> fieldNameRef =
					rapidjson::StringRef(itMetaField->name.c_str(), itMetaField->name.length());
			buffer.AddMember(
					rapidjson::Value().SetString(fieldNameRef, allocator),
					elemOutput,
					allocator
			);
			return 0;
		}

		int DoSerializeNullField(std::vector<detail::MetaField>::const_iterator itMetaField,
								 JsonValue &buffer, JsonAllocator &allocator) const
		{
			// Init
			rapidjson::GenericStringRef<char> fieldNameRef =
					rapidjson::StringRef(itMetaField->name.c_str(), itMetaField->name.length());
			rapidjson::Value fieldNameVal;
			fieldNameVal.SetString(fieldNameRef);

			// Add member, field name is not need deep copy
			// Do not check if there is a existing key, that's a feature
			buffer.AddMember(
					rapidjson::Value().SetString(fieldNameRef),
					rapidjson::Value().SetNull(),
					allocator
			);
			return 0;
		}

		/**
		 * Write to string
		 */
		int DoWrite(detail::JsonWriter& writer, bool pushAllField) const
		{
			if (!writer.StartObject()) {
				return Err::kWriteFailed;
			}
			// Write fields
			for (std::vector<detail::MetaField>::const_iterator itMetaField = m_pMetaClass->metaFields.begin();
				 itMetaField != m_pMetaClass->metaFields.end(); ++itMetaField)
			{
				// Check field state
				EFStatus fstatus = GetStatusByOffset(itMetaField->offset);
				switch (fstatus) {
					case FStatus::kValid:
					case FStatus::kMissing:
					case FStatus::kParseFailed:
					{
						if (fstatus != FStatus::kValid && !pushAllField) {
							continue;
						}

						const void *pFieldValue = GetFieldByOffset(itMetaField->offset);
						if(!writer.Key(itMetaField->name.c_str(), itMetaField->name.length())) {
							return Err::kWriteFailed;
						}
						WriteReq req(writer, pFieldValue, pushAllField);
						WriteResp resp;
						int ret = itMetaField->serializerInterface->Write(req, resp);
						if (ret != 0) {
							return ret;
						}
					}
						break;

					case FStatus::kNull:
					{
						if(!writer.Key(itMetaField->name.c_str(), itMetaField->name.length())) {
							return Err::kWriteFailed;
						}
						if (!writer.Null()) {
							return Err::kWriteFailed;
						}
					}
						break;

					case FStatus::kNotAField:
					default:
						// Error occurs
						assert(false);
						return Err::kInnerError;
				}

			}

			// Write Buffer
			assert(m_pBuffer->IsObject());
			for (rapidjson::Value::ConstMemberIterator itMember = m_pBuffer->MemberBegin();
				 itMember != m_pBuffer->MemberEnd(); ++itMember)
			{
				// Write key
				const JsonValue& key = itMember->name;
				if (!writer.Key(key.GetString(), key.GetStringLength())) {
					return Err::kWriteFailed;
				}
				// Write value
				if (!itMember->value.Accept(writer)) {
					return Err::kWriteFailed;
				}
			}

			if (!writer.EndObject()) {
				return Err::kWriteFailed;
			}
			return 0;
		}

		/**
		 * Deserialize move from stream
		 * @note Make sure the lifecycle of allocator of the stream is longer than this object
		 */
		int DoMoveDeserialize(JsonValue& stream, EUnknownMode unknownMode,
							  std::string* pErrMsgOut, IJST_OUT size_t& fieldCountOut)
		{
			if (!stream.IsObject())
			{
				IJSTI_SET_ERRMSG(pErrMsgOut, "value is not object");
				return Err::kDeserializeValueTypeError;
			}

			fieldCountOut = 0;
			// For each member
			rapidjson::Value::MemberIterator itNextRemain = stream.MemberBegin();
			for (rapidjson::Value::MemberIterator itMember = stream.MemberBegin();
				 itMember != stream.MemberEnd(); ++itMember)
			{

				// Get related field info
				const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
				std::map<std::string, const detail::MetaField *>::const_iterator
						itMetaField = m_pMetaClass->mapName.find(fieldName);

				if (itMetaField == m_pMetaClass->mapName.end()) {
					// Not a field in struct
					switch (unknownMode) {
						case UnknownMode::kKeep:
							// Move unknown fields to the front of array first
							// TODO: This is relay on the implementation details of rapidjson's object storage (array), how to check?
							if (itNextRemain != itMember) {
								itNextRemain->name.SetNull().Swap(itMember->name);
								itNextRemain->value.SetNull().Swap(itMember->value);
							}
							++itNextRemain;
							break;
						case UnknownMode::kError:
							IJSTI_SET_ERRMSG(pErrMsgOut, "Member in object is unknown" + fieldName);
							return Err::kDeserializeSomeUnknownMember;
						case UnknownMode::kIgnore:
							break;
						default:
							assert(false);
					}
					continue;
				}

				// Move member out of object
				JsonValue memberStream(rapidjson::kNullType);
				memberStream.Swap(itMember->value);

				int ret = DoDeserializeField(itMetaField, memberStream, unknownMode, /*canMoveSrc=*/true,
											 pErrMsgOut);
				if (ret != 0) {
					return ret;
				}
				++fieldCountOut;
			}

			// Clean deserialized
			if (stream.MemberCount() != 0) {
				stream.EraseMember(itNextRemain, stream.MemberEnd());
			}
			if (unknownMode == UnknownMode::kKeep) {
				m_pBuffer->SetNull().Swap(stream);
			}
			else {
				m_pBuffer->SetObject();
			}

			int ret = CheckFieldState(pErrMsgOut);
			return ret;
		}

		//! Deserialize from stream
		int DoDeserialize(const JsonValue& stream, EUnknownMode unknownMode,
						  std::string* pErrMsgOut, IJST_OUT size_t& fieldCountOut)
		{
			if (!stream.IsObject()) {
				IJSTI_SET_ERRMSG(pErrMsgOut, "value is not object");
				return Err::kDeserializeValueTypeError;
			}

			m_pBuffer->SetObject();
			fieldCountOut = 0;
			// For each member
			for (rapidjson::Value::ConstMemberIterator itMember = stream.MemberBegin();
				 itMember != stream.MemberEnd(); ++itMember)
			{
				// Get related field info
				const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
				std::map<std::string, const detail::MetaField *>::const_iterator
						itMetaField = m_pMetaClass->mapName.find(fieldName);

				if (itMetaField == m_pMetaClass->mapName.end())
				{
					// Not a field in struct
					switch (unknownMode) {
						case UnknownMode::kKeep:
							m_pBuffer->AddMember(
									rapidjson::Value().SetString(fieldName.c_str(), fieldName.length(), *m_pAllocator),
									rapidjson::Value().CopyFrom(itMember->value, *m_pAllocator, true),
									*m_pAllocator
							);
							break;
						case UnknownMode::kIgnore:
							break;
						case UnknownMode::kError:
							IJSTI_SET_ERRMSG(pErrMsgOut, "Member in object is unknown" + fieldName);
							return Err::kDeserializeSomeUnknownMember;
						default:
							assert(false);
					}
					continue;
				}

				JsonValue& memberStream = const_cast<JsonValue&>(itMember->value);
				int ret = DoDeserializeField(itMetaField, memberStream,
											 unknownMode, /*canMoveSrc=*/false, pErrMsgOut);
				if (ret != 0) {
					return ret;
				}
				++fieldCountOut;
			}

			int ret = CheckFieldState(pErrMsgOut);
			return ret;
		}

		int DoDeserializeField(std::map<std::string, const detail::MetaField *>::const_iterator itMetaField,
							   JsonValue& stream, EUnknownMode unknownMode, bool canMoveSrc,
							   std::string *pErrMsgOut)
		{
			const detail::MetaField *metaField = itMetaField->second;
			// Check nullable
			if (isBitSet(metaField->desc, FDesc::Nullable)
				&& stream.IsNull())
			{
				(*m_pFieldStatus)[metaField->offset] = FStatus::kNull;
			}
			else
			{
				void *pField = GetFieldByOffset(metaField->offset);
				DeserializeReq elemReq(stream, *m_pAllocator, unknownMode, canMoveSrc, pField);
				const bool needErrMsg = pErrMsgOut != IJST_NULL;
				DeserializeResp elemResp(needErrMsg);
				int ret = metaField->serializerInterface->Deserialize(elemReq, elemResp);
				// Check return
				if (ret != 0) {
					(*m_pFieldStatus)[metaField->offset] = FStatus::kParseFailed;
					IJSTI_SET_ERRMSG(
							pErrMsgOut,
							("Deserialize field error. name: " + metaField->name + ", err: " + elemResp.errMsg)
					);
					return ret;
				}
				// Check elem size
				if (isBitSet(metaField->desc, FDesc::ElemNotEmpty)
					&& elemResp.fieldCount == 0)
				{
					IJSTI_SET_ERRMSG(
							pErrMsgOut,
							"Elem in field is empty. name: " + metaField->name
					);
					return Err::kDeserializeElemEmpty;
				}
				// succ
				(*m_pFieldStatus)[metaField->offset] = FStatus::kValid;
			}
			return 0;
		}

		int CheckFieldState(std::string *pErrMsgOut) const
		{
			// Check all required field status
			std::stringstream invalidNameOss;
			bool hasErr = false;
			for (std::vector<detail::MetaField>::const_iterator itField = m_pMetaClass->metaFields.begin();
				 itField != m_pMetaClass->metaFields.end(); ++itField)
			{
				if (isBitSet(itField->desc, FDesc::Optional))
				{
					// Optional
					continue;
				}

				EFStatus fStatus = GetStatusByOffset(itField->offset);
				if (fStatus == FStatus::kValid
					|| fStatus == FStatus::kNull)
				{
					// Correct
					continue;
				}

				// Has error
				hasErr = true;
				if (pErrMsgOut != IJST_NULL)
				{
					invalidNameOss << itField->name << ", ";
				}
			}
			if (hasErr)
			{
				IJSTI_SET_ERRMSG(
						pErrMsgOut,
						"Some fields are invalid: " + invalidNameOss.str()
				);
				return Err::kDeserializeSomeFiledsInvalid;
			}
			return 0;
		}

		inline std::size_t GetFieldOffset(const void *ptr) const
		{
			const unsigned char *filed_ptr = static_cast<const unsigned char *>(ptr);
			return filed_ptr - m_pOuter;
		}

		inline void *GetFieldByOffset(std::size_t offset) const
		{
			return (void *) (m_pOuter + offset);
		}

		EFStatus GetStatusByOffset(const size_t offset) const
		{
			FieldStatusType::const_iterator itera = m_pFieldStatus->find(offset);
			if (itera != m_pFieldStatus->end()) {
				return itera->second;
			}

			if (m_pMetaClass->mapOffset.find(offset) != m_pMetaClass->mapOffset.end()) {
				return FStatus::kMissing;
			}

			return FStatus::kNotAField;
		}

		static inline bool isBitSet(unsigned int val, unsigned int bit)
		{
			return (val & bit) != 0;
		}

		// Use pointers to make class Accessor be a standard-layout type struct
		typedef std::map<std::size_t, EFStatus> FieldStatusType;
		FieldStatusType* m_pFieldStatus;
		const detail::MetaClass* m_pMetaClass;

		rapidjson::Value* m_pBuffer;
		// Should use document instead of Allocator because document can swap allocator
		rapidjson::Document* m_pOwnDoc;

		JsonAllocator* m_pAllocator;
		const unsigned char *m_pOuter;

		bool m_isValid;
		//</editor-fold>
	};

	//! copy version of Accessor::AppendInnerToBuffer
	template <>
	inline void Accessor::template AppendInnerToBuffer<false, const Accessor>(
			const Accessor& accessor, JsonValue&buffer, JsonAllocator& allocator)
	{
		const Accessor& rThis = accessor;
		// Copy
		for (rapidjson::Value::ConstMemberIterator itMember = rThis.m_pBuffer->MemberBegin();
			 itMember != rThis.m_pBuffer->MemberEnd(); ++itMember)
		{
			rapidjson::Value name;
			rapidjson::Value val;
			name.CopyFrom(itMember->name, allocator, true);
			val.CopyFrom(itMember->value, allocator, true);
			buffer.AddMember(name, val, allocator);
		}
	}

	//! move version of Accessor::AppendInnerToBuffer
	template <>
	inline void Accessor::template AppendInnerToBuffer<true, Accessor>(
			Accessor& accessor, JsonValue&buffer, JsonAllocator& allocator)
	{
		Accessor& rThis = accessor;
		// append inner data to buffer
		if (&allocator == rThis.m_pAllocator) {
			// Move
			for (rapidjson::Value::MemberIterator itMember = rThis.m_pBuffer->MemberBegin();
				 itMember != rThis.m_pBuffer->MemberEnd(); ++itMember)
			{
				buffer.AddMember(itMember->name, itMember->value, allocator);
				// both itMember->name, itMember->value are null now
			}
		}
		else
		{
			// Copy
			Accessor::template AppendInnerToBuffer<false, const Accessor>(accessor, buffer, allocator);
			// The object will be release after serialize in most suitation, so do not need clear own allocator
		}

		rThis.m_pBuffer->SetObject();
	}

	//! Wrapper of IJST_DEFINE_STRUCT_IMPL_*
	//! @param needGetter: must be T or F
	#define IJSTI_DEFINE_STRUCT_IMPL(N, stName, needGetter, ...) \
		IJSTI_PP_CONCAT(IJSTI_DEFINE_STRUCT_IMPL_, N)(stName, needGetter, ##__VA_ARGS__)

	//! Define getter of fields
	#define IJSTI_DEFINE_GETTER_T(N, ...)	\
		IJSTI_PP_CONCAT(IJSTI_DEFINE_GETTER_IMPL_, N) (__VA_ARGS__)
	#define IJSTI_DEFINE_GETTER_F(N, ...)	// empty

	// Expands to the concatenation of its two arguments.
	#define IJSTI_PP_CONCAT(x, y) IJSTI_PP_CONCAT_PRIMITIVE(x, y)
	#define IJSTI_PP_CONCAT_PRIMITIVE(x, y) x ## y

	#define IJSTI_IDL_FTYPE(fType, fName, sName, desc)		fType
	#define IJSTI_IDL_FNAME(fType, fName, sName, desc)		fName
	#define IJSTI_IDL_SNAME(fType, fName, sName, desc)		sName
	#define IJSTI_IDL_DESC(fType, fName, sName, desc)		desc

	#define IJSTI_DEFINE_FIELD(fType, fName, ... )												\
			::ijst::detail::FSerializer< fType>::VarType fName;

	#define IJSTI_FIELD_TYPEDEF_START()															\
			struct _TypeDef {

	#define IJSTI_FIELD_TYPEDEF(fType, fName, ...) 												\
				typedef ::ijst::detail::FSerializer< fType>::VarType fName;

	#define IJSTI_FIELD_TYPEDEF_END()															\
			};

	#define IJSTI_FIELD_GETTER(fType, fName, ... )												\
			::ijst::Optional<const _TypeDef::fName> IJSTI_PP_CONCAT(Get, fName)() const 		\
			{																					\
				if (!this->_.IsValid() || this->_.GetStatus(&fName) != ijst::FStatus::kValid)	\
					{ return ::ijst::Optional<const _TypeDef::fName>(IJST_NULL); }				\
				return ::ijst::Optional<const _TypeDef::fName>(&fName);							\
			}																					\
			::ijst::Optional<_TypeDef::fName> IJSTI_PP_CONCAT(Get, fName)()						\
			{																					\
				if (!this->_.IsValid() || this->_.GetStatus(&fName) != ijst::FStatus::kValid)	\
					{ return ::ijst::Optional<_TypeDef::fName>(IJST_NULL); }					\
				return ::ijst::Optional<_TypeDef::fName>(&fName);								\
			}

	#define IJSTI_METAINFO_DEFINE_START(stName, N)												\
			typedef ::ijst::detail::MetaInfo<stName> MetaInfoT;									\
			typedef ::ijst::detail::Singleton<MetaInfoT> MetaInfoS;								\
			friend MetaInfoT;																	\
			static void InitMetaInfo(MetaInfoT* metaInfo)										\
			{																					\
				IJSTI_TRY_INIT_META_BEFORE_MAIN(MetaInfoT);										\
				/*Do not call MetaInfoS::GetInstance() int this function */			 			\
				metaInfo->metaClass.tag = #stName;												\
				metaInfo->metaClass.accessorOffset = IJST_OFFSETOF(stName, _);					\
				metaInfo->metaClass.metaFields.reserve(N);

	#define IJSTI_METAINFO_ADD(stName, fDef)  								\
			metaInfo->metaClass.PushMetaField(								\
				IJSTI_IDL_SNAME fDef, 										\
				IJST_OFFSETOF(stName, IJSTI_IDL_FNAME fDef),				\
				IJSTI_IDL_DESC fDef, 										\
				IJSTI_FSERIALIZER_INS(IJSTI_IDL_FTYPE fDef)					\
			);

	#define IJSTI_METAINFO_DEFINE_END()															\
				metaInfo->metaClass.InitMap();													\
			}

}	// namespace ijst

#include "ijst_repeat_def.inc"

#endif //_IJST_HPP_INCLUDE_