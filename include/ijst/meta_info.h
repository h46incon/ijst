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
	int index;
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

		// search in bucket
		assert(searchRet.index < m_hashedFieldIndexes.size());
		const std::vector<unsigned> &fieldIndexes = m_hashedFieldIndexes[searchRet.index];
		for (size_t i = 0, iSize = fieldIndexes.size(); i < iSize; ++i)
		{
			const MetaFieldInfo<Ch>& fieldInfo = m_fieldsInfo[fieldIndexes[i]];
			const std::basic_string<Ch>& fieldJsonName = fieldInfo.jsonName;
			if ( (fieldJsonName.length() == length) && (fieldJsonName.compare(0, fieldJsonName.length(), name, length) == 0) ) {
				return &fieldInfo;
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
	friend class detail::MetaClassInfoSetter<CharType>;		// use CharType instead of Ch to make IDE happy
	template<typename T> friend class detail::MetaClassInfoTyped;
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
	std::vector<std::vector<unsigned> > m_hashedFieldIndexes;
	std::vector<size_t> m_offsets;

	bool m_mapInited;
};

} // namespace ijst

#endif //IJST_META_INFO_H
