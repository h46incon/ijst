/**************************************************************************************************
 *		Serialization implementation of Primitive types
 *		Created by h46incon on 2017/11/33.
 **************************************************************************************************/

#ifndef _IJST_TYPES_TIME_HPP_INCLUDE_
#define	_IJST_TYPES_TIME_HPP_INCLUDE_

#include "ijst.h"
#include <cstdio>
#include <ctime>
#if __cplusplus >= 201103L
	#include <cstdint>
#else
	#include <stdint.h>
#endif

#define IJST_TTIME()				::ijst::detail::TypeClassTime
#define IJST_TFTIME(_TimeZone)	::ijst::detail::TypeClassFastTime< _TimeZone>

namespace ijst{
	typedef std::time_t FStoreTime;
	typedef int64_t FStoreFastTime;

	namespace detail{
		struct TypeClassTime {
			// nothing
		};

		template<int kTimeZone>
		struct TypeClassFastTime{
			// nothing
		};
	} // namespace detail


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
		}	// namespace fasttime::detail

		//! Return time stamp form 1970-01-01 00:00:00
		inline int64_t Mktime(int year, int mon, int day, int hour, int min, int sec)
		{
			// 1..12 -> 11,12,1..10
			if (0 >= (mon -= 2))
			{
				mon += 12;      /* Puts Feb last since it has leap day */
				year -= 1;
			}

			return ((((int64_t)
							  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
					  year*365 - 719499
					 )*24 + hour /* now have hours - midnight tomorrow handled here */
					)*60 + min /* now have minutes */
				   )*60 + sec; /* finally seconds */

		}

		inline void ParseTimeStamp(int64_t timeStamp,
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

			int64_t days, rem, remainder;
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

	}; // namespace fasttime

	namespace detail{
		template<>
		class FSerializer<TypeClassTime> : public SerializerInterface {
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

		template<int kTimeZone>
		class FSerializer<TypeClassFastTime<kTimeZone> > : public SerializerInterface {
		public:
			typedef ijst::FStoreFastTime VarType;

			virtual int Serialize(const SerializeReq &req, SerializeResp &resp) IJSTI_OVERRIDE
			{
				const VarType *pField = static_cast<const VarType *>(req.pField);
				int year, mon, day, hour, min, sec;
				const int64_t utcTimeStamp = *pField + kTimeZone * 3600;
				// try use function pointer to prevent inlineing the function
				void (*pParseTimeStamp)(int64_t, int&, int&, int&, int&, int&, int&) = fasttime::ParseTimeStamp;
				pParseTimeStamp(utcTimeStamp, year, mon, day, hour, min, sec);
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

				// try use function pointer to prevent inlineing the function
				int64_t (*pMKTime)(int, int, int, int, int, int) = fasttime::Mktime;
				int64_t timeStamp = pMKTime(year, mon, day, hour, min, sec);
				VarType *pField = static_cast<VarType *>(req.pFieldBuffer);
				*pField = timeStamp - kTimeZone * 3600;
				return 0;
			}
		};

	}	//namespace detail
}	//namespace ijst
#endif //_IJST_TYPES_TIME_HPP_INCLUDE_

