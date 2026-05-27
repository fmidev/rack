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

#ifndef DRAIN_CONVERT_FUNDAMENTAL
#define DRAIN_CONVERT_FUNDAMENTAL

#include <map>
#include <list>
#include <iterator>
#include <sstream>


namespace drain {

/// Utility class with static conversions


/// Conversion traits — specialize this for custom source types.
/** Default: trivial copy for same type, stringstream for different types. */
template <class T>
struct Converter {

	static void to(const T & src, T & dst){
		dst = src;
	}

	template <class U>
	static void to(const T & src, U & dst){
		std::stringstream sstr;
		sstr << std::boolalpha << src;
		sstr >> dst;
	}
};

/// String source: trivial copy, bool parse, or stream parse.
template <>
struct Converter<std::string> {

	static void to(const std::string & src, std::string & dst){ dst = src; }

	static void to(const std::string & src, bool & dst){
		dst = (src == "true" || src == "True" || src == "TRUE");
	}

	template <class U>
	static void to(const std::string & src, U & dst){
		std::stringstream sstr(src);
		sstr >> dst;
	}
};

/// const char* source: delegate to Converter<std::string>.
template <>
struct Converter<const char *> {
	template <class U>
	static void to(const char * src, U & dst){
		Converter<std::string>::to(std::string(src), dst);
	}
};

/// char* source (string literals deduce through const T& as char[N], decaying to char*).
template <>
struct Converter<char *> {
	template <class U>
	static void to(const char * src, U & dst){
		Converter<const char *>::to(src, dst);
	}
};

/// Bool source: "true"/"false" for strings, 1/0 for numerics.
template <>
struct Converter<bool> {

	static void to(const bool & src, bool & dst){ dst = src; }

	static void to(const bool & src, std::string & dst){ dst = src ? "true" : "false"; }

	template <class U>
	static
	typename std::enable_if<std::is_arithmetic<U>::value && !std::is_same<U, bool>::value>::type
	to(const bool & src, U & dst){
		dst = static_cast<U>(src);
	}
};



class Convert {

public:

	/// General case: dispatch to Converter<T1> traits.
	/** std::decay ensures array types (e.g. const char[N]) map to their pointer specialization.
	template <class T1, class T2>
	static void convert(const T1 & src, T2 & dst){
		Converter<typename std::decay<T1>::type>::to(src, dst);
	}
	*/

	/// General case: dispatch to Converter<T1> traits.
	template <class T1, class T2>
	static void convert(const T1 & src, T2 & dst){
		Converter<T1>::to(src, dst);
	}

	template <class T2>
	static void convert(const char *src, T2 & dst){
		Converter<const char *>::to(src, dst);
	}

	/// Numeric -> bool: zero is false, any other value is true.
	template <class T>
	static
	typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value>::type
	convert(const T & src, bool & dst){
		dst = (src != T(0));
	}

	/// Formatted output.
	template <class T>
	static void convert(const T & src, std::ostream & ostr, const char * format){
		std::stringstream sstr(src);
		ostr << sstr.str();
	}

	template <class T>
	static void convert(const T & src, std::ostream & ostr, const std::string & format){
		convert(src, ostr, format.c_str());
	}

};




/// Utility class with static conversions
/**
 *
 */
template <class T>
class ConvertOld {

public:

	/// Trivial case: source and destination are of same class.
	static
	void convert(const T & src, T & dst){
		dst = src;
	}

	static
	void convert(const char * src, T & dst){
		std::stringstream sstr(src);
		sstr >> dst;
	}



	template <class D>
	static inline
	void convert(const T & src, D & dst){
		convertFrom(src, dst);
	}


	template <class S>
	static inline
	void convert(const S & src, T & dst){
		convertTo(src, dst);
	}


	/// Convert with cast source type.
	template <class D>
	static
	void convertFrom(const T & src, D & dst){
		std::stringstream sstr;
		sstr << src;
		sstr >> dst;
	}

	/// Convert with cast target type.
	template <class S>
	static
	void convertTo(const S & src, T & dst){
		std::stringstream sstr;
		sstr << src;
		sstr >> dst;
	}


};


/*
inline
std::ostream & operator<<(std::ostream & ostr, const StringMapper & strmap){
	return strmap.toStream(ostr);
}
*/


} // drain

#endif /* STRINGMAPPER_H_ */

// Drain
