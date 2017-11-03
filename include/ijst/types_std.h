/**************************************************************************************************
 *		Serialization implementation of Primitive types
 *		Created by h46incon on 2017/11/2.
 **************************************************************************************************/

#ifndef _IJST_TYPES_STD_HPP_INCLUDE_
#define	_IJST_TYPES_STD_HPP_INCLUDE_

#include "ijst.h"
#include <cstdint>
#include <ctime>

#define IJST_TPRI(_T)	::ijst::detail::TypeClassPrim< ::ijst::FType::_T>
#define IJST_TFTIME(_TimeZone)	::ijst::detail::TypeClassFTime< _TimeZone>

namespace ijst{

struct FType {
public:
	enum _E {
		Bool,
		Int,
		UInt32,
		UInt64,
		Int32,
		Int64,
		String,
		Raw,
		Time,
	};
};

namespace detail{

template<FType::_E _T>
struct TypeClassPrim {
	// nothing
};

template<int kTimeZone>
struct TypeClassFTime{
	// nothing
};
}


typedef unsigned char FStoreBool; 		// Could not use bool type because std::vector<bool> is not a container!
typedef int FStoreInt;
typedef std::uint32_t FStoreUInt32;
typedef std::uint64_t FStoreUInt64;
typedef std::int32_t FStoreInt32;
typedef std::int64_t FStoreInt64;
typedef std::string FStoreString;
typedef std::time_t FStoreTime;
typedef std::int64_t FStoreFastTime;

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
		v.CopyFrom(rhs.v, *m_pAllocator);
	}

#if __cplusplus >= 201103L
	FStoreRaw(FStoreRaw &&rhs)
	{
		m_pOwnDoc = IJSTI_NULL;
		m_pAllocator = IJSTI_NULL;
		Steal(rhs);
	}
#endif

	FStoreRaw &operator=(FStoreRaw rhs)
	{
		Steal(rhs);
		return *this;
	}

	void Steal(FStoreRaw& raw)
	{
		delete m_pOwnDoc;
		m_pOwnDoc = raw.m_pOwnDoc;
		raw.m_pOwnDoc = IJSTI_NULL;

		m_pAllocator = raw.m_pAllocator;
		raw.m_pAllocator = IJSTI_NULL;
		v = raw.v;
	}

	~FStoreRaw()
	{
		delete m_pOwnDoc;
		m_pOwnDoc = IJSTI_NULL;
	}

	StoreType& V() {return v;}
	const StoreType& V() const {return v;}
	AllocatorType& GetAllocator() {return *m_pAllocator;}
	const AllocatorType& GetAllocator() const {return *m_pAllocator;}

private:
	friend class detail::FSerializer<detail::TypeClassPrim<FType::Raw> >;
	StoreType v;
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
		*pField = static_cast<unsigned char>(req.stream.GetBool() ? 1 : 0);
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
class FSerializer<TypeClassPrim<FType::String> > : public SerializerInterface {
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
		req.buffer.CopyFrom(pField->V(), req.allocator);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		pField->v.Swap(req.stream);
		pField->m_pAllocator = &req.allocator;
		return 0;
	}

	virtual int SetAllocator(void *pField, AllocatorType &allocator) IJSTI_OVERRIDE
	{
		VarType *pFieldT = static_cast<VarType *>(pField);
		if (pFieldT->m_pAllocator == &allocator) {
			return 0;
		}
		StoreType temp;
		temp = pFieldT->v;
		pFieldT->v.CopyFrom(temp, allocator);
		pFieldT->m_pAllocator = &allocator;
		return 0;
	}
};

template<>
class FSerializer<TypeClassPrim<FType::Time> > : public SerializerInterface {
public:
	typedef ijst::FStoreTime VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		tm *p = localtime(pField);
		char strBuf[32];
		snprintf(strBuf, 32, "%04d-%02d-%02d %02d:%02d:%02d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour,
				 p->tm_min, p->tm_sec
		);
		req.buffer.SetString(strBuf, req.allocator);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsString()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not String");
			return Err::kDeserializeValueTypeError;
		}

		tm t;
		// use a dummy to check if any not-whitespace trailing
		char dummy;
		int matched = sscanf(req.stream.GetString(),
							 "%d-%d-%d %d:%d:%d %c",
							 &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &dummy
		);
		if (matched != 6) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is string but not a time format");
			return Err::kDeserializeValueTypeError;
		}
		t.tm_year -= 1900;
		t.tm_mon -= 1;
		t.tm_isdst = 0;

		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = mktime(&t);
		return 0;
	}
};

namespace fasttime {
namespace detail {
/*
* true if YEAR is a leap year (every 4 years,
* except every 100th isn't, and every 400th is).
*/
inline bool IsLeap(long year)
{
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

/* do a mathdiv for long type */
inline long MathDiv(long a, long b)
{
	return a / b - (a % b < 0);
}

/* How many leap years between y1 and y2, y1 must less or equal to y2 */
inline long LeapsBetween(long y1, long y2)
{
	long leaps1 = MathDiv(y1 - 1, 4) - MathDiv(y1 - 1, 100)
				  + MathDiv(y1 - 1, 400);
	long leaps2 = MathDiv(y2 - 1, 4) - MathDiv(y2 - 1, 100)
				  + MathDiv(y2 - 1, 400);
	return leaps2 - leaps1;
}
}	// namespace detail

//! Return time stamp form 1970-01-01 00:00:00
template <int _Dummy = 0>
std::int64_t Mktime(int year, int mon, int day, int hour, int min, int sec)
{
	// 1..12 -> 11,12,1..10
	if (0 >= (mon -= 2))
	{
		mon += 12;      /* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((std::int64_t)
					  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
			  year*365 - 719499
			 )*24 + hour /* now have hours - midnight tomorrow handled here */
			)*60 + min /* now have minutes */
		   )*60 + sec; /* finally seconds */

}

template <int _Dummy = 0>
void ParseTimeStamp(std::int64_t timeStamp,
						   IJST_OUT int& yearOut, IJST_OUT int& monOut, IJST_OUT int& dayOut,
						   IJST_OUT int& hourOut, IJST_OUT int& minOut, IJST_OUT int& secOut)
{
	const int kSecsPerHour = (60 * 60);
	const int kSecsPerDay = (kSecsPerHour * 24);

	/* How many days come before each month (0-12). */
	static const unsigned short kMonDays[2][13] = {
			/* Normal years. */
			{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
			/* Leap years. */
			{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
	};

	std::int64_t days, rem, remainder;
	const unsigned short *ip;

	if (timeStamp >= 0) {
		days = timeStamp / kSecsPerDay;
		remainder = timeStamp % kSecsPerDay;
	}
	else {
		days = -(-timeStamp / kSecsPerDay);
		remainder = -(-timeStamp % kSecsPerDay);
	}
	rem = remainder;
	while (rem < 0) {
		rem += kSecsPerDay;
		--days;
	}
	while (rem >= kSecsPerDay) {
		rem -= kSecsPerDay;
		++days;
	}

	hourOut = static_cast<int>(rem / kSecsPerHour);
	rem %= kSecsPerHour;
	minOut = static_cast<int>(rem / 60);
	secOut = static_cast<int>(rem % 60);

	int y = 1970;

	while (days < 0 || days >= (detail::IsLeap(y) ? 366 : 365)) {
		/* Guess a corrected year, assuming 365 days per year. */
		int yg = static_cast<int>(y + detail::MathDiv(days, 365));

		/* Adjust DAYS and Y to match the guessed year. */
		days -= (yg - y) * 365 + detail::LeapsBetween(y, yg);
		y = yg;
	}

	yearOut = y;

	dayOut = static_cast<int>(days);

	ip = kMonDays[detail::IsLeap(y)];
	for (y = 11; days < ip[y]; y--)
		continue;
	days -= ip[y];

	monOut = y + 1;
	dayOut = static_cast<int>(days + 1);
}

}; // fasttime

template<int kTimeZone>
class FSerializer<TypeClassFTime<kTimeZone> > : public SerializerInterface {
public:
	typedef ijst::FStoreFastTime VarType;

	virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
	{
		const VarType *pField = static_cast<const VarType *>(req.pField);
		int year, mon, day, hour, min, sec;
		const std::int64_t utcTimeStamp = *pField + kTimeZone * 3600;
		fasttime::ParseTimeStamp(utcTimeStamp, year, mon, day, hour, min, sec);
		char strBuf[32];
		snprintf(strBuf, 32, "%04d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
		req.buffer.SetString(strBuf, req.allocator);
		return 0;
	}

	virtual int Deserialize(const DeserializeReq &req, IJST_OUT DeserializeResp &resp) IJSTI_OVERRIDE
	{
		if (!req.stream.IsString()) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is not String");
			return Err::kDeserializeValueTypeError;
		}

		int year, mon, day, hour, min, sec;
		// use a dummy to check if any not-whitespace trailing
		char dummy;
		int matched = sscanf(req.stream.GetString(),
							 "%d-%d-%d %d:%d:%d %c",
							 &year, &mon, &day, &hour, &min, &sec, &dummy
		);
		if (matched != 6) {
			resp.fStatus = FStatus::kParseFailed;
			resp.SetErrMsg("Value is string but not a time format");
			return Err::kDeserializeValueTypeError;
		}

		std::int64_t timeStamp = fasttime::Mktime(year, mon, day, hour, min, sec);
		VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
		*pField = timeStamp - kTimeZone * 3600;
		return 0;
	}

private:
};

}	//namespace detail
}	//namespace ijst
#endif //_IJST_TYPES_STD_HPP_INCLUDE_
