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
#ifndef VARIABLE_FORMATTER_ODIM
#define VARIABLE_FORMATTER_ODIM

#include <ostream>
#include <cmath>
#include <string>
#include <set>
#include <algorithm>
//
#include <drain/util/VariableFormatter.h>  // for VariableHandler
#include <drain/util/Time.h>
#include "ODIM.h"

//#include "EncodingODIM.h"

namespace rack {

/// Formats variables to output stream
/**
 *  This class inherits the following formatting options from the base class:
 *
 *  # printf() style formatting, like "%08.2d"
 *  # bash substring style formatting, like ":2:3" (take 3 letters, starting from position 2)
 *
 *  In addition, this class formats ODIM time stamps (what:time, what:startdate, ...)
 *  # Time formatting, like the date command
 *
 *  \tparam T - type of the variables: std::string, drain::Variable for example.
 *
 *  \see StringLet
 *  \see StringMapper
 *
 */
template <class T>
class VariableFormatterODIM : public drain::VariableFormatter<T>{

public:

	typedef typename drain::VariableFormatter<T>::map_t map_t;

	/*
	typedef std::set<std::string> nameSet;

	static
	const nameSet timeKeys; // = {"time", "starttime", "endtime"};

	static
	const nameSet dateKeys; // = {"date", "startdate", "enddate"};

	static
	const nameSet locationKeys; // = {"site", "src", "lat", "lon", "PLC", "NOD", "WMO"};
	*/

	virtual inline
	~VariableFormatterODIM(){};

	/// Recognizes and format a date. Assumes that the variable name (\c key ) ends with "date".
	static
	bool formatDate(std::ostream & ostr, const std::string & key, const T & value, const std::string & format = "%Y/%m/%d"){

		/*
		const size_t i = key.find(':');
		if (i != std::string::npos){
			return formatDate(ostr, key.substr(i+1), value, format);
		}
		*/

		if (ODIM::dateKeys.count(key) > 0){
			// if (drain::StringTools::endsWith(key, "date")){
			// ostr << "DATE:" << key << "=" << value << " {" << format << "} -> ";
			ostr << drain::Time(value, "%Y%m%d").str(format);
			return true;
		}
		else {
			return false;
		}
	}

	/// Reformat a time. Assumes that the variable name (\c key ) ends with "time".
	static
	bool formatTime(std::ostream & ostr, const std::string & key, const T & value, const std::string & format = "%H:%M"){// add UTC

		/*
		const size_t i = key.find(':');
		if (i != std::string::npos){
			return formatTime(ostr, key.substr(i+1), value, format);
		}
		*/

		if (ODIM::timeKeys.count(key) > 0){
			// if (drain::StringTools::endsWith(key, "time")){
			// ostr << "TIME:" << key << "=" << value << " {" << format << "} -> ";
			ostr << drain::Time(value, "%H%M%S").str(format);
			return true;
		}
		else {
			return false;
		}
	}

	/// Currently, only recognizes a place, and writes it directly in stream.
	static
	bool formatPlace(std::ostream & ostr, const std::string & key, const T & value, const std::string & format = ""){

		if (ODIM::locationKeys.count(key) > 0){
			// Recognize location/place, but still apply default formatter, to support numeric formatting like ${where:lon|%04.3f}
			// drain::VariableFormatter<T>::formatVariable(key, value, format, ostr);
			ostr << value;
			return true;
		}
		else {
			return false;
		}
	}

	/// Checks if variables have ODIM names (keys), and apply special formatting (currently with time stamps)
	virtual
	bool formatVariable(const std::string & key, const T & value, const std::string & format, std::ostream & ostr) const override;



};

/// Checks if variables have ODIM names (keys), and apply special formatting (currently with time stamps)
template <class T>
bool VariableFormatterODIM<T>::formatVariable(const std::string & key, const T & value, const std::string & format, std::ostream & ostr) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.warn("trying time format: ", key, " + ", format);

	if (format.find('%') != std::string::npos){
		// Time formatting (instead of C-stype printf formatting)td::ostream & ostr
		if (formatDate(ostr, key, value, format)){
			// mout.attention("formatting DATE");
			return true;
		}
		else if (formatTime(ostr, key, value, format)){
			// mout.attention("formatting TIME");
			return true;
		}
		else if (ODIM::locationKeys.count(key) > 0){  // skip formatPlace
			// if (formatPlace(ostr, key, value, format)){
			// Recognize location/place, but still apply default formatter, to support numeric formatting like ${where:lon|%04.3f}
			drain::VariableFormatter<T>::formatVariable(key, value, format, ostr);
			// mout.attention("formatting PLACE");
			return true;
		}
	}

	// Else, use default formatting:
	return drain::VariableFormatter<T>::formatVariable(key, value, format, ostr); // basic/trad.
}



}  // namespace rack


#endif
