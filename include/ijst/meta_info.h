#ifndef IJST_META_INFO_HPP_INCLUDE_
#define	IJST_META_INFO_HPP_INCLUDE_

#include "ijst.h"
#include "detail/utils.h"
//NOTE: do not include detail/detail.h

#include <string>

namespace ijst {

namespace detail {
	// forward declaration
	// these declaration is only use for friend class declaration
	template<typename T> class IjstStructMeta;
	template<typename CharType> class MetaClassInfoSetter;
}
/**
 * @brief Meta information of field.
 *
 * @tparam CharType		character type of string
 *
 * @see MetaClassInfo
 */
template<typename CharType = char>
struct MetaFieldInfo { // NOLINT
	typedef CharType Ch;
	//! The index of this fields in the meta information in the class. (Fields are sorted by offset inside class)
	std::size_t index;
	//! Field description.
	FDesc::Mode desc;
	//! Field's offset inside class.
	std::size_t offset;
	//! Json name when (de)serialization.
	std::basic_string<Ch> jsonName;
	//! field name.
	std::string fieldName;
	//! @private private serializer interface.
	void* serializerInterface;		// type: detail::SerializerInterface<Encoding>*
};

/**
 * @brief Meta information of class.
 *
 * @tparam CharType		character type of string
 *
 * @see MetaFieldInfo
 */
template <typename CharType = char>
class MetaClassInfo {
public:
	typedef CharType Ch;

	/**
	 * @brief Find index of field by offset.
	 *
	 * @param offset 	field's offset
	 * @return 			index if offset found, -1 else
	 *
	 * @note log(FieldSize) complexity.
	 */
	int FindIndex(size_t offset) const
	{
		const detail::Util::VectorBinarySearchResult searchRet =
				detail::Util::VectorBinarySearch(m_offsets, m_fieldSize, offset);
		if (searchRet.isFind) {
			return static_cast<int>(searchRet.index);
		}
		else {
			return -1;
		}
	}

	/**
	 * @brief Find meta information of filed by json name.
	 *
	 * @param name		field's json name
	 * @param length	field's json name length
	 * @return			pointer of info if found, null else
	 *
	 * @note log(FieldSize) complexity.
	 */
	const MetaFieldInfo<Ch>* FindFieldByJsonName(const Ch* name, size_t length) const
	{
		const uint32_t hash = StringHash(name, length);
		const detail::Util::VectorBinarySearchResult searchRet =
				detail::Util::VectorBinarySearch(m_nameHashVal, m_fieldSize, hash);

		if (!searchRet.isFind) {
			// not find
			return NULL;
		}

		// step back to find the first value with hash
		size_t i = searchRet.index;
		for (; i > 0 && m_nameHashVal[i -1] == hash; --i) {
		}

		// compare each json name with target hash
		for (; i < m_fieldSize && m_nameHashVal[i] == hash; ++i) {
			const MetaFieldInfo<Ch>* pField = m_hashedFieldPtr[i];
			const std::basic_string<Ch>& fieldJsonName = pField->jsonName;
			if (fieldJsonName.compare(0, fieldJsonName.length(), name, length) == 0) {
				return pField;
			}
		}

		return NULL;
	}

	/**
	 * @brief Find meta information of filed by json name.
	 *
	 * @param name		field's json name
	 * @return			pointer of info if found, null else
	 *
	 * @note log(FieldSize) complexity.
	 */
	const MetaFieldInfo<Ch>* FindFieldByJsonName(const std::basic_string<Ch>& name) const
	{ return FindFieldByJsonName(name.data(), name.length()); }

	//! Get array of meta information of all fields in class.
	//! The returned vector is sorted by offset size is return from @ref GetFieldSize()
	const MetaFieldInfo<Ch>* GetFieldInfos() const { assert(m_fieldsInfo); return m_fieldsInfo; }
	//! Get field size.
	size_t GetFieldSize() const { return m_fieldSize; }

	//! Get name of class.
	const std::string& GetClassName() const { return m_structName; }
	//! Get the offset of Accessor object.
	std::size_t GetAccessorOffset() const { return m_accessorOffset; }

private:
	template<typename> friend class detail::MetaClassInfoSetter;
	template<typename> friend class detail::IjstStructMeta;
	MetaClassInfo() :
			m_accessorOffset(0),
			m_fieldSize(0),
			m_fieldsInfo(NULL),
			m_nameHashVal(NULL),
			m_hashedFieldPtr(NULL),
			m_offsets(NULL),
			m_mapInited(false),
			m_isResourceOwner(false)
	{ }

	~MetaClassInfo()
	{
		m_fieldSize = 0;
#define IJSTIM_DELETE_ARRAY(field)	if (m_isResourceOwner) { delete [] field; } field = NULL;
		IJSTIM_DELETE_ARRAY(m_fieldsInfo)
		IJSTIM_DELETE_ARRAY(m_nameHashVal)
		IJSTIM_DELETE_ARRAY(m_hashedFieldPtr)
		IJSTIM_DELETE_ARRAY(m_offsets)
#undef IJSTIM_DELETE_ARRAY
	}

	void ShadowFrom(const MetaClassInfo& src, const std::string& structName)
	{
		this->~MetaClassInfo();

#define IJSTIM_COPY_FIELD(field)	field = src.field;

		IJSTIM_COPY_FIELD(m_mapInited)
		IJSTIM_COPY_FIELD(m_accessorOffset)
		IJSTIM_COPY_FIELD(m_fieldSize)

		IJSTIM_COPY_FIELD(m_fieldsInfo)
		IJSTIM_COPY_FIELD(m_nameHashVal)
		IJSTIM_COPY_FIELD(m_hashedFieldPtr)
		IJSTIM_COPY_FIELD(m_offsets)

#undef IJSTIM_COPY_FIELD

		m_structName = structName;
		m_isResourceOwner = false;
	}

	MetaClassInfo(const MetaClassInfo&) IJSTI_DELETED;
	MetaClassInfo& operator=(MetaClassInfo) IJSTI_DELETED;

	static uint32_t StringHash(const Ch* str, size_t length)
	{
		// Use 32-bit FNV-1a hash
		const uint32_t kPrime = (1 << 24) + (1 << 8) + 0x93;
		const uint32_t kBasis = 0x811c9dc5;
		uint32_t hash = kBasis;
		for (size_t i = 0; i < length;
			 ++i, ++str)
		{
			hash ^= *str;
			hash *= kPrime;
		}
		return hash;
	}

	std::string m_structName;
	std::size_t m_accessorOffset;
	std::size_t m_fieldSize;

	typedef const MetaFieldInfo<Ch> CMetaFieldInfo;
	MetaFieldInfo<Ch>* m_fieldsInfo;
	uint32_t* m_nameHashVal;
	CMetaFieldInfo** m_hashedFieldPtr;
	size_t* m_offsets;

	bool m_mapInited;
	bool m_isResourceOwner;
};

class OverrideMetaInfos {
public:
	struct MetaInfo {
		bool isFieldDescSet;
		FDesc::Mode fieldDesc;						//! override field desc, available if isFieldDescSet is true
		const OverrideMetaInfos* ijstFieldMetaInfo;	//! override meta info of ijst field, available if not nullptr

		MetaInfo(): isFieldDescSet(false), fieldDesc(FDesc::NoneFlag), ijstFieldMetaInfo(NULL) {}

		void SetFieldDesc(FDesc::Mode desc)
		{isFieldDescSet = true; fieldDesc = desc;}
	};

	explicit OverrideMetaInfos(size_t _fieldSize)
	: filedSize(_fieldSize), metaInfos(new MetaInfo[_fieldSize])
	{ }

	~OverrideMetaInfos()
	{
		delete [] metaInfos;
		const_cast<MetaInfo*&>(metaInfos) = NULL;
		const_cast<std::size_t&>(filedSize) = 0;
	}

	//! Copy from _src if not empty, else new a empty one
	//! The caller must release the pointer returned
	static OverrideMetaInfos* NewFromSrcOrEmpty(const OverrideMetaInfos* _src, size_t _fieldSize)
	{
		if (_src == NULL) {
			return new OverrideMetaInfos(_fieldSize);
		}
		else {
			assert(_src->filedSize == _fieldSize);
			OverrideMetaInfos *ret = new OverrideMetaInfos(_fieldSize);
			for (size_t i = 0; i < _fieldSize; ++i) {
				ret->metaInfos[i] = _src->metaInfos[i];
			}
			return ret;
		}

	}

	const std::size_t filedSize;		//! size of metaInfos
	MetaInfo* const metaInfos;			//! array of MetaInfo

private:
	OverrideMetaInfos(const OverrideMetaInfos&) IJSTI_DELETED;
	OverrideMetaInfos& operator=(OverrideMetaInfos) IJSTI_DELETED;
};

} // namespace ijst

#endif //IJST_META_INFO_H
