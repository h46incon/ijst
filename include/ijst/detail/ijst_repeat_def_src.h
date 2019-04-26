/********************************************************************************************
 * Using following command to generate .h file of thie meta macro file (require Boost):
 * g++ -I. -P -E ijst_repeat_def_src.h > ijst_repeat_def.inc
 */
// Define the max filed number supported here:
#define IJSTM_MAX_FIELD_NUM		64

#define IJSTM_HASH 		#
#define IJSTM_BSLASH	\\

#if !BOOST_PP_IS_ITERATING

	#ifndef ITEAR_HPP_INCLUDE
	#define ITEAR_HPP_INCLUDE

	#include <boost/preprocessor/iteration/iterate.hpp>
	#include <boost/preprocessor/iteration/local.hpp>
	#include <boost/preprocessor/repetition.hpp>
	#include <boost/preprocessor/arithmetic.hpp>
	#include <boost/preprocessor/comparison/equal.hpp>
	#include <boost/preprocessor/cat.hpp>

	// Generate IJSTI_PP_NFIELD
	// Params: encoding, stName, fileds. So fileds_count = count(__VA_ARGS__) - 2
	IJSTM_HASH ifdef _MSC_VER
		#define IJSTM_PP_COUNT_DESC(z, n, max)	,BOOST_PP_SUB(max, n)
		IJSTM_HASH define IJSTI_PP_NFIELD(...)													IJSTM_BSLASH
				IJSTI_EXPAND(IJSTI_PP_NFIELD_IMPL(__VA_ARGS__									IJSTM_BSLASH
					BOOST_PP_REPEAT(
							BOOST_PP_INC(IJSTM_MAX_FIELD_NUM),
							IJSTM_PP_COUNT_DESC,
							IJSTM_MAX_FIELD_NUM)												IJSTM_BSLASH
				))
	IJSTM_HASH else
		#define IJSTM_PP_COUNT_DESC(z, n, max)	,BOOST_PP_SUB(max, n)
		IJSTM_HASH define IJSTI_PP_NFIELD(...)													IJSTM_BSLASH
				IJSTI_PP_NFIELD_IMPL(__VA_ARGS__												IJSTM_BSLASH
					BOOST_PP_REPEAT(
						BOOST_PP_INC(IJSTM_MAX_FIELD_NUM),
						IJSTM_PP_COUNT_DESC,
						IJSTM_MAX_FIELD_NUM)													IJSTM_BSLASH
				)
	IJSTM_HASH endif

	IJSTM_HASH define IJSTI_PP_NFIELD_IMPL(														IJSTM_BSLASH
			en1, BOOST_PP_ENUM_PARAMS(BOOST_PP_INC(IJSTM_MAX_FIELD_NUM), e), 					IJSTM_BSLASH
			N, ...) N

	#define BOOST_PP_ITERATION_LIMITS (0, IJSTM_MAX_FIELD_NUM)
	#define BOOST_PP_FILENAME_1       "ijst_repeat_def_src.h"
	#include BOOST_PP_ITERATE()

	#endif	// #define ITEAR_HPP_INCLUDE

#else // #define BOOST_PP_IS_ITERATING

	#define IJSTM_DEFINE_STRUCT(i) 			BOOST_PP_CAT(IJSTI_DEFINE_STRUCT_IMPL_, i)
	#define IJSTM_METAINFO_ADD(z, i, _) 	IJSTI_METAINFO_ADD(stName, f##i)
	#define IJSTM_FIELD_INIT(z, i, _)		,IJSTI_IDL_FNAME f##i ()
	#define IJSTM_DEFINE_FIELD(z, i, _) 	IJSTI_DEFINE_FIELD f##i

	#define IJSTM_DEFINE_GETTER(i) 			BOOST_PP_CAT(IJSTI_DEFINE_GETTER_IMPL_, i)
	#define IJSTM_FIELD_GETTER(z, i, _) 	IJSTI_FIELD_GETTER f##i

	// Add splitter
	IJSTM_HASH undef IJSTM_SPLITTER_________________________________________________________________________________________________

	#define n BOOST_PP_ITERATION()

	//* Define Getter
	IJSTM_HASH define IJSTM_DEFINE_GETTER(n)(BOOST_PP_ENUM_PARAMS(n, f))									IJSTM_BSLASH
		BOOST_PP_REPEAT(n, IJSTM_FIELD_GETTER, _)

	//* Define Struct
	IJSTM_HASH define IJSTM_DEFINE_STRUCT(n)( 																IJSTM_BSLASH
						isRawVal, needGetter, encoding, stName BOOST_PP_ENUM_TRAILING_PARAMS(n, f)) 		IJSTM_BSLASH
	class stName{	 																						IJSTM_BSLASH
	public:																									IJSTM_BSLASH
		IJSTI_STRUCT_PUBLIC_DEFINE(encoding)																IJSTM_BSLASH
		BOOST_PP_REPEAT(n, IJSTM_DEFINE_FIELD, ~)															IJSTM_BSLASH
		IJSTI_PP_CONCAT(IJSTI_DEFINE_GETTER_, needGetter) (n BOOST_PP_ENUM_TRAILING_PARAMS(n,f))			IJSTM_BSLASH
		explicit stName(bool isValid = true): 	 															IJSTM_BSLASH
			_(&(::ijst::detail::Singleton<_ijst_MetaInfoT>().metaClass), isRawVal, isValid)	 				IJSTM_BSLASH
			BOOST_PP_REPEAT(n, IJSTM_FIELD_INIT, ~)															IJSTM_BSLASH
			{}	 																							IJSTM_BSLASH
	private:	 																							IJSTM_BSLASH
		IJSTI_METAINFO_DEFINE_START(stName, n)	 															IJSTM_BSLASH
			BOOST_PP_REPEAT(n, IJSTM_METAINFO_ADD, ~)														IJSTM_BSLASH
		IJSTI_METAINFO_DEFINE_END()	 																		IJSTM_BSLASH
	IJSTI_DEFINE_CLASS_END(stName)

	#undef n

#endif // BOOST_PP_IS_ITERATING
