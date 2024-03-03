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
	Time(){
		reset();
	};

	inline
	Time(time_t unixSeconds){
		reset();
		setTime(unixSeconds);
	};

	inline
	Time(const std::string &time, const std::string &format, bool STRICT=true){
		reset();
		setTime(time, format, STRICT);
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
	 *  \par time - unix seconds (integer)
	 */
	inline
	void setTime(time_t time){
		tm *gt = gmtime(&time);
		*(tm *)this = *gt;  // = *gmtime(&time);
		//delete gt;
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
	void setTime(const std::string &time, const std::string &format, bool STRICT=true);

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
	const tm & getTm() const {
		return *this;
	};

	/// Returns time in seconds.
	inline
	time_t getTime() const { return timegm((tm *)this);};


	/// Returns the std::string using formatting as defined by strftime()
	const std::string & str(const std::string & format = "") const;

	void reset();

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
