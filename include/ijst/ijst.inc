#define IJSTI_DEFINE_STRUCT_IMPL(N, ...) \
    IJSTI_PP_CONCAT(IJSTI_DEFINE_STRUCT_IMPL_, N)(__VA_ARGS__)

// Expands to the concatenation of its two arguments.
#define IJSTI_PP_CONCAT(x, y) IJSTI_PP_CONCAT_PRIMITIVE(x, y)
#define IJSTI_PP_CONCAT_PRIMITIVE(x, y) x ## y


#define IJSTI_IDL_FTYPE(fType, fName, sName, desc)		fType
#define IJSTI_IDL_FNAME(fType, fName, sName, desc)		fName
#define IJSTI_IDL_SNAME(fType, fName, sName, desc)		sName
#define IJSTI_IDL_DESC(fType, fName, sName, desc)		desc

#define IJSTI_DEFINE_FIELD(fDef) 							\
        ::ijst::detail::FSerializer<IJSTI_IDL_FTYPE fDef>::VarType IJSTI_IDL_FNAME fDef

#define IJSTI_METAINFO_ADD(stName, fDef)  				\
		metaInfo->metaClass.PushMetaField(			\
			IJSTI_IDL_SNAME fDef, 						\
			offsetof(stName, IJSTI_IDL_FNAME fDef),	\
			IJSTI_IDL_DESC fDef, 						\
			IJSTI_FSERIALIZER_INS(IJSTI_IDL_FTYPE fDef)		\
		)

#define IJSTI_METAINFO_DEFINE_START(stName, N)												\
	private:																				\
		typedef ::ijst::detail::MetaInfo<stName> MetaInfoT;									\
		typedef ::ijst::detail::Singleton<MetaInfoT> MetaInfoS;								\
		friend MetaInfoT;																	\
		static void InitMetaInfo(MetaInfoT* metaInfo)										\
		{																					\
			MetaInfoS::InitInstanceBeforeMain();											\
			std::cout << "InitMetaInfo: "  << #stName << std::endl;							\
			metaInfo->metaClass.tag = #stName;												\
			metaInfo->metaClass.accessorOffset = offsetof(stName, _);						\
			metaInfo->metaClass.metaFields.reserve(N);

#define IJSTI_METAINFO_DEFINE_END()															\
			metaInfo->metaClass.InitMap();													\
		}

// Macro expanding to the number of field it is passed.	(Thus sizeof __VA_ARGS__ - 1 )
// It is undefined behavior if `n > 64` or if `n == 0`.
#define IJSTI_PP_NFIELD(...) 															\
    IJSTI_PP_NFIELD_IMPL(__VA_ARGS__, 64, 63, 62, 61, 60, 								\
		59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
		39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
		19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define IJSTI_PP_NFIELD_IMPL(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, 								\
	e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, \
	e31, e32, e33, e34, e35, e36, e37, e38, e39, e40, e41, e42, e43, e44, e45, e46, e47, e48, e49, e50, \
	e51, e52, e53, e54, e55, e56, e57, e58, e59, e60, e61, e62, e63, e64, N, ...) N

#define IJSTI_DEFINE_STRUCT_IMPL_4(stName, f1, f2, f3, f4) 									\
class stName{																				\
	IJSTI_METAINFO_DEFINE_START(stName, 4)													\
		IJSTI_METAINFO_ADD(stName, f1);														\
		IJSTI_METAINFO_ADD(stName, f2);														\
		IJSTI_METAINFO_ADD(stName, f3);														\
		IJSTI_METAINFO_ADD(stName, f4);														\
	IJSTI_METAINFO_DEFINE_END()																\
public:																						\
	stName(): \
		_(&(MetaInfoS::GetInstance()->metaClass))		\
		,IJSTI_IDL_FNAME f1 ()		\
		,IJSTI_IDL_FNAME f2 ()		\
		,IJSTI_IDL_FNAME f3 ()		\
		,IJSTI_IDL_FNAME f4 ()		\
		{}	\
	::ijst::detail::Accessor _;		\
	IJSTI_DEFINE_FIELD(f1);				\
	IJSTI_DEFINE_FIELD(f2);				\
	IJSTI_DEFINE_FIELD(f3);				\
	IJSTI_DEFINE_FIELD(f4);				\
};


