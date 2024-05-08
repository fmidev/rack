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

#ifndef VARIABLE_FORMATTER_H_
#define VARIABLE_FORMATTER_H_

#include <drain/Log.h>
#include <map>
#include <list>
#include <iterator>
#include <sstream>

#include <drain/Convert.h>
#include <drain/RegExp.h>
#include <drain/String.h>
#include <drain/VariableAssign.h>
#include <drain/SmartMapTools.h>
//#include "Variable.h"

#include "IosFormat.h"
//#include "MapTools.h"

//#include "Sprinter.h"
#include "Time.h"

namespace drain {


/// Formats variables to output stream
/**
 *  This base class supports following formatting:
 *
 *  # printf() style formatting, like "%08.2d"
 *  # bash substring style formatting, like ":2:3" (take 3 letters, starting from position 2)
 *
 *  \tparam T - type of the variables.
 *
 *  \see StringLet
 *  \see StringMapper
 *
 */
template <class T=std::string>
class VariableFormatter {

public:

	inline virtual
	~VariableFormatter(){};

	IosFormat iosFormat;

	typedef std::map<std::string,T> map_t;

	/// Searches given key in a map, and if found, processes (formats) the value to ostream.  Return false, if variable not found.
	/**
	 *   Return false, if variable not found.
	 *   Then, further processors may handle the variable tag (remove, change, leave it).
	 */
	virtual
	bool handle(const std::string & key, const map_t & variables, std::ostream & ostr) const {

		drain::Logger mout(__FILE__, __FUNCTION__);

		std::string k,format;
		drain::StringTools::split2(key, k, format, '|');
		// mout.attention("split '", key, "' to ", k, " + ", format);

		const typename std::map<std::string,T>::const_iterator it = variables.find(k);
		if (it == variables.end()) {
			// Don't format.
			return false;
		}


		if (format.empty()){
			iosFormat.copyTo(ostr);
			//vostr.width(width);
			//vstd::cerr << __FILE__ << " assign -> " << stringlet << std::endl;
			//vstd::cerr << __FILE__ << " assign <---- " << mit->second << std::endl;
			ostr <<  it->second;
			return true;
		}
		else {
    		// mout.attention("delegating '", k, "' to formatVariable: ", format);
			//return formatVariable(k, variables, format, ostr);
			return formatVariable(k, it->second, format, ostr);
		}

	}


	/// Given a key, retrieve an associated value from the map and print the value formatted into a stream.
	/**
	 *  Default implementation discards \c key.
	 *
	 *  \return false if value not found in the map, else print it formatted in a stream.
	 */
	// NOTE: must return false, if not found. Then, further processors may handle the variable tag (remove, change, leave it).
	virtual
	bool formatVariable(const std::string & key, const T & value, const std::string & format, std::ostream & ostr) const {

		drain::Logger mout(__FILE__, __FUNCTION__);

		// Default implementation discards \c key.
		return formatValue(value, format, ostr);

		/*
		typename std::map<std::string,T>::const_iterator it = variables.find(key);

		if (it != variables.end()){
			return formatValue(it->second, format, ostr);
		}
		else {
			return false;
		}
		*/

	}

	/// Given a value, print it formatted to stream.
	/**
	 *
	 */
	static
	bool formatValue(const T & value, const std::string & format, std::ostream & ostr) {

		drain::Logger mout(__FILE__, __FUNCTION__);

		const char firstChar = format.at(0);
		const char lastChar = format.at(format.size()-1);

		if (firstChar == ':'){

			mout.attention<LOG_DEBUG>("substring extraction: ", format);

			std::string s;
			drain::Convert2<T>::convert(value, s);
			//drain::StringTools::import(variable, s);

			std::vector<size_t> v;
			drain::StringTools::split(format, v, ':');
			size_t pos   = 0;
			size_t count = s.size();

			switch (v.size()) {
			case 3:
				count = v[2];
				// no break
			case 2:
				pos = v[1];
				if (pos >= s.size()){
					mout.warn("index ", pos, " greater than size (", s.size(), ") of string value '", s, "' of '", value, "'");
					return true;
				}
				count = std::min(count, s.size()-pos);
				ostr << s.substr(v[1], count);
				break;
			default:
				mout.warn("unsupported formatting '", format, "' for variable '", value, "'");
				mout.advice("use :startpos or :startpos:count for substring extraction");
			}
			return true;

		}
		else if (firstChar == '%'){

			mout.attention<LOG_DEBUG>("string formatting: ", format);

			//else if (format.find('%') != std::string::npos){
			//drain::MapTools::get(variables, key, s);

			const size_t BUFFER_SIZE = 256;
			char buffer[BUFFER_SIZE];
			buffer[0] = '\0';
			size_t n = 0;

			switch (lastChar){
			case 's':
			{
				std::string s;
				drain::Convert2<T>::convert(value, s);
				// drain::StringTools::import(variable, s);
				n = std::sprintf(buffer, format.c_str(), s.c_str());
			}
			break;
			case 'c':
			{
				std::string s;
				drain::Convert2<T>::convert(value, s);
				//drain::StringTools::import(variable, s);
				n = std::sprintf(buffer, format.c_str(), s.at(0)); // ?
			}
			break;
			case 'p':
				mout.unimplemented("pointer type: ", format);
				break;
			case 'f':
			case 'F':
			case 'e':
			case 'E':
			case 'a':
			case 'A':
			case 'g':
			case 'G':
			{
				double d = NAN; //nand()
				drain::Convert2<T>::convert(value, d);
				// drain::StringTools::import(variable, d);
				//drain::MapTools::get(variables, key, d);
				// ostr << d << "=>"; // debugging
				n = std::sprintf(buffer, format.c_str(), d);
			}
			break;
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
			{
				int i = 0;
				drain::Convert::convert(value, i);
				// drain::convertAny(variable, i);
				// drain::StringTools::import(variable, i);
				// drain::MapTools::get(variables, key, i);
				// ostr << i << "=>"; // debugging
				n = std::sprintf(buffer, format.c_str(), i);
			}
			break;
			default:
				mout.warn("formatting '", format, "' requested for '", value, "' : unsupported type key: '", lastChar, "'");
				return false; // could be also true, if seen as handled this way?
			}

			ostr << buffer;
			if (n > BUFFER_SIZE){
				mout.fail("formatting with '", format, "' exceeded buffer size (", BUFFER_SIZE, ')');
			}

			// mout.warn("time formatting '", format, "' requested for '", k, "' not ending with 'time' or 'date'!");
		}

		return true;
	}



};

/*
inline
std::ostream & operator<<(std::ostream & ostr, const StringMapper & strmap){
	return strmap.toStream(ostr);
}
*/


} // NAMESPACE

#endif /* STRINGMAPPER_H_ */

// Drain
