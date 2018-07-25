//
// Created by h46incon on 2017/9/19.
//

#ifndef IJST_IJST_HPP_INCLUDE_
#define IJST_IJST_HPP_INCLUDE_

#include <cassert>		// assert

/**	========================================================================================
 *				Public Interface
 */

/** @file */

/** @defgroup IJST_CONFIG ijst configuration
 *  @brief Configuration macros for library features
 *
 *  Some features are configurable to adapt the library to a wide
 *  variety of platforms, environments and usage scenarios.  Most of the
 *  features can be configured in terms of overriden or predefined
 *  preprocessor macros at compile-time.
 *
 *  @note These macros should be given on the compiler command-line
 *        (where applicable)  to avoid inconsistent values when compiling
 *        different translation units of a single application.
 */

/**
 * @ingroup IJST_CONFIG
 *
 * ijst generator getter methods when using IJST_DEFINE_STRUCT_WITH_GETTER or IJST_DEFINE_VALUE_WITH_GETTER that names
 * are IJST_GETTER_PREFIX + FIELD_NAME. By default, the prefix is "get", e.g., a getter method could name get_int1().
 *
 * @see IJST_DEFINE_STRUCT_WITH_GETTER, IJST_DEFINE_VALUE_WITH_GETTER
 */
#ifndef IJST_GETTER_PREFIX
	#define IJST_GETTER_PREFIX get_
#endif

/**
 * @ingroup IJST_CONFIG
 *
 *  ijst use static object in function to implement singleton, but it's not thread-safe before C++11.
 *  If declare IJST_TRY_INIT_META_BEFORE_MAIN to 1, ijst will init the singleton's instance when
 *  a static member of template class initialization, that init before main() in many compilers to avoid thread-safety problem.
 *	The feature is enable default before C++11.
 */
#ifndef IJST_TRY_INIT_META_BEFORE_MAIN
	#if __cplusplus < 201103L
		#define IJST_TRY_INIT_META_BEFORE_MAIN		1
	#else
		#define IJST_TRY_INIT_META_BEFORE_MAIN		0
	#endif
#endif

/**
 * @ingroup IJST_CONFIG
 *
 *	By default, ijst uses assert() for errors that indicate a bug.
 *	User can override it by defining IJST_ASSERT(x) macro.
 */
#ifndef IJST_ASSERT
	#define IJST_ASSERT(x) assert(x)
#endif

/**
 * @ingroup IJST_CONFIG
 *
 *	By default, ijst allocator a buffer in heap to calculate fields' offset of ijst struct.
 *	User can override it by defining IJST_OFFSET_BUFFER_NEW, IJST_OFFSET_BUFFER_DELETE macro.
 *	E.g, allocator in stack, or use nullptr.
 */
// alter 1: use nullptr
// #define IJST_OFFSET_BUFFER_NEW(ptrId, size)	char* ptrId = NULL
// #define IJST_OFFSET_BUFFER_DELETE(ptrId)		// empty

// alter 2: use buffer in stack
// #define IJST_OFFSET_BUFFER_NEW(ptrId, size)	char ptrId[size]
// #define IJST_OFFSET_BUFFER_DELETE(ptrId)		// empty

#ifndef IJST_OFFSET_BUFFER_NEW
	//! customization point for new buffer to calculate ijst struct offset
	#define IJST_OFFSET_BUFFER_NEW(ptrId, size)		char* ptrId = new char[size]
#endif
#ifndef IJST_OFFSET_BUFFER_DELETE
	//! customization point for delete buffer allocated by IJST_OFFSET_BUFFER_NEW
	#define IJST_OFFSET_BUFFER_DELETE(ptrId)		delete[] ptrId
#endif

/** @defgroup IJST_MACRO_API ijst macro API
 *  @brief macro API
 *
 */

/**
 * @ingroup IJST_MACRO_API
 *
 * ijst support extern template to speed up compilation. To enable this option,
 * set the macro to 1 before including ijst headers. User should declare
 * IJST_EXPLICIT_TEMPLATE in one of cpp files to instantiation the templates.
 *
 * @note require extern template support (since C++11) to enable this option
 *
 * @see IJST_EXPLICIT_TEMPLATE
 */
#ifndef IJST_EXTERN_TEMPLATE
	#define IJST_EXTERN_TEMPLATE	0
#endif

//! @brief Empty macro to mark a param is a output.
//! @ingroup IJST_MACRO_API
#define IJST_OUT

// Declare | and |= operator of enum
#define IJSTI_DECLARE_ENUM_OPERATOR_OR(Type) \
inline Type operator | (const Type& v1, const Type& v2) \
{ return static_cast<Type>(static_cast<unsigned>(v1) | static_cast<unsigned>(v2)); } \
inline Type& operator |= (Type& src, const Type& v) \
{ src = static_cast<Type>(static_cast<unsigned>(src) | static_cast<unsigned>(v)); return src;}

namespace ijst {

//! Field description.
struct FDesc {
	typedef unsigned int Mode;
	//! Field is optional.
	static const Mode Optional 			= 0x00000001;
	//! Field can be null.
	static const Mode Nullable 			= 0x00000002;
	//! Field's value is not default, e.g, int is not 0, string, vector, map is not empty
	static const Mode NotDefault 		= 0x00000004;
	// FDesc of Element (i.e. Nullable, NotDefault) inside container is hard to represent, has not plan to implement it
};

//! Field status.
struct FStatus {
	//! @private use EFStatus instead
	typedef unsigned char Enum;

	static const Enum kNotAField 		= (Enum)0;
	static const Enum kMissing 			= (Enum)1;
	static const Enum kNull 			= (Enum)2;
	static const Enum kValid 			= (Enum)3;
};
typedef FStatus::Enum EFStatus;

/**
 * @brief Serialization options about fields.
 *
 * Options can be combined by bitwise OR operator (|).
 */
struct SerFlag {
	enum Flag {
		//! does not set any option.
		kNoneFlag					= 0x0000
		//! set if ignore fields with kMissing status.
		, kIgnoreMissing			= 0x0001
		//! set if ignore unknown fields, otherwise will serialize all unknown fields.
		, kIgnoreUnknown			= 0x0002
		//! set if ignore fields with kNull status.
		, kIgnoreNull				= 0x0004
	};
};
IJSTI_DECLARE_ENUM_OPERATOR_OR(SerFlag::Flag)

/**
 * @brief Deserialization options about fields.
 *
 * Options can be combined by bitwise OR operator (|).
 */
struct DeserFlag {
	enum Flag{
		//! Does not set any option.
		kNoneFlag					= 0x0000
		//! Set if return error when meet unknown fields, otherwise will keep all unknown fields.
		, kErrorWhenUnknown			= 0x0001
		//! Set if ignore unknown fields, otherwise will keep all unknown fields.
		, kIgnoreUnknown			= 0x0002
		//! Set if ignore field status, otherwise will check if field status is matched requirement
		, kNotCheckFieldStatus		= 0x0004
		/**
		 * @brief  Set if move resource (to unknown or T_raw fields) from intermediate document when deserialize.
		 *
		 * This option will speed up deserialization. But the nested object will use parent's allocator.
		 * And the the parent will free allocator only when:
		 * 1. Destroy
		 * 2. Assigned from other object (operator =)
		 * 3. Call Deserialize() or FromJson() methods
		 *
		 * otherwise the parent will not free allocator, this may waste memory in some case.
		 * Be careful when moving the nested object to another object (e.g, calling RValue copy constructor or assignment),
		 *
		 * User could call Accessor::ShrinkAllocator() to recopy unknown and T_raw fields with own allocator in nested object.
		 *
		 * @note	Using this option VERY CAREFULLY.
		 * @see		Accessor::ShrinkAllocator()
		 */
		, kMoveFromIntermediateDoc	= 0x0008
	};
};
IJSTI_DECLARE_ENUM_OPERATOR_OR(DeserFlag::Flag)

//! Error codes.
struct ErrorCode {
	static const int kSucc 							= 0x00000000;
	static const int kDeserializeValueTypeError 	= 0x00001001;
	static const int kDeserializeSomeFieldsInvalid 	= 0x00001002;
	static const int kDeserializeParseFailed 		= 0x00001003;
	static const int kDeserializeValueIsDefault		= 0x00001004;
	static const int kDeserializeSomeUnknownMember	= 0x00001005;
	static const int kDeserializeMapKeyDuplicated	= 0x00001006;
	static const int kInnerError 					= 0x00002001;
	static const int kWriteFailed					= 0x00003001;
};

} // namespace ijst
#endif //IJST_IJST_HPP_INCLUDE_
