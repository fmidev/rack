/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

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
class Time : private tm {
public:

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

	//virtual
	~Time(){};

	/// Sets time to current UTC time.
	inline
	void setTime(){
		setTime(time(NULL));
		//_time =  time((tm *)this);
	};

	/// Sets time to UTC time.
	inline
	void setTime(const time_t &time){
		//_time = time;
		*(tm *)this = *gmtime(&time);
	};

	/// Sets time
	/**
	 *  \par time -   std::string presentation of time.
	 *  \par format - format of time parameter, for example "%Y/%m/%d %H:%M".
	 *  \par strictness - if \true, a runtime error is thrown when parsing fails.
	 *
	 *  The time can be adjusted using several subsequent calls.
	 */
	inline
	void setTime(const std::string &time, const std::string &format, bool strict=true){
		const char *t = strptime(time.c_str(), format.c_str(), (tm *)this);
		if (strict && (t == NULL)){
			throw std::runtime_error(std::string("setTime(): parse error for '") + time + "', format '" + format + "'");
		}
		/*
		else if (*t != '\0') {
			std::cerr << "Remaining std::string:" << std::endl;
		}
		*/
		// std::cerr << t << std::endl;
		//std::cerr << "Address:"  << (size_t)t << std::endl;
	//_time = timegm((tm *)this);
};

	inline
	void setTime(const tm &time){
		//this->_tm = time;
		(tm &)*this = time;
		//_time = timegm((tm *)&time);
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
			const size_t length = strftime(tmp, maxSize, format.c_str(),(tm *)this);
			timeStr.assign(tmp,length);
		}
		return timeStr;
	};

	inline
	void debug(){
		std::cerr << "H=" << this->tm_hour << ", DST="  << this->tm_isdst << '\n';
	}

	//int mika;

protected:
	//tm _tm;
	//time_t _time;
	mutable std::string timeStr;

};

}

#endif /* TIME_H_ */
