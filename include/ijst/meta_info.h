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
	template<typename T> class MetaClassInfoTyped;
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
				detail::Util::VectorBinarySearch(m_offsets, offset);
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
				detail::Util::VectorBinarySearch(m_nameHashVal, hash);

		if (!searchRet.isFind) {
			// not find
			return NULL;
		}

		// step back to find the first value with hash
		size_t i = searchRet.index;
		for (; i > 0 && m_nameHashVal[i -1] == hash; --i) {
		}

		// compare each json name with target hash
		for (const size_t iSize = m_hashedFieldPtr.size(); i < iSize && m_nameHashVal[i] == hash; ++i) {
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

	//! Get meta information of all fields in class. The returned vector is sorted by offset.
	const std::vector<MetaFieldInfo<Ch> >& GetFieldsInfo() const { return m_fieldsInfo; }
	//! Get name of class.
	const std::string& GetClassName() const { return structName; }
	//! Get the offset of Accessor object.
	std::size_t GetAccessorOffset() const { return accessorOffset; }

private:
	template<typename> friend class detail::MetaClassInfoSetter;
	template<typename> friend class detail::MetaClassInfoTyped;
	MetaClassInfo() : accessorOffset(0), m_mapInited(false) { }

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

	std::vector<MetaFieldInfo<Ch> > m_fieldsInfo;
	std::string structName;
	std::size_t accessorOffset;

	std::vector<uint32_t> m_nameHashVal;
	std::vector<const MetaFieldInfo<Ch>*> m_hashedFieldPtr;
	std::vector<size_t> m_offsets;

	bool m_mapInited;
};

class OverrideMetaInfos {
public:
	struct MetaInfo {
		bool isFieldDescSet;
		FDesc::Mode fieldDesc;					//! override field desc, available if isFieldDescSet is true
		OverrideMetaInfos* ijstFieldMetaInfo;	//! override meta info of ijst field, available if not nullptr

		MetaInfo(): isFieldDescSet(false), fieldDesc(FDesc::NoneFlag), ijstFieldMetaInfo(NULL) {}
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

	const std::size_t filedSize;		//! size of metaInfos
	MetaInfo* const metaInfos;			//! array of MetaInfo

private:
	OverrideMetaInfos(const OverrideMetaInfos&) IJSTI_DELETED;
	OverrideMetaInfos& operator=(OverrideMetaInfos) IJSTI_DELETED;
};

} // namespace ijst

#endif //IJST_META_INFO_H
