/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
*/
/*
 * Time.h
 *
 *  Created on: Aug 31, 2010
 *      Author: mpeura
 */

#include <ctime>
#include <string>
#include <stdexcept>
#include <iostream>

#ifndef TIME_H_
#define TIME_H_

namespace drain {

/// Utility for handling time. Internally, uses tm (C time structure).
/**
 *  Note: This class does nout (yet) support format strings of arbitrary size.
 *  (output str length has limited size).
 *
 */
class Time : private tm {
public:

	inline
	Time() {
		tm_sec = 0;			    /* Seconds.	[0-60] (1 leap second) */
		tm_min = 0;			    /* Minutes.	[0-59] */
		tm_hour = 0;			/* Hours.	[0-23] */
		tm_mday = 1;			/* Day.		[1-31] */
		tm_mon = 0;			    /* Month.	[0-11] */
		tm_year = 0;			/* Year	- 1900.  */
		tm_wday = 0;			/* Day of week.	[0-6] */
		tm_yday = 0;			/* Days in year.[0-365]	*/
		tm_isdst = 1;			/* DST.		[-1/0/1]*/
		tm_gmtoff = 0;
		tm_zone = "";
		setTime();
	};

	inline
	~Time(){};

	/// Sets time to current UTC time.
	inline
	void setTime(){
		setTime(time(NULL));
	};

	/// Sets time to UTC time.
	/**
	 *  \par time -   std::string presentation of time.
	 */
	inline
	void setTime(const time_t &time){
		*(tm *)this = *gmtime(&time);
	};

	/// Sets time
	/**
	 *  \par time -   time given as a std::string .
	 *  \par format - format of time parameter, for example "%Y/%m/%d %H:%M".
	 *  \par strictness - if \true, a runtime error is thrown when parsing fails.
	 *
	 *  Note: handling of Time Zone is odd:
	 *  - Giving unix seconds (input format="%s") changes time zone to local time (EET in Finland)
	 *  - Giving time zone explicitly (input format="%Z"), eg. "2018/11/16 22:58 EET" has no effect; the time is understood
	 *    as GMT time
	 *
	 *  The time can be adjusted using several subsequent calls.
	 */
	inline
	void setTime(const std::string &time, const std::string &format, bool strict=true){
		// setTime(0); reset seconds?
		const char *t = strptime(time.c_str(), format.c_str(), (tm *)this);
		if (strict && (t == NULL)){
			throw std::runtime_error(std::string("setTime(): parse error for '") + time + "', format '" + format + "'");
		}
		/*
		if (*t != '\0') {
			std::cerr << "Remaining std::string:" << std::endl;
		}
		*/
	};

	inline
	void setTime(const tm &time){
		(tm &)*this = time;
	};

	/// Sets time to UTC time.
	/*
	inline
	void setLocalTime(const time_t &time){
		_time = time;
		this->_tm = *localtime(&_time);
	};
	*/

	/// Returns the C struct.
	inline
	const tm & getTm() const { return *this;};//_tm;};

	/// Returns time in seconds.
	inline
	time_t getTime() const { return timegm((tm *)this);};


	/// Returns the std::string using formatting as defined by strftime()
	inline
	const std::string & str(std::string format = "") const {
		if (format.empty())
			timeStr = asctime((tm *)this);
		else {
			//const unsigned int maxSize = 256;
			const size_t maxSize = 256;
			static char tmp[maxSize];
			const size_t length = strftime(tmp, maxSize, format.c_str(), (tm *)this);
			timeStr.assign(tmp,length);
			if (length == maxSize){
				std::cerr << __FILE__ << ':' << __FUNCTION__ << " max time str length("<< maxSize << ") exceeded " << std::endl;
				// TODO: string mapper
			}
		}
		return timeStr;
	};

	inline
	void debug(){
		std::cerr << "H=" << this->tm_hour << ", DST="  << this->tm_isdst << '\n';
	}


protected:

	mutable std::string timeStr;

};

}

#endif /* TIME_H_ */

// Drain
