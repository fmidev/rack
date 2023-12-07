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

//#include "Log.h"


#include <string.h>
//#include <stdexcept>
#include <iostream>

#include "Log.h"
#include "Time.h"

namespace drain {

void Time::setTime(const std::string &time, const std::string &format, bool strict){
	// setTime(0); reset seconds?
	const char *t = strptime(time.c_str(), format.c_str(), (tm *)this);
	if (strict && (t == NULL)){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.error("parse error for '"  , time , "', format='" , format , "'" );
		//throw std::runtime_error(std::string("setTime(): parse error for '") + time + "', format '" + format + "'");
	}
	/*
		if (*t != '\0') {
			std::cerr << "Remaining std::string:" << t << std::endl;
		}
	 */
}


const std::string & Time::str(const std::string &format) const {
	if (format.empty()){
		return Time::str("%c"); // "writes standard date and time string, e.g. Sun Oct 17 04:41:13 2010 (locale dependent)"
		/*
		char *s = asctime((tm *)this);
		char *newline  = strchr(s, '\n');
		if (newline){
			timeStr.assign(s, (newline-s));
		}
		else {
			timeStr.assign(s);
		}
		*/
	}
	else {
		static const size_t maxSize = 256;
		static char tmp[maxSize];
		const size_t length = strftime(tmp, maxSize, format.c_str(), (tm *)this);
		timeStr.assign(tmp,length);
		if (length == maxSize){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.error(" max time str length(", maxSize , ") exceeded " );
			//std::cerr << __FILE__ << ':' << __FUNCTION__ << " max time str length("<< maxSize << ") exceeded " << std::endl;
			// TODO: string mapper
		}
	}
	return timeStr;
}

}

// Drain
